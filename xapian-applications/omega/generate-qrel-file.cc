/** @file generate_qrel_file.cc
 * @brief generates the qrel file needed to prepare training file for letor.
 */
/* Copyright (C) 2017 Vivek Pal
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include <config.h>

#include <iostream>
#include <fstream>
#include <string>
#include <utility>
#include <vector>

#include "gnu_getopt.h"

#include <clickmodel/simplifieddbn.h>

using namespace std;

#define NAME "generate-qrel-file"
#define DESC "Command to generate qrel file needed to prepare training file for letor"

static void help() {
    cout << "usage: ./" NAME " [<option>] final_log qrel_file\n\n"

    "      final_log        Path to final log file\n"
    "      qrel_file        Path to save qrel file\n"
    "      --help           display this help message and exit\n";
}

#define OPT_HELP 1
#define DOCIDS 1

int main(int argc, char **argv) {
    const char * opts = "";
    static const struct option long_opts[] = {
	{ "help",		no_argument, 0, OPT_HELP },
	{ NULL,			0, 0, 0}
    };

    string final_log_path, qrel_path;

    int c;
    while ((c = gnu_getopt_long(argc, argv, opts, long_opts, 0)) != -1) {
	switch (c) {
	    case OPT_HELP:
		cout << DESC "\n\n";
		help();
		exit(0);
	    case '?': // unknown option
		help();
		exit(1);
	}
    }

    if (argc - optind != 2) {
	help();
	exit(1);
    }

    string final_log_file = argv[optind];
    string qrel_file = argv[optind + 1];

    SimplifiedDBN sdbn;

    vector<Session> sessions;
    try {
	sessions = sdbn.build_sessions(final_log_file);
    } catch (std::exception &ex) {
	cerr << ex.what() << endl;
	exit(1);
    }

    ofstream file_q;
    file_q.open(qrel_file, ios::out);

    sdbn.train(sessions);

    // Extract doc relevances and doc ids from each session and write
    // to the qrel file in the required format.
    for (auto&& session : sessions) {
	vector<pair<string, double>> docid_relevances =
	    sdbn.get_predicted_relevances(session);

	auto reliter = docid_relevances.begin();

	for (; reliter != docid_relevances.end(); ++reliter)
	    file_q << session.get_qid() << " Q0 " << (*reliter).first << ' '
		   << (*reliter).second << endl;
    }

    file_q.close();
}
