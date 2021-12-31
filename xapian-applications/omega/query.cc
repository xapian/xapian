/** @file
 * @brief query executor for omega
 */
/* Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2001 James Aylett
 * Copyright 2001,2002 Ananova Ltd
 * Copyright 2002 Intercede 1749 Ltd
 * Copyright 2002,2003,2004,2005,2006,2007,2008,2009,2010,2011,2013,2014,2015,2016,2017,2018,2019,2020,2021 Olly Betts
 * Copyright 2008 Thomas Viehmann
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

#include <algorithm>
#include <iostream>
#include <map>
#include <random>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <cassert>
#include <cctype>
#include <cerrno>
#include <stdio.h>
#include <cstdlib>
#include <cstring>
#include "strcasecmp.h"
#include <ctime>

#include "safeunistd.h"
#include <sys/types.h>
#include "safesysstat.h"
#include "safefcntl.h"

#include "realtime.h"

#include <cdb.h>

#include "csvescape.h"
#include "date.h"
#include "datevalue.h"
#include "fields.h"
#include "jsonescape.h"
#include "utils.h"
#include "omega.h"
#include "query.h"
#include "cgiparam.h"
#include "loadfile.h"
#include "sample.h"
#include "sort.h"
#include "str.h"
#include "stringutils.h"
#include "transform.h"
#include "urldecode.h"
#include "urlencode.h"
#include "unixperm.h"
#include "values.h"
#include "weight.h"
#include "expand.h"
#include "md5wrap.h"
#include "parseint.h"
#include <xapian.h>

using namespace std;

using Xapian::Utf8Iterator;

using Xapian::Unicode::is_wordchar;

#ifndef SNPRINTF
#include <cstdarg>

static int my_snprintf(char *str, size_t size, const char *format, ...)
{
    int res;
    va_list ap;
    va_start(ap, format);
    str[size - 1] = '\0';
    res = vsprintf(str, format, ap);
    if (str[size - 1] || res < 0 || size_t(res) >= size)
	abort(); /* Overflowed! */
    va_end(ap);
    return res;
}
#else
#define my_snprintf SNPRINTF
#endif

/// Map shard to DB parameter value and stats to allow docid mapping.
vector<SubDB> subdbs;

static bool query_parsed = false;
static bool done_query = false;
static Xapian::docid last = 0;
static Xapian::docid topdoc = 0;

static Xapian::MSet mset;
static Xapian::RSet rset;

static map<Xapian::docid, bool> ticked;

static void ensure_query_parsed();
static void ensure_match();

static Xapian::Query query;
//static string url_query_string;
Xapian::Query::op default_op = Xapian::Query::OP_AND; // default matching mode

// Maintain an explicit date_filter_set flag - date_filter.empty() will also
// be true if a date filter is specified which simplifies to
// Query::MatchNothing at construction time.
static bool date_filter_set = false;
static Xapian::Query date_filter;

static Xapian::QueryParser qp;
static Xapian::NumberRangeProcessor * size_rp = NULL;
static Xapian::Stem *stemmer = NULL;

static string eval_file(const string& fmtfile, bool* p_not_found = nullptr);

static set<string> termset;

// Holds mapping from term prefix to user prefix (e.g. 'S' -> 'subject:').
static map<string, string> termprefix_to_userprefix;

static string queryterms;

static string error_msg;

static double secs = -1;

static const char DEFAULT_LOG_ENTRY[] =
	"$or{$env{REMOTE_HOST},$env{REMOTE_ADDR},-}\t"
	"[$date{$now,%d/%b/%Y:%H:%M:%S} +0000]\t"
	"$if{$cgi{X},add,$if{$cgi{MORELIKE},morelike,query}}\t"
	"$dbname\t"
	"$query\t"
	"$msize$if{$env{HTTP_REFERER},\t$env{HTTP_REFERER}}";

class MyStopper : public Xapian::Stopper {
  public:
    bool operator()(const string &t) const {
	switch (t[0]) {
	    case 'a':
		return (t == "a" || t == "about" || t == "an" || t == "and" ||
			t == "are" || t == "as" || t == "at");
	    case 'b':
		return (t == "be" || t == "by");
	    case 'e':
		return (t == "en");
	    case 'f':
		return (t == "for" || t == "from");
	    case 'h':
		return (t == "how");
	    case 'i':
		return (t == "i" || t == "in" || t == "is" || t == "it");
	    case 'o':
		return (t == "of" || t == "on" || t == "or");
	    case 't':
		return (t == "that" || t == "the" || t == "this" || t == "to");
	    case 'w':
		return (t == "was" || t == "what" || t == "when" ||
			t == "where" || t == "which" || t == "who" ||
			t == "why" || t == "will" || t == "with");
	    case 'y':
		return (t == "you" || t == "your");
	    default:
		return false;
	}
    }
};

static size_t
prefix_from_term(string* prefix, const string& term)
{
    if (!term.empty()) {
	if (term[0] == 'X') {
	    const string::const_iterator begin = term.begin();
	    string::const_iterator i = begin + 1;
	    while (i != term.end() && C_isupper(*i))
		++i;
	    if (prefix)
		prefix->assign(begin, i);
	    if (i != term.end() && *i == ':')
		++i;
	    return i - begin;
	}

	if (C_isupper(term[0])) {
	    if (prefix)
		*prefix = term[0];
	    return 1;
	}
    }

    if (prefix)
	prefix->resize(0);
    return 0;
}

// Don't allow ".." in format names, log file names, etc as this would allow
// people to open a format "../../etc/passwd" or similar.
// FIXME: make this check more exact ("foo..bar" is safe)
// FIXME: log when this check fails
static bool
vet_filename(const string &filename)
{
    string::size_type i = filename.find("..");
    return (i == string::npos);
}

// Heuristics:
// * If any terms have been removed, it's a "fresh query" so we discard any
//   relevance judgements
// * If all previous terms are there but more have been added then we keep
//   the relevance judgements, but return the first page of hits
//
// NEW_QUERY entirely new query
// SAME_QUERY unchanged query
// EXTENDED_QUERY new query, but based on the old one
// BAD_QUERY parse error (message in error_msg)
typedef enum { NEW_QUERY, SAME_QUERY, EXTENDED_QUERY, BAD_QUERY } querytype;

static multimap<string, string> query_strings;

void
add_query_string(const string& prefix, const string& s)
{
    string query_string = s;
    // Strip leading and trailing whitespace from query_string.
    trim(query_string);
    if (!query_string.empty())
	query_strings.insert(make_pair(prefix, query_string));
}

static unsigned
read_qp_flags(const string & opt_pfx, unsigned f)
{
    map<string, string>::const_iterator i = option.lower_bound(opt_pfx);
    for (; i != option.end() && startswith(i->first, opt_pfx); ++i) {
	unsigned mask = 0;
	const char * s = i->first.c_str() + opt_pfx.size();
	switch (s[0]) {
	    case 'a':
                // Note that the ``Xapian::QueryParser::FLAG_ACCUMULATE`` flag
                // is or-ed in below because it's needed for ``$stoplist`` and
                // ``$unstem`` to work correctly, and so is deliberately not
                // available to specify here.
		if (strcmp(s, "auto_multiword_synonyms") == 0) {
		    mask = Xapian::QueryParser::FLAG_AUTO_MULTIWORD_SYNONYMS;
		    break;
		}
		if (strcmp(s, "auto_synonyms") == 0) {
		    mask = Xapian::QueryParser::FLAG_AUTO_SYNONYMS;
		    break;
		}
		break;
	    case 'b':
		if (strcmp(s, "boolean") == 0) {
		    mask = Xapian::QueryParser::FLAG_BOOLEAN;
		    break;
		}
		if (strcmp(s, "boolean_any_case") == 0) {
		    mask = Xapian::QueryParser::FLAG_BOOLEAN_ANY_CASE;
		    break;
		}
		break;
	    case 'c':
		if (strcmp(s, "cjk_ngram") == 0) {
		    mask = Xapian::QueryParser::FLAG_CJK_NGRAM;
		    break;
		}
		if (strcmp(s, "cjk_words") == 0) {
		    mask = Xapian::QueryParser::FLAG_CJK_WORDS;
		    break;
		}
		break;
	    case 'd':
		if (strcmp(s, "default") == 0) {
		    mask = Xapian::QueryParser::FLAG_DEFAULT;
		    break;
		}
		break;
	    case 'f':
		if (strcmp(s, "fuzzy") == 0) {
		    mask = Xapian::QueryParser::FLAG_FUZZY;
		    break;
		}
		break;
	    case 'l':
		if (strcmp(s, "lovehate") == 0) {
		    mask = Xapian::QueryParser::FLAG_LOVEHATE;
		    break;
		}
		break;
	    case 'n':
		if (strcmp(s, "no_positions") == 0) {
		    mask = Xapian::QueryParser::FLAG_NO_POSITIONS;
		    break;
		}
		break;
	    case 'p':
		if (strcmp(s, "partial") == 0) {
		    mask = Xapian::QueryParser::FLAG_PARTIAL;
		    break;
		}
		if (strcmp(s, "phrase") == 0) {
		    mask = Xapian::QueryParser::FLAG_PHRASE;
		    break;
		}
		if (strcmp(s, "pure_not") == 0) {
		    mask = Xapian::QueryParser::FLAG_PURE_NOT;
		    break;
		}
		break;
	    case 's':
		if (strcmp(s, "spelling_correction") == 0) {
		    mask = Xapian::QueryParser::FLAG_SPELLING_CORRECTION;
		    break;
		}
		if (strcmp(s, "synonym") == 0) {
		    mask = Xapian::QueryParser::FLAG_SYNONYM;
		    break;
		}
		break;
	    case 'w':
		if (strcmp(s, "wildcard") == 0) {
		    mask = Xapian::QueryParser::FLAG_WILDCARD;
		    break;
		}
#if XAPIAN_AT_LEAST(1,5,0)
		if (strcmp(s, "wildcard_glob") == 0) {
		    mask = Xapian::QueryParser::FLAG_WILDCARD_GLOB;
		    break;
		}
		if (strcmp(s, "wildcard_multi") == 0) {
		    mask = Xapian::QueryParser::FLAG_WILDCARD_MULTI;
		    break;
		}
		if (strcmp(s, "wildcard_single") == 0) {
		    mask = Xapian::QueryParser::FLAG_WILDCARD_SINGLE;
		    break;
		}
#endif
		break;
	}

	if (i->second.empty()) {
	    f &= ~mask;
	} else {
	    f |= mask;
	}
    }
    // Always enable FLAG_ACCUMULATE so that $stoplist and $unstem report
    // values accumulated over all query strings parsed as part of a query, not
    // just the last one parsed.
    return f | Xapian::QueryParser::FLAG_ACCUMULATE;
}

