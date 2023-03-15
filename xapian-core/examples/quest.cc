/** @file
 * @brief Command line search tool using Xapian::QueryParser.
 */
/* Copyright (C) 2004-2022 Olly Betts
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

#include <xapian.h>

#include <cstdlib>
#include <cstring>

#include <algorithm>
#include <iostream>

#include "gnu_getopt.h"

using namespace std;

#define PROG_NAME "quest"
#define PROG_DESC "Xapian command line search tool"

// Stopwords:
static const char * const sw[] = {
    "a", "about", "an", "and", "are", "as", "at",
    "be", "by",
    "en",
    "for", "from",
    "how",
    "i", "in", "is", "it",
    "of", "on", "or",
    "that", "the", "this", "to",
    "was", "what", "when", "where", "which", "who", "why", "will", "with"
};

/** Common string to integer map entry for option decoding. */
struct tab_entry {
    const char* s;

    unsigned f;

    bool operator<(const char* s_) const {
	return strcmp(s, s_) < 0;
    }
};

/** Decode a string to an integer.
 *
 *  @param table  Array of tab_entry in ascending string order.
 *  @param s      The string to decode.
 */
template<typename T, std::size_t N>
static int
decode(const T (&table)[N], const char* s)
{
    auto p = lower_bound(begin(table), end(table), s);
    if (p == end(table) || strcmp(s, p->s) != 0)
	return -1;
    return p->f;
}

static const tab_entry flag_tab[] = {
    { "accumulate", Xapian::QueryParser::FLAG_ACCUMULATE },
    { "auto_multiword_synonyms", Xapian::QueryParser::FLAG_AUTO_MULTIWORD_SYNONYMS },
    { "auto_synonyms", Xapian::QueryParser::FLAG_AUTO_SYNONYMS },
    { "boolean", Xapian::QueryParser::FLAG_BOOLEAN },
    { "boolean_any_case", Xapian::QueryParser::FLAG_BOOLEAN_ANY_CASE },
    { "cjk_ngram", Xapian::QueryParser::FLAG_CJK_NGRAM },
    { "default", Xapian::QueryParser::FLAG_DEFAULT },
    { "lovehate", Xapian::QueryParser::FLAG_LOVEHATE },
    { "ngrams", Xapian::QueryParser::FLAG_NGRAMS },
    { "no_positions", Xapian::QueryParser::FLAG_NO_POSITIONS },
    { "partial", Xapian::QueryParser::FLAG_PARTIAL },
    { "phrase", Xapian::QueryParser::FLAG_PHRASE },
    { "pure_not", Xapian::QueryParser::FLAG_PURE_NOT },
    { "spelling_correction", Xapian::QueryParser::FLAG_SPELLING_CORRECTION },
    { "synonym", Xapian::QueryParser::FLAG_SYNONYM },
    { "wildcard", Xapian::QueryParser::FLAG_WILDCARD }
};

static const tab_entry default_op_tab[] = {
    { "and", Xapian::Query::OP_AND },
    { "elite_set", Xapian::Query::OP_ELITE_SET },
    { "max", Xapian::Query::OP_MAX },
    { "near", Xapian::Query::OP_NEAR },
    { "or", Xapian::Query::OP_OR },
    { "phrase", Xapian::Query::OP_PHRASE },
    { "synonym", Xapian::Query::OP_SYNONYM }
};

enum {
    WEIGHT_BB2,
    WEIGHT_BM25,
    WEIGHT_BM25PLUS,
    WEIGHT_BOOL,
    WEIGHT_COORD,
    WEIGHT_DLH,
    WEIGHT_DPH,
    WEIGHT_IFB2,
    WEIGHT_INEB2,
    WEIGHT_INL2,
    WEIGHT_LM,
    WEIGHT_PL2,
    WEIGHT_PL2PLUS,
    WEIGHT_TFIDF,
    WEIGHT_TRAD
};

static const tab_entry wt_tab[] = {
    { "bb2",	WEIGHT_BB2 },
    { "bm25",	WEIGHT_BM25 },
    { "bm25+",	WEIGHT_BM25PLUS },
    { "bool",	WEIGHT_BOOL },
    { "coord",	WEIGHT_COORD },
    { "dlh",	WEIGHT_DLH },
    { "dph",	WEIGHT_DPH },
    { "ifb2",	WEIGHT_IFB2 },
    { "ineb2",	WEIGHT_INEB2 },
    { "inl2",	WEIGHT_INL2 },
    { "lm",	WEIGHT_LM },
    { "pl2",	WEIGHT_PL2 },
    { "pl2+",	WEIGHT_PL2PLUS },
    { "tfidf",	WEIGHT_TFIDF },
    { "trad",	WEIGHT_TRAD }
};

/** The number of spaces to indent by in print_table.
 *
 *  This needs to match the indent in the help message in show_usage() below.
 */
#define INDENT \
"                                    "

/** Print string from a string to integer mapping table.
 *
 *  @param table  Array of tab_entry in ascending string order.
 */
