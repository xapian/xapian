/* @file letor-prepare.cc
 * @brief Command line tool for preparing letor training data
 *
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
#include <map>
#include <set>

#include <cstdlib>
#include <cstring>
#include <cstdio> 
#include <cmath>

#include "gnu_getopt.h"

using namespace std;

#define PROG_NAME "letor-prepare"
#define PROG_DESC "Xapian command line tool for preparing letor training data"

static void
show_usage() {
    cout << "Usage: "PROG_NAME" [options] features_file query_file qrel_file output_file\n\n"
PROG_DESC"\n\n"
"Options:\n"
"  -d, --db=DIRECTORY       database used to be the context (multiple databases may be specified)\n"
"  -s, --size=SIZE          maximum number of matches to return (Default: 100)\n"
"  -r, --ranker=FLAG        set the ranker to be used (Default: 0)\n"
"       0 -- SVM ranker\n"
"  -n, --normalizer=FLAG    set the normalizer to be used (Default: 0)\n"
"       0 -- Default normalizer\n"
"  -h, --help               display this help and exit\n"
"  -v, --version            output version information and exit\n";
}

int
main(int argc, char **argv)
try {
    const char * opts = "d:s:r:n:h:v";
    static const struct option long_opts[] = {
        { "db",             required_argument, 0, 'd' },
        { "size",           required_argument, 0, 's' },
        { "ranker",         required_argument, 0, 'r' },
        { "normalizer",     required_argument, 0, 'n' },
        { "help",           no_argument,       0, 'h' },
        { "version",        no_argument,       0, 'v' },
        { NULL,             0,                 0, 0   }
    };

    Xapian::Database database;
    bool have_database = false;

    // Default arguments
    Xapian::Ranker::ranker_t ranker_flag = Xapian::Ranker::SVM_RANKER;
    Xapian::Normalizer::normalizer_t normalizer_flag = Xapian::Normalizer::DEFAULT_NORMALIZER;
    int size = 100;

    int c;
    while ((c = gnu_getopt_long(argc, argv, opts, long_opts, 0)) != -1) {
        switch (c) {
            case 'd':
                database.add_database(Xapian::Database(optarg));
                have_database = true;
                break;
            case 's':
                size = atoi(optarg);
                break;
            case 'r':
                ranker_flag = atoi(optarg);
                break;
            case 'n':
                normalizer_flag = atoi(optarg);
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

    if (argc - optind != 4) {
        show_usage();
        exit(1);
    }

    if (!have_database) {
        cout << "No database specified, so not running the training process.\n";
        exit(1);
    }

    string features_file    = string(argv[optind]);
    string query_file       = string(argv[optind+1]);
    string qrel_file        = string(argv[optind+2]);
    string output_file      = string(argv[optind+3]);

    vector<Xapian::Feature::feature_t> features = Xapian::Feature::read_from_file(features_file);

    Xapian::Letor ltr;
    ltr.set_database(database);
    ltr.set_features(features);
    ltr.set_ranker(ranker_flag);
    ltr.set_normalizer(normalizer_flag);

    ltr.prepare_training_file(query_file, qrel_file, output_file, size);

    return 0;
}
catch (const Xapian::Error & err)
{
    cout << err.get_description() << endl;
    exit(1);
}