static querytype
parse_queries(const string& oldp)
{
    // Parse the query string.
    auto opt_it = option.find("stem_strategy");
    if (opt_it != option.end()) {
	if (opt_it->second == "all") {
	    qp.set_stemming_strategy(Xapian::QueryParser::STEM_ALL);
	} else if (opt_it->second == "all_z") {
	    qp.set_stemming_strategy(Xapian::QueryParser::STEM_ALL_Z);
	} else if (opt_it->second == "none") {
	    qp.set_stemming_strategy(Xapian::QueryParser::STEM_NONE);
	} else if (opt_it->second == "some") {
	    qp.set_stemming_strategy(Xapian::QueryParser::STEM_SOME);
	} else if (opt_it->second == "some_full_pos") {
	    qp.set_stemming_strategy(Xapian::QueryParser::STEM_SOME_FULL_POS);
	}
    } else {
	opt_it = option.find("stem_all");
	if (opt_it != option.end() && opt_it->second == "true") {
	    qp.set_stemming_strategy(Xapian::QueryParser::STEM_ALL);
	}
    }
    qp.set_stopper((new MyStopper())->release());
    qp.set_default_op(default_op);
    qp.set_database(db);
    // FIXME: provide a custom RP which handles size:10..20K, etc.
    if (!size_rp)
	size_rp = new Xapian::NumberRangeProcessor(VALUE_SIZE, "size:");
    qp.add_rangeprocessor(size_rp);
    map<string, string>::const_iterator pfx = option.lower_bound("prefix,");
    for (; pfx != option.end() && startswith(pfx->first, "prefix,"); ++pfx) {
	string user_prefix(pfx->first, 7);
	const string & term_pfx_list = pfx->second;
	string::size_type i = 0;
	do {
	    string::size_type i0 = i;
	    i = term_pfx_list.find('\t', i);
	    const string & term_pfx = term_pfx_list.substr(i0, i - i0);
	    qp.add_prefix(user_prefix, term_pfx);
	    // std::map::insert() won't overwrite an existing entry, so we'll
	    // prefer the first user_prefix for which a particular term prefix
	    // is specified.
	    termprefix_to_userprefix.insert(make_pair(term_pfx, user_prefix));
	} while (++i);
    }
    pfx = option.lower_bound("boolprefix,");
    for (; pfx != option.end() && startswith(pfx->first, "boolprefix,"); ++pfx) {
	string user_prefix(pfx->first, 11, string::npos);
	auto it = option.find("nonexclusiveprefix," + pfx->second);
	bool exclusive = (it == option.end() || it->second.empty());
	qp.add_boolean_prefix(user_prefix, pfx->second, exclusive);
	termprefix_to_userprefix.insert(make_pair(pfx->second, user_prefix));
    }

    try {
	unsigned default_flags = read_qp_flags("flag_", 0);

	vector<Xapian::Query> queries;
	queries.reserve(query_strings.size());

	for (auto& j : query_strings) {
	    const string& prefix = j.first;
	    const string& query_string = j.second;

	    // Choose the stemmer to use for this input.
	    string stemlang = option[prefix + ":stemmer"];
	    if (stemlang.empty())
		stemlang = option["stemmer"];
	    qp.set_stemmer(Xapian::Stem(stemlang));

	    // Work out the flags to use for this input.
	    unsigned f = read_qp_flags(prefix + ":flag_", default_flags);

	    Xapian::Query q = qp.parse_query(query_string, f, prefix);
	    if (!q.empty())
		queries.push_back(q);
	}
	query = Xapian::Query(query.OP_AND, queries.begin(), queries.end());
    } catch (Xapian::QueryParserError &e) {
	error_msg = e.get_msg();
	return BAD_QUERY;
    }

    Xapian::termcount n_new_terms = 0;
    for (Xapian::TermIterator i = query.get_terms_begin();
	 i != query.get_terms_end(); ++i) {
	if (termset.find(*i) == termset.end()) {
	    termset.insert(*i);
	    if (!queryterms.empty()) queryterms += '\t';
	    queryterms += *i;
	}
	n_new_terms++;
    }

    // Check new query against the previous one
    if (oldp.empty()) {
	// If oldp was empty that means there were no parsed query terms
	// before, so if there are now this is a new query.
	return n_new_terms ? NEW_QUERY : SAME_QUERY;
    }

    // The terms in oldp are separated by tabs.
    const char oldp_separator = '\t';
    size_t n_old_terms = count(oldp.begin(), oldp.end(), oldp_separator) + 1;

    // short-cut: if the new query has fewer terms, it must be a new one
    if (n_new_terms < n_old_terms) return NEW_QUERY;

    const char *term = oldp.c_str();
    const char *pend;
    while ((pend = strchr(term, oldp_separator)) != NULL) {
	if (termset.find(string(term, pend - term)) == termset.end())
	    return NEW_QUERY;
	term = pend + 1;
    }
    if (*term) {
	if (termset.find(string(term)) == termset.end())
	    return NEW_QUERY;
    }

    // Use termset.size() rather than n_new_terms so we correctly handle
    // the case when the query has repeated terms.
    // This works wrongly in the case when the user extends the query
    // by adding a term already in it, but that's unlikely and the behaviour
    // isn't too bad (we just don't reset page 1).  We also mishandle a few
    // other obscure cases e.g. adding quotes to turn a query into a phrase.
    if (termset.size() > n_old_terms) return EXTENDED_QUERY;
    return SAME_QUERY;
}

static multimap<string, string> filter_map;
static set<string> neg_filters;

void add_bterm(const string &term) {
    string prefix;
    if (prefix_from_term(&prefix, term) > 0)
	filter_map.insert(multimap<string, string>::value_type(prefix, term));
}

void add_nterm(const string &term) {
    if (!term.empty())
	neg_filters.insert(term);
}

void
add_date_filter(const string& date_start,
		const string& date_end,
		const string& date_span,
		Xapian::valueno date_value_slot)
{
    if (date_start.empty() && date_end.empty() && date_span.empty())
	return;

    Xapian::Query q;
    if (date_value_slot != Xapian::BAD_VALUENO) {
	// The values can be a time_t in 4 bytes, or YYYYMMDD... (with the
	// latter the sort order just works correctly between different
	// precisions).
	bool as_time_t =
	    db.get_value_lower_bound(date_value_slot).size() == 4 &&
	    db.get_value_upper_bound(date_value_slot).size() == 4;
	q = date_value_range(as_time_t, date_value_slot,
			     date_start, date_end,
			     date_span);
    } else {
	q = date_range_filter(date_start, date_end, date_span);
	q |= Xapian::Query("Dlatest");
    }

    if (date_filter_set) {
	date_filter &= q;
    } else {
	date_filter_set = true;
	date_filter = q;
    }
}

static void
run_query()
{
    string scheme;
    bool force_boolean = false;
    if (!filter_map.empty()) {
	// OR together filters with the same prefix (or AND for non-exclusive
	// prefixes), then AND together the resultant groups.
	vector<Xapian::Query> filter_vec;
	vector<string> same_vec;
	string current;
	for (auto i = filter_map.begin(); ; ++i) {
	    bool over = (i == filter_map.end());
	    if (over || i->first != current) {
		switch (same_vec.size()) {
		    case 0:
			break;
		    case 1:
			filter_vec.push_back(Xapian::Query(same_vec[0]));
			break;
		    default: {
			Xapian::Query::op op = Xapian::Query::OP_OR;
			auto it = option.find("nonexclusiveprefix," + current);
			if (it != option.end() && !it->second.empty()) {
			    op = Xapian::Query::OP_AND;
			}
			filter_vec.push_back(Xapian::Query(op,
							   same_vec.begin(),
							   same_vec.end()));
			break;
		    }
		}
		same_vec.clear();
		if (over) break;
		current = i->first;
	    }
	    same_vec.push_back(i->second);
	}

	Xapian::Query filter(Xapian::Query::OP_AND,
			     filter_vec.begin(), filter_vec.end());

	if (query.empty()) {
	    // If no query strings were provided then promote the filters
	    // to be THE query - filtering an empty query will give no
	    // matches.
	    std::swap(query, filter);
	    auto&& it = option.find("weightingpurefilter");
	    if (it != option.end() && !it->second.empty()) {
		scheme = it->second;
	    } else {
		force_boolean = true;
	    }
	} else {
	    query = Xapian::Query(Xapian::Query::OP_FILTER, query, filter);
	}
    }

    if (date_filter_set) {
	// If no query strings were provided then promote the daterange
	// filter to be THE query instead of filtering an empty query.
	if (query.empty()) {
	    query = date_filter;
	    force_boolean = true;
	} else {
	    query = Xapian::Query(Xapian::Query::OP_FILTER, query, date_filter);
	}
    }

    if (!neg_filters.empty()) {
	// OR together all negated filters.
	Xapian::Query filter(Xapian::Query::OP_OR,
			     neg_filters.begin(), neg_filters.end());

	if (query.empty() && !date_filter_set) {
	    // If we only have a negative filter for the query, use MatchAll as
	    // the query to apply the filters to.
	    query = Xapian::Query::MatchAll;
	    force_boolean = true;
	}
	query = Xapian::Query(Xapian::Query::OP_AND_NOT, query, filter);
    }

    if (!enquire || !error_msg.empty()) return;

    if (!force_boolean && scheme.empty()) {
	auto&& it = option.find("weighting");
	if (it != option.end()) scheme = it->second;
    }
    set_weighting_scheme(*enquire, scheme, force_boolean);

    enquire->set_cutoff(threshold);

    if (sort_keymaker) {
	if (sort_after) {
	    enquire->set_sort_by_relevance_then_key(sort_keymaker,
						    reverse_sort);
	} else {
	    enquire->set_sort_by_key_then_relevance(sort_keymaker,
						    reverse_sort);
	}
    } else if (sort_key != Xapian::BAD_VALUENO) {
	if (sort_after) {
	    enquire->set_sort_by_relevance_then_value(sort_key, reverse_sort);
	} else {
	    enquire->set_sort_by_value_then_relevance(sort_key, reverse_sort);
	}
    }

    enquire->set_docid_order(docid_order);

    if (collapse) {
	enquire->set_collapse_key(collapse_key);
    }

    if (!query.empty()) {
#if 0
	// FIXME: If we start doing permissions checks based on $REMOTE_USER
	// we're going to break some existing setups if users upgrade.  We
	// probably want a way to set this from OmegaScript.
	const char * remote_user = getenv("REMOTE_USER");
	if (remote_user)
	    apply_unix_permissions(query, remote_user);
#endif

	enquire->set_query(query);
	// We could use the value of topdoc as first parameter, but we
	// need to know the first few items in the mset to fake a
	// relevance set for topterms.
	//
	// If min_hits isn't set, check at least one extra result so we
	// know if we've reached the end of the matches or not - then we
	// can avoid offering a "next" button which leads to an empty page.
	mset = enquire->get_mset(0, topdoc + hits_per_page,
				 topdoc + max(hits_per_page + 1, min_hits),
				 &rset);
    }
}

string
html_escape(const string &str)
{
    string res;
    string::size_type p = 0;
    while (p < str.size()) {
	char ch = str[p++];
	switch (ch) {
	    case '<':
		res += "&lt;";
		continue;
	    case '>':
		res += "&gt;";
		continue;
	    case '&':
		res += "&amp;";
		continue;
	    case '"':
		res += "&quot;";
		continue;
	    default:
		res += ch;
	}
    }
    return res;
}

static string
html_strip(const string &str)
{
    string res;
    string::size_type p = 0;
    bool skip = false;
    while (p < str.size()) {
	char ch = str[p++];
	switch (ch) {
	    case '<':
		skip = true;
		continue;
	    case '>':
		skip = false;
		continue;
	    default:
		if (!skip) res += ch;
	}
    }
    return res;
}

class WordList {
    static string prev_list;
    static unordered_map<string, int> word_to_occurrence;
  public:
    void build_word_map(const string& list) {
	// Don't build map again if passed list of terms is same as before.
	if (prev_list == list) return;
	word_to_occurrence.clear();
	string::size_type split = 0, split2;
	int word_index = 0;
	string word;
	while ((split2 = list.find('\t', split)) != string::npos) {
	    word = list.substr(split, split2 - split);
	    if (word_to_occurrence.emplace(make_pair(word, word_index)).second)
		++word_index;
	    split = split2 + 1;
	}
	word = list.substr(split, list.size() - split);
	if (word_to_occurrence.emplace(make_pair(word, word_index)).second)
	    ++word_index;
	prev_list = list;
    }

