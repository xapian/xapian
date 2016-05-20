/** @file msetpostlist.h
 *  @brief PostList returning entries from an MSet
 */
/* Copyright (C) 2006,2007,2008,2009,2011,2015 Olly Betts
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

#ifndef XAPIAN_INCLUDED_MSETPOSTLIST_H
#define XAPIAN_INCLUDED_MSETPOSTLIST_H

#include "xapian/enquire.h"

#include "api/omenquireinternal.h"
#include "api/postlist.h"

/** PostList returning entries from an MSet.
 *
 *  This class is used with the remote backend.  We perform a match on the
 *  remote server, then serialise the resulting MSet and pass it back to the
 *  client where we include it in the match by wrapping it in an MSetPostList.
 */
class MSetPostList : public PostList {
    /// Don't allow assignment.
    void operator=(const MSetPostList &);

    /// Don't allow copying.
    MSetPostList(const MSetPostList &);

    /// The MSet element that this PostList is pointing to.
    int cursor;

    /// The MSet::Internal object which we're returning entries from.
    Xapian::Internal::intrusive_ptr<Xapian::MSet::Internal> mset_internal;

    /** Is the sort order such the relevance decreases down the MSet?
     *
     *  This is true for sort_by_relevance and sort_by_relevance_then_value.
     */
    bool decreasing_relevance;

  public:
    MSetPostList(const Xapian::MSet mset, bool decreasing_relevance_)
	: cursor(-1), mset_internal(mset.internal),
	  decreasing_relevance(decreasing_relevance_) { }

    Xapian::doccount get_termfreq_min() const;

    Xapian::doccount get_termfreq_est() const;

    Xapian::doccount get_termfreq_max() const;

    double get_maxweight() const;

    Xapian::docid get_docid() const;

    double get_weight() const;

    const std::string * get_sort_key() const;

    const std::string * get_collapse_key() const;

    /// Not implemented for MSetPostList.
    Xapian::termcount get_doclength() const;

    Xapian::termcount get_unique_terms() const;

    double recalc_maxweight();

    PostList *next(double w_min);

    /// Not meaningful for MSetPostList.
    PostList *skip_to(Xapian::docid did, double w_min);

    bool at_end() const;

    std::string get_description() const;
};

#endif /* XAPIAN_INCLUDED_MSETPOSTLIST_H */
