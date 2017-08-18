/** @file sdbntest.cc
 * @brief tests for the Simplified DBN clickmodel class.
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

#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include <clickmodel/simplifieddbn.h>

using namespace std;

#define QID 0
#define DOCIDS 1
#define CLICKS 2

struct sessions_items {
    const string qid;
    const string docids;
    const string clicks;
};

struct sessions_testcase {
    const string logfile;
    const sessions_items sessions;
};

string sample_log1 = "test1.log";
string sample_log2 = "test2.log";
string sample_log3 = "test3.log";

static sessions_testcase sessions_tests[] = {
    {sample_log1, {"821f03288846297c2cf43c34766a38f7",
		   "45,36,14,54,42",
		   "45:0,36:0,14:0,54:2,42:0"}},
    {sample_log2, {"","",""}}
};

static bool
almost_equal(double x, double y, double epsilon = 0.001) {
    return (abs(x - y) < epsilon);
}

int main() {
    SimplifiedDBN sdbn;
    int failure_count = 0;

    // Tests for SimplifiedDBN::build_sessions method.
    for (size_t i = 0; i < sizeof(sessions_tests) / sizeof(sessions_tests[0]); ++i) {
	vector<vector<string>> result;
	try {
	    result = sdbn.build_sessions(sessions_tests[i].logfile);
	} catch (std::exception &ex) {
	    cout << ex.what() << endl;
	    ++failure_count;
	    // Specified file doesn't exist. Skip subsequent tests.
	    continue;
	}

	for (auto&& x : result) {
	    if (x[QID] != sessions_tests[i].sessions.qid) {
		cerr << "ERROR: Query ID mismatch occurred. " << endl
		     << "Expected: " << sessions_tests[i].sessions.qid
		     << " Received: "<< x[QID] << endl;
		++failure_count;
	    }
	    if (x[DOCIDS] != sessions_tests[i].sessions.docids) {
		cerr << "ERROR: Doc ID(s) mismatch occurred. " << endl
		     << "Expected: " << sessions_tests[i].sessions.docids
		     << " Received: " << x[DOCIDS] << endl;
		++failure_count;
	    }
	    if (x[CLICKS] != sessions_tests[i].sessions.clicks) {
		cerr << "ERROR: Clicks mismatch occurred. " << endl
		     << "Expected: " << sessions_tests[i].sessions.clicks
		     << " Received: " << x[CLICKS] << endl;
		++failure_count;
	    }
	}
    }

    // Train Simplified DBN model on a dummy training file.
    vector<vector<string>> training_sessions;
    try {
	training_sessions = sdbn.build_sessions(sample_log3);
    } catch (std::exception &ex) {
	cout << ex.what() << endl;
	++failure_count;
    }

    sdbn.train(training_sessions);

    vector<vector<double>>
    expected_relevances = {{0.166, 0.166, 0.166, 0.444, 0},
			   {0.444, 0, 0, 0, 0}};

    // Tests for SimplifiedDBN::get_predicted_relevances. 
    for (size_t i = 0; i < training_sessions.size(); ++i) {
	vector<double>
	predicted_relevances = sdbn.get_predicted_relevances(training_sessions[i]);

	if (predicted_relevances.size() != expected_relevances[i].size()) {
	    cerr << "ERROR: Relevance lists sizes do not match." << endl;
	    ++failure_count;
	    // Skip subsequent tests.
	    continue;
	}

	for (size_t j = 0; j < expected_relevances[i].size(); ++j) {
	    if (!almost_equal(predicted_relevances[j], expected_relevances[i][j])) {
		cerr << "ERROR: Relevances do not match." << endl
		     << "Expected: " << expected_relevances[i][j]
		     << " Received: " << predicted_relevances[j] << endl;
		++failure_count;
	    }
	}
    }

    if (failure_count != 0) {
	cout << "Total failures occurred: " << failure_count << endl;
	exit(1);
    }
}
