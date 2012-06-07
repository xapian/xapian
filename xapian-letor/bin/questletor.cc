/** @file questletor.cc
 * @brief Command line search tool using Xapian::QueryParser and Xapian::Letor
 */
/* Copyright (C) 2004,2005,2006,2007,2008,2009,2010 Olly Betts
 * Copyright (C) 2011 Parth Gupta
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
#include <xapian/letor.h>

#include <cstdlib>
#include <cstring>
#include <stdio.h>
#include <stdlib.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include <set>
#include <math.h>

#include "gnu_getopt.h"

using namespace std;

#define PROG_NAME "letor"
#define PROG_DESC "Xapian command line search tool with Lerning to Rank Facility"

typedef std::pair<Xapian::docid, double> MyPair;

struct MyTestCompare {
    bool operator()(const MyPair& firstPair, const MyPair& secondPair) const {
	return firstPair.second < secondPair.second;
    }
};

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

static void show_usage() {
    cout << "Usage: "PROG_NAME" [OPTIONS] 'QUERY'\n"
"NB: QUERY should be quoted to protect it from the shell.\n\n"
"Options:\n"
"  -d, --db=DIRECTORY  database to search (multiple databases may be specified)\n"
"  -m, --msize=MSIZE   maximum number of matches to return\n"
"  -s, --stemmer=LANG  set the stemming language, the default is 'english'\n"
"                      (pass 'none' to disable stemming)\n"
"  -p, --prefix=PFX:TERMPFX  Add a prefix\n"
"  -b, --boolean-prefix=PFX:TERMPFX  Add a boolean prefix\n"
"  -h, --help          display this help and exit\n"
"  -v, --version       output version information and exit\n";
}

int
main(int argc, char **argv)
try {
    const char * opts = "d:m:s:p:b:hv";
    static const struct option long_opts[] = {
	{ "db",		required_argument, 0, 'd' },
	{ "msize",	required_argument, 0, 'm' },
	{ "stemmer",	required_argument, 0, 's' },
	{ "prefix",	required_argument, 0, 'p' },
	{ "boolean-prefix",	required_argument, 0, 'b' },
	{ "help",	no_argument, 0, 'h' },
	{ "version",	no_argument, 0, 'v' },
	{ NULL,		0, 0, 0}
    };

    Xapian::SimpleStopper mystopper(sw, sw + sizeof(sw) / sizeof(sw[0]));
    Xapian::Stem stemmer("english");
    int msize = 10;

    bool have_database = false;

    Xapian::Database db;
    Xapian::QueryParser parser;
	parser.add_prefix("title","S");
	parser.add_prefix("subject","S");

    int c;
    while ((c = gnu_getopt_long(argc, argv, opts, long_opts, 0)) != -1) {
	switch (c) {
	    case 'm':
		msize = atoi(optarg);
		break;
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

    /* This module converts the query terms inclusive of the query terms
     * in titles
     * Example:
     * original query = "parth gupta"
     * converted query = "title:parth title:gupta parth gupta"
     */

    string qq=argv[optind];
    istringstream iss(argv[optind]);
    string title="title:";
    while(iss) {
	string t;
	iss >> t;
	if (t=="")
	    break;
	string temp="";
	temp.append(title);
	temp.append(t);
	temp.append(" ");
	temp.append(qq);
	qq=temp;
    }
    cout << "Final Query " << qq << "\n";

    Xapian::Query query = parser.parse_query(qq,
					     parser.FLAG_DEFAULT|
					     parser.FLAG_SPELLING_CORRECTION);
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

    Xapian::MSet mset = enquire.get_mset(0, msize);

    Xapian::TermIterator qt,qt_end,temp,temp_end,docterms,docterms_end;
    Xapian::PostingIterator p,pend;

    Xapian::Letor ltr;

    ltr.set_database(db);
    ltr.set_query(query);
    ltr.create_ranker(0);

    ltr.prepare_training_file("/home/encoder/gsoc/inex/topics.txt.short","/home/encoder/gsoc/inex/2010-assessments/inex2010-article.qrels",100);

    ltr.letor_learn_model(4,0);
    map<Xapian::docid,double> letor_mset = ltr.letor_score(mset);

    set<MyPair,MyTestCompare> s;
    map<Xapian::docid, double>::iterator iter = letor_mset.begin();
    map<Xapian::docid, double>::iterator endIter = letor_mset.end();

    for (; iter != endIter; ++iter) {
       s.insert(*iter);
    }

    set<MyPair,MyTestCompare>::iterator it;

    int rank=1;
    for (it = s.end(); it != s.begin(); it--) {
	cout << "Item: " << rank << "\t" << (*it).second << "\n";

	Xapian::Document doc = db.get_document((*it).first);
	cout << doc.get_data() << "\n";
	rank++;
    }

    cout << flush;
} catch (const Xapian::QueryParserError & e) {
    cout << "Couldn't parse query: " << e.get_msg() << endl;
    exit(1);
} catch (const Xapian::Error & err) {
    cout << err.get_description() << endl;
    exit(1);
}
