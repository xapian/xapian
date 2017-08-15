/** @file simplifieddbn.h
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

#include <map>
#include <string>
#include <vector>

using namespace std;

enum {
    PARAM_ATTR_PROB,
    PARAM_SAT_PROB,
    PARAM_COUNT_
};

/** 
 * SimplifiedDBN class implementing the SDBN click model.
 *
 * For more information of DBN click model, see the following paper:
 *
 * Olivier Chapelle and Ya Zhang. 2009. A dynamic bayesian network click
 * model for web search ranking. In Proceedings of the 18th international
 * conference on World wide web (WWW '09). 
 */
class SimplifiedDBN {
    map<string, map<string, double[PARAM_COUNT_]>> url_relevances;
  public:
    /// Return the name of the click model.
    string name();

    /** Build and return search sessions from the input log file.
     *
     * @param logfile		Path to the final log file.
     */
    vector<vector<string>> build_sessions(const string &logfile);

    /** Train the model.
     *
     * @param sessions 		List of all sessions.
     */
    void train(vector<vector<string>> &sessions);

    /** Return predicted relevance of each document in a session.
     *
     * @param sessions		List of all sessions.
     */
    vector<double> get_predicted_relevances(const vector<string> &session);
};
