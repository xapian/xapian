/* omega.cc: Main module for omega (example CGI frontend for Xapian)
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2001 James Aylett
 * Copyright 2001 Ananova Ltd
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 * -----END-LICENCE-----
 */

#include <stdio.h>
#include <time.h>
#include <algo.h>

#include <fcntl.h>
#ifdef WIN32
#include <io.h>
#else
#include <unistd.h>
#endif

#include "omega.h"
#include "utils.h"
#include "cgiparam.h"
#include "query.h"

OmEnquire * enquire;
OmDatabase * omdb;
OmRSet * rset;

map<string, string> option;

string date1, date2, daysminus;

const string default_dbname = "default";

string dbname;
string log_dir = "/tmp";
string fmtname = "query";

om_docid topdoc = 0;
om_docid hits_per_page = 0;

// percentage cut-off
int threshold = 0;

static void
make_log_entry(const string &action, long matches)
{
    string log_buf = log_dir + "/omega.log";
    int fd = open(log_buf.c_str(), O_CREAT|O_APPEND|O_WRONLY, 0644);
       
    if (fd == -1) return;

    // (remote host)\t(date/time)\t(action)\t(db)\t(query)\t(referer)
    // 193.131.74.35 [01/Jan/1997:09:07:22 +0000] query db1 test http://x.com/
    char *var;
    string line;
    time_t t = time(NULL);
    var = getenv("REMOTE_HOST");
    if (var == NULL) {
	var = getenv("REMOTE_ADDR");
	if (var == NULL) var = "-";
    }
    line = var;    
    char buf[80];
    strftime(buf, 80, "\t[%d/%b/%Y:%H:%M:%S", gmtime(&t));
    line += buf;
    line = line + " +0000]\t" + action + '\t' + dbname + '\t' + raw_prob + '\t';
    sprintf(buf, "%li ", matches);
    line = line + buf;
    var = getenv("HTTP_REFERER");
    if (var != NULL) {
	line += '\t';
	line += var;
    }
    line += '\n';
    write(fd, line.data(), line.length());
    close(fd);
}

static string
map_dbname_to_dir(const string &dbname)
{
    return database_dir + dbname;
}

