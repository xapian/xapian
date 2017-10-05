/** @file mergepostlist.h
 * @brief PostList class for searching multiple databases together
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

#ifndef XAPIAN_INCLUDED_MERGEPOSTLIST_H
#define XAPIAN_INCLUDED_MERGEPOSTLIST_H

#include "wrapperpostlist.h"

class ValueStreamDocument;

/// PostList class for searching multiple databases together.
class MergePostList : public WrapperPostList {
    /// The postlists for the shards.
    PostList** shard_pls;

    /// The number of shards.
    Xapian::doccount n_shards;

    /** The current shard.
     *
     *  We're at_end() when shard == n_shards.
     */
    Xapian::doccount shard = 0;

    /** Document proxy used for valuestream caching.
     *
     *  Each time we move to a new shard we must notify this object so it can
     *  invalidate any cached valuestreams (which are specific to the shard).
     */
    ValueStreamDocument& vsdoc;

  public:
    MergePostList(PostList** pls,
		  Xapian::doccount n_shards_,
		  ValueStreamDocument& vsdoc_)
	: WrapperPostList(pls[0]),
	  shard_pls(pls),
	  n_shards(n_shards_),
	  vsdoc(vsdoc_) {}

    ~MergePostList();

    Xapian::doccount get_termfreq_min() const;

    Xapian::doccount get_termfreq_max() const;

    Xapian::doccount get_termfreq_est() const;

    Xapian::docid get_docid() const;

    const std::string* get_sort_key() const;

    const std::string* get_collapse_key() const;

    bool at_end() const;

    double recalc_maxweight();

    TermFreqs get_termfreq_est_using_stats(
	    const Xapian::Weight::Internal& stats) const;

    PostList* next(double w_min);

    PostList* skip_to(Xapian::docid did, double w_min);

    std::string get_description() const;
};

#endif // XAPIAN_INCLUDED_MERGEPOSTLIST_H