    int word_in_list(const string& word) {
	auto it = word_to_occurrence.find(word);
	if (it == word_to_occurrence.end()) return -1;
	return it->second;
    }
};

string WordList::prev_list;
unordered_map<string, int> WordList::word_to_occurrence;

// Not a character in an identifier
static inline bool
p_notid(unsigned int c)
{
    return !C_isalnum(c) && c != '_';
}

// Not a character in an HTML tag name
static inline bool
p_nottag(unsigned int c)
{
    return !C_isalnum(c) && c != '.' && c != '-';
}

// FIXME: shares algorithm with indextext.cc!
static string
html_highlight(const string &s, const string &list,
	       const string &bra, const string &ket)
{
    if (!stemmer) {
	stemmer = new Xapian::Stem(option["stemmer"]);
    }

    string res;

    Utf8Iterator j(s);
    const Utf8Iterator s_end;
    while (true) {
	Utf8Iterator first = j;
	while (first != s_end && !is_wordchar(*first)) ++first;
	if (first == s_end) break;
	Utf8Iterator term_end;
	string term;
	string word;
	const char *l = j.raw();
	if (*first < 128 && C_isupper(*first)) {
	    j = first;
	    Xapian::Unicode::append_utf8(term, *j);
	    while (++j != s_end && *j == '.' && ++j != s_end && *j < 128 && C_isupper(*j)) {
		Xapian::Unicode::append_utf8(term, *j);
	    }
	    if (term.length() < 2 || (j != s_end && is_wordchar(*j))) {
		term.resize(0);
	    }
	    term_end = j;
	}
	if (term.empty()) {
	    j = first;
	    while (is_wordchar(*j)) {
		Xapian::Unicode::append_utf8(term, *j);
		++j;
		if (j == s_end) break;
		if (*j == '&' || *j == '\'') {
		    Utf8Iterator next = j;
		    ++next;
		    if (next == s_end || !is_wordchar(*next)) break;
		    term += *j;
		    j = next;
		}
	    }
	    term_end = j;
	    if (j != s_end && (*j == '+' || *j == '-' || *j == '#')) {
		string::size_type len = term.length();
		if (*j == '#') {
		    term += '#';
		    do { ++j; } while (j != s_end && *j == '#');
		} else {
		    while (j != s_end && (*j == '+' || *j == '-')) {
			Xapian::Unicode::append_utf8(term, *j);
			++j;
		    }
		}
		if (term.size() - len > 3 || (j != s_end && is_wordchar(*j))) {
		    term.resize(len);
		} else {
		    term_end = j;
		}
	    }
	}
	j = term_end;
	term = Xapian::Unicode::tolower(term);
	WordList w;
	w.build_word_map(list);
	int match = w.word_in_list(term);
	if (match == -1) {
	    string stem = "Z";
	    stem += (*stemmer)(term);
	    match = w.word_in_list(stem);
	}
	if (match >= 0) {
	    res += html_escape(string(l, first.raw() - l));
	    if (!bra.empty()) {
		res += bra;
	    } else {
		static const char * colours[] = {
		    "ffff66", "99ff99", "99ffff", "ff66ff", "ff9999",
		    "990000", "009900", "996600", "006699", "990099"
		};
		size_t idx = match % (sizeof(colours) / sizeof(colours[0]));
		const char * bg = colours[idx];
		if (strchr(bg, 'f')) {
		    res += "<b style=\"color:black;background-color:#";
		} else {
		    res += "<b style=\"color:white;background-color:#";
		}
		res += bg;
		res += "\">";
	    }
	    word.assign(first.raw(), j.raw() - first.raw());
	    res += html_escape(word);
	    if (!bra.empty()) {
		res += ket;
	    } else {
		res += "</b>";
	    }
	} else {
	    res += html_escape(string(l, j.raw() - l));
	}
    }
    if (j != s_end) res += html_escape(string(j.raw(), j.left()));
    return res;
}

#if 0
static void
print_query_string(const char *after)
{
    if (after && strncmp(after, "&B=", 3) == 0) {
	char prefix = after[3];
	string::size_type start = 0, amp = 0;
	while (true) {
	    amp = url_query_string.find('&', amp);
	    if (amp == string::npos) {
		cout << url_query_string.substr(start);
		return;
	    }
	    amp++;
	    while (url_query_string[amp] == 'B' &&
		   url_query_string[amp + 1] == '=' &&
		   url_query_string[amp + 2] == prefix) {
		cout << url_query_string.substr(start, amp - start - 1);
		start = url_query_string.find('&', amp + 3);
		if (start == string::npos) return;
		amp = start + 1;
	    }
	}
    }
    cout << url_query_string;
}
#endif

class CachedFields : private Fields {
    Xapian::docid did_cached = 0;

  public:
    CachedFields() {}

    const string& get_field(Xapian::docid did, const string& name) {
	if (did != did_cached) {
	    did_cached = did;
	    auto it = option.find("fieldnames");
	    Fields::parse_fields(db.get_document(did).get_data(),
				 it == option.end() ? nullptr : &it->second);
	}
	return Fields::get_field(name);
    }
};

static CachedFields fields;
static Xapian::docid q0;
static Xapian::doccount hit_no;
static int percent;
static double weight;
static Xapian::doccount collapsed;

static string print_caption(const string& fmt, vector<string>& param);

enum tagval {
CMD_,
CMD_add,
CMD_addfilter,
CMD_allterms,
CMD_and,
CMD_base64,
CMD_cgi,
CMD_cgilist,
CMD_cgiparams,
CMD_chr,
CMD_collapsed,
CMD_cond,
CMD_contains,
CMD_csv,
CMD_date,
CMD_dbname,
CMD_dbsize,
CMD_def,
CMD_defaultop,
CMD_div,
CMD_emptydocs,
CMD_env,
CMD_eq,
CMD_error,
CMD_field,
CMD_filesize,
CMD_filters,
CMD_filterterms,
CMD_find,
CMD_fmt,
CMD_foreach,
CMD_freq,
CMD_ge,
CMD_gt,
CMD_hash,
CMD_highlight,
CMD_hit,
CMD_hitlist,
CMD_hitsperpage,
CMD_hostname,
CMD_html,
CMD_htmlstrip,
CMD_httpheader,
CMD_id,
CMD_if,
CMD_include,
CMD_json,
CMD_jsonarray,
CMD_jsonbool,
CMD_jsonobject,
CMD_keys,
CMD_last,
CMD_lastpage,
CMD_le,
CMD_length,
CMD_list,
CMD_log,
CMD_lookup,
CMD_lower,
CMD_lt,
CMD_map,
CMD_match,
CMD_max,
CMD_min,
CMD_mod,
CMD_msize,
CMD_msizeexact,
CMD_msizelower,
CMD_msizeupper,
CMD_mul,
CMD_muldiv,
CMD_ne,
CMD_nice,
CMD_not,
CMD_now,
CMD_opt,
CMD_or,
CMD_ord,
CMD_pack,
CMD_percentage,
CMD_prettyterm,
CMD_prettyurl,
CMD_query,
CMD_querydescription,
CMD_queryterms,
CMD_random,
CMD_range,
CMD_record,
CMD_relevant,
CMD_relevants,
CMD_score,
CMD_set,
CMD_seterror,
CMD_setmap,
CMD_setrelevant,
CMD_slice,
CMD_snippet,
CMD_sort,
CMD_split,
CMD_srandom,
CMD_stoplist,
CMD_sub,
CMD_subdb,
CMD_subid,
CMD_substr,
CMD_suggestion,
CMD_switch,
CMD_termprefix,
CMD_terms,
CMD_thispage,
CMD_time,
CMD_topdoc,
CMD_topterms,
CMD_transform,
CMD_truncate,
CMD_uniq,
CMD_unique,
CMD_unpack,
CMD_unprefix,
CMD_unstem,
CMD_upper,
CMD_url,
CMD_value,
CMD_version,
CMD_weight,
CMD_MACRO // special tag for macro evaluation
};

struct func_attrib {
    int tag;
    int minargs, maxargs, evalargs;
    char ensure;
};

#define T(F,A,B,C,D) {STRINGIZE(F),{CMD_##F,A,B,C,D}}
struct func_desc {
    const char *name;
    struct func_attrib a;
};

