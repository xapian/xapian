/** @file result.h
 * @brief A result in an MSet
 */
/* Copyright 2017 Olly Betts
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
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

#ifndef XAPIAN_INCLUDED_RESULT_H
#define XAPIAN_INCLUDED_RESULT_H

#include "backends/multi.h"
#include "xapian/types.h"

#include <string>

/** A result in an MSet. */
class Result {
    double weight;

    Xapian::docid did;

    Xapian::doccount collapse_count = 0;

    std::string collapse_key;

    std::string sort_key;

  public:
    /// FIXME: Try to eliminate non-move assignment.
    Result& operator=(const Result&) = default;

    /// FIXME: Try to eliminate copying.
    Result(const Result&) = default;

    /// Move constructor.
    Result(Result&& o) = default;

    /// Move assignment.
    Result& operator=(Result&& o) = default;

    /// Constructor.
    Result(double weight_, Xapian::docid did_)
	: weight(weight_), did(did_) {}

    /// Constructor used by MSet::Internal::unserialise().
    Result(double weight_, Xapian::docid did_,
	   std::string&& collapse_key_,
	   Xapian::doccount collapse_count_,
	   std::string&& sort_key_)
	: weight(weight_), did(did_),
	  collapse_count(collapse_count_),
	  collapse_key(std::move(collapse_key_)),
	  sort_key(std::move(sort_key_)) {}

    void swap(Result& o);

    Xapian::docid get_docid() const { return did; }

    double get_weight() const { return weight; }

    Xapian::doccount get_collapse_count() const { return collapse_count; }

    const std::string& get_collapse_key() const { return collapse_key; }

    const std::string& get_sort_key() const { return sort_key; }

    void set_weight(double weight_) { weight = weight_; }

    void set_collapse_count(Xapian::doccount c) { collapse_count = c; }

    void set_collapse_key(const std::string& k) { collapse_key = k; }

    void set_sort_key(const std::string& k) { sort_key = k; }

    void unshard_docid(Xapian::doccount shard, Xapian::doccount n_shards) {
	did = unshard(did, shard, n_shards);
    }

    std::string get_description() const;
};

#endif // XAPIAN_INCLUDED_RESULT_H
