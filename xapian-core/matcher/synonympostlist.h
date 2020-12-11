/** @file
 * @brief Combine subqueries, weighting as if they are synonyms
 */
/* Copyright 2007,2009 Lemur Consulting Ltd
 * Copyright 2009,2011,2014,2017,2018 Olly Betts
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
    const Xapian::Weight * wt;

    /// Flag indicating whether the weighting object needs the wdf.
    bool want_wdf;

    /// Flag indicating whether the weighting object needs the wdfdocmax.
    bool want_wdfdocmax;

    /** Are the subquery's wdf contributions disjoint?
     *
     *  This is true is each wdf from the document contributes at most itself
     *  to the wdf of the subquery.  That means that the wdf of the subquery
     *  can't possibly ever exceed the document length, so we can avoid the
     *  need to check and clamp wdf to be <= document length.
     */
    bool wdf_disjoint;

    PostListTree* pltree;

    /// Lower bound on doclength in the subdatabase we're working over.
    Xapian::termcount doclen_lower_bound;

  public:
    SynonymPostList(PostList * subtree,
		    const Xapian::Database::Internal* db,
		    PostListTree* pltree_,
		    bool wdf_disjoint_)
	: WrapperPostList(subtree), wt(NULL), want_wdf(false),
	  want_wdfdocmax(false), wdf_disjoint(wdf_disjoint_),
	  pltree(pltree_),
	  doclen_lower_bound(db->get_doclength_lower_bound()) { }

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

    // Note - we don't need to implement get_termfreq_est_using_stats()
    // because a synonym when used as a child of a synonym will be optimised
    // to an OR.

    Xapian::termcount count_matching_subqs() const;

    std::string get_description() const;
};

#endif /* XAPIAN_INCLUDED_SYNONYMPOSTLIST_H */