static int
main2(int argc, char *argv[])
{
    read_config_file();

    string big_buf;
    bool more = false;
    char *method;
    MCI val;
    std::pair<MCI, MCI> g;

    // set default thousands and decimal separators: e.g. "16,729 hits" "1.4K"
    option["decimal"] = ".";
    option["thousand"] = ",";
    
    // FIXME: set cout to linebuffered not stdout.  Or just flush regularly...
    //setvbuf(stdout, NULL, _IOLBF, 0);

    method = getenv("REQUEST_METHOD");
    if (method == NULL) {
	// Seems we're running from the command line so print a version number
	// and allow a query to be entered for testing
	cout << PROGRAM_NAME" - "PACKAGE" "VERSION" (compiled "__DATE__" "__TIME__")\n"
	        "Enter NAME=VALUE lines, end with blank line\n";
        decode_test();
    } else {
	cout << "Content-type: text/html\n\n";
	if (*method == 'P')
	    decode_post();
	else
	    decode_get();
    }

    hits_per_page = 0;
    val = cgi_params.find("HITSPERPAGE");
    if (val != cgi_params.end()) hits_per_page = atol(val->second.c_str());
    if (hits_per_page <= 10) {
	hits_per_page = 10;
    } else if (hits_per_page >= 1000) {
	hits_per_page = 1000;
    }

    omdb = new OmDatabase();
    try {
	// get database(s) to search
	dbname = "";
	g = cgi_params.equal_range("DB");
	for (MCI i = g.first; i != g.second; i++) {
	    string v = i->second;
	    if (!v.empty()) {
		vector<string>dbs = split(v, '/');
		dbname = "";
		vector<string>::const_iterator i;
		for (i = dbs.begin(); i != dbs.end(); i++) {
		    if (!i->empty()) {
			// Translate DB parameter to path of database directory
			if (!dbname.empty()) dbname += '/';
			dbname += *i;
			OmSettings params;          
			params.set("backend", "auto");
			params.set("auto_dir", map_dbname_to_dir(*i));
			omdb->add_database(params);
		    }
		}
	    }
	}
	if (dbname.empty()) {
	    dbname = default_dbname;
	    OmSettings params;          
	    params.set("backend", "auto");
	    params.set("auto_dir", map_dbname_to_dir(dbname));
	    omdb->add_database(params);
	}
    }
    catch (const OmError &e) {
	// FIXME: make this more helpful (and use a template?)
	// odds are it's not a database
	cout << "<HTML><HEAD>\n"
	        "<TITLE>Database `" << dbname << "' not found</TITLE></HEAD>\n"
	        "<BODY BGCOLOR=white>\n"
	        "<H3>Database <i>" << dbname << "</i> not found "
	        "(or not readable)</H3>\n"
	        "</BODY></HTML>\n";
        cout << "<!-- " << e.get_msg() << " -->\n";
	exit(0);
    }

    val = cgi_params.find("DEFAULTOP");
    if (val != cgi_params.end()) {
	if (val->second == "AND" || val->second == "and")
	    default_op = OmQuery::OP_AND;
    }

    enquire = new OmEnquire(*omdb);
   
    big_buf = "";

    val = cgi_params.find("FMT");
    if (val != cgi_params.end()) {
	string v = val->second;
	if (!v.empty()) fmtname = v;
    }

    val = cgi_params.find("MORELIKE");
    if (val != cgi_params.end()) {
	int doc = atol(val->second.c_str());
       
	OmRSet tmprset;

	tmprset.add_document(doc);

	OmSettings eoptions;
	eoptions.set("expand_use_query_terms", false);
	ExpandDeciderOmega decider;
	OmESet topterms = enquire->get_eset(6, tmprset, &eoptions, &decider);

	for (OmESetIterator i = topterms.begin(); i != topterms.end(); i++) {
	    if (more) big_buf += ' ';
	    big_buf += *i;
	    more = true;
	}
	if (more) goto got_query_from_morelike;
    }

    // collect the prob fields
    g = cgi_params.equal_range("P");
    for (MCI i = g.first; i != g.second; i++) {
	string v = i->second;
	if (!v.empty()) {
	    if (more) big_buf += ' ';
	    big_buf += v;
	    more = true;
	}
    }

    // add expand/topterms terms if appropriate
    if (cgi_params.find("ADD") != cgi_params.end()) {
	g = cgi_params.equal_range("X");
	for (MCI i = g.first; i != g.second; i++) {
	    string v = i->second;
	    if (!v.empty()) {
		if (more) big_buf += ' ';
		big_buf += v;
		more = true;
	    }
	}
    }
   
    got_query_from_morelike:

    // set any boolean filters
    g = cgi_params.equal_range("B");
    for (MCI i = g.first; i != g.second; i++) {
	string v = i->second;
        // we'll definitely get empty B fields from "-ALL-" options
	if (!v.empty() && isalnum(v[0])) add_bterm(v);
    }

    // date range filters
    val = cgi_params.find("DATE1");
    if (val != cgi_params.end()) date1 = val->second;
    val = cgi_params.find("DATE2");
    if (val != cgi_params.end()) date2 = val->second;
    val = cgi_params.find("DAYSMINUS");
    if (val != cgi_params.end()) daysminus = val->second;
    
    rset = new OmRSet();
    string v;
    // get list of terms from previous iteration of query
    val = cgi_params.find("OLDP");
    if (val != cgi_params.end()) {
	v = val->second;
    }
    if (set_probabilistic(big_buf, v) != NEW_QUERY) {
	// work out which mset element is at top of the new page of hits
	val = cgi_params.find("TOPDOC");
	if (val != cgi_params.end()) topdoc = atol(val->second.c_str());

	// Handle next, previous, and page links
	if (cgi_params.find(">") != cgi_params.end()) {
	    topdoc += hits_per_page;
	} else if (cgi_params.find("<") != cgi_params.end()) {
	    topdoc -= hits_per_page;
	} else if ((val = cgi_params.find("[")) != cgi_params.end() ||
		   (val = cgi_params.find("#")) != cgi_params.end()) {
	    topdoc = (atol(val->second.c_str()) - 1) * hits_per_page;
	}

	// snap topdoc to page boundary
	topdoc = (topdoc / hits_per_page) * hits_per_page;
	if (topdoc < 0) topdoc = 0;
    
	// put documents marked as relevant into the rset
	g = cgi_params.equal_range("R");
	for (MCI i = g.first; i != g.second; i++) {
	    string v = i->second;
	    if (!v.empty()) {
		vector<string> r = split(v, '.');
		vector<string>::const_iterator i;
		for (i = r.begin(); i != r.end(); i++) {
		    om_docid d = string_to_int(*i);
		    if (d) {
			rset->add_document(d);
			ticked[d] = true;
		    }
		}
	    }
	}
    }

    // Percentage relevance cut-off
    val = cgi_params.find("THRESHOLD");
    if (val != cgi_params.end()) {
        threshold = atoi(val->second.c_str());
        if (threshold < 0) threshold = 0;
        if (threshold > 100) threshold = 100;
    }

    // process commands
    long matches = do_match();
    if (cgi_params.find("X") != cgi_params.end()) {
	make_log_entry("add", matches);
    } else if (cgi_params.find("MORELIKE") != cgi_params.end()) {
	make_log_entry("morelike", matches);
    } else if (!big_buf.empty()) {
	make_log_entry("query", matches);
    }
    return 0;
}

int main(int argc, char *argv[])
{
    try {
	return main2(argc, argv);
    }
    catch (const OmError &e) {
	cout << "Exception: " << e.get_msg() << endl;
    }
    catch (const string &s) {
	cout << "Exception: " << s << endl;
    }
    catch (const char *s) {
	cout << "Exception: " << s << endl;
    }
    catch (...) {
	cout << "Caught unknown exception" << endl;
    }
    return 0;
}