template<typename T>
static char
print_table(const T& table)
{
    size_t pos = 256;
    for (auto& i : table) {
	size_t len = strlen(i.s);
	if (pos < 256) cout << ',';
	if (pos + len >= 78) {
	    cout << "\n" INDENT;
	    pos = sizeof(INDENT) - 2;
	} else {
	    cout << ' ';
	}
	cout << i.s;
	pos += len + 2;
    }
    return '\n';
}

static void show_usage() {
    cout << "Usage: " PROG_NAME " [OPTIONS] 'QUERY'\n"
"NB: QUERY should be quoted to protect it from the shell.\n\n"
"Options:\n"
"  -d, --db=DIRECTORY                database to search (multiple databases may\n"
"                                    be specified)\n"
"  -m, --msize=MSIZE                 maximum number of matches to return\n"
"  -c, --check-at-least=HOWMANY      minimum number of matches to check\n"
"  -s, --stemmer=LANG                set the stemming language, the default is\n"
"                                    'english' (pass 'none' to disable stemming)\n"
"  -p, --prefix=PFX:TERMPFX          add a prefix\n"
"  -b, --boolean-prefix=PFX:TERMPFX  add a boolean prefix\n"
"  -f, --flags=FLAG1[,FLAG2]...      specify QueryParser flags (default:\n"
"                                    default).  Valid flags:"
<< print_table(flag_tab) <<
"  -o, --default-op=OP               specify QueryParser default operator\n"
"                                    (default: or).  Valid operators:"
<< print_table(default_op_tab) <<
"  -w, --weight=SCHEME               specify weighting scheme to use\n"
"                                    (default: bm25).  Valid schemes:"
<< print_table(wt_tab) <<
"  -F, --freqs                       show query term frequencies\n"
"  -h, --help                        display this help and exit\n"
"  -v, --version                     output version information and exit\n";
}

