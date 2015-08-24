/* omega.cc: Main module for omega (example CGI frontend for Xapian)
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2001 James Aylett
 * Copyright 2001,2002 Ananova Ltd
 * Copyright 2002,2003,2004,2006,2007,2008,2009,2010,2011,2014 Olly Betts
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301
 * USA
 */

#include <config.h>

#include <cassert>
#include <cstdio>
#include <ctime>

#include <algorithm>
#include <cstring>
#include <iostream>
#include <set>

#include "safefcntl.h"
#include "safeunistd.h"

#include "omega.h"
#include "utils.h"
#include "cgiparam.h"
#include "query.h"
#include "str.h"
#include "stringutils.h"
#include "expand.h"

using namespace std;

static const char DEFAULT_STEM_LANGUAGE[] = "english";

Xapian::Enquire * enquire;
Xapian::Database db;
Xapian::RSet rset;

map<string, string> option;

string date_start, date_end, date_span;

const string default_dbname = "default";

bool set_content_type = false;

bool suppress_http_headers = false;

string dbname;
string fmtname = "query";
string filters;

Xapian::docid topdoc = 0;
Xapian::docid hits_per_page = 0;
Xapian::docid min_hits = 0;

// the probabilistic query
string query_string;

// percentage cut-off
int threshold = 0;

Xapian::valueno sort_key = Xapian::BAD_VALUENO; // Don't sort.
bool sort_ascending = true;
bool sort_after = false;
Xapian::Enquire::docid_order docid_order = Xapian::Enquire::ASCENDING;

Xapian::valueno collapse_key = 0;
bool collapse = false;

static string
map_dbname_to_dir(const string &database_name)
{
    return database_dir + database_name;
}

