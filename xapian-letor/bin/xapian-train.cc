/** @file
 * @brief Command line tool to train and save the LTR model
 */
/* Copyright (C) 2004,2005,2006,2007,2008,2009,2010,2015 Olly Betts
 * Copyright (C) 2011 Parth Gupta
 * Copyright (C) 2016 Ayush Tomar
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
#include <xapian-letor.h>

#include <iostream>
#include <string>

#include "gnu_getopt.h"

using namespace std;

#define PROG_NAME "xapian-train"
#define PROG_DESC "Command line tool to train and save the LTR model"

#define OPT_HELP 1
#define OPT_VERSION 2

static void show_usage() {
    cout << "Usage: " PROG_NAME " [OPTIONS] PATH_TO_TRAINING_FILE MODEL_METADATA_KEY\n"
    "Options:\n"
    "  -d, --db=DIRECTORY  path to database to search\n"
    "      --help          display this help and exit\n"
    "      --version       output version information and exit\n";
}

int
main(int argc, char **argv)
try {
    const char * opts = "d:h:v";
    static const struct option long_opts[] = {
	{ "db",		required_argument, 0, 'd' },
	{ "help",	no_argument, 0, OPT_HELP },
	{ "version",	no_argument, 0, OPT_VERSION },
	{ NULL,		0, 0, 0}
    };

    bool have_database = false;

    string db_path;

    int c;
    while ((c = gnu_getopt_long(argc, argv, opts, long_opts, 0)) != -1) {
	switch (c) {
	    case 'd':
		db_path = optarg;
		have_database = true;
		break;
	    case OPT_HELP:
		cout << PROG_NAME " - " PROG_DESC "\n\n";
		show_usage();
		exit(0);
	    case OPT_VERSION:
		cout << PROG_NAME " - " PACKAGE_STRING << endl;
		exit(0);
	    case ':': // missing parameter
	    case '?': // unknown option
		show_usage();
		exit(1);
	}
    }

    if (argc - optind != 2) {
	show_usage();
	exit(1);
    }

    string trainingfile = argv[optind];
    string model_metadata_key = argv[optind + 1];

    if (!have_database) {
	cout << "No database specified so not running the query." << endl;
	exit(0);
    }

    // Initialise Ranker object.
    // See Ranker documentation for available Ranker subclass options.
    Xapian::Ranker * ranker = new Xapian::ListNETRanker();

    // Set database
    ranker->set_database_path(db_path);

    // Perform training and save model as database metadata with key "model_metadata_key"
    ranker->train_model(trainingfile, model_metadata_key);

    delete ranker;

    cout << flush;

} catch (const Xapian::Error & err) {
    cout << err.get_description() << endl;
    exit(1);
}
