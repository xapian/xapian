/* omega.cc: Main module for omega (example CGI frontend for Xapian)
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2001 James Aylett
 * Copyright 2001,2002 Ananova Ltd
 * Copyright 2002,2003 Olly Betts
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

#include <algorithm>
#include <iostream>
#include <set>

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

using namespace std;

Xapian::Enquire * enquire;
Xapian::Database db;
Xapian::RSet rset;

map<string, string> option;

string date_start, date_end, date_span;

const string default_dbname = "default";

string dbname;
string fmtname = "query";
string filters;

Xapian::docid topdoc = 0;
Xapian::docid hits_per_page = 0;
Xapian::docid min_hits = 0;

// percentage cut-off
int threshold = 0;

bool sort_numeric = true;
Xapian::valueno sort_key = 0;
int sort_bands = 0; // Don't sort
Xapian::valueno collapse_key = 0;
bool collapse = false;

const static char filter_sep = '-';
// Any choice of character for filter_sep could conceivably lead to
// false positives, but the situation is contrived, and just means that if
// someone changed a filter, the first page wouldn't be forced.
// That's hardly the end of the world...

static string
map_dbname_to_dir(const string &dbname)
{
    return database_dir + dbname;
}

int main(int argc, char *argv[])
try {
    read_config_file();

    string big_buf;
    bool more = false;
    char *method;
    MCI val;
#ifdef __SUNPRO_CC
    pair<multimap<string, string>::iterator,
	 multimap<string, string>::iterator> g;
#else
    pair<MCI, MCI> g;
#endif

    // set default thousands and decimal separators: e.g. "16,729 hits" "1.4K"
    option["decimal"] = ".";
    option["thousand"] = ",";
    
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
	    cout << PROGRAM_NAME" - "PACKAGE" "VERSION" "
		"(compiled "__DATE__" "__TIME__")\n";
	    if (argc > 1) exit(0);
	    cout << "Enter NAME=VALUE lines, end with blank line\n";
	    decode_test();
	}
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
    if (hits_per_page == 0) {
	hits_per_page = 10;
    } else if (hits_per_page > 1000) {
	hits_per_page = 1000;
    }

    try {
	// get database(s) to search
	dbname = "";
	set<string> seen; // only add a repeated db once
	g = cgi_params.equal_range("DB");
	for (MCI i = g.first; i != g.second; ++i) {
	    string v = i->second;
	    if (!v.empty()) {
		vector<string> dbs = split(v, '/');
		vector<string>::const_iterator j;
		for (j = dbs.begin(); j != dbs.end(); ++j) {
		    if (!j->empty() && seen.find(*j) == seen.end()) {
			// Translate DB parameter to path of database directory
			if (!dbname.empty()) dbname += '/';
			dbname += *j;
			db.add_database(Xapian::Auto::open(map_dbname_to_dir(*j)));
			seen.insert(*j);
		    }
		}
	    }
	}
	if (dbname.empty()) {
	    dbname = default_dbname;
	    db.add_database(Xapian::Auto::open(map_dbname_to_dir(dbname)));
	}
	enquire = new Xapian::Enquire(db);
    }
    catch (const Xapian::Error &e) {
	enquire = NULL;
    }

    val = cgi_params.find("DEFAULTOP");
    if (val != cgi_params.end()) {
	if (val->second == "AND" || val->second == "and")
	    default_op = Xapian::Query::OP_AND;
    }

    big_buf = "";

    val = cgi_params.find("FMT");
    if (val != cgi_params.end()) {
	string v = val->second;
	if (!v.empty()) fmtname = v;
    }

    val = cgi_params.find("MORELIKE");
    if (enquire && val != cgi_params.end()) {
	Xapian::docid docid = atol(val->second.c_str());
	if (docid == 0) {
	    // Assume it's MORELIKE=Quid1138 and that Quid1138 is a UID
	    // from an external source - we just find the correspond docid
	    Xapian::PostingIterator p = db.postlist_begin(val->second);
	    if (p != db.postlist_end(val->second)) docid = *p;
	}
	
	if (docid != 0) {
	    Xapian::RSet tmprset;
	    tmprset.add_document(docid);

	    ExpandDeciderOmega decider(db);
	    Xapian::ESet eset(enquire->get_eset(6, tmprset, &decider));
	    for (Xapian::ESetIterator i = eset.begin(); i != eset.end(); i++) {
		if ((*i).empty()) continue;
		if (more) big_buf += ' ';
		big_buf += pretty_term(*i);
		more = true;
	    }
	    if (more) goto got_query_from_morelike;
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

    {
	// set any boolean filters
	g = cgi_params.equal_range("B");
	vector<string> filter_v;
	for (MCI i = g.first; i != g.second; i++) {
	    string v = i->second;
	    // we'll definitely get empty B fields from "-ALL-" options
	    if (!v.empty() && isalnum(v[0])) {
		add_bterm(v);
		filter_v.push_back(v);
	    }
	}
	sort(filter_v.begin(), filter_v.end());
	vector<string>::const_iterator i;
	for (i = filter_v.begin(); i != filter_v.end(); ++i) {
	    filters += *i;
	    filters += filter_sep;
	}
    }

    // date range filters
    val = cgi_params.find("START");
    // DATE1 is the deprecated name - check for backward compatibility
    if (val == cgi_params.end()) val = cgi_params.find("DATE1");
    if (val != cgi_params.end()) date_start = val->second;
    val = cgi_params.find("END");
    // DATE2 is the deprecated name - check for backward compatibility
    if (val == cgi_params.end()) val = cgi_params.find("DATE2");
    if (val != cgi_params.end()) date_end = val->second;
    val = cgi_params.find("SPAN");
    // DAYSMINUS is the deprecated name - check for backward compatibility
    if (val == cgi_params.end()) {
	val = cgi_params.find("DAYSMINUS");
	if (val != cgi_params.end()) {
	    // Range used to be DAYSMINUS days before DATE1
	    // Now it's SPAN days after START or before END
	    date_end = date_start;
	    date_start = "";
	}
    }
    if (val != cgi_params.end()) date_span = val->second;

    filters += date_start + filter_sep + date_end + filter_sep + date_span
	+ (default_op == Xapian::Query::OP_AND ? 'A' : 'O');

    // collapsing
    val = cgi_params.find("COLLAPSE");
    if (val != cgi_params.end() && !val->second.empty()) {
	collapse_key = atoi(val->second.c_str());
	collapse = true;
	filters += filter_sep + val->second;
    }

    // sorting
    val = cgi_params.find("SORT");
    if (val != cgi_params.end()  && !val->second.empty()) {
	if (val->second[0] == '#') {
	    sort_numeric = true;
	    sort_key = atoi(val->second.c_str() + 1);
	} else {
	    sort_key = atoi(val->second.c_str());
	}
	sort_bands = 1; // sorting is off unless this is set
	val = cgi_params.find("SORTBANDS");
	if (val != cgi_params.end()) {
	    sort_bands = atoi(val->second.c_str());
	    if (sort_bands <= 0) sort_bands = 1;
	}
    }

    // min_hits (fill mset past topdoc+(hits_per_page+1) to
    // topdoc+max(hits_per_page+1,min_hits)
    val = cgi_params.find("MINHITS");
    // In Omega <= 0.6.3, MINHITS was MIN_HITS - renamed to be consistent
    // with the naming of other CGI parameters.
    if (val == cgi_params.end()) val = cgi_params.find("MIN_HITS");
    if (val != cgi_params.end()) {
	min_hits = atol(val->second.c_str());
    } else {
        min_hits = 0;
    }

    // Should we discard the existing R-set recorded in R CGI parameters?
    bool discard_rset = true;

    // Should we force the first page of hits (and ignore [ > < # and TOPDOC
    // CGI parameters)?
    bool force_first_page = true;

    // raw_search means don't snap TOPDOC to a multiple of HITSPERPAGE.
    // Normally we snap TOPDOC like this so that things work nicely if
    // HITSPERPAGE is in a picker or on radio buttons.  If we're postprocessing
    // the output of omega and want variable sized pages, this is unhelpful.
    bool raw_search = false; 
    val = cgi_params.find("RAWSEARCH");
    // In Omega <= 0.6.3, RAWSEARCH was RAW_SEARCH - renamed to be consistent
    // with the naming of other CGI parameters.
    if (val == cgi_params.end()) val = cgi_params.find("RAW_SEARCH");
    if (val != cgi_params.end()) {
	raw_search = bool(atol(val->second.c_str()));
    }

    string v;
    // get list of terms from previous iteration of query
    val = cgi_params.find("xP");
    if (val == cgi_params.end()) val = cgi_params.find("OLDP");
    if (val != cgi_params.end()) {
	v = val->second;
    } else {
	// if xP not given, default to keeping the rset and don't force page 1
	discard_rset = false;
	force_first_page = false;
    }
    int result = set_probabilistic(big_buf, v);
    switch (result) {
	case BAD_QUERY:
	    // Hmm, how to handle this...
	    break;
	case NEW_QUERY:
	    break;
	case SAME_QUERY:
        case EXTENDED_QUERY:
	    // If we've changed database, force the first page of hits
	    // and discard the R-set (since the docids will have changed)
	    val = cgi_params.find("xDB");
	    if (val != cgi_params.end() && val->second != dbname) break;
	    if (result == SAME_QUERY && force_first_page) {
		force_first_page = false;
		val = cgi_params.find("xFILTERS");
		string xfilters;
		if (val != cgi_params.end()) {
		    xfilters = val->second;
		} else {
		    // compatibility with older xB/xDATE/... scheme
		    val = cgi_params.find("xB");
		    if (val != cgi_params.end())
			xfilters = val->second + filter_sep;
		    static const char * check_vars[] = {
			"DATE1", "DATE2", "DAYSMINUS", NULL
		    };
		    for (const char **pv = check_vars; *pv; ++pv) {
			val = cgi_params.find('x' + string(*pv));
			if (val != cgi_params.end()) xfilters += val->second;
			xfilters += filter_sep;
		    }
		    val = cgi_params.find("xDEFAULTOP");
		    if (val == cgi_params.end() && xfilters.length() == 3) {
			// no x values, so don't force first page
			xfilters = filters;
		    } else {
			char ch = 'O';
			if (val != cgi_params.end() && val->second == "and")
			    ch = 'A';
			xfilters[xfilters.length() - 1] = ch;
		    }
		}
		if (filters != xfilters) {
		    // Filters changed since last query
		    force_first_page = true;
		}
	    }
	    discard_rset = false;
	    break;
    }

    if (!force_first_page) {
	// Work out which mset element is the first hit we want
	// to display
	val = cgi_params.find("TOPDOC");
	if (val != cgi_params.end()) {
	    topdoc = atol(val->second.c_str());
	}

	// Handle next, previous, and page links
	if (cgi_params.find(">") != cgi_params.end()) {
	    topdoc += hits_per_page;
	} else if (cgi_params.find("<") != cgi_params.end()) {
	    if (topdoc >= hits_per_page)
		topdoc -= hits_per_page;
	    else
		topdoc = 0;
	} else if ((val = cgi_params.find("[")) != cgi_params.end() ||
		   (val = cgi_params.find("#")) != cgi_params.end()) {
	    topdoc = (atol(val->second.c_str()) - 1) * hits_per_page;
	}

	// snap topdoc to page boundary
	if (!raw_search) topdoc = (topdoc / hits_per_page) * hits_per_page;
    }

    if (!discard_rset) {
	// put documents marked as relevant into the rset
	g = cgi_params.equal_range("R");
	for (MCI i = g.first; i != g.second; i++) {
	    string v = i->second;
	    if (!v.empty()) {
		vector<string> r = split(v, '.');
		vector<string>::const_iterator i;
		for (i = r.begin(); i != r.end(); i++) {
		    Xapian::docid d = string_to_int(*i);
		    if (d) {
			rset.add_document(d);
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
    do_match();
} catch (const Xapian::Error &e) {
    cout << "Exception: " << e.get_msg() << endl;
} catch (const string &s) {
    cout << "Exception: " << s << endl;
} catch (const char *s) {
    cout << "Exception: " << s << endl;
} catch (...) {
    cout << "Caught unknown exception" << endl;
}
