/* @file letor-select.cc
 * @brief Command line tool for selecting letor features
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

#define PROG_NAME "letor-select"
#define PROG_DESC "Xapian command line tool for selecting letor features"

static void
show_usage() {
    cout << "Usage: "PROG_NAME" [options] features_file training_data_file validation_data\n"
PROG_DESC"\n\n"
"Options:\n"
"  -d, --db=DIRECTORY  database to search (multiple databases may be specified)\n"
"  -k, --k=K           Select top K features (Default: 1)\n"
"  -h, --help          display this help and exit\n"
"  -v, --version       output version information and exit\n";
}

int
main(int argc, char **argv) {
    const char * opts = "d:m:s:p:b:h:v";
    static const struct option long_opts[] = {
        { "db",             required_argument, 0, 'd' },
        { "k",              required_argument, 0, 'k' },
        { "help",           no_argument,       0, 'h' },
        { "version",        no_argument,       0, 'v' },
        { NULL,             0,                 0, 0   }
    };

    // Default arguments
    int  k = 1;

    int c;
    while ((c = gnu_getopt_long(argc, argv, opts, long_opts, 0)) != -1) {
        switch (c) {
            case 'k':
                k = atoi(optarg);
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

    if (argc - optind != 3) {
        show_usage();
        exit(1);
    }

    vector<int> features = Feature::read_from_file();

    string features_file        = string(argv[optind]);
    string training_data_file   = string(argv[optind+1]);
    string validation_data_file = string(argv[optind+2]);

    vector<int> features = Feature::read_from_file(features_file);
    vector<FeatureVector> training_data = FeatureVector::read_from_file(training_data_file);
    vector<FeatureVector> validation_data = FeatureVector::read_from_file(validation_data_file);

    FeautureSelector fs;
    vector<int> selected_features = fs.select(features, k, training_data, validation_data);

    cout << "Selected features:" << endl;
    for (vector<int>::iterator it = selected_features.begin(); it != selected_features.end(); ++it) {
        cout << "   " << *it << endl;
    }

    return 0;
}
