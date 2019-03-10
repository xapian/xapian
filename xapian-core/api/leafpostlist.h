/** @file leafpostlist.h
 * @brief Abstract base class for leaf postlists.
 */
/* Copyright (C) 2007,2009,2011,2013,2015,2016,2017 Olly Betts
 * Copyright (C) 2009 Lemur Consulting Ltd
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

#ifndef XAPIAN_INCLUDED_LEAFPOSTLIST_H
#define XAPIAN_INCLUDED_LEAFPOSTLIST_H

#include "postlist.h"

#include <string>

namespace Xapian {
    class Weight;
}

/** Abstract base class for leaf postlists.
 *
 *  This class provides the following features in addition to the PostList
 *  class:
 */
class LeafPostList : public PostList {
    /// Don't allow assignment.
    void operator=(const LeafPostList &) = delete;

    /// Don't allow copying.
    LeafPostList(const LeafPostList &) = delete;

  protected:
    const Xapian::Weight * weight;

    /// The term name for this postlist (empty for an alldocs postlist).
    std::string term;

    /// Only constructable as a base class for derived classes.
    explicit LeafPostList(const std::string & term_)
	: weight(0), term(term_) { }

  public:
    ~LeafPostList();

    /** Set the weighting scheme to use during matching.
     *
     *  If this isn't called, get_weight() and recalc_maxweight() will both
     *  return 0.
     *
     *  You should not call this more than once on a particular object.
     *
     *  @param weight_	The weighting object to use.  Must not be NULL.
     */
    void set_termweight(const Xapian::Weight * weight_) {
	// This method shouldn't be called more than once on the same object.
	Assert(!weight);
	weight = weight_;
    }

    double resolve_lazy_termweight(Xapian::Weight * weight_,
				   Xapian::Weight::Internal * stats,
				   Xapian::termcount qlen,
				   Xapian::termcount wqf,
				   double factor)
    {
	weight_->init_(*stats, qlen, term, wqf, factor);
	// There should be an existing LazyWeight set already.
	Assert(weight);
	const Xapian::Weight * const_weight_ = weight_;
	std::swap(weight, const_weight_);
	delete const_weight_;
	stats->termfreqs[term].max_part += weight->get_maxpart();
	return stats->termfreqs[term].max_part;
    }

    /** Return the exact term frequency.
     *
     *  Leaf postlists have an exact termfreq, which get_termfreq_min(),
     *  get_termfreq_max(), and get_termfreq_est() all report.
     */
    virtual Xapian::doccount get_termfreq() const = 0;

    Xapian::doccount get_termfreq_min() const;
    Xapian::doccount get_termfreq_max() const;
    Xapian::doccount get_termfreq_est() const;

    double get_weight(Xapian::termcount doclen,
		      Xapian::termcount unique_terms) const;

    double recalc_maxweight();

    TermFreqs get_termfreq_est_using_stats(
	const Xapian::Weight::Internal & stats) const;

    Xapian::termcount count_matching_subqs() const;

    void gather_position_lists(OrPositionList* orposlist);

    /** Open another postlist from the same database.
     *
     *  @param term_	The term to open a postlist for (must not be an empty
     *			string).  If term_ is near to this postlist's term,
     *			then this can be a lot more efficient (and if it isn't
     *			very near, there's not much of a penalty).  Using this
     *			method can make a wildcard expansion much more memory
     *			efficient.
     *
     *  @param need_read_pos
     *			Does the postlist need to support read_position_list()?
     *			Note that open_position_list() may still be called even
     *			if need_read_pos is false.
     *
     *  @return		The new postlist object, or NULL if not supported
     *			(in which case the caller should probably open the
     *			postlist via the database instead).
     */
    virtual LeafPostList * open_nearby_postlist(const std::string & term_,
						bool need_read_pos) const;

    /** Set the term name.
     *
     *  This is useful when we optimise a term matching all documents to an
     *  all documents postlist under OP_SYNONYM, as the term name is used by
     *  LeafPostList::get_termfreq_est_using_stats() to locate the appropriate
     *  TermFreqs object.
     */
    void set_term(const std::string & term_) { term = term_; }
};

#endif // XAPIAN_INCLUDED_LEAFPOSTLIST_H