int
main(int argc, char **argv)
try {
    const char * opts = "d:m:c:s:p:b:f:o:w:Fhv";
    static const struct option long_opts[] = {
	{ "db",		required_argument, 0, 'd' },
	{ "msize",	required_argument, 0, 'm' },
	{ "check-at-least",	required_argument, 0, 'c' },
	{ "stemmer",	required_argument, 0, 's' },
	{ "prefix",	required_argument, 0, 'p' },
	{ "boolean-prefix",	required_argument, 0, 'b' },
	{ "flags",	required_argument, 0, 'f' },
	{ "default-op",	required_argument, 0, 'o' },
	{ "weight",	required_argument, 0, 'w' },
	{ "freqs",	no_argument, 0, 'F' },
	{ "help",	no_argument, 0, 'h' },
	{ "version",	no_argument, 0, 'v' },
	{ NULL,		0, 0, 0}
    };

    Xapian::SimpleStopper mystopper(sw, sw + sizeof(sw) / sizeof(sw[0]));
    Xapian::Stem stemmer("english");
    Xapian::doccount msize = 10;
    Xapian::doccount check_at_least = 0;

    bool have_database = false;

    Xapian::Database db;
    Xapian::QueryParser parser;
    unsigned flags = 0;
    bool flags_set = false;
    bool show_termfreqs = false;
    int weight = -1;

    int c;
    while ((c = gnu_getopt_long(argc, argv, opts, long_opts, 0)) != -1) {
	switch (c) {
	    case 'm': {
		char * p;
		unsigned long v = strtoul(optarg, &p, 10);
		msize = static_cast<Xapian::doccount>(v);
		if (*p || v != msize) {
		    cerr << PROG_NAME": Bad value '" << optarg
			 << "' passed for msize\n";
		    exit(1);
		}
		break;
	    }
	    case 'c': {
		char * p;
		unsigned long v = strtoul(optarg, &p, 10);
		check_at_least = static_cast<Xapian::doccount>(v);
		if (*p || v != check_at_least) {
		    cerr << PROG_NAME": Bad value '" << optarg
			 << "' passed for check_at_least\n";
		    exit(1);
		}
		break;
	    }
	    case 'd':
		db.add_database(Xapian::Database(optarg));
		have_database = true;
		break;
	    case 's':
		try {
		    stemmer = Xapian::Stem(optarg);
		} catch (const Xapian::InvalidArgumentError &) {
		    cerr << "Unknown stemming language '" << optarg << "'.\n"
			    "Available language names are: "
			 << Xapian::Stem::get_available_languages() << '\n';
		    exit(1);
		}
		break;
	    case 'b': case 'p': {
		const char * colon = strchr(optarg, ':');
		if (colon == NULL) {
		    cerr << argv[0] << ": need ':' when setting prefix\n";
		    exit(1);
		}
		string prefix(optarg, colon - optarg);
		string termprefix(colon + 1);
		if (c == 'b') {
		    parser.add_boolean_prefix(prefix, termprefix);
		} else {
		    parser.add_prefix(prefix, termprefix);
		}
		break;
	    }
	    case 'f':
		flags_set = true;
		do {
		    char * comma = strchr(optarg, ',');
		    if (comma)
			*comma++ = '\0';
		    int flag = decode(flag_tab, optarg);
		    if (flag < 0) {
			cerr << "Unknown flag '" << optarg << "'\n";
			exit(1);
		    }
		    flags |= unsigned(flag);
		    optarg = comma;
		} while (optarg);
		break;
	    case 'o': {
		int op = decode(default_op_tab, optarg);
		if (op < 0) {
		    cerr << "Unknown op '" << optarg << "'\n";
		    exit(1);
		}
		parser.set_default_op(static_cast<Xapian::Query::op>(op));
		break;
	    }
	    case 'w': {
		weight = decode(wt_tab, optarg);
		if (weight < 0) {
		    cerr << "Unknown weighting scheme '" << optarg << "'\n";
		    exit(1);
		}
		break;
	    }
	    case 'F':
		show_termfreqs = true;
		break;
	    case 'v':
		cout << PROG_NAME " - " PACKAGE_STRING "\n";
		exit(0);
	    case 'h':
		cout << PROG_NAME " - " PROG_DESC "\n\n";
		show_usage();
		exit(0);
	    case ':': // missing parameter
	    case '?': // unknown option
		show_usage();
		exit(1);
	}
    }

    if (argc - optind != 1) {
	show_usage();
	exit(1);
    }

    parser.set_database(db);
    parser.set_stemmer(stemmer);
    parser.set_stemming_strategy(Xapian::QueryParser::STEM_SOME);
    parser.set_stopper(&mystopper);

    if (!flags_set) {
	flags = Xapian::QueryParser::FLAG_DEFAULT;
    }
    Xapian::Query query = parser.parse_query(argv[optind], flags);
    const string & correction = parser.get_corrected_query_string();
    if (!correction.empty())
	cout << "Did you mean: " << correction << "\n\n";

    cout << "Parsed Query: " << query.get_description() << '\n';

    if (!have_database) {
	cout << "No database specified so not running the query.\n";
	exit(0);
    }

    Xapian::Enquire enquire(db);
    enquire.set_query(query);

    switch (weight) {
	case WEIGHT_BB2:
	    enquire.set_weighting_scheme(Xapian::BB2Weight());
	    break;
	case WEIGHT_BOOL:
	    enquire.set_weighting_scheme(Xapian::BoolWeight());
	    break;
	case WEIGHT_COORD:
	    enquire.set_weighting_scheme(Xapian::CoordWeight());
	    break;
	case WEIGHT_BM25:
	    enquire.set_weighting_scheme(Xapian::BM25Weight());
	    break;
	case WEIGHT_BM25PLUS:
	    enquire.set_weighting_scheme(Xapian::BM25PlusWeight());
	    break;
	case WEIGHT_DLH:
	    enquire.set_weighting_scheme(Xapian::DLHWeight());
	    break;
	case WEIGHT_DPH:
	    enquire.set_weighting_scheme(Xapian::DPHWeight());
	    break;
	case WEIGHT_IFB2:
	    enquire.set_weighting_scheme(Xapian::IfB2Weight());
	    break;
	case WEIGHT_INEB2:
	    enquire.set_weighting_scheme(Xapian::IneB2Weight());
	    break;
	case WEIGHT_INL2:
	    enquire.set_weighting_scheme(Xapian::InL2Weight());
	    break;
	case WEIGHT_LM:
	    enquire.set_weighting_scheme(Xapian::LMWeight());
	    break;
	case WEIGHT_PL2:
	    enquire.set_weighting_scheme(Xapian::PL2Weight());
	    break;
	case WEIGHT_PL2PLUS:
	    enquire.set_weighting_scheme(Xapian::PL2PlusWeight());
	    break;
	case WEIGHT_TFIDF:
	    enquire.set_weighting_scheme(Xapian::TfIdfWeight());
	    break;
	case WEIGHT_TRAD:
	    enquire.set_weighting_scheme(Xapian::TradWeight());
	    break;
    }

    Xapian::MSet mset = enquire.get_mset(0, msize, check_at_least);

    if (show_termfreqs) {
	cout << "Query term frequencies:\n";
	for (auto t = query.get_terms_begin();
	     t != query.get_terms_end();
	     ++t) {
	    const string& term = *t;
	    cout << "    " << mset.get_termfreq(term) << '\t' << term << '\n';
	}
    }
    auto lower_bound = mset.get_matches_lower_bound();
    auto estimate = mset.get_matches_estimated();
    auto upper_bound = mset.get_matches_upper_bound();
    if (lower_bound == upper_bound) {
	cout << "Exactly " << estimate << " matches\n";
    } else {
	cout << "Between " << lower_bound << " and " << upper_bound
	     << " matches, best estimate is " << estimate << '\n';
    }

    cout << "MSet:\n";
    for (Xapian::MSetIterator i = mset.begin(); i != mset.end(); ++i) {
	Xapian::Document doc = i.get_document();
	string data = doc.get_data();
	cout << *i << ": [" << i.get_weight() << "]\n" << data << "\n";
    }
    cout << flush;
} catch (const Xapian::QueryParserError & e) {
    cout << "Couldn't parse query: " << e.get_msg() << '\n';
    exit(1);
} catch (const Xapian::Error & err) {
    cout << err.get_description() << '\n';
    exit(1);
}
