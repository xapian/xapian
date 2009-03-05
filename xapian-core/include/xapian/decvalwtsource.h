/** @file decvalwtsource.h
 * @brief A posting source which returns decreasing weights from a value.
 */
/* Copyright (C) 2009 Lemur Consulting Ltd
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

#ifndef XAPIAN_INCLUDED_DECVALWTSOURCE_H
#define XAPIAN_INCLUDED_DECVALWTSOURCE_H

#include <xapian/types.h>
#include <xapian/postingsource.h>
#include <xapian/visibility.h>

namespace Xapian {

/** Read weights from a value which is known to decrease as docid increases.
 *
 *  This posting source can be used, like ValueWeightPostingSource, to add a
 *  weight contribution to a query based on the values stored in a slot.  The
 *  values in the slot must be serialised as by @a sortable_serialise().
 *
 *  However, this posting source is additionally given a range of document IDs,
 *  within which the weight is known to be decreasing.  ie, for all documents
 *  with ids A and B within this range, where A is less than B, the weight of A
 *  is less than or equal to the weight of B.  This can allow the posting
 *  source to skip to the end of the range quickly if insufficient weight is
 *  left in the posting source for a particular source.
 *
 *  By default, the range is assumed to cover all document IDs.
 *
 *  The ordering property can be arranged at index time, or by sorting an
 *  indexed database to produce a new, sorted, database.
 */
class XAPIAN_VISIBILITY_DEFAULT DecreasingValueWeightPostingSource
	: public Xapian::ValueWeightPostingSource {
    Xapian::docid range_start;
    Xapian::docid range_end;
    double curr_weight;

    // Skip the iterator forward if in the decreasing range, and weight is low.
    void skip_if_in_range(Xapian::weight min_wt);
  public:
    DecreasingValueWeightPostingSource(Xapian::valueno slot_,
				       Xapian::docid range_start_ = 0,
				       Xapian::docid range_end_ = 0);

    Xapian::weight get_weight() const;
    ValueWeightPostingSource * clone() const;
    std::string name() const;
    std::string serialise() const;
    PostingSource * unserialise(const std::string &s) const;
    void reset(const Xapian::Database & db_);

    void next(Xapian::weight min_wt);
    void skip_to(Xapian::docid min_docid, Xapian::weight min_wt);
    bool check(Xapian::docid min_docid, Xapian::weight min_wt);

    std::string get_description() const;
};

}

#endif // XAPIAN_INCLUDED_DECVALWTSOURCE_H
