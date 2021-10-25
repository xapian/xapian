/** @file
 * @brief: Omega functions for running queries, etc.
 */
/* Copyright (C) 2007,2011,2016,2018 Olly Betts
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
#include <vector>

extern Xapian::Query::op default_op;

/** Information for mapping a docid to a DB parameter value and docid in that
 *  subset of databases.
 *
 *  A DB parameter value could point to a stub database file which can list
 *  multiple shards, and in this case we have multiple SubDB objects with the
 *  same name, and then index and out_of allow us to map docids.
 */
class SubDB {
    std::string name;
    size_t index;
    size_t out_of;

  public:
    SubDB(const std::string& name_,
	  size_t index_,
	  size_t out_of_)
	: name(name_), index(index_), out_of(out_of_) { }

    const std::string& get_name() const { return name; }

    Xapian::docid map_docid(Xapian::docid did) const {
	return (did - 1) * out_of + index + 1;
    }
};

extern std::vector<SubDB> subdbs;

void add_bterm(const std::string & term);

void add_nterm(const std::string & term);

void add_date_filter(const string& date_start,
		     const string& date_end,
		     const string& date_span,
		     Xapian::valueno date_value_slot);

void add_query_string(const std::string& prefix, const std::string& s);

void parse_omegascript();

std::string pretty_term(std::string term);

class OmegaExpandDecider : public Xapian::ExpandDecider {
    Xapian::Database db;
    std::set<std::string> exclude_stems;
  public:
    OmegaExpandDecider(const Xapian::Database& db_,
		       std::set<std::string>* querytermset = NULL);
    bool operator()(const std::string& term) const;
};

std::string html_escape(const std::string &str);

#endif // OMEGA_INCLUDED_QUERY_H
