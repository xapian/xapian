/** @file exactphrasepostlist.h
 * @brief Return docs containing terms forming a particular exact phrase.
 *
 * Copyright (C) 2006 Olly Betts
 * Copyright (C) 2009 Lemur Consulting Ltd
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

#ifndef XAPIAN_INCLUDED_EXACTPHRASEPOSTLIST_H
#define XAPIAN_INCLUDED_EXACTPHRASEPOSTLIST_H

#include "selectpostlist.h"
#include <vector>

/** Postlist which matches an exact phrase using positional information.
 *
 *  ExactPhrasePostList only returns a posting for documents contains
 *  all the terms (this part is implemented using an AndPostList) and
 *  additionally the terms occur somewhere in the document in the order given
 *  and at adjacent term positions.
 *
 *  The weight of a posting is the sum of the weights of the
 *  sub-postings (just like an AndPostList).
 */
class ExactPhrasePostList : public SelectPostList {
    std::vector<PostList*> terms;

    PositionList ** poslists;

    unsigned * order;

    /// Start reading from the i-th position list.
    void start_position_list(unsigned i);

    /// Test if the current document contains the terms as an exact phrase.
    bool test_doc();

  public:
    ExactPhrasePostList(PostList *source_,
			const std::vector<PostList*>::const_iterator &terms_begin,
			const std::vector<PostList*>::const_iterator &terms_end);

    ~ExactPhrasePostList();

    Xapian::termcount get_wdf() const;

    Xapian::doccount get_termfreq_est() const;

    TermFreqs get_termfreq_est_using_stats(
	const Xapian::Weight::Internal & stats) const;

    std::string get_description() const;
};

#endif
