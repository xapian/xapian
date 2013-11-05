/* quest.cc - Command line search tool using Xapian::QueryParser.
 *
 * Copyright (C) 2004,2005,2006,2007,2008,2009,2010,2012,2013 Olly Betts
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
static const char * sw[] = {
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

struct qp_flag { const char * s; unsigned f; };
static qp_flag flag_tab[] = {
    { "auto_multiword_synonyms", Xapian::QueryParser::FLAG_AUTO_MULTIWORD_SYNONYMS },
    { "auto_synonyms", Xapian::QueryParser::FLAG_AUTO_SYNONYMS },
    { "boolean", Xapian::QueryParser::FLAG_BOOLEAN },
    { "boolean_any_case", Xapian::QueryParser::FLAG_BOOLEAN_ANY_CASE },
    { "default", Xapian::QueryParser::FLAG_DEFAULT },
    { "lovehate", Xapian::QueryParser::FLAG_LOVEHATE },
    { "partial", Xapian::QueryParser::FLAG_PARTIAL },
    { "phrase", Xapian::QueryParser::FLAG_PHRASE },
    { "pure_not", Xapian::QueryParser::FLAG_PURE_NOT },
    { "spelling_correction", Xapian::QueryParser::FLAG_SPELLING_CORRECTION },
    { "synonym", Xapian::QueryParser::FLAG_SYNONYM },
    { "wildcard", Xapian::QueryParser::FLAG_WILDCARD }
};
const int n_flag_tab = sizeof(flag_tab) / sizeof(flag_tab[0]);

inline bool operator<(const qp_flag & f1, const qp_flag & f2) {
    return strcmp(f1.s, f2.s) < 0;
}

static void show_usage() {
    cout << "Usage: "PROG_NAME" [OPTIONS] 'QUERY'\n"
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
"  -f, --flags=FLAG1[,FLAG2]...      specify QueryParser flags.  Valid flags:";
#define INDENT \
"                                    "
    int pos = 256;
    for (const qp_flag * i = flag_tab; i - flag_tab < n_flag_tab; ++i) {
	size_t len = strlen(i->s);
	if (pos < 256) cout << ',';
	if (pos + len >= 78) {
	    cout << "\n"INDENT;
	    pos = sizeof(INDENT) - 2;
	} else {
	    cout << ' ';
	}
	cout << i->s;
	pos += len + 2;
    }
    cout << "\n"
"  -h, --help                        display this help and exit\n"
"  -v, --version                     output version information and exit\n";
}

static unsigned
decode_qp_flag(const char * s)
{
    qp_flag f;
    f.s = s;
    const qp_flag * p = lower_bound(flag_tab, flag_tab + n_flag_tab, f);
    if (p == flag_tab + n_flag_tab || f < *p)
	return 0;
    return p->f;
}

int
main(int argc, char **argv)
try {
    const char * opts = "c:d:m:s:p:b:f:hv";
    static const struct option long_opts[] = {
	{ "db",		required_argument, 0, 'd' },
	{ "msize",	required_argument, 0, 'm' },
	{ "check-at-least",	required_argument, 0, 'c' },
	{ "stemmer",	required_argument, 0, 's' },
	{ "prefix",	required_argument, 0, 'p' },
	{ "boolean-prefix",	required_argument, 0, 'b' },
	{ "flags",	required_argument, 0, 'f' },
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
    unsigned flags = parser.FLAG_DEFAULT|parser.FLAG_SPELLING_CORRECTION;

    int c;
    while ((c = gnu_getopt_long(argc, argv, opts, long_opts, 0)) != -1) {
	switch (c) {
	    case 'm': {
		char * p;
		unsigned long v = strtoul(optarg, &p, 10);
		msize = static_cast<Xapian::doccount>(v);
		if (*p || v != msize) {
		    cerr << PROG_NAME": Bad value '" << optarg
			 << "' passed for msize" << endl;
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
			 << "' passed for check_at_least " << endl;
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
			 << Xapian::Stem::get_available_languages() << endl;
		    exit(1);
		}
		break;
	    case 'b': case 'p': {
		const char * colon = strchr(optarg, ':');
		if (colon == NULL) {
		    cerr << argv[0] << ": need ':' when setting prefix" << endl;
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
		flags = 0;
		do {
		    char * comma = strchr(optarg, ',');
		    if (comma)
			*comma++ = '\0';
		    unsigned flag = decode_qp_flag(optarg);
		    if (flag == 0) {
			cerr << "Unknown flag '" << optarg << "'" << endl;
			exit(1);
		    }
		    flags |= flag;
		    optarg = comma;
		 } while (optarg);
		 break;
	    case 'v':
		cout << PROG_NAME" - "PACKAGE_STRING << endl;
		exit(0);
	    case 'h':
		cout << PROG_NAME" - "PROG_DESC"\n\n";
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
    parser.set_default_op(Xapian::Query::OP_OR);
    parser.set_stemmer(stemmer);
    parser.set_stemming_strategy(Xapian::QueryParser::STEM_SOME);
    parser.set_stopper(&mystopper);

    Xapian::Query query = parser.parse_query(argv[optind], flags);
    const string & correction = parser.get_corrected_query_string();
    if (!correction.empty())
	cout << "Did you mean: " << correction << "\n\n";

    cout << "Parsed Query: " << query.get_description() << endl;

    if (!have_database) {
	cout << "No database specified so not running the query." << endl;
	exit(0);
    }

    Xapian::Enquire enquire(db);
    enquire.set_query(query);

    enquire.set_sort_by_relevance_then_value(0, true);
    Xapian::MSet mset = enquire.get_mset(0, msize, check_at_least);

    cout << "MSet:" << endl;
    for (Xapian::MSetIterator i = mset.begin(); i != mset.end(); i++) {
	Xapian::Document doc = i.get_document();
	string data = doc.get_data();
	cout << *i << ": [" << i.get_weight() << "]\n" << data << "\n";
    }
    cout << flush;
} catch (const Xapian::QueryParserError & e) {
    cout << "Couldn't parse query: " << e.get_msg() << endl;
    exit(1);
} catch (const Xapian::Error & err) {
    cout << err.get_description() << endl;
    exit(1);
}
