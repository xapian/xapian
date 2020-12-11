/** @file
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
#include <utility>
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
get_docid_list(const string &str_docids)
{
    vector<string> docids;
    string docid;
    for (size_t j = 0; j <= str_docids.length(); ++j) {
	char ch = str_docids[j];
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
get_click_list(const string &str_clicks)
{
    vector<int> clicks;
    string clickstring, clickcount;
    int click = 0;

    for (size_t j = 0; j <= str_clicks.length(); ++j) {
	char ch = str_clicks[j];
	if (ch != ',' && ch != '\0') {
	    // Get clickstring of the form "docid:click_count".
	    clickstring += str_clicks[j];

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

vector<Session>
SimplifiedDBN::build_sessions(const string &logfile)
{
    ifstream file;
    file.open(logfile, ios::in);

    if (!file) {
	throw runtime_error("Couldn't open file: " + logfile);
    }

    string line;

    // Skip the first line.
    getline(file, line);

    vector<Session> sessions;

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

	Session s;
	s.create_session(qid, docids, clicks);
	sessions.push_back(s);
    }

    file.close();
    return sessions;
}

void
SimplifiedDBN::train(const vector<Session> &sessions)
{
    map<string, map<string, map<int, map<int, double>>>> doc_rel_fractions;

    for (auto&& session : sessions) {
	string qid = session.get_qid();

	vector<string> docids = get_docid_list(session.get_docids());

	vector<int> clicks = get_click_list(session.get_clicks());

	int last_clicked_pos = clicks.size() - 1;

	for (size_t j = 0; j < clicks.size(); ++j)
	    if (clicks[j] != 0)
		last_clicked_pos = j;

	// Initialise some values.
	for (int k = 0; k <= last_clicked_pos; ++k) {
	    doc_rel_fractions[qid][docids[k]][PARAM_ATTR_PROB][0] = 1.0;
	    doc_rel_fractions[qid][docids[k]][PARAM_ATTR_PROB][1] = 1.0;
	    doc_rel_fractions[qid][docids[k]][PARAM_SAT_PROB][0] = 1.0;
	    doc_rel_fractions[qid][docids[k]][PARAM_SAT_PROB][1] = 1.0;
	}

	for (int k = 0; k <= last_clicked_pos; ++k) {
	    if (clicks[k] != 0) {
		doc_rel_fractions[qid][docids[k]][PARAM_ATTR_PROB][1] += 1;
		if (int(k) == last_clicked_pos)
		    doc_rel_fractions[qid][docids[k]][PARAM_SAT_PROB][1] += 1;
		else
		    doc_rel_fractions[qid][docids[k]][PARAM_SAT_PROB][0] += 1;
	    } else {
		doc_rel_fractions[qid][docids[k]][PARAM_ATTR_PROB][0] += 1;
	    }
	}
    }

    for (auto i = doc_rel_fractions.begin(); i != doc_rel_fractions.end();
	++i) {
	string qid = i->first;
	for (auto&& j : i->second) {
	    doc_relevances[qid][j.first][PARAM_ATTR_PROB] =
		j.second[PARAM_ATTR_PROB][1] /
		(j.second[PARAM_ATTR_PROB][1] +
		j.second[PARAM_ATTR_PROB][0]);
	    doc_relevances[qid][j.first][PARAM_SAT_PROB] =
		j.second[PARAM_SAT_PROB][1] /
		(j.second[PARAM_SAT_PROB][1] +
		j.second[PARAM_SAT_PROB][0]);
	}
    }
}

vector<pair<string, double>>
SimplifiedDBN::get_predicted_relevances(const Session &session)
{
    vector<pair<string, double>> docid_relevances;

    vector<string> docids = get_docid_list(session.get_docids());

    for (size_t i = 0; i < docids.size(); ++i) {
	double attr_prob =
		doc_relevances[session.get_qid()][docids[i]][PARAM_ATTR_PROB];
	double sat_prob =
		doc_relevances[session.get_qid()][docids[i]][PARAM_SAT_PROB];
	docid_relevances.push_back(make_pair(docids[i], attr_prob * sat_prob));
    }
    return docid_relevances;
}
