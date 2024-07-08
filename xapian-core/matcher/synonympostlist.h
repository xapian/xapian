/** @file
 * @brief Combine subqueries, weighting as if they are synonyms
 */
/* Copyright 2007,2009 Lemur Consulting Ltd
 * Copyright 2009,2011,2014,2017,2018,2024 Olly Betts
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

#ifndef XAPIAN_INCLUDED_SYNONYMPOSTLIST_H
#define XAPIAN_INCLUDED_SYNONYMPOSTLIST_H

#include "wrapperpostlist.h"

#include "backends/databaseinternal.h"

class PostListTree;

/** A postlist comprising several postlists SYNONYMed together.
 *
 *  This postlist returns all postings in the OR of the sub postlists, but
 *  returns weights as if they represented a single term.  The term frequency
 *  portion of the weight is approximated.
 *
 *  The wrapped postlist starts as an OR of all the sub-postlists being
 *  joined with Synonym, but may decay into something else.
 */
class SynonymPostList : public WrapperPostList {
    /// Weighting object used for calculating the synonym weights.
    const Xapian::Weight* wt = nullptr;

    /// Flag indicating whether the weighting object needs the wdf.
    bool want_wdf = false;

    /// Flag indicating whether the weighting object needs the wdfdocmax.
    bool want_wdfdocmax = false;

    /** Does the synonym need the document length?
     *
     *  This is true if either of these are true:
     *
     *  Each wdf from the document contributes at most itself to the wdf of
     *  the subquery.  That means that the wdf of the subquery can't possibly
     *  ever exceed the document length, so we don't need to check and clamp
     *  wdf to be <= document length.
     *
     *  Or the weighting scheme in use doesn't use document length, in which
     *  case we assume that it doesn't rely on wdf <= document_length being
     *  an invariant, and so we don't enforce it.
     */
    bool needs_doclen;

    PostListTree* pltree;

  public:
    SynonymPostList(PostList * subtree,
		    PostListTree* pltree_,
		    bool needs_doclen_)
	: WrapperPostList(subtree),
	  needs_doclen(needs_doclen_),
	  pltree(pltree_) { }

    ~SynonymPostList();

    /** Set the weight object to be used for the synonym postlist.
     *
     *  Ownership of the weight object passes to the synonym postlist - the
     *  caller must not delete it after use.
     */
    void set_weight(const Xapian::Weight * wt_);

    PostList *next(double w_min);
    PostList *skip_to(Xapian::docid did, double w_min);

    double get_weight(Xapian::termcount doclen,
		      Xapian::termcount unique_terms,
		      Xapian::termcount wdfdocmax) const;
    double recalc_maxweight();

    Xapian::termcount count_matching_subqs() const;

    std::string get_description() const;
};

#endif /* XAPIAN_INCLUDED_SYNONYMPOSTLIST_H */
