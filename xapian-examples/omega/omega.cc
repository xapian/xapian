/* main.cc: Main module for ferretfx
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

#include "main.h"

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
OmMSet mset;
OmRSet * rset;

map<string, string> option;

static bool ssi = false;

const string default_db_name = "ompf";

static string map_dbname_to_dir(const string &db_name);

static void make_log_entry(const string &action, long matches);
static void make_query_log_entry(const string &buf);

string db_name;
static string db_dir;
string fmt;
string fmtfile = "t/fmt";

static const string muscat_dir = "/usr/muscat";

static int main2(int argc, char *argv[]);

int main(int argc, char *argv[])
{
    try {
	return main2(argc, argv);
    }
    catch (OmError &e) {
	cout << "Exception: " << e.get_msg() << endl;
    }
    catch (...) {
	cout << "Caught unknown exception" << endl;
    }
    return 0;
}

static int main2(int argc, char *argv[])
{
    string big_buf;
    om_docid list_size;
    bool more = false;
    int      is_old;
    om_docid topdoc = 0;
    char     *method;
    multimap<string, string>::const_iterator val;
    multimap<string, string>::const_iterator notfound = cgi_params.end();
    typedef multimap<string, string>::const_iterator MCI;
    pair<MCI, MCI> g;

    // FIXME: set cout to linebuffered not stdout.  Or just flush regularly...
    setvbuf(stdout, NULL, _IOLBF, 0);

    method = getenv("REQUEST_METHOD");
    if (method == NULL) {
	// Seems we're running from the command line so print a version number
	// and allow a query to be entered for testing
	cout << "ompf "FX_VERSION_STRING" (compiled "__DATE__" "__TIME__")\n"
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
    if (val != notfound) list_size = atol(val->second.c_str());
    if (list_size <= 10) {
	list_size = 10;
    } else if (list_size >= 1000) {
	list_size = 1000;
    }

    val = cgi_params.find("TOPDOC");
    if (val != notfound) topdoc = atol(val->second.c_str());

    // Handle NEXT and PREVIOUS page
    if (cgi_params.find(">") != notfound) {
	topdoc += list_size;
    } else if (cgi_params.find("<") != notfound) {
	topdoc -= list_size;
    } else if ((val = cgi_params.find("F")) != notfound) {
	topdoc = atol(val->second.c_str());
    }

    topdoc = (topdoc / list_size) * list_size;
    if (topdoc < 0) topdoc = 0;
    
    // get database name
    // FIXME: allow multiple DB parameters?  Or A,B,C???
    val = cgi_params.find("DB");
    if (val != notfound) {
	db_name = val->second;
    } else {
	db_name = default_db_name;
    }
    /* if we're called from a SSI page, set flag to use query-ssi, etc */
    if (getenv("REDIRECT_QUERY_STRING")) ssi = true;

    /* Translate DB parameter to path to database directory */
    db_dir = map_dbname_to_dir(db_name);

    // Open enquire system
    OmDatabaseGroup omdb;

    try {
	vector<string> params;
	params.push_back(db_dir);
        omdb.add_database("auto", params);
    } catch (OmError &e) {
	// odds are it's not a database
	cout << "<HTML><HEAD>"
	        "<TITLE>Database '" << db_name << "' not found</TITLE></HEAD>"
	        "<BODY BGCOLOR=white>"
	        "<H3>Database '" << db_name << "' not found "
	        "(or not readable)</H3>\n"
	        "</BODY></HTML>";
	// cout << e.get_msg() << endl;
	exit(0);
    }

    // read t/vars
    string vars_file = db_dir + "/t/vars";
    std::ifstream vars_in(vars_file.c_str());
    if (vars_in) {
	string line;
	while (!vars_in.eof()) {
	    getline(vars_in, line);
	    if (line[0] != '\'') continue;
	    size_t i = line.find('\'', 1);
	    if (i == string::npos) continue;
	    string key = line.substr(1, i - 1);
	    i++;
	    i = line.find('\'', i);
	    if (i == string::npos) continue;
	    i++;
	    size_t j = line.find('\'', i + 1);
	    if (j == string::npos) continue;
	    option[key] = line.substr(i, j - i);
	}
	vars_in.close();
    }

    enquire = new OmEnquire(omdb);
   
    // Create rset to put relevant items in.
    rset = new OmRSet();
       
    /* read thousands and decimal separators: e.g. 16<thou>729<dec>8037 */
    
    if (!option["dec_sep"].empty()) dec_sep = option["dec_sep"][0];
    if (!option["thou_sep"].empty()) thou_sep = option["thou_sep"][0];

    val = cgi_params.find("MATCHOP");
    if (val != notfound) {
	if (val->second == "AND" || val->second == "and") op = OM_MOP_AND;
    } else if ((val = cgi_params.find("THRESHOLD")) != notfound) {
	if (atoi(val->second.c_str()) == 100) op = OM_MOP_AND;
    }

    big_buf = "";

    val = cgi_params.find("MORELIKE");
    if (val != notfound) {
	int doc = atol(val->second.c_str());
       
	OmRSet tmprset;

	tmprset.add_document(doc);

	OmExpandOptions eoptions;
	eoptions.set_use_query_terms(false);
	ExpandDeciderFerret decider;
	OmESet topterms =
		enquire->get_eset(100, tmprset, &eoptions, &decider);

	int c = 0;
	vector<OmESetItem>::const_iterator i;
	for (i = topterms.items.begin(); i != topterms.items.end(); i++) {
	    string term = i->tname;
	    if (term.empty()) continue;
	    if (more) big_buf += ' ';
	    big_buf += term;
	    more = true;
	    if (++c >= 6) break;
	}
	if (more) goto got_query_from_morelike;
    }

    val = cgi_params.find("IDSPISPOPD");
    if (val != cgi_params.end()) {
	int doc = atol(val->second.c_str());
	
	cout << "<b>Clunk<b> ... <i>god mode engaged!</i><hr>\n"
	        "Raw record #" << doc << ":<br>\n";
	
	html_escape(enquire->get_doc(doc).get_data().value);
	
	
	cout << "<hr>\nTerms indexing this record<br>\n"
	        "<table><tr><th>Term</th><th>Freq</th></tr>\n"
		"<FORM NAME=P METHOD=GET ACTION=\"/\">\n"
		"<NOSCRIPT><INPUT TYPE=hidden NAME=ADD VALUE=1></NOSCRIPT>\n"
		"<SCRIPT> <!--\n"
		"document.write('<INPUT NAME=P VALUE=\"\" SIZE=65>')\n"
		"// -->\n"
		"</SCRIPT>\n"
		"<INPUT ALIGN=middle TYPE=image HEIGHT=56 WIDTH=56 BORDER=0 "
		"SRC=\"http://www.euroferret.com/fx-gif/find.gif\" "
		"VALUE=Find>\n";

	cout << "<INPUT TYPE=hidden NAME=DB VALUE=" << db_name << ">\n";

	// FIXME: dump terms
#if 0
	// for each term
	      cout << "<tr><td>";
	      if (isupper(*p)) {
		  cout << "<input type=checkbox name=B value=\"" << p << "\">";
	      } else if (strchr(p, ' ')) {
		  cout << "<input type=checkbox name=X onclick=C(this) "
		          "value=\"&quot;" << p << "&quot;\">";
	      } else {
		  cout << "<input type=checkbox name=X onclick=C(this) "
		          "value=\"" << p << ".\">";
	      }
	      cout << " <A HREF=\"/?DB=" << db_name << "&";
	      if (isupper(*p)) {
		  cout << "B=" << p << "\">";
	      } else if (strchr(p, ' ')) {
		  char *q = p;
		  cout << "P=%22";
		  while (*q) {
		      if (*q == ' ')
			  cout << '+';
		      else
			  cout << *q;
		      q++;
		  }
		  cout << "%22\">";
	      } else {
		  char *q = p;
		  cout << "P=";
		  while (*q) {
		      if (*q == '+')
			  cout << "%2b";
		      else if (*q == '&')
			  cout << "%26";
		      else
			  cout << *q;
		      q++;
		  }
		  cout << ".\">";
	      }

	      cout << "</A></td><td>" << freq << "</td></tr>\n";	
#endif

	cout << "<hr>\nExpand terms<br>\n"
	        "<table><tr><th>Term</th><th>Freq</th></tr>\n";
	// FIXME: generate lots of expand terms for this doc and list them

	cout << "<hr>\n";
	exit(0);
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
    if (cgi_params.find("ADD") != notfound) {
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

    val = cgi_params.find("FMT");
    if (val != notfound) {
	string v = val->second;
	if (!v.empty()) {
	    size_t i = v.find_first_not_of("abcdefghijklmnopqrstuvwxyz");
	    if (i == string::npos) {
		fmt = v;
		fmtfile = "t/fmt." + fmt;
	    }
	}
    }
   
    /*** get old prob query (if any) ***/
    val = cgi_params.find("OLDP");
    if (val == notfound) {
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
    long matches = do_match(topdoc, list_size);
    if (cgi_params.find("X") != notfound) {
	make_log_entry("add", matches);
    } else if (cgi_params.find("MORELIKE") != notfound) {
	make_log_entry("morelike", matches);
    } else if (!big_buf.empty()) {
	make_log_entry("query", matches);
    }
    make_query_log_entry(big_buf + "\n");

    return 0;
}

static string map_dbname_to_dir(const string &db_name) {
    size_t i = db_name.find("..");
    if (i != string::npos) throw "naughty hacker"; // FIXME db_name has .. in
    return muscat_dir + "/data/" + db_name;
}

/**************************************************************/

/* support function for opening the template html pages */
extern FILE *page_fopen(const string &page) {
    FILE *fp;
    string fnm;

    fnm = db_dir + "/html/";
    fnm += page;

    if (ssi) fnm += "-ssi";

    fp = fopen(fnm.c_str(), "r");
    if (fp) return fp;

    /* Perhaps the filename was truncated to 8.3 by DOS or ISO-9660 */
    /* No real need to be too clever about this right now ... */
    if (page == "expand_error")
	return page_fopen("expand_e");

    cout << "Couldn't open file \"" << fnm << "\"\n";
    return NULL;
}

// Logging code

static void
make_log_entry(const string &action, long matches)
{
    string log_buf = db_dir + "/fx.log";
    int fd = open(log_buf.c_str(), O_CREAT|O_APPEND|O_WRONLY, 0644);
       
    if (fd != -1) {
	/* (remote host) (remote logname from identd) (remote user from auth) (time)  */
	/* \"(first line of request)\" (last status of request) (bytes sent)  */
	/* " - - [01/Jan/1997:09:07:22 +0000] \"GET /path HTTP/1.0\" 200 12345\n";*/
	char *var;
	string line;
	time_t t;

	t = time(NULL);
	var = getenv("REMOTE_HOST");
	if (var == NULL) {
	    var = getenv("REMOTE_ADDR");
	    if (var == NULL) var = "-";
	}
	line = var;

	char buf[80];
	strftime(buf, 80, " - - [%d/%b/%Y:%H:%M:%S", gmtime(&t));
	line += buf;
	line += " +0000] \"GET /" + db_name + "/" + action + "\" 200 ";
	sprintf(buf, "%li ", matches);
	line += buf;
	var = getenv("HTTP_REFERER");
	if (var != NULL) {
	    line += '"';
	    line += var;
	    line += "\"\n";
	} else {
	    line += "-\n";
	}
	write(fd, line.data(), line.length());
	close(fd);
    }
}

static void
make_query_log_entry(const string &buf)
{
    string log_buf = db_dir + "/query.log";
    int fd = open(log_buf.c_str(), O_CREAT|O_APPEND|O_WRONLY, 0644);
    if (fd != -1) {
	write(fd, buf.data(), buf.size());
	close(fd);
    }
}
