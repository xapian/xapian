/** @file
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

static string get_srcdir() {
    const char *p = getenv("srcdir");
    if (!p) p = ".";
    return string(p);
}

static bool
almost_equal(double x, double y, double epsilon = 0.001) {
    return (abs(x - y) < epsilon);
}

int main() {
    string srcdir = get_srcdir();

    string sample_log1 = srcdir + "/clickmodel/testdata/test1.log";
    string sample_log2 = srcdir + "/clickmodel/testdata/test2.log";
    string sample_log3 = srcdir + "/clickmodel/testdata/test3.log";

    sessions_testcase sessions_tests[] = {
	{sample_log1, {"821f03288846297c2cf43c34766a38f7",
		       "45,36,14,54,42",
		       "45:0,36:0,14:0,54:2,42:0"}},
	{sample_log2, {"","",""}}
    };

    SimplifiedDBN sdbn;

    int failure_count = 0;

    // Tests for SimplifiedDBN::build_sessions method.
    for (size_t i = 0; i < sizeof(sessions_tests) / sizeof(sessions_tests[0]);
	++i) {
	vector<Session> result;
	try {
	    result = sdbn.build_sessions(sessions_tests[i].logfile);
	} catch (std::exception &ex) {
	    cout << ex.what() << endl;
	    ++failure_count;
	    // Specified file doesn't exist. Skip subsequent tests.
	    continue;
	}

	for (auto&& x : result) {
	    if (x.get_qid() != sessions_tests[i].sessions.qid) {
		cerr << "ERROR: Query ID mismatch occurred. " << endl
		     << "Expected: " << sessions_tests[i].sessions.qid
		     << " Received: " << x.get_qid() << endl;
		++failure_count;
	    }
	    if (x.get_docids() != sessions_tests[i].sessions.docids) {
		cerr << "ERROR: Doc ID(s) mismatch occurred. " << endl
		     << "Expected: " << sessions_tests[i].sessions.docids
		     << " Received: " << x.get_docids() << endl;
		++failure_count;
	    }
	    if (x.get_clicks() != sessions_tests[i].sessions.clicks) {
		cerr << "ERROR: Clicks mismatch occurred. " << endl
		     << "Expected: " << sessions_tests[i].sessions.clicks
		     << " Received: " << x.get_clicks() << endl;
		++failure_count;
	    }
	}
    }

    // Train Simplified DBN model on a dummy training file.
    vector<Session> training_sessions;
    try {
	training_sessions = sdbn.build_sessions(sample_log3);
    } catch (std::exception &ex) {
	cout << ex.what() << endl;
	++failure_count;
    }

    sdbn.train(training_sessions);

    pair<string, double> relevance11("45", 0.166);
    pair<string, double> relevance12("36", 0.166);
    pair<string, double> relevance13("14", 0.166);
    pair<string, double> relevance14("54", 0.444);
    pair<string, double> relevance15("42", 0);

    pair<string, double> relevance21("35", 0.444);
    pair<string, double> relevance22("47", 0);
    pair<string, double> relevance23("31", 0);
    pair<string, double> relevance24("14", 0);
    pair<string, double> relevance25("45", 0);

    vector<vector<pair<string, double>>> expected_relevances = {
	{ relevance11, relevance12, relevance13, relevance14, relevance15 },
	{ relevance21, relevance22, relevance23, relevance24, relevance25 }
    };

    int k = 0;
    // Tests for SimplifiedDBN::get_predicted_relevances.
    for (auto&& session : training_sessions) {
	vector<pair<string, double>> predicted_relevances =
	    sdbn.get_predicted_relevances(session);

	if (predicted_relevances.size() != expected_relevances[k].size()) {
	    cerr << "ERROR: Relevance lists sizes do not match." << endl;
	    ++failure_count;
	    // Skip subsequent tests.
	    continue;
	}

	for (size_t j = 0; j < expected_relevances[k].size(); ++j) {
	    if (!almost_equal(predicted_relevances[j].second,
			      expected_relevances[k][j].second)) {
		cerr << "ERROR: Relevances do not match." << endl
		     << "Expected: " << expected_relevances[k][j].second
		     << " Received: " << predicted_relevances[j].second << endl;
		++failure_count;
	    }
	}
	++k;
    }

    if (failure_count != 0) {
	cout << "Total failures occurred: " << failure_count << endl;
	exit(1);
    }
}
