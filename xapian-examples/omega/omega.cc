/* omega.cc: Main module for omega (example CGI frontend for Open Muscat)
 *
 * ----START-LICENCE----
 * Copyright 1999,2000 BrightStation PLC
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

#include "omega.h"

#include <fstream>

#include <string.h>
#include <ctype.h>

#include <time.h>

#include <fcntl.h>
#ifdef WIN32
#include <io.h>
#else
#include <unistd.h>
#endif

#include "cgiparam.h"
#include "query.h"

OmEnquire * enquire;
OmRSet * rset;

map<string, string> option;

const string default_dbname = "default";

static string map_dbname_to_dir(const string &dbname);

static void make_log_entry(const string &action, long matches);
static void make_query_log_entry(const string &buf);

string dbname;
string log_dir = "/tmp";
string fmtname = "query";

om_docid topdoc = 0;
om_docid list_size = 0;

static const string muscat_dir = "/usr/om";

static int main2(int argc, char *argv[]);

int main(int argc, char *argv[])
{
    try {
	return main2(argc, argv);
    }
    catch (OmError &e) {
	cout << "Exception: " << e.get_msg() << endl;
    }
    catch (string &s) {
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

static int main2(int argc, char *argv[])
{
    string big_buf;
    bool more = false;
    int      is_old;
    char     *method;
    MCI val;
    pair<MCI, MCI> g;

    // FIXME: set cout to linebuffered not stdout.  Or just flush regularly...
    setvbuf(stdout, NULL, _IOLBF, 0);

    method = getenv("REQUEST_METHOD");
    if (method == NULL) {
	// Seems we're running from the command line so print a version number
	// and allow a query to be entered for testing
	cout << PROGRAM_NAME" - "PACKAGE" "VERSION" (compiled "__DATE__" "__TIME__")\n"
	        "Enter NAME=VALUE lines, end with blank line\n";
    }

    cout << "Content-type: text/html\n\n";
    
    if (method == NULL)
        decode_test();
    else if (*method == 'P')
        decode_post();
    else
        decode_get();

    list_size = 0;
    val = cgi_params.find("MAXHITS");
    if (val != cgi_params.end()) list_size = atol(val->second.c_str());
    if (list_size <= 10) {
	list_size = 10;
    } else if (list_size >= 1000) {
	list_size = 1000;
    }

    val = cgi_params.find("TOPDOC");
    if (val != cgi_params.end()) topdoc = atol(val->second.c_str());

    // Handle NEXT and PREVIOUS page
    if (cgi_params.find(">") != cgi_params.end()) {
	topdoc += list_size;
    } else if (cgi_params.find("<") != cgi_params.end()) {
	topdoc -= list_size;
    } else if ((val = cgi_params.find("[")) != cgi_params.end()) {
	topdoc = (atol(val->second.c_str()) - 1) * list_size;
    }

    topdoc = (topdoc / list_size) * list_size;
    if (topdoc < 0) topdoc = 0;
    
    // get database(s) to search
    dbname = "";
    g = cgi_params.equal_range("DB");
    for (MCI i = g.first; i != g.second; i++) {
	string v = i->second;
	if (!v.empty()) {
	    if (dbname.size()) dbname += '/';
	    dbname += v;
	}
    }
    if (dbname.size() == 0) dbname = default_dbname;

    // Open enquire system
    OmDatabaseGroup omdb;

    try {
	size_t p, q;
	q = 0;
	string tmp = dbname;
	dbname = "";
	while (1) {
	    p = tmp.find_first_of('/', q);
	    // Translate DB parameter to path of database directory
	    if (p != q && tmp[q] != '.') {
		vector<string> params;		
		string db = tmp.substr(q, p - q);
		if (dbname.size()) dbname += '/';
		dbname += db;
		params.push_back(map_dbname_to_dir(db));
		omdb.add_database("auto", params);
	    }
	    if (p == string::npos) break;
	    q = p + 1;
	}
    } catch (OmError &e) {
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

    // read thousands and decimal separators: e.g. 16<thousand>729<decimal>8037
    option["decimal"] = ".";
    option["thousand"] = ",";

    enquire = new OmEnquire(omdb);
   
    // Create rset to put relevant items in.
    rset = new OmRSet();
       
    val = cgi_params.find("MATCHOP");
    if (val != cgi_params.end()) {
	if (val->second == "AND" || val->second == "and") op = OM_MOP_AND;
    } else if ((val = cgi_params.find("THRESHOLD")) != cgi_params.end()) {
	if (atoi(val->second.c_str()) == 100) op = OM_MOP_AND;
    }

    big_buf = "";

    val = cgi_params.find("MORELIKE");
    if (val != cgi_params.end()) {
	int doc = atol(val->second.c_str());
       
	OmRSet tmprset;

	tmprset.add_document(doc);

	OmExpandOptions eoptions;
	eoptions.set_use_query_terms(false);
	ExpandDeciderOmega decider;
	OmESet topterms = enquire->get_eset(6, tmprset, &eoptions, &decider);

	vector<OmESetItem>::const_iterator i;
	for (i = topterms.items.begin(); i != topterms.items.end(); i++) {
	    string term = i->tname;
	    if (more) big_buf += ' ';
	    big_buf += term;
	    more = true;
	}
	if (more) goto got_query_from_morelike;
    }

    val = cgi_params.find("FMT");
    if (val != cgi_params.end()) {
	string v = val->second;
	if (!v.empty()) {
	    size_t i = v.find_first_not_of("abcdefghijklmnopqrstuvwxyz");
	    if (i == string::npos) fmtname = v;
	}
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

    /*** set Boolean ***/
    g = cgi_params.equal_range("B");
    for (MCI i = g.first; i != g.second; i++) {
	string v = i->second;
        // we'll definitely get empty B fields from "-ALL-" options
	if (!v.empty() && isalnum(v[0])) add_bterm(v);
    }

    /*** get old prob query (if any) ***/
    val = cgi_params.find("OLDP");
    if (val == cgi_params.end()) {
	set_probabilistic(big_buf, "");
	is_old = 1; // not really, but it should work
    } else {
	is_old = set_probabilistic(big_buf, val->second);
    }

    /* if query has changed, force first page of hits */
    if (is_old < 1) topdoc = 0;

    ticked.clear();
    if (is_old != 0) {
	// set up the R-set
	g = cgi_params.equal_range("R");
	for (MCI i = g.first; i != g.second; i++) {
	    string v = i->second;
	    if (!v.empty()) {
		om_docid d = atoi(v.c_str());
		if (d) {
		    rset->add_document(d);
		    ticked[d] = true;
		}
	    }
	}
    }

    /*** process commands ***/
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

static string map_dbname_to_dir(const string &dbname) {    
    return muscat_dir + "/data/" + dbname;
}

/**************************************************************/

// Logging code

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
    line = line + " +0000]\t" + action + '\t' + dbname + '\t' + raw_prob;
    sprintf(buf, "%li ", matches);
    line = line + buf + '\t';
    var = getenv("HTTP_REFERER");
    if (var == NULL) var = "-";
    line += var;
    line += '\n';
    write(fd, line.data(), line.length());
    close(fd);
}
