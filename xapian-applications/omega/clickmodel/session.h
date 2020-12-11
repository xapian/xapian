/** @file
 * @brief Session class for handling search session data.
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

#ifndef OMEGA_INCLUDED_SESSION_H
#define OMEGA_INCLUDED_SESSION_H

#include <string>
#include <vector>

#define QID 0
#define DOCIDS 1
#define CLICKS 2

/**
 * Session class for handling search session data elements.
 */
class Session {
    /// Each session contains data elements stored as std::string.
    std::vector<std::string> session;
  public:
    /** Creates a new session with the given data elements.
     *
     * @param qid		Query id.
     * @param docids		Document ids in the search session.
     * @param clicks		Click information corresponding to the docids.
     */
    void create_session(std::string qid, std::string docids,
			std::string clicks) {
	session.push_back(qid);
	session.push_back(docids);
	session.push_back(clicks);
    }

    /// Retrieve the query id of the session.
    std::string get_qid() const { return session[QID]; }

    /// Retrieve the docids string of the session.
    std::string get_docids() const { return session[DOCIDS]; }

    /// Retrieve the clicks string of the session.
    std::string get_clicks() const { return session[CLICKS]; }
};

#endif // OMEGA_INCLUDED_SESSION_H
