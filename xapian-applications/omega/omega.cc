/** @file
 * @brief Main module for omega (example CGI frontend for Xapian)
 */
/* Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2001 James Aylett
 * Copyright 2001,2002 Ananova Ltd
 * Copyright 2002-2024 Olly Betts
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

#define FILTER_CODE \
    "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz_-"

template<typename T>
static void
filters_encode_uint(T v)
{
    do {
	if (v >= 64)
	    filters += ' ';
	filters += FILTER_CODE[v & 63];
	v >>= 6;
    } while (v);
}

static void
filters_append(const string& bterm, const string* prev)
{
    auto reuse = prev ? common_prefix_length(*prev, bterm) : 0u;
    if (prev)
	filters_encode_uint(reuse);
    filters_encode_uint(bterm.size() - reuse);
    filters.append(bterm, reuse);

    auto e = bterm.find(filter_sep);
    if (usual(e == string::npos)) {
	old_filters += bterm;
    } else {
	// For old_filters, we don't try to reuse part of the previous term,
	// and if a filter contains filter_sep then we double it to escape.
	// Each filter must start with an alnum (checked before we get called)
	// and the value after the last filter is the default op, which is
	// encoded as a non-alnum so filter_sep followed by something other
	// than filter_sep must be separating filters.
	string::size_type b = 0;
	while (e != string::npos) {
	    old_filters.append(bterm, b, e + 1 - b);
	    b = e;
	    e = bterm.find(filter_sep, b + 1);
	}
	old_filters.append(bterm, b, string::npos);
    }
    old_filters += filter_sep;
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
	const string* prev = NULL;
	for (const string& bterm : filter_v) {
	    filters_append(bterm, prev);
	    prev = &bterm;
	}
    }

    // Current filters format:
    //
    // [<encoded length><boolean filter term>]*
    // ['!'[<encoded length><negated boolean filter term>]*]?
    // ['.'<collapse key>]?
    // ['$'<encoded date range slot (omitted for term-based)>?
    //     ['!'<date start>]?
    //     ['.'<date end>]?
    //     ['~'<date span>]?
    // [['-'?<sort key>[['-'|'+']<sort key>]+]|<sort key>|]?
    // <encoded integer of default_op, docid_order, sort_after, sort_reverse>
    //
    // (filter terms in ascending byte sorted order, and with second and
    // subsequent actually stored as <reuse character><tail>)
    //
    // old_filters format:
    //
    // [<boolean filter term with any '~' escaped to '~~'>'~']*
    // ['!'<negated boolean filter term with any '~' escaped to '~~'>'~']*
    // ['$'<date range slot>'$'<date start>'$'<date end>'$'<date span>]*
    // ['.'|'-']   ; default_op AND vs OR
    // <date start>'~'<date end>'~'<date span>['~'<date value slot>]?
    // ['~'<collapse key>]?   ; present if <collapse key> non-empty or
    //                        ; previous element present
    // ['D'|'X']? ; 'D' for docid_order DESCENDING; 'X' for DONT_CARE.
    // [['-'?<sort key>[['-'|'+']<sort key>]+]|<sort key>]? ['R'|'F'|'f']?
    //
    // (filter terms in ascending byte sorted order)

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
	if (!filter_v.empty()) {
	    filters += '!';
	    sort(filter_v.begin(), filter_v.end());
	    const string* prev = NULL;
	    for (const string& nterm : filter_v) {
		old_filters += '!';
		filters_append(nterm, prev);
		prev = &nterm;
	    }
	}
    }

    // collapsing
    val = cgi_params.find("COLLAPSE");
    if (val != cgi_params.end()) {
	const string& v = val->second;
	if (!v.empty()) {
	    if (!parse_unsigned(val->second.c_str(), collapse_key)) {
		throw "COLLAPSE parameter must be >= 0";
	    }
	    collapse = true;
	    filters += '.';
	    filters_encode_uint(collapse_key);
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

    string date_start, date_end, date_span;
    val = cgi_params.find("START");
    if (val != cgi_params.end()) {
	date_start = val->second;
    }
    val = cgi_params.find("END");
    if (val != cgi_params.end()) {
	date_end = val->second;
    }
    val = cgi_params.find("SPAN");
    if (val != cgi_params.end()) {
	date_span = val->second;
    }
    val = cgi_params.find("DATEVALUE");
    Xapian::valueno date_value_slot = Xapian::BAD_VALUENO;
    if (val != cgi_params.end() &&
	!parse_unsigned(val->second.c_str(), date_value_slot)) {
	throw "DATEVALUE slot must be >= 0";
    }
    if (date_value_slot != Xapian::BAD_VALUENO ||
	!date_start.empty() ||
	!date_end.empty() ||
	!date_span.empty()) {
	// Process DATEVALUE=n and associated values unless we saw START.n=...
	// or END.n=... or SPAN.n=...
	date_ranges.emplace(date_value_slot,
			    date_range{date_start, date_end, date_span});
    }
    for (auto i : date_ranges) {
	auto slot = i.first;
	auto r = i.second;
	add_date_filter(r.start, r.end, r.span, slot);
	filters += '$';
	if (slot != Xapian::BAD_VALUENO) {
	    filters_encode_uint(slot);
	    if (slot != date_value_slot) {
		old_filters += '$';
		old_filters += str(slot);
		old_filters += '$';
		old_filters += r.start;
		old_filters += '$';
		old_filters += r.end;
		old_filters += '$';
		old_filters += r.span;
	    }
	}
	if (!r.start.empty()) {
	    filters += '!';
	    filters += r.start;
	}
	if (!r.end.empty()) {
	    filters += '.';
	    filters += r.end;
	}
	if (!r.span.empty()) {
	    filters += '~';
	    filters += r.span;
	}
    }

    old_filters += (default_op == Xapian::Query::OP_AND ? '.' : '-');
    old_filters += date_start;
    old_filters += filter_sep;
    old_filters += date_end;
    old_filters += filter_sep;
    old_filters += date_span;
    if (date_value_slot != Xapian::BAD_VALUENO) {
	// This means we'll force the first page when reloading or changing
	// page starting from existing URLs upon upgrade to 1.4.1, but the
	// exact same existing URL could be for a search without the date
	// filter where we want to force the first page, so there's an inherent
	// ambiguity there.  Forcing first page in this case seems the least
	// problematic side-effect.
	old_filters += filter_sep;
	old_filters += str(date_value_slot);
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

    if (collapse) {
	old_filters += filter_sep;
	old_filters += str(collapse_key);
    } else if (date_value_slot != Xapian::BAD_VALUENO) {
	// We need to either omit filter_sep for both or neither, or else the
	// encoding is ambiguous.
	old_filters += filter_sep;
    }

    // docid order
    val = cgi_params.find("DOCIDORDER");
    if (val != cgi_params.end()) {
	const string & v = val->second;
	if (!v.empty()) {
	    char ch = v[0];
	    if (ch == 'D') {
		docid_order = Xapian::Enquire::DESCENDING;
		old_filters += 'D';
	    } else if (ch != 'A') {
		docid_order = Xapian::Enquire::DONT_CARE;
	    } else {
		// This was a bug in the 1.4.x filter encoding (we should have
		// added nothing here and 'X' in the `ch != 'A'` case), but the
		// current "DONT_CARE" implementation actually always results
		// in ascending docid order so it wasn't worth breaking
		// compatibility in a stable release series to fix.
		old_filters += 'X';
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
		if (reverse_sort) {
		    filters += '-';
		    old_filters += '-';
		}
		filters_encode_uint(sort_key);
		old_filters += str(sort_key);

		sort_keymaker = new Xapian::MultiValueKeyMaker;
		sort_keymaker->add_value(sort_key, !reverse_sort);
		sort_key = Xapian::BAD_VALUENO;
		reverse_sort = true;
	    }

	    if (sort_keymaker) {
		filters += (rev ? '-' : '+');
		old_filters += (rev ? '-' : '+');
		filters_encode_uint(slot);
		old_filters += str(slot);
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
	if (!sort_keymaker) {
	    filters_encode_uint(sort_key);
	    old_filters += str(sort_key);
	}
	if (sort_after) {
	    if (reverse_sort) {
		old_filters += 'R';
	    } else {
		old_filters += 'F';
	    }
	} else {
	    if (!reverse_sort) {
		old_filters += 'f';
	    }
	}
    }

    {
	// Encode default_op, docid_order, reverse_sort and sort_after together
	// in a single character.
	unsigned v = 0;
	switch (default_op) {
	  case Xapian::Query::OP_AND:
	    break;
	  case Xapian::Query::OP_OR:
	    v = 0x01 * 12;
	    break;
	  default:
	    // Additional supported value should encode as:
	    // 0x02 * 12
	    // 0x03 * 12
	    // ...
	    break;
	}
	v |= 0x04 * static_cast<unsigned>(docid_order);
	if (reverse_sort) v |= 0x01;
	if (sort_after) v |= 0x02;
	filters_encode_uint(v);
    }

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
