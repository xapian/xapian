/** @file simplifiedDBN.cc
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

#include "simplifiedDBN.h"

#include <algorithm>
#include <iomanip>
#include <iostream>
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
getDocidsList(vector<vector<string>> &sessions)
{
    vector<string> docids;
    string docid;
    for (size_t i = 0; i < sessions.size(); i++) {
	for (size_t j = 0; j < sessions[i][DOCIDS].length(); j++) {
	    if (sessions[i][DOCIDS][j] != ',') {
		docid += sessions[i][DOCIDS][j];
	    } else {
		docids.push_back(docid);
		docid.clear();
	    }
	}
    }
    return docids;
}

static vector<int>
getClicksList(vector<vector<string>> &sessions)
{
    vector<int> clicks;
    string clickstring, clickcount;
    int click = 0;
    for (size_t i = 0; i < sessions.size(); i++) {
	for (size_t j = 0; j <= sessions[i][CLICKS].length(); j++) {
	    if (sessions[i][CLICKS][j] == '\0') {
		clicks.push_back(click);
	    } else if (sessions[i][CLICKS][j] != ',') {
		// Get clickstring of the form "docid:click_count".
		clickstring += sessions[i][CLICKS][j];

		// Get click count string.
		string delimiter = ":";
		int delimiterPos = clickstring.find(delimiter);
		clickcount = clickstring.substr(delimiterPos + 1, clickstring.length() - 1 - delimiterPos);
	
		// Convert click count string to an integer.
		stringstream ss(clickcount);
		ss >> click;
	    } else {
		clickstring.clear();
		clicks.push_back(click);
	    }
	}
    }
    return clicks;
}

vector<vector<string>>
SimplifiedDBN::buildSessions(string logfile)
{
    ifstream file;
    file.open(logfile, ios::in);

    string line;
	
    // Skip the first line.
    getline(file, line);

    vector<vector<string>> sessions;
    vector<string> sessionEntry;

    // Start reading file from the second line.
    while (getline(file, line)) {

	istringstream ss(line);

	vector<string> rowData;

	while (ss >> std::ws) {
	    string columnElement;
	    if (ss.peek() == '"') {
		ss >> quoted(columnElement);
		string discard;
		getline(ss, discard, ',');
	    } else {
		getline(ss, columnElement, ',');
	    }
	    rowData.push_back(columnElement);
	}

	string qid = rowData[0];
	string query = rowData[1];
	string docids = rowData[2];
	string clicks = rowData[4];

	sessionEntry.push_back(qid);
	sessionEntry.push_back(docids);
	sessionEntry.push_back(clicks);

	sessions.push_back(sessionEntry);

	sessionEntry.clear();
    }

    file.close();
    return sessions;
}

void
SimplifiedDBN::train(vector<vector<string>> &sessions)
{
    map<string, map<string, map<char, array<double, 2>>>> urlRelFractions;

    for (size_t i = 0; i < sessions.size(); i++) {
	string qid = sessions[i][QID];

	vector<string> docids = getDocidsList(sessions);

	vector<int> clicks = getClicksList(sessions);

	int lastClickedPos = clicks.size() - 1;

	for (size_t j = 0; j < clicks.size(); j++)
	    if (clicks[j] != 0)
		lastClickedPos = j;

	vector<int> sessionClicks;
	for (int x = 0; x <= lastClickedPos + 1; x++)
	    sessionClicks.push_back(clicks[x]);

	for (size_t k = 0; k < sessionClicks.size(); k++) {
	    if (sessionClicks[k] != 0) {
		urlRelFractions[qid][docids[k]]['a'][1] += 1;
		if (int(k) == lastClickedPos)
		    urlRelFractions[qid][docids[k]]['s'][1] += 1;
		else
		    urlRelFractions[qid][docids[k]]['s'][0] += 1;
	    } else {
		urlRelFractions[qid][docids[k]]['a'][0] += 1;
	    }
	}
    }

    for (auto i = urlRelFractions.begin(); i != urlRelFractions.end(); ++i) {
	string qid = i->first;
	if (i->second.empty())
	    continue;
	for (auto j = i->second.begin(); j != i->second.end(); ++j) {
	    auto relFractions = j->second;
	    urlRelevances[qid][j->first]['a'] = relFractions['a'][1] /
						(relFractions['a'][1] +
						relFractions['a'][0]);
	    urlRelevances[qid][j->first]['s'] = relFractions['s'][1] /
						(relFractions['s'][1] +
						relFractions['s'][0]);
	}
    }
}

vector<int>
SimplifiedDBN::get_predicted_relevances(vector<vector<string>> &sessions)
{
    vector<int> relevances;
    double a, s;

    vector<string> docids = getDocidsList(sessions);

    for (size_t i = 0; i < docids.size(); i++) {
	a = urlRelevances[sessions[i][QID]][docids[i]]['a'];
	s = urlRelevances[sessions[i][QID]][docids[i]]['s'];
	relevances.push_back(a * s);
    }
    return relevances;
}
