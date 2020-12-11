/** @file
 * @brief Return document ids from an external source.
 */
/* Copyright 2008,2009,2011 Olly Betts
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

#include "api/postlist.h"

namespace Xapian {
    class PostingSource;
}

class MultiMatch;

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
     *  @param matcher	The matcher to notify when maximum weight changes.
     */
    ExternalPostList(const Xapian::Database & db,
		     Xapian::PostingSource *source_,
		     double factor_,
		     MultiMatch* matcher,
		     Xapian::doccount shard_index);

    Xapian::doccount get_termfreq_min() const;

    Xapian::doccount get_termfreq_est() const;

    Xapian::doccount get_termfreq_max() const;

    double get_maxweight() const;

    Xapian::docid get_docid() const;

    double get_weight() const;

    Xapian::termcount get_doclength() const;

    Xapian::termcount get_unique_terms() const;

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
