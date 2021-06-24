/** @file
 * @brief Main module for omega (example CGI frontend for Xapian)
 */
/* Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2001 James Aylett
 * Copyright 2001,2002 Ananova Ltd
 * Copyright 2002,2003,2004,2006,2007,2008,2009,2010,2011,2014,2015,2016,2018 Olly Betts
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

#include <cerrno>
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
#include "parseint.h"

using namespace std;

static const char DEFAULT_STEM_LANGUAGE[] = "english";

// A character which doesn't require URL encoding, and isn't likely to appear
// in filter values.
const char filter_sep = '~';

// What we used for filter_sep in Omega < 1.3.4.
const char filter_sep_old = '-';

Xapian::Enquire * enquire;
Xapian::Database db;

map<string, string> option;

bool set_content_type = false;

bool suppress_http_headers = false;

string dbname;
string fmtname;
string filters, old_filters;

Xapian::docid hits_per_page = 0;
Xapian::docid min_hits = 0;

// percentage cut-off
int threshold = 0;

Xapian::MultiValueKeyMaker* sort_keymaker = NULL;
Xapian::valueno sort_key = Xapian::BAD_VALUENO; // Don't sort.
bool reverse_sort = true;
bool sort_after = false;
Xapian::Enquire::docid_order docid_order = Xapian::Enquire::ASCENDING;

Xapian::valueno collapse_key = 0;
bool collapse = false;

static string
map_dbname_to_dir(const string &database_name)
{
    return database_dir + database_name;
}

static void
add_database(const string& this_dbname)
{
    if (!dbname.empty()) dbname += '/';
    dbname += this_dbname;

    Xapian::Database this_db(map_dbname_to_dir(this_dbname));
    db.add_database(this_db);

    size_t this_db_size = this_db.size();
    size_t db_size = db.size();
    size_t i = 0;
    while (subdbs.size() != db_size) {
	subdbs.emplace_back(this_dbname, i++, this_db_size);
    }
}

// Get database(s) to search.
template<typename IT>
void
parse_db_params(const pair<IT, IT>& dbs)
{
    dbname.resize(0);
    // Only add a repeated db once.
    set<string> seen;
    for (auto i = dbs.first; i != dbs.second; ++i) {
	const string& v = i->second;
	if (v.empty()) continue;
	size_t p = 0, q;
	while (true) {
	    q = v.find('/', p);
	    string s(v, p, q - p);
	    if (!s.empty() && seen.find(s) == seen.end()) {
		add_database(s);
		seen.insert(s);
	    }
	    if (q == string::npos) break;
	    p = q + 1;
	}
    }
}

int main(int argc, char *argv[])
try {
    {
	// Check for SERVER_PROTOCOL=INCLUDED, which is set when we're being
	// included in a page via a server-side include directive.  In this
	// case we suppress sending a Content-Type: header.
	const char* p = getenv("SERVER_PROTOCOL");
	if (p && strcmp(p, "INCLUDED") == 0) {
	    suppress_http_headers = true;
	}
    }

    read_config_file();

    option["flag_default"] = "true";

    // set default thousands and decimal separators: e.g. "16,729 hits" "1.4K"
    option["decimal"] = ".";
    option["thousand"] = ",";

    // set the default stemming language
    option["stemmer"] = DEFAULT_STEM_LANGUAGE;

    // FIXME: set cout to linebuffered not stdout.  Or just flush regularly...
    // setvbuf(stdout, NULL, _IOLBF, 0);

    const char * method = getenv("REQUEST_METHOD");
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
	parse_db_params(cgi_params.equal_range("DB"));
	if (dbname.empty()) {
	    add_database(default_db);
	}
	enquire = new Xapian::Enquire(db);
    } catch (const Xapian::Error &) {
	enquire = NULL;
	db = Xapian::Database();
    }

    hits_per_page = 0;
    auto val = cgi_params.find("HITSPERPAGE");
    if (val != cgi_params.end()) {
	if (!parse_unsigned(val->second.c_str(), hits_per_page)) {
	    throw "HITSPERPAGE parameter must be >= 0";
	}
    }
    if (hits_per_page == 0) {
	hits_per_page = 10;
    } else if (hits_per_page > 1000) {
	hits_per_page = 1000;
    }

    val = cgi_params.find("DEFAULTOP");
    if (val != cgi_params.end()) {
	const string & v = val->second;
	if (v == "OR" || v == "or")
	    default_op = Xapian::Query::OP_OR;
    }

    val = cgi_params.find("FMT");
    if (val != cgi_params.end()) {
	const string & v = val->second;
	if (!v.empty()) fmtname = v;
    }
    if (fmtname.empty())
	fmtname = default_template;

    auto ml = cgi_params.equal_range("MORELIKE");
    if (enquire && ml.first != ml.second) {
	Xapian::RSet tmprset;
	for (auto i = ml.first; i != ml.second; ++i) {
	    const string& v = i->second;
	    Xapian::docid docid = atol(v.c_str());
	    if (docid == 0) {
		// Assume it's MORELIKE=Quid1138 and that Quid1138 is a UID
		// from an external source - we just find the correspond docid.
		Xapian::PostingIterator p = db.postlist_begin(v);
		if (p != db.postlist_end(v)) docid = *p;
	    }
	    if (docid != 0) {
		tmprset.add_document(docid);
	    }
	}

	if (!tmprset.empty()) {
	    OmegaExpandDecider decider(db);
	    set_expansion_scheme(*enquire, option);
	    Xapian::ESet eset(enquire->get_eset(40, tmprset, &decider));
	    string morelike_query;
	    for (auto&& term : eset) {
		if (!morelike_query.empty()) {
		    if (default_op == Xapian::Query::OP_OR) {
			morelike_query += ' ';
		    } else {
			morelike_query += " OR ";
		    }
		}
		morelike_query += pretty_term(term);
	    }
	    add_query_string(string(), morelike_query);
	}
    } else {
	// add expand/topterms terms if appropriate
	string expand_terms;
	if (cgi_params.find("ADD") != cgi_params.end()) {
	    auto g = cgi_params.equal_range("X");
	    for (auto i = g.first; i != g.second; ++i) {
		const string & v = i->second;
		if (!v.empty()) {
		    if (!expand_terms.empty())
			expand_terms += ' ';
		    expand_terms += v;
		}
	    }
	}

	// collect the unprefixed prob fields
	auto g = cgi_params.equal_range("P");
	for (auto i = g.first; i != g.second; ++i) {
	    const string & v = i->second;
	    if (!v.empty()) {
		// If there are expand terms, append them to the first
		// non-empty P parameter.
		if (!expand_terms.empty()) {
		    string q = v;
		    q += ' ';
		    q += expand_terms;
		    add_query_string(string(), q);
		    expand_terms = string();
		} else {
		    add_query_string(string(), v);
		}
	    }
	}

	if (!expand_terms.empty()) {
	    add_query_string(string(), expand_terms);
	}
    }

    auto begin = cgi_params.lower_bound("P.");
    auto end = cgi_params.lower_bound("P/"); // '/' is '.' + 1.
    for (auto i = begin; i != end; ++i) {
	const string & v = i->second;
	if (!v.empty()) {
	    string pfx(i->first, 2, string::npos);
	    add_query_string(pfx, v);
	}
    }

    // set any boolean filters
    auto g = cgi_params.equal_range("B");
    if (g.first != g.second) {
	vector<string> filter_v;
	for (auto i = g.first; i != g.second; ++i) {
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
	    const string & bterm = *j;
	    string::size_type e = bterm.find(filter_sep);
	    if (usual(e == string::npos)) {
		filters += bterm;
	    } else {
		// If a filter contains filter_sep then double it to escape.
		// Each filter must start with an alnum (checked above) and
		// the value after the last filter is the default op, which
		// is encoded as a non-alnum so filter_sep followed by
		// something other than filter_sep must be separating filters.
		string::size_type b = 0;
		while (e != string::npos) {
		    filters.append(bterm, b, e + 1 - b);
		    b = e;
		    e = bterm.find(filter_sep, b + 1);
		}
		filters.append(bterm, b, string::npos);
	    }
	    filters += filter_sep;
	    old_filters += bterm;
	    old_filters += filter_sep_old;
	}
    }

    // set any negated boolean filters
    g = cgi_params.equal_range("N");
    if (g.first != g.second) {
	vector<string> filter_v;
	for (auto i = g.first; i != g.second; ++i) {
	    const string & v = i->second;
	    // we'll definitely get empty N fields from "-ALL-" options
	    if (!v.empty() && C_isalnum(v[0])) {
		add_nterm(v);
		filter_v.push_back(v);
	    }
	}
	sort(filter_v.begin(), filter_v.end());
	vector<string>::const_iterator j;
	for (j = filter_v.begin(); j != filter_v.end(); ++j) {
	    const string & nterm = *j;
	    string::size_type e = nterm.find(filter_sep);
	    filters += '!';
	    if (usual(e == string::npos)) {
		filters += nterm;
	    } else {
		// If a filter contains filter_sep then double it to escape.
		// Each filter must start with an alnum (checked above) and
		// the value after the last filter is the default op, which
		// is encoded as a non-alnum so filter_sep followed by
		// something other than filter_sep must be separating filters.
		string::size_type b = 0;
		while (e != string::npos) {
		    filters.append(nterm, b, e + 1 - b);
		    b = e;
		    e = nterm.find(filter_sep, b + 1);
		}
		filters.append(nterm, b, string::npos);
	    }
	    filters += filter_sep;
	    // old_filters predates 'N' terms, so if there are 'N' terms this
	    // is definitely a different query.
	    old_filters.clear();
	}
    }

    // date range filters
    struct date_range {
	string start, end, span;
    };
    map<Xapian::valueno, date_range> date_ranges;
    begin = cgi_params.lower_bound("START.");
    end = cgi_params.lower_bound("START/"); // '/' is '.' + 1.
    for (auto i = begin; i != end; ++i) {
	const string & v = i->second;
	if (!v.empty()) {
	    Xapian::valueno slot;
	    if (!parse_unsigned(i->first.c_str() +
				CONST_STRLEN("START."), slot)) {
		throw "START slot value must be >= 0";
	    }
	    date_ranges[slot].start = v;
	}
    }
    begin = cgi_params.lower_bound("END.");
    end = cgi_params.lower_bound("END/"); // '/' is '.' + 1.
    for (auto i = begin; i != end; ++i) {
	const string & v = i->second;
	if (!v.empty()) {
	    Xapian::valueno slot;
	    if (!parse_unsigned(i->first.c_str() +
				CONST_STRLEN("END."), slot)) {
		throw "END slot value must be >= 0";
	    }
	    date_ranges[slot].end = v;
	}
    }
    begin = cgi_params.lower_bound("SPAN.");
    end = cgi_params.lower_bound("SPAN/"); // '/' is '.' + 1.
    for (auto i = begin; i != end; ++i) {
	const string & v = i->second;
	if (!v.empty()) {
	    Xapian::valueno slot;
	    if (!parse_unsigned(i->first.c_str() +
				CONST_STRLEN("SPAN."), slot)) {
		throw "SPAN slot value must be >= 0";
	    }
	    date_ranges[slot].span = v;
	}
    }
    if (!date_ranges.empty()) {
	// old_filters predates START.N, END.N and SPAN.N so use of any of
	// these means this is definitely a different query.
	old_filters.clear();
    }
    for (auto i : date_ranges) {
	auto slot = i.first;
	auto r = i.second;
	add_date_filter(r.start, r.end, r.span, slot);
	filters += '$';
	filters += str(slot);
	filters += '$';
	filters += r.start;
	filters += '$';
	filters += r.end;
	filters += '$';
	filters += r.span;
    }

    string date_start, date_end, date_span;
    val = cgi_params.find("DATEVALUE");
    Xapian::valueno date_value_slot = Xapian::BAD_VALUENO;
    if (val != cgi_params.end() &&
	!parse_unsigned(val->second.c_str(), date_value_slot)) {
	throw "DATEVALUE slot must be >= 0";
    }
    // Process DATEVALUE=n and associated values unless we saw START.n=...
    // or END.n=... or SPAN.n=...
    if (date_ranges.find(date_value_slot) == date_ranges.end()) {
	val = cgi_params.find("START");
	if (val != cgi_params.end()) date_start = val->second;
	val = cgi_params.find("END");
	if (val != cgi_params.end()) date_end = val->second;
	val = cgi_params.find("SPAN");
	if (val != cgi_params.end()) date_span = val->second;
	add_date_filter(date_start, date_end, date_span, date_value_slot);
    }

    // If more default_op values are supported, encode them as non-alnums
    // other than filter_sep, '!' or '$'.
    filters += (default_op == Xapian::Query::OP_AND ? '.' : '-');
    filters += date_start;
    filters += filter_sep;
    filters += date_end;
    filters += filter_sep;
    filters += date_span;
    if (date_value_slot != Xapian::BAD_VALUENO) {
	// This means we'll force the first page when reloading or changing
	// page starting from existing URLs upon upgrade to 1.4.1, but the
	// exact same existing URL could be for a search without the date
	// filter where we want to force the first page, so there's an inherent
	// ambiguity there.  Forcing first page in this case seems the least
	// problematic side-effect.
	filters += filter_sep;
	filters += str(date_value_slot);
    }

    if (!old_filters.empty()) {
	old_filters += date_start;
	old_filters += filter_sep_old;
	old_filters += date_end;
	old_filters += filter_sep_old;
	old_filters += date_span;
	old_filters += (default_op == Xapian::Query::OP_AND ? 'A' : 'O');
    }

    // Percentage relevance cut-off
    val = cgi_params.find("THRESHOLD");
    if (val != cgi_params.end()) {
	unsigned int temp;
	if (val->second[0] == '-') {
	    if (!parse_unsigned(val->second.c_str() + 1, temp)) {
		throw "THRESHOLD parameter must be an integer";
	    }
	    threshold = 0;
	} else if (!parse_unsigned(val->second.c_str(), temp)) {
	    throw "THRESHOLD parameter must be an integer";
	}
	if (temp > 100) {
	    threshold = 100;
	} else {
	    threshold = temp;
	}
    }

    // collapsing
    val = cgi_params.find("COLLAPSE");
    if (val != cgi_params.end()) {
	const string & v = val->second;
	if (!v.empty()) {
	    if (!parse_unsigned(val->second.c_str(), collapse_key)) {
		throw "COLLAPSE parameter must be >= 0";
	    }
	    collapse = true;
	    filters += filter_sep;
	    filters += str(collapse_key);
	    if (!old_filters.empty()) {
		old_filters += filter_sep_old;
		old_filters += str(collapse_key);
	    }
	}
    }
    if (!collapse && date_value_slot != Xapian::BAD_VALUENO) {
	// We need to either omit filter_sep for both or neither, or else the
	// encoding is ambiguous.
	filters += filter_sep;
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
		if (!old_filters.empty()) old_filters += 'D';
	    } else if (ch != 'A') {
		docid_order = Xapian::Enquire::DONT_CARE;
	    } else {
		// This is a bug (should add nothing here and 'X' in the (ch !=
		// 'A') case, but the current "DONT_CARE" implementation
		// actually always results in ascending docid order so it's not
		// worth breaking compatibility to fix - let's just do it next
		// time we change the encoding $filters uses.
		filters += 'X';
		if (!old_filters.empty()) old_filters += 'X';
	    }
	}
    }

    // sorting
    val = cgi_params.find("SORT");
    if (val != cgi_params.end()) {
	const char * base = val->second.c_str();
	const char * p = base;
	do {
	    bool rev = (*p != '+');
	    if (*p == '-' || *p == '+') {
		// old_filters predates support for direction in SORT, so if
		// there's a direction specified this is definitely a different
		// query.
		old_filters.clear();
		++p;
	    }
	    if (!C_isdigit(*p)) {
		// Invalid.
		break;
	    }
	    errno = 0;
	    char * q;
	    Xapian::valueno slot = strtoul(p, &q, 10);
	    p = q;
	    if (errno != 0) {
		// Invalid.
		break;
	    }

	    if (sort_key != Xapian::BAD_VALUENO) {
		// Multiple sort keys specified, so we need a KeyMaker.

		// Omit leading '+'.
		if (reverse_sort) filters += '-';
		filters += str(sort_key);

		sort_keymaker = new Xapian::MultiValueKeyMaker;
		sort_keymaker->add_value(sort_key, !reverse_sort);
		sort_key = Xapian::BAD_VALUENO;
		reverse_sort = true;
		// old_filters predates multiple sort keys, so if there are
		// multiple sort keys this is definitely a different query.
		old_filters.clear();
	    }

	    if (sort_keymaker) {
		filters += (rev ? '-' : '+');
		filters += str(slot);
		sort_keymaker->add_value(slot, !rev);
	    } else {
		sort_key = slot;
		reverse_sort = rev;
	    }
	    while (C_isspace(*p) || *p == ',') ++p;
	} while (*p);

	val = cgi_params.find("SORTREVERSE");
	if (val != cgi_params.end()) {
	    unsigned int temp;
	    if (!parse_unsigned(val->second.c_str(), temp)) {
		throw "SORTREVERSE parameter must be >= 0";
	    }
	    if (temp != 0) {
		reverse_sort = !reverse_sort;
	    }
	}
	val = cgi_params.find("SORTAFTER");
	if (val != cgi_params.end()) {
	    unsigned int temp;
	    if (!parse_unsigned(val->second.c_str(), temp)) {
		throw "SORTAFTER parameter must be >= 0";
	    }
	    sort_after = bool(temp);
	}

	// Add the sorting related options to filters too.
	//
	// Note: old_filters really does encode a reversed sort as 'F' and a
	// non-reversed sort as 'R' or 'r'.
	//
	// filters has them the other way around for sanity (except in
	// development snapshot 1.3.4, which was when the new filter encoding
	// was introduced).
	if (!sort_keymaker) filters += str(sort_key);
	if (!old_filters.empty()) old_filters += str(sort_key);
	if (sort_after) {
	    if (reverse_sort) {
		filters += 'R';
		if (!old_filters.empty()) old_filters += 'F';
	    } else {
		filters += 'F';
		if (!old_filters.empty()) old_filters += 'R';
	    }
	} else {
	    if (!reverse_sort) {
		filters += 'f';
		if (!old_filters.empty()) old_filters += 'r';
	    }
	}
    }

    if (old_filters.empty()) old_filters = filters;

    // min_hits (fill mset past topdoc+(hits_per_page+1) to
    // topdoc+max(hits_per_page+1,min_hits)
    val = cgi_params.find("MINHITS");
    if (val != cgi_params.end()) {
	if (!parse_unsigned(val->second.c_str(), min_hits)) {
	    throw "MINHITS parameter must be >= 0";
	}
    }

    parse_omegascript();
} catch (const Xapian::Error &e) {
    if (!set_content_type && !suppress_http_headers)
	cout << "Content-Type: text/html\n\n";
    cout << "Exception: " << html_escape(e.get_description()) << endl;
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