#define N (-1)
#define M 'M'
#define Q 'Q'
// NB when adding a new command which ensures M or Q, update the list in
// docs/omegascript.rst
static const struct func_desc func_tab[] = {
//name minargs maxargs evalargs ensure
{"",{CMD_,	   N, N, 0, 0}},// commented out code
T(add,		   0, N, N, 0), // add a list of numbers
T(addfilter,	   1, 2, N, 0), // add filter term
T(allterms,	   0, 1, N, 0), // list of all terms matching document
T(and,		   1, N, 0, 0), // logical shortcutting and of a list of values
T(base64,	   1, 1, N, 0), // base64 encode
T(cgi,		   1, 1, N, 0), // return cgi parameter value
T(cgilist,	   1, 1, N, 0), // return list of values for cgi parameter
T(cgiparams,	   0, 0, N, 0), // return list of cgi parameter names
T(chr,		   1, 1, N, 0), // return UTF-8 for given Unicode codepoint
T(collapsed,	   0, 0, N, 0), // return number of hits collapsed into this
T(cond,		   2, N, 0, 0), // cascaded conditionals
T(contains,	   2, 2, N, 0), // return position of substring, or empty string
T(csv,		   1, 2, N, 0), // CSV string escaping
T(date,		   1, 2, N, 0), // convert time_t to strftime format
				// (default: YYYY-MM-DD)
T(dbname,	   0, 0, N, 0), // database name
T(dbsize,	   0, 0, N, 0), // database size (# of documents)
T(def,		   2, 2, 1, 0), // define a macro
T(defaultop,	   0, 0, N, 0), // default operator: "and" or "or"
T(div,		   2, 2, N, 0), // integer divide
T(emptydocs,	   0, 1, N, 0), // list of empty documents
T(env,		   1, 1, N, 0), // environment variable
T(eq,		   2, 2, N, 0), // test equality
T(error,	   0, 0, N, 0), // error message
T(field,	   1, 2, N, 0), // lookup field in record
T(filesize,	   1, 1, N, 0), // pretty printed filesize
T(filters,	   0, 0, N, 0), // serialisation of current filters
T(filterterms,	   1, 1, N, 0), // list of terms with a given prefix
T(find,		   2, 2, N, 0), // find entry in list
T(fmt,		   0, 0, N, 0), // name of current format
T(foreach,	   2, 2, 1, 0), // evaluate something for every entry in a list
T(freq,		   1, 1, N, 0), // frequency of a term
T(ge,		   2, 2, N, 0), // test >=
T(gt,		   2, 2, N, 0), // test >
T(hash,		   2, 2, N, 0), // hash a string using the specified hash function
T(highlight,	   2, 4, N, 0), // html escape and highlight words from list
T(hit,		   0, 0, N, 0), // hit number of current mset entry (0-based)
T(hitlist,	   1, 1, 0, M), // display hitlist using format in argument
T(hitsperpage,	   0, 0, N, 0), // hits per page
T(hostname,	   1, 1, N, 0), // extract hostname from URL
T(html,		   1, 1, N, 0), // html escape string (<>&")
T(htmlstrip,	   1, 1, N, 0), // html strip tags string (s/<[^>]*>?//g)
T(httpheader,	   2, 2, N, 0), // arbitrary HTTP header
T(id,		   0, 0, N, 0), // docid of current doc
T(if,		   1, 3, 1, 0), // conditional
T(include,	   1, 2, 1, 0), // include another file
T(json,		   1, 1, N, 0), // JSON string escaping
T(jsonarray,	   1, 2, 1, 0), // Format list as a JSON array
T(jsonbool,	   1, 1, 1, 0), // Format list as a JSON bool
T(jsonobject,	   1, 3, 1, 0), // Format map as JSON object
T(keys,		   1, 1, N, 0), // list of keys from a map
T(last,		   0, 0, N, M), // hit number one beyond end of current page
T(lastpage,	   0, 0, N, M), // number of last hit page
T(le,		   2, 2, N, 0), // test <=
T(length,	   1, 1, N, 0), // length of list
T(list,		   2, 5, N, 0), // pretty print list
T(log,		   1, 2, 1, 0), // create a log entry
T(lookup,	   2, 2, N, 0), // lookup in named cdb file
T(lower,	   1, 1, N, 0), // convert string to lower case
T(lt,		   2, 2, N, 0), // test <
T(map,		   2, 2, 1, 0), // map a list into another list
T(match,	   2, 3, N, 0), // regex match
T(max,		   1, N, N, 0), // maximum of a list of values
T(min,		   1, N, N, 0), // minimum of a list of values
T(mod,		   2, 2, N, 0), // integer modulus
T(msize,	   0, 0, N, M), // number of matches (estimated)
T(msizeexact,	   0, 0, N, M), // is $msize exact?
T(msizelower,	   0, 0, N, M), // number of matches (lower bound)
T(msizeupper,	   0, 0, N, M), // number of matches (upper bound)
T(mul,		   2, N, N, 0), // multiply a list of numbers
T(muldiv,	   3, 3, N, 0), // calculate A*B/C
T(ne,		   2, 2, N, 0), // test not equal
T(nice,		   1, 1, N, 0), // pretty print integer (with thousands sep)
T(not,		   1, 1, N, 0), // logical not
T(now,		   0, 0, N, 0), // current date/time as a time_t
T(opt,		   1, 2, N, 0), // lookup an option value
T(or,		   1, N, 0, 0), // logical shortcutting or of a list of values
T(ord,		   1, 1, N, 0), // return codepoint for first character of UTF-8 string
T(pack,		   1, 1, N, 0), // convert a number to a 4 byte big endian binary string
T(percentage,	   0, 0, N, 0), // percentage score of current hit
T(prettyterm,	   1, 1, N, Q), // pretty print term name
T(prettyurl,	   1, 1, N, 0), // pretty version of URL
T(query,	   0, 1, N, Q), // query
T(querydescription,0, 0, N, M), // query.get_description() (run_query() adds filters so M)
T(queryterms,	   0, 0, N, Q), // list of query terms
T(random,	   1, 1, N, 0), // return a random number
T(range,	   2, 2, N, 0), // return list of values between start and end
T(record,	   0, 1, N, 0), // record contents of document
T(relevant,	   0, 1, N, Q), // is document relevant?
T(relevants,	   0, 0, N, Q), // return list of relevant documents
T(score,	   0, 0, N, 0), // score (0-10) of current hit
T(set,		   2, 2, N, 0), // set option value
T(seterror,	   1, 1, N, 0), // set error_msg, setting it early stops query execution
T(setmap,	   1, N, N, 0), // set map of option values
T(setrelevant,	   1, 1, N, Q), // set rset
T(slice,	   2, 2, N, 0), // slice a list using a second list
T(snippet,	   1, 6, N, M), // generate snippet from text
T(sort,		   1, 2, N, 0), // alpha sort a list
T(split,	   1, 2, N, 0), // split a string to give a list
T(srandom,	   1, 1, N, 0), // seed for random number
T(stoplist,	   0, 0, N, Q), // return list of stopped terms
T(sub,		   2, 2, N, 0), // subtract
T(subdb,	   0, 1, N, 0), // name of subdb docid is in
T(subid,	   0, 1, N, 0), // docid in the subdb#
T(substr,	   2, 3, N, 0), // substring
T(suggestion,	   0, 0, N, Q), // misspelled word correction suggestion
T(switch,	   3, N, 1, 0), // return position of substring, or empty string
T(termprefix,	   1, 1, N, 0), // get any prefix from a term
T(terms,	   0, 1, N, M), // list of matching terms
T(thispage,	   0, 0, N, M), // page number of current page
T(time,		   0, 0, N, M), // how long the match took (in seconds)
T(topdoc,	   0, 0, N, M), // first document on current page of hit list
				// (counting from 0)
T(topterms,	   0, 1, N, M), // list of up to N top relevance feedback terms
				// (default 16)
T(transform,	   3, 4, N, 0), // transform with a regexp
T(truncate,	   2, 4, N, 0), // truncate after a word
T(uniq,		   1, 1, N, 0), // removed duplicates from a sorted list
T(unique,	   1, 1, N, 0), // removed duplicates from any list
T(unpack,	   1, 1, N, 0), // convert 4 byte big endian binary string to a number
T(unprefix,	   1, 1, N, 0), // remove any prefix from a term
T(unstem,	   1, 1, N, Q), // return list of terms from the parsed query
				// which stemmed to this term
T(upper,	   1, 1, N, 0), // convert string to upper case
T(url,		   1, 1, N, 0), // url encode argument
T(value,	   1, 2, N, 0), // return document value
T(version,	   0, 0, N, 0), // omega version string
T(weight,	   0, 0, N, 0), // weight of the current hit
{ NULL,{0,	   0, 0, 0, 0}}
};

#undef T // Leaving T defined screws up Sun's C++ compiler!

static vector<string> macros;

// Call write() repeatedly until all data is written or we get a
// non-recoverable error.
static ssize_t
write_all(int fd, const char * buf, size_t count)
{
    while (count) {
	ssize_t r = write(fd, buf, count);
	if (rare(r < 0)) {
	    if (errno == EINTR) continue;
	    return r;
	}
	buf += r;
	count -= r;
    }
    return 0;
}

// mersenne twister for RNG
static mt19937 rng;
static bool seed_set = false;

static string eval(const string& fmt, vector<string>& param);

/** Implements $foreach{} and $map{}. */
static string
foreach(const string& list,
	const string& pat,
	vector<string>& param,
	char sep = '\0')
{
    string result;
    string saved_arg0 = std::move(param[0]);
    string::size_type i = 0, j;
    while (true) {
	j = list.find('\t', i);
	param[0].assign(list, i, j - i);
	result += eval(pat, param);
	if (j == string::npos) break;
	if (sep) result += sep;
	i = j + 1;
    }
    param[0] = std::move(saved_arg0);
    return result;
}

