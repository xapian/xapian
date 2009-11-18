/** @file query.h
 * @brief: Omega functions for running queries, etc.
 *
 * Copyright (C) 2007 Olly Betts
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#ifndef OMEGA_INCLUDED_QUERY_H
#define OMEGA_INCLUDED_QUERY_H

#include <xapian.h>

#include <set>
#include <string>

extern Xapian::Query::op default_op;

void add_bterm(const std::string & term);

void parse_omegascript();

std::string pretty_term(std::string term);

class OmegaExpandDecider : public Xapian::ExpandDecider {
    Xapian::Database db;
    set<string> exclude_stems;
  public:
    OmegaExpandDecider(const Xapian::Database & db,
		       set<string> * querytermset = NULL);
    bool operator()(const string & term) const;
};

std::string html_escape(const std::string &str);

#endif // OMEGA_INCLUDED_QUERY_H
