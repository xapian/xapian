/** @file letor-request.cc
 * @brief Command line search tool for letor using Xapian::QueryParser and Xapian::Letor
 *
 * Copyright (C) 2004,2005,2006,2007,2008,2009,2010 Olly Betts
 * Copyright (C) 2011 Parth Gupta
 * Copyright (C) 2014 Jiarong Wei
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

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include <cstdlib>
#include <cstring>
#include <cstdio> 
#include <cmath>

#include "gnu_getopt.h"

using std::cout;
using std::string;
using std::vector;

#define PROG_NAME "letor-request"
#define PROG_DESC "Xapian command line search tool for letor"

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

static void
show_usage() {
    cout << "Usage: "PROG_NAME" [options] features_file model_file 'QUERY' \n"
"NB: QUERY should be quoted to protect it from the shell.\n\n"
"Options:\n"
"  -d, --db=DIRECTORY                   database to search (multiple databases may be specified)\n"
"  -m, --msize=MSIZE                    maximum number of matches to return\n"
"  -r, --ranker=FLAG                    set the ranker to be used (Default: 0)\n"
"       0 -- SVM ranker\n"
"  -n, --normalizer=FLAG                set the normalizer to be used (Default: 0)\n"
"       0 -- Default normalizer\n"
"  -s, --stemmer=LANG                   set the stemming language, the default is 'english'\n"
"                                           (pass 'none' to disable stemming)\n"
"  -p, --prefix=PFX:TERMPFX             Add a prefix\n"
"  -b, --boolean-prefix=PFX:TERMPFX     Add a boolean prefix\n"
"  -h, --help                           display this help and exit\n"
"  -v, --version                        output version information and exit\n";
}


string
parse_query(string query_str) {
    istringstream iss(query_str);
    string title = "title:";

    while(iss) {
        string t;
        iss >> t;
        if (t == "")
            break;
        string temp = "";
        temp.append(title);
        temp.append(t);
        temp.append(" ");
        temp.append(query_str);
        query_str = temp;
    }
    return query_str;
}


int
main(int argc, char **argv)
try {
    const char * opts = "d:m:s:p:b:h:v";
    static const struct option long_opts[] = {
        { "db",             required_argument, 0, 'd' },
        { "msize",          required_argument, 0, 'm' },
        { "ranker",         required_argument, 0, 'r' },
        { "normalizer",     required_argument, 0, 'n' },
        { "stemmer",        required_argument, 0, 's' },
        { "prefix",         required_argument, 0, 'p' },
        { "boolean-prefix", required_argument, 0, 'b' },
        { "help",           no_argument,       0, 'h' },
        { "version",        no_argument,       0, 'v' },
        { NULL,             0,                 0, 0   }
    };

    Xapian::SimpleStopper mystopper(sw, sw + sizeof(sw) / sizeof(sw[0]));
    Xapian::Stem stemmer("english");

    // Default arguments
    int msize = 10;
    int ranker_flag = 0;
    int normalizer_flag = 0;
    bool have_database = false;

    Xapian::Database db;
    Xapian::QueryParser parser;
    parser.add_prefix("title", "S");
    parser.add_prefix("subject", "S");

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
            case 'r':
                ranker_flag = atoi(optarg);
                break;
            case 'n':
                normalizer_flag = atoi(optarg);
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
            case 'b':
            case 'p':
                const char * colon = strchr(optarg, ':');
                if (colon == NULL) {
                    cerr << argv[0] << ": need ':' when setting prefix" << '\n';
                    exit(1);
                }
                
                string prefix(optarg, colon - optarg);
                string termprefix(colon + 1);
                if (c == 'b') {
                    parser.add_boolean_prefix(prefix, termprefix);
                }
                else {
                    parser.add_prefix(prefix, termprefix);
                }
                break;
            case 'v':
                cout << PROG_NAME" - "PACKAGE_STRING"\n";
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

    if (argc - optind != 3) {
        show_usage();
        exit(1);
    }

    if (!have_database) {
        cout << "No database specified so not running the query." << '\n';
        exit(0);
    }

    string features_file    = string(argv[optind]);
    string model_file       = string(argv[optind+1]);
    string query_str        = parse_query(argv[optind+2]);

    // Parse query
    parser.set_database(db);
    parser.set_default_op(Xapian::Query::OP_OR);
    parser.set_stemmer(stemmer);
    parser.set_stemming_strategy(Xapian::QueryParser::STEM_SOME);
    parser.set_stopper(&mystopper);

    cout << "Final Query: " << query_str << '\n';

    Xapian::Query query = parser.parse_query(query_str,
                         parser.FLAG_DEFAULT|
                         parser.FLAG_SPELLING_CORRECTION);

    const string & correction = parser.get_corrected_query_string();
    
    if (!correction.empty())
        cout << "Did you mean: " << correction << "\n\n";

    cout << "Parsed Query: " << query.get_description() << '\n';

    // Fetch MSet
    Xapian::Enquire enquire(db);
    enquire.set_query(query);

    Xapian::MSet mset = enquire.get_mset(0, msize);

    // Update MSet
    vector<int> features = Feature::read_from_file(features_file);

    Xapian::Letor ltr;
    Xapian::Ranker * ranker = Ranker::generate(ranker_flag);
    Xapian::Normalizer * normalizer = Normalizer::generate(normalizer_flag);

    ltr.update_context(database, features, *ranker, *normalizer);
    
    ltr.load_model_file(model_file);

    Xapian::MSet updated_mset = ltr.update_mset(query, mset);

    // Output the updated MSet
    for (Xapian::MSetIterator mset_it = updated_mset.begin(), rank = 0;
            mset_it != updated_mset.end(); ++mset_it, ++rank) {
        Xapian::Document doc = mset_it->get_document();

        cout << "Rank: " << rank << '\n'
        cout << doc->get_data() << "\n\n";
    }

    return 0;
}
catch (const Xapian::QueryParserError & e)
{
    cout << "Couldn't parse query: " << e.get_msg() << '\n';
    exit(1);
}
catch (const Xapian::Error & err)
{
    cout << err.get_description() << '\n';
    exit(1);
}