static string
eval(const string& fmt, vector<string>& param)
{
    static map<string, const struct func_attrib *> func_map;
    if (func_map.empty()) {
	for (auto p = func_tab; p->name != NULL; ++p) {
	    func_map[string(p->name)] = &(p->a);
	}
    }
    string res;
    string::size_type p = 0, q;
    while ((q = fmt.find('$', p)) != string::npos) try {
	res.append(fmt, p, q - p);
	string::size_type code_start = q; // note down for error reporting
	q++;
	if (q >= fmt.size()) break;
	unsigned char ch = fmt[q];
	switch (ch) {
	    // Magic sequences:
	    // '$$' -> '$', '$(' -> '{', '$)' -> '}', '$.' -> ','
	    case '$':
		res += '$';
		p = q + 1;
		continue;
	    case '(':
		res += '{';
		p = q + 1;
		continue;
	    case ')':
		res += '}';
		p = q + 1;
		continue;
	    case '.':
		res += ',';
		p = q + 1;
		continue;
	    case '_':
		ch = '0';
		// FALL THRU
	    case '1': case '2': case '3': case '4': case '5':
	    case '6': case '7': case '8': case '9':
		ch -= '0';
		if (ch < param.size()) res += param[ch];
		p = q + 1;
		continue;
	    case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
	    case 'g': case 'h': case 'i': case 'j': case 'k': case 'l':
	    case 'm': case 'n': case 'o': case 'p': case 'q': case 'r':
	    case 's': case 't': case 'u': case 'v': case 'w': case 'x':
	    case 'y': case 'z':
	    case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
	    case 'G': case 'H': case 'I': case 'J': case 'K': case 'L':
	    case 'M': case 'N': case 'O': case 'P': case 'Q': case 'R':
	    case 'S': case 'T': case 'U': case 'V': case 'W': case 'X':
	    case 'Y': case 'Z':
	    case '{':
		break;
	    default:
		string msg = "Unknown $ code in: $";
		msg.append(fmt, q, string::npos);
		throw msg;
	}
	p = find_if(fmt.begin() + q, fmt.end(), p_notid) - fmt.begin();
	string var(fmt, q, p - q);
	map<string, const struct func_attrib *>::const_iterator func;
	func = func_map.find(var);
	if (func == func_map.end()) {
	    throw "Unknown function '" + var + "'";
	}
	vector<string> args;
	if (fmt[p] == '{') {
	    q = p + 1;
	    int nest = 1;
	    while (true) {
		p = fmt.find_first_of(",{}", p + 1);
		if (p == string::npos)
		    throw "missing } in " + fmt.substr(code_start);
		if (fmt[p] == '{') {
		    ++nest;
		} else {
		    if (nest == 1) {
			// should we split the args
			if (func->second->minargs != N) {
			    args.push_back(fmt.substr(q, p - q));
			    q = p + 1;
			}
		    }
		    if (fmt[p] == '}' && --nest == 0) break;
		}
	    }
	    if (func->second->minargs == N)
		args.push_back(fmt.substr(q, p - q));
	    ++p;
	}

	if (func->second->minargs != N) {
	    if (int(args.size()) < func->second->minargs)
		throw "too few arguments to $" + var;
	    if (func->second->maxargs != N &&
		int(args.size()) > func->second->maxargs)
		throw "too many arguments to $" + var;

	    vector<string>::size_type n;
	    if (func->second->evalargs != N)
		n = func->second->evalargs;
	    else
		n = args.size();

	    for (vector<string>::size_type j = 0; j < n; ++j)
		args[j] = eval(args[j], param);
	}
	if (func->second->ensure == 'Q' || func->second->ensure == 'M')
	    ensure_query_parsed();
	if (func->second->ensure == 'M') ensure_match();
	string value;
	switch (func->second->tag) {
	    case CMD_:
		break;
	    case CMD_add: {
		int total = 0;
		for (auto&& arg : args)
		    total += string_to_int(arg);
		value = str(total);
		break;
	    }
	    case CMD_addfilter:
		if (args.size() == 1 || args[1].empty() || args[1] == "B") {
		    add_bterm(args[0]);
		} else if (args[1] == "N") {
		    add_nterm(args[0]);
		} else {
		    string msg = "Invalid $addfilter type '";
		    msg += args[1];
		    msg += "'";
		    throw msg;
		}
		break;
	    case CMD_allterms: {
		// list of all terms indexing document
		Xapian::docid id = q0;
		if (!args.empty() &&
		    (!parse_unsigned(args[0].c_str(), id) || id == 0)) {
		    throw "Document id for command allterms should be > 0";
		}
		for (Xapian::TermIterator term = db.termlist_begin(id);
		     term != db.termlist_end(id); ++term) {
		    value += *term;
		    value += '\t';
		}

		if (!value.empty()) value.erase(value.size() - 1);
		break;
	    }
	    case CMD_and: {
		value = "true";
		for (auto&& arg : args) {
		    if (eval(arg, param).empty()) {
			value.resize(0);
			break;
		    }
		}
		break;
	    }
	    case CMD_base64: {
		const static char encode[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdef"
					     "ghijklmnopqrstuvwxyz0123456789+/";
		const char pad = '=';
		const string& input = args[0];
		value.reserve((input.size() + 2) / 3 * 4);
		auto it = input.begin();
		auto n = input.size() / 3;
		while (n--) {
		    uint32_t v = uint8_t(*it++);
		    v = (v << 8) | uint8_t(*it++);
		    v = (v << 8) | uint8_t(*it++);
		    value += encode[v >> 18];
		    value += encode[(v >> 12) & 63];
		    value += encode[(v >> 6) & 63];
		    value += encode[v & 63];
		}
		switch (input.size() % 3) {
		    case 2: {
			uint32_t v = uint8_t(*it++);
			v = (v << 8) | uint8_t(*it++);
			value += encode[v >> 10];
			value += encode[(v >> 4) & 63];
			value += encode[(v << 2) & 63];
			value += pad;
			break;
		    }
		    case 1: {
			uint32_t v = uint8_t(*it++);
			value += encode[v >> 2];
			value += encode[(v << 4) & 63];
			value += pad;
			value += pad;
			break;
		    }
		}
		break;
	    }
	    case CMD_cgi: {
		auto i = cgi_params.find(args[0]);
		if (i != cgi_params.end()) value = i->second;
		break;
	    }
	    case CMD_cgilist: {
		auto g = cgi_params.equal_range(args[0]);
		for (auto i = g.first; i != g.second; ++i) {
		    value += i->second;
		    value += '\t';
		}
		if (!value.empty()) value.erase(value.size() - 1);
		break;
	    }
	    case CMD_cgiparams: {
		const string* prev = NULL;
		for (auto&& i : cgi_params) {
		    if (prev && i.first == *prev) continue;
		    value += i.first;
		    value += '\t';
		    prev = &i.first;
		}
		if (!value.empty()) value.erase(value.size() - 1);
		break;
	    }
	    case CMD_chr: {
		unsigned int codepoint;
		if (!parse_unsigned(args[0].c_str(), codepoint)) {
		    throw "Unicode codepoint for command chr should be >= 0";
		}
		Xapian::Unicode::append_utf8(value, codepoint);
		break;
	    }
	    case CMD_collapsed: {
		value = str(collapsed);
		break;
	    }
	    case CMD_cond:
		for (size_t i = 0; i < args.size(); i += 2) {
		    if (i == args.size() - 1) {
			// Handle optional "else" value.
			value = eval(args[i], param);
			break;
		    }
		    if (!eval(args[i], param).empty()) {
			value = eval(args[i + 1], param);
			break;
		    }
		}
		break;
	    case CMD_contains: {
		size_t pos = args[1].find(args[0]);
		if (pos != string::npos) {
		    value = str(pos);
		}
		break;
	    }
	    case CMD_csv:
		value = args[0];
		if (args.size() > 1 && !args[1].empty()) {
		    csv_escape_always(value);
		} else {
		    csv_escape(value);
		}
		break;
	    case CMD_date:
		value = args[0];
		if (!value.empty()) {
		    char buf[64] = "";
		    time_t date;
		    if (!parse_signed(value.c_str(), date)) {
			throw "Date (in secs) for command date should "
			      "be an integer";
		    }
		    if (date != static_cast<time_t>(-1)) {
			struct tm *then;
			then = gmtime(&date);
			string date_fmt = "%Y-%m-%d";
			if (args.size() > 1) date_fmt = eval(args[1], param);
			strftime(buf, sizeof buf, date_fmt.c_str(), then);
		    }
		    value = buf;
		}
		break;
	    case CMD_dbname:
		value = dbname;
		break;
	    case CMD_dbsize: {
		static Xapian::doccount dbsize;
		if (!dbsize) dbsize = db.get_doccount();
		value = str(dbsize);
		break;
	    }
	    case CMD_def: {
		func_attrib *fa = new func_attrib;
		fa->tag = CMD_MACRO + macros.size();
		fa->minargs = 0;
		fa->maxargs = 9;
		fa->evalargs = N; // FIXME: or 0?
		fa->ensure = 0;

		macros.push_back(args[1]);
		func_map[args[0]] = fa;
		break;
	    }
	    case CMD_defaultop:
		if (default_op == Xapian::Query::OP_AND) {
		    value = "and";
		} else {
		    value = "or";
		}
		break;
	    case CMD_div: {
		int denom = string_to_int(args[1]);
		if (denom == 0) {
		    value = "divide by 0";
		} else {
		    value = str(string_to_int(args[0]) / denom);
		}
		break;
	    }
	    case CMD_emptydocs: {
		string t;
		if (!args.empty())
		    t = args[0];
		Xapian::PostingIterator i;
		for (i = db.postlist_begin(t); i != db.postlist_end(t); ++i) {
		    if (i.get_doclength() != 0) continue;
		    if (!value.empty()) value += '\t';
		    value += str(*i);
		}
		break;
	    }
	    case CMD_env: {
		char *env = getenv(args[0].c_str());
		if (env != NULL) value = env;
		break;
	    }
	    case CMD_eq:
		if (args[0] == args[1]) value = "true";
		break;
	    case CMD_error:
		if (error_msg.empty() && enquire == NULL && !dbname.empty()) {
		    error_msg = "Database '" + dbname + "' couldn't be opened";
		}
		value = error_msg;
		break;
	    case CMD_field: {
		Xapian::docid did = q0;
		if (args.size() > 1 &&
		    (!parse_unsigned(args[1].c_str(), did) || did == 0)) {
		    throw "Document id for command field should be > 0";
		}
		value = fields.get_field(did, args[0]);
		break;
	    }
	    case CMD_filesize: {
		if (args[0].empty()) break;
		// FIXME: rounding?  i18n?
		int size;
		if (!parse_signed(args[0].c_str(), size)) {
		    throw "Filesize must be an integer";
		}
		int intpart = size;
		int fraction = -1;
		const char * format = 0;
		if (size < 0) {
		    // Negative size -> empty result.
		} else if (size == 1) {
		    format = "%d byte";
		} else if (size < 1024) {
		    format = "%d bytes";
		} else {
		    if (size < 1024 * 1024) {
			format = "%d.%cK";
		    } else {
			size /= 1024;
			if (size < 1024 * 1024) {
			    format = "%d.%cM";
			} else {
			    size /= 1024;
			    format = "%d.%cG";
			}
		    }
		    intpart = unsigned(size) / 1024;
		    fraction = unsigned(size) % 1024;
		}
		if (format) {
		    char buf[200];
		    int len;
		    if (fraction == -1) {
			len = my_snprintf(buf, sizeof(buf), format, intpart);
		    } else {
			fraction = (fraction * 10 / 1024) + '0';
			len = my_snprintf(buf, sizeof(buf), format, intpart, fraction);
		    }
		    if (len < 0 || unsigned(len) > sizeof(buf)) len = sizeof(buf);
		    value.assign(buf, len);
		}
		break;
	    }
	    case CMD_filters:
		value = filters;
		break;
	    case CMD_filterterms: {
		Xapian::TermIterator term = db.allterms_begin();
		term.skip_to(args[0]);
		while (term != db.allterms_end()) {
		    string t = *term;
		    if (!startswith(t, args[0])) break;
		    value += t;
		    value += '\t';
		    ++term;
		}

		if (!value.empty()) value.erase(value.size() - 1);
		break;
	    }
	    case CMD_find: {
		string l = args[0], s = args[1];
		string::size_type i = 0, j = 0;
		size_t count = 0;
		while (j != l.size()) {
		    j = l.find('\t', i);
		    if (j == string::npos) j = l.size();
		    if (j - i == s.length()) {
			if (memcmp(s.data(), l.data() + i, j - i) == 0) {
			    value = str(count);
			    break;
			}
		    }
		    ++count;
		    i = j + 1;
		}
		break;
	    }
	    case CMD_fmt:
		value = fmtname;
		break;
	    case CMD_foreach:
		if (!args[0].empty()) {
		    value = foreach(args[0], args[1], param);
		}
		break;
	    case CMD_freq: {
		const string& term = args[0];
		Xapian::doccount termfreq = 0;
		if (done_query) {
		    termfreq = mset.get_termfreq(term);
		}
		if (termfreq == 0) {
		    // We want $freq to work before the match is run, and we
		    // don't want using it to force the match to run.
		    termfreq = db.get_termfreq(term);
		}
		value = str(termfreq);
		break;
	    }
	    case CMD_ge:
		if (string_to_int(args[0]) >= string_to_int(args[1]))
		    value = "true";
		break;
	    case CMD_gt:
		if (string_to_int(args[0]) > string_to_int(args[1]))
		    value = "true";
		break;
	    case CMD_hash: {
		const string& data = args[0];
		const string& hash = args[1];
		if (hash == "md5") {
		    string md5;
		    md5_string(data, md5);
		    value.reserve(md5.size() * 2);
		    for (unsigned char byte : md5) {
			value += "0123456789abcdef"[byte >> 4];
			value += "0123456789abcdef"[byte & 0x0f];
		    }
		} else {
		    throw "Unknown hash function: " + hash;
		}
		break;
	    }
	    case CMD_highlight: {
		string bra, ket;
		if (args.size() > 2) {
		    bra = args[2];
		    if (args.size() > 3) {
			ket = args[3];
		    } else {
			string::const_iterator i;
			i = find_if(bra.begin() + 2, bra.end(), p_nottag);
			ket = "</";
			ket.append(bra, 1, i - bra.begin() - 1);
			ket += '>';
		    }
		}

		value = html_highlight(args[0], args[1], bra, ket);
		break;
	    }
	    case CMD_hit:
		// 0-based mset index
		value = str(hit_no);
		break;
	    case CMD_hitlist: {
#if 0
		url_query_string = "?DB=";
		url_query_string += dbname;
		for (auto& j : query_strings) {
		    if (j.first.empty()) {
			url_query_string += "&P=";
		    } else {
			url_query_string += "&P."
			url_query_string += j.first;
			url_query_string += '=';
		    }
		    const char *q = j.second.c_str();
		    int ch;
		    while ((ch = *q++) != '\0') {
			switch (ch) {
			  case '+':
			    url_query_string += "%2b";
			    break;
			  case '"':
			    url_query_string += "%22";
			    break;
			  case '%':
			    url_query_string += "%25";
			    break;
			  case '&':
			    url_query_string += "%26";
			    break;
			  case ' ':
			    ch = '+';
			    /* fall through */
			  default:
			    url_query_string += ch;
			}
		    }
		}
		// add any boolean terms
		for (auto i = filter_map.begin(); i != filter_map.end(); ++i) {
		    url_query_string += "&B=";
		    url_query_string += i->second;
		}
#endif
		auto save_hit_no = hit_no;
		for (hit_no = topdoc; hit_no < last; ++hit_no)
		    value += print_caption(args[0], param);
		hit_no = save_hit_no;
		break;
	    }
	    case CMD_hitsperpage:
		value = str(hits_per_page);
		break;
	    case CMD_hostname: {
		value = args[0];
		// remove URL scheme and/or path
		string::size_type i = value.find("://");
		if (i == string::npos) i = 0; else i += 3;
		value = value.substr(i, value.find('/', i) - i);
		// remove user@ or user:password@
		i = value.find('@');
		if (i != string::npos) value.erase(0, i + 1);
		// remove :port
		i = value.find(':');
		if (i != string::npos) value.resize(i);
		break;
	    }
	    case CMD_html:
		value = html_escape(args[0]);
		break;
	    case CMD_htmlstrip:
		value = html_strip(args[0]);
		break;
	    case CMD_httpheader:
		if (!suppress_http_headers) {
		    cout << args[0] << ": " << args[1] << endl;
		    if (!set_content_type && args[0].length() == 12 &&
			    strcasecmp(args[0].c_str(), "Content-Type") == 0) {
			set_content_type = true;
		    }
		}
		break;
	    case CMD_id:
		// document id
		value = str(q0);
		break;
	    case CMD_if:
		if (args.size() > 1 && !args[0].empty())
		    value = eval(args[1], param);
		else if (args.size() > 2)
		    value = eval(args[2], param);
		break;
	    case CMD_include: {
		if (args.size() == 1) {
		    value = eval_file(args[0]);
		} else {
		    bool fallback = false;
		    value = eval_file(args[0], &fallback);
		    if (fallback) {
			value = eval(args[1], param);
		    }
		}
		break;
	    }
	    case CMD_json:
		value = args[0];
		json_escape(value);
		break;
	    case CMD_jsonarray: {
		const string & l = args[0];
		string::size_type i = 0, j;
		if (l.empty()) {
		    value = "[]";
		    break;
		}
		vector<string> new_args(1);
		value = "[";
		while (true) {
		    j = l.find('\t', i);
		    string elt(l, i, j - i);
		    if (args.size() == 1) {
			value += '"';
			json_escape(elt);
			value += elt;
			value += '"';
		    } else {
			new_args[0] = std::move(elt);
			value += eval(args[1], new_args);
		    }
		    if (j == string::npos) break;
		    value += ',';
		    i = j + 1;
		}
		value += ']';
		break;
	    }
	    case CMD_jsonbool:
		value = args[0].empty() ? "false" : "true";
		break;
	    case CMD_jsonobject: {
		vector<string> new_args;
		new_args.push_back(string());

		class map_range {
		    typedef map<string, string>::const_iterator iterator;
		    iterator b, e;

		  public:
		    map_range(iterator b_, iterator e_) : b(b_), e(e_) {}

		    iterator begin() const { return b; }
		    iterator end() const { return e; }
		};

		string prefix = args[0] + ',';
		auto b = option.lower_bound(prefix);
		++prefix.back();
		auto e = option.lower_bound(prefix);
		value = to_json(map_range(b, e),
				[&](const string& k) {
				    string key(k, prefix.size());
				    if (args.size() > 1 && !args[1].empty()) {
					new_args[0] = std::move(key);
					key = eval(args[1], new_args);
				    }
				    return key;
				},
				[&](const string& v) {
				    if (args.size() > 2 && !args[2].empty()) {
					new_args[0] = v;
					return eval(args[2], new_args);
				    }
				    string r(1, '"');
				    string elt = v;
				    json_escape(elt);
				    r += elt;
				    r += '"';
				    return r;
				});
		break;
	    }
	    case CMD_keys: {
		string prefix = args[0] + ',';
		auto i = option.lower_bound(prefix);
		for (; i != option.end() && startswith(i->first, prefix); ++i) {
		    const string& key = i->first;
		    if (!value.empty()) value += '\t';
		    value.append(key, prefix.size(), string::npos);
		}
		break;
	    }
	    case CMD_last:
		value = str(last);
		break;
	    case CMD_lastpage: {
		int l = mset.get_matches_estimated();
		if (l > 0) l = (l - 1) / hits_per_page + 1;
		value = str(l);
		break;
	    }
	    case CMD_le:
		if (string_to_int(args[0]) <= string_to_int(args[1]))
		    value = "true";
		break;
	    case CMD_length:
		if (args[0].empty()) {
		    value = "0";
		} else {
		    size_t length = count(args[0].begin(), args[0].end(), '\t');
		    value = str(length + 1);
		}
		break;
	    case CMD_list: {
		if (!args[0].empty()) {
		    string pre, inter, interlast, post;
		    switch (args.size()) {
		     case 2:
			inter = interlast = args[1];
			break;
		     case 3:
			inter = args[1];
			interlast = args[2];
			break;
		     case 4:
			pre = args[1];
			inter = interlast = args[2];
			post = args[3];
			break;
		     case 5:
			pre = args[1];
			inter = args[2];
			interlast = args[3];
			post = args[4];
			break;
		    }
		    value += pre;
		    string list = args[0];
		    string::size_type split = 0, split2;
		    while ((split2 = list.find('\t', split)) != string::npos) {
			if (split) value += inter;
			value.append(list, split, split2 - split);
			split = split2 + 1;
		    }
		    if (split) value += interlast;
		    value.append(list, split, string::npos);
		    value += post;
		}
		break;
	    }
	    case CMD_log: {
		if (!vet_filename(args[0])) {
		    value = "filename can't contain \"..\"";
		    break;
		}
		string logfile = log_dir + args[0];
		int fd = open(logfile.c_str(), O_CREAT|O_APPEND|O_WRONLY, 0644);
		if (fd == -1) {
		    value = "open failed: ";
		    value += strerror(errno);
		    break;
		}
		vector<string> noargs;
		noargs.resize(1);
		string line;
		if (args.size() > 1) {
		    line = args[1];
		} else {
		    line = DEFAULT_LOG_ENTRY;
		}
		line = eval(line, noargs);
		line += '\n';
		if (write_all(fd, line.data(), line.length()) < 0) {
		    value = "write failed: ";
		    value += strerror(errno);
		}
		close(fd);
		break;
	    }
	    case CMD_lookup: {
		if (!vet_filename(args[0])) break;
		string cdbfile = cdb_dir + args[0];
		int fd = open(cdbfile.c_str(), O_RDONLY);
		if (fd == -1) break;

		struct cdb cdb;
		if (cdb_init(&cdb, fd) < 0) {
		    close(fd);
		    break;
		}

		if (cdb_find(&cdb, args[1].data(), args[1].length()) > 0) {
		    size_t datalen = cdb_datalen(&cdb);
		    const void *dat = cdb_get(&cdb, datalen, cdb_datapos(&cdb));
		    if (dat) {
			value.assign(static_cast<const char *>(dat), datalen);
		    }
		}

		cdb_free(&cdb);
		close(fd); // FIXME: cache fds?
		break;
	    }
	    case CMD_lower:
		value = Xapian::Unicode::tolower(args[0]);
		break;
	    case CMD_lt:
		if (string_to_int(args[0]) < string_to_int(args[1]))
		    value = "true";
		break;
	    case CMD_map:
		if (!args[0].empty()) {
		    value = foreach(args[0], args[1], param, '\t');
		}
		break;
	    case CMD_match:
		omegascript_match(value, args);
		break;
	    case CMD_max: {
		vector<string>::const_iterator i = args.begin();
		int val = string_to_int(*i++);
		for (; i != args.end(); ++i) {
		    int x = string_to_int(*i);
		    if (x > val) val = x;
		}
		value = str(val);
		break;
	    }
	    case CMD_min: {
		vector<string>::const_iterator i = args.begin();
		int val = string_to_int(*i++);
		for (; i != args.end(); ++i) {
		    int x = string_to_int(*i);
		    if (x < val) val = x;
		}
		value = str(val);
		break;
	    }
	    case CMD_mod: {
		int denom = string_to_int(args[1]);
		if (denom == 0) {
		    value = "divide by 0";
		} else {
		    value = str(string_to_int(args[0]) % denom);
		}
		break;
	    }
	    case CMD_msize:
		// Estimated number of matches.
		value = str(mset.get_matches_estimated());
		break;
	    case CMD_msizeexact:
		// Is msize exact?
		if (mset.get_matches_lower_bound()
		    == mset.get_matches_upper_bound())
		    value = "true";
		break;
	    case CMD_msizelower:
		// Lower bound on number of matches.
		value = str(mset.get_matches_lower_bound());
		break;
	    case CMD_msizeupper:
		// Upper bound on number of matches.
		value = str(mset.get_matches_upper_bound());
		break;
	    case CMD_mul: {
		vector<string>::const_iterator i = args.begin();
		int total = string_to_int(*i++);
		while (i != args.end())
		    total *= string_to_int(*i++);
		value = str(total);
		break;
	    }
	    case CMD_muldiv: {
		int denom = string_to_int(args[2]);
		if (denom == 0) {
		    value = "divide by 0";
		} else {
		    int num = string_to_int(args[0]) * string_to_int(args[1]);
		    value = str(num / denom);
		}
		break;
	    }
	    case CMD_ne:
		if (args[0] != args[1]) value = "true";
		break;
	    case CMD_nice: {
		string::const_iterator i = args[0].begin();
		int len = args[0].length();
		while (len) {
		    value += *i++;
		    if (--len && len % 3 == 0) value += option["thousand"];
		}
		break;
	    }
	    case CMD_not:
		if (args[0].empty()) value = "true";
		break;
	    case CMD_now:
		value = str(static_cast<unsigned long>(time(NULL)));
		break;
	    case CMD_opt:
		if (args.size() == 2) {
		    value = option[args[0] + "," + args[1]];
		} else {
		    value = option[args[0]];
		}
		break;
	    case CMD_or: {
		for (auto&& arg : args) {
		    value = eval(arg, param);
		    if (!value.empty()) break;
		}
		break;
	    }
	    case CMD_ord: {
		if (!args[0].empty()) {
		    Utf8Iterator it(args[0]);
		    value = str(*it);
		}
		break;
	    }
	    case CMD_pack: {
		int number;
		if (!parse_signed(args[0].c_str(), number)) {
		    throw "NUMBER parameter for pack command "
			  "must be an integer";
		}
		value = int_to_binary_string(number);
		break;
	    }
	    case CMD_percentage:
		// percentage score
		value = str(percent);
		break;
	    case CMD_prettyterm:
		value = pretty_term(args[0]);
		break;
	    case CMD_prettyurl:
		value = args[0];
		url_prettify(value);
		break;
	    case CMD_query: {
		auto r = query_strings.equal_range(args.empty() ?
						   string() : args[0]);
		for (auto j = r.first; j != r.second; ++j) {
		    if (!value.empty()) value += '\t';
		    const string & s = j->second;
		    size_t start = 0, tab;
		    while ((tab = s.find('\t', start)) != string::npos) {
			value.append(s, start, tab - start);
			value += ' ';
			start = tab + 1;
		    }
		    value.append(s, start, string::npos);
		}
		break;
	    }
	    case CMD_querydescription:
		value = query.get_description();
		break;
	    case CMD_queryterms:
		value = queryterms;
		break;
	    case CMD_random: {
		if (!seed_set) {
		    random_device rd;
		    rng.seed(rd());
		    seed_set = true;
		}
		uniform_int_distribution<int>
		    distr(0, string_to_int(args[0]));
		value = str(distr(rng));
		break;
	    }
	    case CMD_range: {
		int start, end;
		if (!parse_signed(args[0].c_str(), start)) {
		    throw "Start value for range command "
			  "must be an integer";
		}
		if (!parse_signed(args[1].c_str(), end)) {
		    throw "End value for range command "
			  "must be an integer";
		}
		while (start <= end) {
		    value += str(start);
		    if (start < end) value += '\t';
		    start++;
		}
		break;
	    }
	    case CMD_record: {
		Xapian::docid id = q0;
		if (!args.empty() &&
		    (!parse_unsigned(args[0].c_str(), id) || id == 0)) {
		    throw "Document id for command record should be > 0";
		}
		value = db.get_document(id).get_data();
		break;
	    }
	    case CMD_relevant: {
		// document id if relevant; empty otherwise
		Xapian::docid id = q0;
		if (!args.empty() &&
		    (!parse_unsigned(args[0].c_str(), id) || id == 0)) {
		    throw "Document id for command relevant should be > 0";
		}
		auto i = ticked.find(id);
		if (i != ticked.end()) {
		    i->second = false; // icky side-effect
		    value = str(id);
		}
		break;
	    }
	    case CMD_relevants:	{
		for (auto i : ticked) {
		    if (i.second) {
			value += str(i.first);
			value += '\t';
		    }
		}
		if (!value.empty()) value.erase(value.size() - 1);
		break;
	    }
	    case CMD_score:
		// Score (0 to 10)
		value = str(percent / 10);
		break;
	    case CMD_set:
		option[args[0]] = args[1];
		break;
	    case CMD_seterror:
		error_msg = args[0];
		break;
	    case CMD_setmap: {
		string base = args[0] + ',';
		if (args.size() % 2 != 1)
		    throw string("$setmap requires an odd number of arguments");
		for (unsigned int i = 1; i + 1 < args.size(); i += 2) {
		    option[base + args[i]] = args[i + 1];
		}
		break;
	    }
	    case CMD_setrelevant: {
		string::size_type i = 0, j;
		while (true) {
		    j = args[0].find_first_not_of("0123456789", i);
		    Xapian::docid id = atoi(args[0].substr(i, j - i).c_str());
		    if (id) {
			rset.add_document(id);
			ticked[id] = true;
		    }
		    if (j == string::npos) break;
		    i = j + 1;
		}
		break;
	    }
	    case CMD_slice: {
		string list = args[0], pos = args[1];
		vector<string> items;
		string::size_type i = 0, j;
		while (true) {
		    j = list.find('\t', i);
		    items.push_back(list.substr(i, j - i));
		    if (j == string::npos) break;
		    i = j + 1;
		}
		i = 0;
		bool have_added = false;
		while (true) {
		    j = pos.find('\t', i);
		    int item = string_to_int(pos.substr(i, j - i));
		    if (item >= 0 && size_t(item) < items.size()) {
			if (have_added) value += '\t';
			value += items[item];
			have_added = true;
		    }
		    if (j == string::npos) break;
		    i = j + 1;
		}
		break;
	    }
	    case CMD_snippet: {
		size_t length = 200;
		if (args.size() > 1 && !args[1].empty()) {
		    if (!parse_unsigned(args[1].c_str(), length)) {
			throw "Snippet length must be >= 0";
		    }
		}
		unsigned flags = mset.SNIPPET_BACKGROUND_MODEL |
				 mset.SNIPPET_EXHAUSTIVE;
		if (args.size() > 2 && !args[2].empty()) {
		    flags = 0;
		    const string& s = args[2];
		    size_t i = 0;
		    while (true) {
			size_t j = s.find('|', i);
			string flag(s, i, j - i);
			for (char& c : flag) {
			    c = C_tolower(c);
			}
			if (startswith(flag, "snippet_")) {
			    flag.erase(0, CONST_STRLEN("snippet_"));
			}
			if (flag == "background_model") {
			    flags |= mset.SNIPPET_BACKGROUND_MODEL;
			} else if (flag == "cjk_ngram") {
			    flags |= mset.SNIPPET_CJK_NGRAM;
			} else if (flag == "cjk_words") {
			    flags |= mset.SNIPPET_CJK_WORDS;
			} else if (flag == "empty_without_match") {
			    flags |= mset.SNIPPET_EMPTY_WITHOUT_MATCH;
			} else if (flag == "exhaustive") {
			    flags |= mset.SNIPPET_EXHAUSTIVE;
			} else {
			    throw "Unknown $snippet flag '" + flag + "'";
			}
			if (j == string::npos) break;
			i = j + 1;
		    }
		}
		string bra, ket, gap;
		if (args.size() > 3) {
		    bra = args[3];
		} else {
		    bra = "<strong>";
		}
		if (args.size() > 4) {
		    ket = args[4];
		} else {
		    ket = "</strong>";
		}
		if (args.size() > 5) {
		    gap = args[5];
		} else {
		    gap = "...";
		}
		if (!stemmer)
		    stemmer = new Xapian::Stem(option["stemmer"]);
		value = mset.snippet(args[0], length, *stemmer, flags,
				     bra, ket, gap);
		break;
	    }
	    case CMD_sort:
		omegascript_sort(args, value);
		break;
	    case CMD_split: {
		string split;
		if (args.size() == 1) {
		    split = " ";
		    value = args[0];
		} else {
		    split = args[0];
		    value = args[1];
		}
		string::size_type i = 0;
		while (true) {
		    if (split.empty()) {
			++i;
			if (i >= value.size()) break;
		    } else {
			i = value.find(split, i);
			if (i == string::npos) break;
		    }
		    value.replace(i, split.size(), 1, '\t');
		    ++i;
		}
		break;
	    }
	    case CMD_srandom: {
		int seed = string_to_int(args[0]);
		rng.seed(seed);
		seed_set = true;
		break;
	    }
	    case CMD_stoplist: {
		Xapian::TermIterator i = qp.stoplist_begin();
		Xapian::TermIterator end = qp.stoplist_end();
		while (i != end) {
		    if (!value.empty()) value += '\t';
		    value += *i;
		    ++i;
		}
		break;
	    }
	    case CMD_sub:
		value = str(string_to_int(args[0]) - string_to_int(args[1]));
		break;
	    case CMD_subdb: {
		Xapian::docid id = q0;
		if (args.size() > 0 &&
		    (!parse_unsigned(args[0].c_str(), id) || id == 0)) {
		    throw "Document id of the subdb command should be > 0";
		}
		value = subdbs[(id - 1) % subdbs.size()].get_name();
		break;
	    }
	    case CMD_subid: {
		Xapian::docid id = q0;
		if (args.size() > 0 &&
		    (!parse_unsigned(args[0].c_str(), id) || id == 0)) {
		    throw "Document id of the subid command should be > 0";
		}
		// This is the docid in the single shard.
		Xapian::docid shard_did = (id - 1) / subdbs.size() + 1;
		// We now need to map this back to the docid in the collection
		// of shards specified by the DB parameter value which $subdb
		// returns.
		const SubDB& subdb = subdbs[(id - 1) % subdbs.size()];
		value = str(subdb.map_docid(shard_did));
		break;
	    }
	    case CMD_substr: {
		int start;
		if (!parse_signed(args[1].c_str(), start)) {
		    throw "Start value for substr command "
			  "must be an integer";
		}
		if (start < 0) {
		    if (static_cast<size_t>(-start) >= args[0].size()) {
			start = 0;
		    } else {
			start = static_cast<int>(args[0].size()) + start;
		    }
		} else {
		    if (static_cast<size_t>(start) >= args[0].size()) break;
		}
		size_t len = string::npos;
		if (args.size() > 2) {
		    int int_len;
		    if (!parse_signed(args[2].c_str(), int_len)) {
			throw "Length value for substr command "
			      "must be an integer";
		    }
		    if (int_len >= 0) {
			len = size_t(int_len);
		    } else {
			len = args[0].size() - start;
			if (static_cast<size_t>(-int_len) >= len) {
			    len = 0;
			} else {
			    len -= static_cast<size_t>(-int_len);
			}
		    }
		}
		value.assign(args[0], start, len);
		break;
	    }
	    case CMD_suggestion:
		value = qp.get_corrected_query_string();
		break;
	    case CMD_switch: {
		const string& val = args[0];
		for (size_t i = 1; i < args.size(); i += 2) {
		    if (i == args.size() - 1) {
			// Handle optional "else" value.
			value = eval(args[i], param);
			break;
		    }
		    if (val == eval(args[i], param)) {
			value = eval(args[i + 1], param);
			break;
		    }
		}
		break;
	    }
	    case CMD_termprefix:
		(void)prefix_from_term(&value, args[0]);
		break;
	    case CMD_terms: {
		// list of matching terms
		if (!enquire) break;
		Xapian::TermIterator term = enquire->get_matching_terms_begin(q0);
		if (args.empty()) {
		    while (term != enquire->get_matching_terms_end(q0)) {
			// check term was in the typed query so we ignore
			// boolean filter terms
			const string & t = *term;
			if (termset.find(t) != termset.end()) {
			    value += t;
			    value += '\t';
			}
			++term;
		    }
		} else {
		    // Return matching terms with specified prefix.  We can't
		    // use skip_to() as the terms aren't ordered by termname.
		    const string & pfx = args[0];
		    while (term != enquire->get_matching_terms_end(q0)) {
			const string & t = *term;
			if (startswith(t, pfx)) {
			    value += t;
			    value += '\t';
			}
			++term;
		    }
		}

		if (!value.empty()) value.erase(value.size() - 1);
		break;
	    }
	    case CMD_thispage:
		value = str(topdoc / hits_per_page + 1);
		break;
	    case CMD_time:
		if (secs >= 0) {
		    char buf[64];
		    my_snprintf(buf, sizeof(buf), "%.6f", secs);
		    // MSVC's snprintf omits the zero byte if the string if
		    // sizeof(buf) long.
		    buf[sizeof(buf) - 1] = '\0';
		    value = buf;
		}
		break;
	    case CMD_topdoc:
		// first document on current page of hit list (counting from 0)
		value = str(topdoc);
		break;
	    case CMD_topterms:
		if (enquire) {
		    int howmany = 16;
		    if (!args.empty()) {
			if (!parse_signed(args[0].c_str(), howmany)) {
			    throw "Number of terms for command "
				  "topterms must be an integer";
			}
		    }
		    if (howmany < 0) howmany = 0;
		    // List of expand terms
		    Xapian::ESet eset;
		    OmegaExpandDecider decider(db, &termset);

		    if (!rset.empty()) {
			set_expansion_scheme(*enquire, option);
			eset = enquire->get_eset(howmany * 2, rset, &decider);
		    } else if (mset.size()) {
			// invent an rset
			Xapian::RSet tmp;

			int c = 5;
			// FIXME: what if mset does not start at first match?
			for (Xapian::docid did : mset) {
			    tmp.add_document(did);
			    if (--c == 0) break;
			}

			set_expansion_scheme(*enquire, option);
			eset = enquire->get_eset(howmany * 2, tmp, &decider);
		    }

		    // Don't show more than one word with the same stem.
		    set<string> stems;
		    Xapian::ESetIterator i;
		    for (i = eset.begin(); i != eset.end(); ++i) {
			string term(*i);
			string stem = (*stemmer)(term);
			if (stems.find(stem) != stems.end()) continue;
			stems.insert(stem);
			value += term;
			value += '\t';
			if (--howmany == 0) break;
		    }
		    if (!value.empty()) value.erase(value.size() - 1);
		}
		break;
	    case CMD_transform:
		omegascript_transform(value, args);
		break;
	    case CMD_truncate: {
		unsigned int length;
		if (!parse_unsigned(args[1].c_str(), length)) {
		    throw "Length for truncate command must be >= 0";
		}
		value = generate_sample(args[0],
					length,
					args.size() > 2 ? args[2] : string(),
					args.size() > 3 ? args[3] : string());
		break;
	    }
	    case CMD_uniq: {
		const string &list = args[0];
		if (list.empty()) break;
		string::size_type split = 0, split2;
		string prev;
		do {
		    split2 = list.find('\t', split);
		    string item(list, split, split2 - split);
		    if (split == 0) {
			value = item;
		    } else if (item != prev) {
			value += '\t';
			value += item;
		    }
		    prev = item;
		    split = split2 + 1;
		} while (split2 != string::npos);
		break;
	    }
	    case CMD_unique: {
		unordered_set<string> seen;
		const string &list = args[0];
		if (list.empty()) break;
		string::size_type split = 0, split2;
		do {
		    split2 = list.find('\t', split);
		    string item(list, split, split2 - split);
		    if (seen.insert(item).second) {
			if (split != 0)
			    value += '\t';
			value += item;
		    }
		    split = split2 + 1;
		} while (split2 != string::npos);
		break;
	    }
	    case CMD_unpack:
		value = str(binary_string_to_int(args[0]));
		break;
	    case CMD_unprefix: {
		size_t prefix_len = prefix_from_term(NULL, args[0]);
		value.assign(args[0], prefix_len, string::npos);
		break;
	    }
	    case CMD_unstem: {
		const string &term = args[0];
		Xapian::TermIterator i = qp.unstem_begin(term);
		Xapian::TermIterator end = qp.unstem_end(term);
		while (i != end) {
		    if (!value.empty()) value += '\t';
		    value += *i;
		    ++i;
		}
		break;
	    }
	    case CMD_upper:
		value = Xapian::Unicode::toupper(args[0]);
		break;
	    case CMD_url:
		url_encode(value, args[0]);
		break;
	    case CMD_value: {
		Xapian::docid id = q0;
		Xapian::valueno value_no;
		if (!parse_unsigned(args[0].c_str(), value_no)) {
		    throw "Valueno of the value command should be >= 0";
		}
		if (args.size() > 1 &&
		    (!parse_unsigned(args[1].c_str(), id) || id == 0)) {
		    throw "Document id for value command must be > 0";
		}
		value = db.get_document(id).get_value(value_no);
		break;
	    }
	    case CMD_version:
		value = PACKAGE_STRING;
		break;
	    case CMD_weight:
		value = double_to_string(weight);
		break;
	    default: {
		args.insert(args.begin(), param[0]);
		int macro_no = func->second->tag - CMD_MACRO;
		assert(macro_no >= 0 && unsigned(macro_no) < macros.size());
		// throw "Unknown function '" + var + "'";
		value = eval(macros[macro_no], args);
		break;
	    }
	}
	res += value;
    } catch (const Xapian::Error & e) {
	// FIXME: this means we only see the most recent error in $error
	// - is that the best approach?
	error_msg = e.get_description();
    }

    res.append(fmt, p, string::npos);
    return res;
}

static string
eval_file(const string& fmtfile, bool* p_not_found)
{
    // Use -1 to indicate vet_filename() failed.
    int eno = -1;
    if (vet_filename(fmtfile)) {
	string file = template_dir + fmtfile;
	string fmt;
	errno = 0;
	if (load_file(file, fmt)) {
	    vector<string> noargs;
	    noargs.resize(1);
	    return eval(fmt, noargs);
	}
	eno = errno;
    }

    if (p_not_found) {
	*p_not_found = true;
	return string();
    }

    // FIXME: report why!
    string msg = string("Couldn't read format template '") + fmtfile + '\'';
    if (eno) {
	msg += " (";
	msg += (eno < 0 ? "name contains '..'" : strerror(eno));
	msg += ')';
    }
    throw msg;
}

extern string
pretty_term(string term)
{
    // Just leave empty strings and single characters alone.
    if (term.length() <= 1) return term;

    // Assume unprefixed terms are unstemmed.
    if (!C_isupper(term[0])) return term;

    // Handle stemmed terms.
    bool stemmed = (term[0] == 'Z');
    if (stemmed) {
	// First of all, check if a term in the query stemmed to this one.
	Xapian::TermIterator u = qp.unstem_begin(term);
	// There might be multiple words with the same stem, but we only want
	// one so just take the first.
	if (u != qp.unstem_end(term)) return *u;

	// Remove the 'Z'.
	term.erase(0, 1);
    }

    bool add_quotes = false;

    // Check if the term has a prefix.
    if (C_isupper(term[0])) {
	// See if we have this prefix in the termprefix_to_userprefix map.  If
	// so, just reverse the mapping (e.g. turn 'Sfish' into 'subject:fish').
	string prefix;
	size_t prefix_len = prefix_from_term(&prefix, term);

	map<string, string>::const_iterator i;
	i = termprefix_to_userprefix.find(prefix);
	if (i != termprefix_to_userprefix.end()) {
	    string user_prefix = i->second;
	    user_prefix += ':';
	    term.replace(0, prefix_len, user_prefix);
	} else {
	    // We don't have a prefix mapping for this, so just set a flag to
	    // add quotes around the term.
	    add_quotes = true;
	}
    }

    if (stemmed) term += '.';

    if (add_quotes) {
	term.insert(0, "\"");
	term.append("\"");
    }

    return term;
}

static string
print_caption(const string& fmt, vector<string>& param)
{
    q0 = *(mset[hit_no]);

    weight = mset[hit_no].get_weight();
    percent = mset.convert_to_percent(mset[hit_no]);
    collapsed = mset[hit_no].get_collapse_count();

    return eval(fmt, param);
}

void
parse_omegascript()
{
    try {
	string output = eval_file(fmtname);
	if (!set_content_type && !suppress_http_headers) {
	    cout << "Content-Type: text/html" << endl;
	    set_content_type = true;
	}
	if (!suppress_http_headers) cout << endl;
	cout << output;
    } catch (...) {
	// Ensure the headers have been output so that any exception gets
	// reported rather than giving a server error.
	if (!set_content_type && !suppress_http_headers) {
	    cout << "Content-Type: text/html" << endl;
	    set_content_type = true;
	}
	if (!suppress_http_headers) cout << endl;
	throw;
    }
}

static void
ensure_query_parsed()
{
    if (query_parsed) return;
    query_parsed = true;

    // Should we discard the existing R-set recorded in R CGI parameters?
    bool discard_rset = false;

    // Should we force the first page of hits (and ignore [ > < # and TOPDOC
    // CGI parameters)?
    bool force_first_page = false;

    string v;
    // get list of terms from previous iteration of query
    auto val = cgi_params.find("xP");
    if (val != cgi_params.end()) {
	v = val->second;
	// If xP given, default to discarding any RSet and forcing the first
	// page of results.  If the query is the same, or an extension of
	// the previous query, we adjust these again below.
	discard_rset = true;
	force_first_page = true;
    }
    querytype result = parse_queries(v);
    switch (result) {
	case BAD_QUERY:
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
		val = cgi_params.find("xFILTERS");
		if (val != cgi_params.end() && val->second != filters &&
		    val->second != old_filters) {
		    // Filters have changed since last query.
		} else {
		    force_first_page = false;
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
	    if (!parse_unsigned(val->second.c_str(), topdoc)) {
		throw "TOPDOC parameter must be >= 0";
	    }
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
	    if (!C_isdigit(val->second[0])) {
		throw "Page parameter must be >= 0";
	    }
	    long page = atol(val->second.c_str());
	    // Do something sensible for page 0 (we count pages from 1).
	    if (page == 0) page = 1;
	    topdoc = (page - 1) * hits_per_page;
	}

	// raw_search means don't snap TOPDOC to a multiple of HITSPERPAGE.
	// Normally we snap TOPDOC like this so that things work nicely if
	// HITSPERPAGE is in a <select> or on radio buttons.  If we're
	// postprocessing the output of omega and want variable sized pages,
	// this is unhelpful.
	bool raw_search = false;
	val = cgi_params.find("RAWSEARCH");
	if (val != cgi_params.end()) {
	    unsigned int temp;
	    if (!parse_unsigned(val->second.c_str(), temp)) {
		throw "RAWSEARCH parameter must be >= 0";
	    }
	    raw_search = bool(temp);
	}

	if (!raw_search) topdoc = (topdoc / hits_per_page) * hits_per_page;
    }

    if (!discard_rset) {
	// put documents marked as relevant into the rset
	auto g = cgi_params.equal_range("R");
	for (auto i = g.first; i != g.second; ++i) {
	    const string & value = i->second;
	    for (size_t j = 0; j < value.size(); j = value.find('.', j)) {
		while (value[j] == '.') ++j;
		Xapian::docid d;
		if (!parse_unsigned(value.c_str() + j, d) || d == 0) {
		    throw "Document id for 'R' parameter must be > 0";
		}
		if (d) {
		    rset.add_document(d);
		    ticked[d] = true;
		}
	    }
	}
    }
}

// run query if we haven't already
static void
ensure_match()
{
    if (done_query) return;

    secs = RealTime::now();
    run_query();
    if (secs != -1)
	secs = RealTime::now() - secs;

    done_query = true;
    last = mset.get_matches_lower_bound();
    if (last == 0) {
	// Otherwise topdoc ends up being -6 if it's non-zero!
	topdoc = 0;
    } else {
	if (topdoc >= last)
	    topdoc = ((last - 1) / hits_per_page) * hits_per_page;
	// last is the count of documents up to the end of the current page
	// (as returned by $last)
	if (topdoc + hits_per_page < last)
	    last = topdoc + hits_per_page;
    }
}

// OmegaExpandDecider methods.

OmegaExpandDecider::OmegaExpandDecider(const Xapian::Database & db_,
				       set<string> * querytermset)
    : db(db_)
{
    // We'll want the stemmer for testing matches anyway.
    if (!stemmer)
	stemmer = new Xapian::Stem(option["stemmer"]);
    if (querytermset) {
	set<string>::const_iterator i;
	for (i = querytermset->begin(); i != querytermset->end(); ++i) {
	    string term(*i);
	    if (term.empty()) continue;

	    unsigned char ch = term[0];
	    bool stemmed = (ch == 'Z');
	    if (stemmed) {
		term.erase(0, 1);
		if (term.empty()) continue;
		ch = term[0];
	    }

	    if (C_isupper(ch)) {
		size_t prefix_len = prefix_from_term(NULL, term);
		term.erase(0, prefix_len);
	    }

	    if (!stemmed) term = (*stemmer)(term);

	    exclude_stems.insert(term);
	}
    }
}

bool
OmegaExpandDecider::operator()(const string & term) const
{
    unsigned char ch = term[0];

    // Reject terms with a prefix.
    if (C_isupper(ch)) return false;

    {
	MyStopper stopper;
	// Don't suggest stopwords.
	if (stopper(term)) return false;
    }

    // Reject small numbers.
    if (term.size() < 4 && C_isdigit(ch)) return false;

    // Reject terms containing a space.
    if (term.find(' ') != string::npos) return false;

    // Skip terms with stems in the exclude_stems set, to avoid suggesting
    // terms which are already in the query in some form.
    string stem = (*stemmer)(term);
    if (exclude_stems.find(stem) != exclude_stems.end())
	return false;

    // Ignore terms that only occur once (hapaxes) since they aren't
    // useful for finding related documents - they only occur in a
    // document that's already been marked as relevant.
    // FIXME: add an expand option to ignore terms where
    // termfreq == rtermfreq.
    if (db.get_termfreq(term) <= 1) return false;

    return true;
}
