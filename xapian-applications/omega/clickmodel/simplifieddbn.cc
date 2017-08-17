/** @file simplifieddbn.cc
 * @brief SimplifiedDBN class - the Simplified DBN click model.
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include <config.h>

#include "simplifieddbn.h"

#include <algorithm>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

#define QID 0
#define DOCIDS 1
#define CLICKS 2

string
SimplifiedDBN::name()
{
    return "SimplifiedDBN";
}

static vector<string>
get_docid_list(const vector<string> &session)
{
    vector<string> docids;
    string docid;
    for (size_t j = 0; j <= session[DOCIDS].length(); ++j) {
	char ch = session[DOCIDS][j];
	if (ch != ',' && ch != '\0') {
	    docid += ch;
	} else {
	    docids.push_back(docid);
	    docid.clear();
	}
    }
    return docids;
}

static vector<int>
get_click_list(const vector<string> &session)
{
    vector<int> clicks;
    string clickstring, clickcount;
    int click = 0;

    for (size_t j = 0; j <= session[CLICKS].length(); ++j) {
	char ch = session[CLICKS][j];
	if (ch != ',' && ch != '\0') {
	    // Get clickstring of the form "docid:click_count".
	    clickstring += session[CLICKS][j];

	    // Get click count string.
	    size_t delimiter_pos = clickstring.find(':');
	    clickcount = clickstring.substr(delimiter_pos + 1);

	    // Convert click count string to an integer.
	    stringstream ss(clickcount);
	    ss >> click;
	} else {
	    clicks.push_back(click);
	    clickstring.clear();
	}
    }
    return clicks;
}

vector<vector<string>>
SimplifiedDBN::build_sessions(const string &logfile)
{
    ifstream file;
    file.open(logfile, ios::in);

    if (!file) {
	throw runtime_error("ERROR: Specified file does not exist.");
    }

    string line;

    // Skip the first line.
    getline(file, line);

    vector<vector<string>> sessions;
    vector<string> session_entry;

    // Start reading file from the second line.
    while (getline(file, line)) {

	istringstream ss(line);

	vector<string> row_data;

	while (ss >> std::ws) {
	    string column_element;
	    if (ss.peek() == '"') {
		int pos = ss.tellg();
		ss.seekg(pos + 1);
		char ch;
		while (ss.get(ch)) {
		    if (ch == '"')
			break;
		    column_element += ch;
		}
	    } else {
		if (ss.peek() == ',') {
		    int pos = ss.tellg();
		    ss.seekg(pos + 1);
		}
		getline(ss, column_element, ',');
	    }
	    row_data.push_back(column_element);
	}

	string qid = row_data[0];
	string query = row_data[1];
	string docids = row_data[2];
	string clicks = row_data[4];

	session_entry.push_back(qid);
	session_entry.push_back(docids);
	session_entry.push_back(clicks);

	sessions.push_back(session_entry);

	session_entry.clear();
    }

    file.close();
    return sessions;
}

void
SimplifiedDBN::train(const vector<vector<string>> &sessions)
{
    map<string, map<string, map<char, double[PARAM_COUNT_]>>> doc_rel_fractions;

    for (size_t i = 0; i < sessions.size(); ++i) {
	string qid = sessions[i][QID];

	vector<string> docids = get_docid_list(sessions[i]);

	vector<int> clicks = get_click_list(sessions[i]);

	int last_clicked_pos = clicks.size() - 1;

	for (size_t j = 0; j < clicks.size(); ++j)
	    if (clicks[j] != 0)
		last_clicked_pos = j;

	// Initialise some values.
	for (int k = 0; k <= last_clicked_pos; ++k) {
	    doc_rel_fractions[qid][docids[k]]['a'][0] = 1.0;
	    doc_rel_fractions[qid][docids[k]]['a'][1] = 1.0;
	    doc_rel_fractions[qid][docids[k]]['s'][0] = 1.0;
	    doc_rel_fractions[qid][docids[k]]['s'][1] = 1.0;
	}

	for (int k = 0; k <= last_clicked_pos; ++k) {
	    if (clicks[k] != 0) {
		doc_rel_fractions[qid][docids[k]]['a'][1] += 1;
		if (int(k) == last_clicked_pos)
		    doc_rel_fractions[qid][docids[k]]['s'][1] += 1;
		else
		    doc_rel_fractions[qid][docids[k]]['s'][0] += 1;
	    } else {
		doc_rel_fractions[qid][docids[k]]['a'][0] += 1;
	    }
	}
    }

    for (auto i = doc_rel_fractions.begin(); i != doc_rel_fractions.end(); ++i) {
	string qid = i->first;
	for (auto&& j : i->second) {
	    doc_relevances[qid][j.first][PARAM_ATTR_PROB] = j.second['a'][1] /
							   (j.second['a'][1] +
							    j.second['a'][0]);
	    doc_relevances[qid][j.first][PARAM_SAT_PROB] = j.second['s'][1] /
							  (j.second['s'][1] +
							   j.second['s'][0]);
	}
    }
}

vector<double>
SimplifiedDBN::get_predicted_relevances(const vector<string> &session)
{
    vector<double> relevances;

    vector<string> docids = get_docid_list(session);

    for (size_t i = 0; i < docids.size(); ++i) {
	double attr_prob = doc_relevances[session[QID]][docids[i]][PARAM_ATTR_PROB];
	double sat_prob = doc_relevances[session[QID]][docids[i]][PARAM_SAT_PROB];
	relevances.push_back(attr_prob * sat_prob);
    }
    return relevances;
}
