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

#ifndef OMEGA_INCLUDED_SIMPLIFIEDDBN_H
#define OMEGA_INCLUDED_SIMPLIFIEDDBN_H

#include <map>
#include <string>
#include <utility>
#include <vector>

#include "session.h"

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
    /// Relevances of documents corresponding to a query in a search session.
    std::map<std::string, std::map<std::string, std::map<int, double>>>
    doc_relevances;
  public:
    /// Return the name of the click model.
    std::string name();

    /** Builds search sessions from the input log file and returns a list
     * of generated sessions. Each session contains three values: queryid,
     * a list of documents in the search result and a list of count of clicks
     * made on each document in the search result.
     *
     * @param logfile		Path to the final log file.
     */
    std::vector<Session> build_sessions(const std::string &logfile);

    /** Trains the model i.e. learning the values of attractiveness
     * and satisfactoriness parameters modelled by the click model.
     *
     * @param sessions		List of all sessions.
     */
    void train(const std::vector<Session> &sessions);

    /** Return predicted relevance of each document in a session i.e. the
     * estimations of the relevance of each document in a given session based
     * on a trained model. Values ranging from 0.0 to 1.0.
     *
     * @param sessions		Session class object representing a session.
     */
    std::vector<std::pair<std::string, double>>
    get_predicted_relevances(const Session &session);
};

#endif // OMEGA_INCLUDED_SIMPLIFIEDDBN_H
