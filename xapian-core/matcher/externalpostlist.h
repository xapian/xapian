/** @file
 * @brief Return document ids from an external source.
 */
/* Copyright 2008,2009,2011,2019 Olly Betts
 * Copyright 2009 Lemur Consulting Ltd
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

#ifndef XAPIAN_INCLUDED_EXTERNALPOSTLIST_H
#define XAPIAN_INCLUDED_EXTERNALPOSTLIST_H

#include "backends/postlist.h"

namespace Xapian {
    class PostingSource;
}

class PostListTree;

class ExternalPostList : public PostList {
    /// Disallow copying.
    ExternalPostList(const ExternalPostList &);

    /// Disallow assignment.
    void operator=(const ExternalPostList &);

    Xapian::Internal::opt_intrusive_ptr<Xapian::PostingSource> source;

    Xapian::docid current;

    double factor;

    PostList * update_after_advance();

  public:
    /** Constructor.
     *
     *  @param max_weight_cached_flag_ptr   Pointer to flag to clear when max
     *					    weight changes.
     */
    ExternalPostList(const Xapian::Database & db,
		     Xapian::PostingSource *source_,
		     double factor_,
		     bool* max_weight_cached_flag_ptr,
		     Xapian::doccount shard_index);

    Xapian::doccount get_termfreq_min() const;

    Xapian::doccount get_termfreq_est() const;

    Xapian::doccount get_termfreq_max() const;

    Xapian::docid get_docid() const;

    double get_weight(Xapian::termcount doclen,
		      Xapian::termcount unique_terms,
		      Xapian::termcount wdfdocmax) const;

    Xapian::termcount get_wdfdocmax() const;

    double recalc_maxweight();

    PositionList * read_position_list();

    PostList * next(double w_min);

    PostList * skip_to(Xapian::docid, double w_min);

    PostList * check(Xapian::docid did, double w_min, bool &valid);

    bool at_end() const;

    Xapian::termcount count_matching_subqs() const;

    std::string get_description() const;
};

#endif /* XAPIAN_INCLUDED_EXTERNALPOSTLIST_H */