int main(int argc, char *argv[])
try {
    read_config_file();

    char *method;
    MCI val;
    pair<MCI, MCI> g;

    option["flag_default"] = "true";

    // set default thousands and decimal separators: e.g. "16,729 hits" "1.4K"
    option["decimal"] = ".";
    option["thousand"] = ",";
    
    // set the default stemming language
    option["stemmer"] = DEFAULT_STEM_LANGUAGE;

    // FIXME: set cout to linebuffered not stdout.  Or just flush regularly...
    //setvbuf(stdout, NULL, _IOLBF, 0);

    method = getenv("REQUEST_METHOD");
    if (method == NULL) {
	if (argc > 1 && (argv[1][0] != '-' || strchr(argv[1], '='))) {
	    // omega 'P=information retrieval' DB=papers
	    // check for a leading '-' on the first arg so "omega --version",
	    // "omega --help", and similar take the next branch
	    decode_argv(argv + 1);
	} else {
	    // Seems we're running from the command line so give version
	    // and allow a query to be entered for testing
	    cout << PROGRAM_NAME " - " PACKAGE " " VERSION "\n";
	    if (argc > 1) exit(0);
	    cout << "Enter NAME=VALUE lines, end with blank line\n";
	    decode_test();
	}
    } else {
	if (*method == 'P')
	    decode_post();
	else
	    decode_get();
    }

    try {
	// get database(s) to search
	dbname.resize(0);
	set<string> seen; // only add a repeated db once
	g = cgi_params.equal_range("DB");
	for (MCI i = g.first; i != g.second; ++i) {
	    const string & v = i->second;
	    if (!v.empty()) {
		size_t p = 0, q;
		while (true) {	    
		    q = v.find('/', p);
		    string s = v.substr(p, q - p);
		    if (!s.empty() && seen.find(s) == seen.end()) {
			// Translate DB parameter to path of database directory
			if (!dbname.empty()) dbname += '/';
			dbname += s;
			db.add_database(Xapian::Database(map_dbname_to_dir(s)));
			seen.insert(s);
		    }
		    if (q == string::npos) break;
		    p = q + 1;
		}
	    }
	}
	if (dbname.empty()) {
	    dbname = default_dbname;
	    db.add_database(Xapian::Database(map_dbname_to_dir(dbname)));
	}
	enquire = new Xapian::Enquire(db);
    }
    catch (const Xapian::Error &) {
	enquire = NULL;
    }

    hits_per_page = 0;
    val = cgi_params.find("HITSPERPAGE");
    if (val != cgi_params.end()) hits_per_page = atol(val->second.c_str());
    if (hits_per_page == 0) {
	hits_per_page = 10;
    } else if (hits_per_page > 1000) {
	hits_per_page = 1000;
    }

    val = cgi_params.find("DEFAULTOP");
    if (val != cgi_params.end()) {
	const string & v = val->second;
	if (v == "AND" || v == "and")
	    default_op = Xapian::Query::OP_AND;
    }

    val = cgi_params.find("FMT");
    if (val != cgi_params.end()) {
	const string & v = val->second;
	if (!v.empty()) fmtname = v;
    }

    // Get the probabilistic query.
    val = cgi_params.find("MORELIKE");
    if (enquire && val != cgi_params.end()) {
	const string & v = val->second;
	Xapian::docid docid = atol(v.c_str());
	if (docid == 0) {
	    // Assume it's MORELIKE=Quid1138 and that Quid1138 is a UID
	    // from an external source - we just find the correspond docid
	    Xapian::PostingIterator p = db.postlist_begin(v);
	    if (p != db.postlist_end(v)) docid = *p;
	}

	if (docid != 0) {
	    Xapian::RSet tmprset;
	    tmprset.add_document(docid);

	    OmegaExpandDecider decider(db);
	    set_expansion_scheme(*enquire, option);
	    Xapian::ESet eset(enquire->get_eset(40, tmprset, 0,
						expand_param_k, &decider));
	    for (Xapian::ESetIterator i = eset.begin(); i != eset.end(); i++) {
		if (!query_string.empty()) query_string += ' ';
		query_string += pretty_term(*i);
	    }
	}
    }

    if (query_string.empty()) {
	// collect the prob fields
	g = cgi_params.equal_range("P");
	for (MCI i = g.first; i != g.second; i++) {
	    const string & v = i->second;
	    if (!v.empty()) {
		if (!query_string.empty()) query_string += ' ';
		query_string += v;
	    }
	}

	// add expand/topterms terms if appropriate
	if (cgi_params.find("ADD") != cgi_params.end()) {
	    g = cgi_params.equal_range("X");
	    for (MCI i = g.first; i != g.second; i++) {
		const string & v = i->second;
		if (!v.empty()) {
		    if (!query_string.empty()) query_string += ' ';
		    query_string += v;
		}
	    }
	}
    } 

    // strip leading and trailing whitespace from query_string
    string::size_type first_nonspace;
    first_nonspace = query_string.find_first_not_of(" \t\r\n\v");
    if (first_nonspace == string::npos) {
	query_string.resize(0);
    } else {
	string::size_type len = query_string.find_last_not_of(" \t\r\n\v");
	assert(len != string::npos);
	if (first_nonspace > 0 || len <= query_string.length() - 1) {
	    len = len + 1 - first_nonspace;
	    query_string = query_string.substr(first_nonspace, len);
	}
    }

    // set any boolean filters
    g = cgi_params.equal_range("B");
    if (g.first != g.second) {
	vector<string> filter_v;
	for (MCI i = g.first; i != g.second; i++) {
	    const string & v = i->second;
	    // we'll definitely get empty B fields from "-ALL-" options
	    if (!v.empty() && C_isalnum(v[0])) {
		add_bterm(v);
		filter_v.push_back(v);
	    }
	}
	sort(filter_v.begin(), filter_v.end());
	vector<string>::const_iterator j;
	for (j = filter_v.begin(); j != filter_v.end(); ++j) {
	    filters += *j;
	    filters += filter_sep;
	}
    }

    // date range filters
    val = cgi_params.find("START");
    if (val != cgi_params.end()) date_start = val->second;
    val = cgi_params.find("END");
    if (val != cgi_params.end()) date_end = val->second;
    val = cgi_params.find("SPAN");
    if (val != cgi_params.end()) date_span = val->second;

    filters += date_start + filter_sep + date_end + filter_sep + date_span
	+ (default_op == Xapian::Query::OP_AND ? 'A' : 'O');

    // Percentage relevance cut-off
    val = cgi_params.find("THRESHOLD");
    if (val != cgi_params.end()) {
        threshold = atoi(val->second.c_str());
        if (threshold < 0) threshold = 0;
        if (threshold > 100) threshold = 100;
    }

    // collapsing
    val = cgi_params.find("COLLAPSE");
    if (val != cgi_params.end()) {
	const string & v = val->second;
	if (!v.empty()) {
	    collapse_key = atoi(v.c_str());
	    collapse = true;
	    filters += filter_sep;
	    filters += str(collapse_key);
	}
    }

    // docid order
    val = cgi_params.find("DOCIDORDER");
    if (val != cgi_params.end()) {
	const string & v = val->second;
	if (!v.empty()) {
	    char ch = v[0];
	    if (ch == 'D') {
		docid_order = Xapian::Enquire::DESCENDING;
		filters += 'D';
	    } else if (ch != 'A') {
		docid_order = Xapian::Enquire::DONT_CARE;
	    } else {
		filters += 'X';
	    }
	}
    }

    // sorting
    val = cgi_params.find("SORT");
    if (val != cgi_params.end()) {
	sort_key = atoi(val->second.c_str());
	val = cgi_params.find("SORTREVERSE");
	if (val != cgi_params.end()) {
	    sort_ascending = (atoi(val->second.c_str()) == 0);
	}
	val = cgi_params.find("SORTAFTER");
	if (val != cgi_params.end()) {
	    sort_after = (atoi(val->second.c_str()) != 0);
	}
	// Add the sorting related options to filters too.
	filters += str(sort_key);
	if (sort_after) {
	    if (sort_ascending) {
		filters += 'F';
	    } else {
		filters += 'R';
	    }
	} else {
	    if (!sort_ascending) {
		filters += 'r';
	    }
	}
    }

    // min_hits (fill mset past topdoc+(hits_per_page+1) to
    // topdoc+max(hits_per_page+1,min_hits)
    val = cgi_params.find("MINHITS");
    if (val != cgi_params.end()) {
	min_hits = atol(val->second.c_str());
    }

    parse_omegascript(); 
} catch (const Xapian::Error &e) {
    if (!set_content_type && !suppress_http_headers)
	cout << "Content-Type: text/html\n\n";
    cout << "Exception: " << html_escape(e.get_msg()) << endl;
} catch (const std::exception &e) {
    if (!set_content_type && !suppress_http_headers)
	cout << "Content-Type: text/html\n\n";
    cout << "Exception: std::exception " << html_escape(e.what()) << endl;
} catch (const string &s) {
    if (!set_content_type && !suppress_http_headers)
	cout << "Content-Type: text/html\n\n";
    cout << "Exception: " << html_escape(s) << endl;
} catch (const char *s) {
    if (!set_content_type && !suppress_http_headers)
	cout << "Content-Type: text/html\n\n";
    cout << "Exception: " << html_escape(s) << endl;
} catch (...) {
    if (!set_content_type && !suppress_http_headers)
	cout << "Content-Type: text/html\n\n";
    cout << "Caught unknown exception" << endl;
}
