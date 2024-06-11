/** @file
 * @brief Abstract base class for leaf postlists.
 */
/* Copyright (C) 2007-2024 Olly Betts
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
#include <string_view>

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
    const Xapian::Weight* weight = nullptr;

    /// The term name for this postlist (empty for an alldocs postlist).
    std::string term;

    /** The collection frequency of the term.
     *
     *  This is the sum of wdf values for the term in all documents.
     */
    Xapian::termcount collfreq;

    /// Only constructable as a base class for derived classes.
    explicit LeafPostList(std::string_view term_)
	: term(term_) { }

  public:
    ~LeafPostList();

    /** Get the collection frequency of the term.
     *
     *  This is the sum of wdf values for the term in all documents.
     */
    Xapian::termcount get_collfreq() const { return collfreq; }

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
				   double factor,
				   const Xapian::Database::Internal* shard)
    {
	weight_->init_(*stats, qlen, term, wqf, factor, shard, this);
	// There should be an existing LazyWeight set already.
	Assert(weight);
	const Xapian::Weight * const_weight_ = weight_;
	std::swap(weight, const_weight_);
	delete const_weight_;
	// We get such terms from the database so they should exist.
	Assert(get_termfreq() > 0);
	double result = weight->get_maxpart();
	double& max_part = stats->termfreqs[term].max_part;
	max_part = std::max(max_part, result);
	return result;
    }

    double get_weight(Xapian::termcount doclen,
		      Xapian::termcount unique_terms,
		      Xapian::termcount wdfdocmax) const;

    double recalc_maxweight();

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
     *  @param[out] pl  If true is returned, set to a new LeafPostList object
     *			(or may be set to NULL if the term doesn't index any
     *			documents).  The caller takes ownership of the returned
     *			object.
     *
     *  @return		true if successful (and pl has been set); false if not
     *			(in which case the caller should probably open the
     *			postlist via the database instead).
     */
    virtual bool open_nearby_postlist(std::string_view term_,
				      bool need_read_pos,
				      LeafPostList*& pl) const;

    virtual Xapian::termcount get_wdf_upper_bound() const = 0;

    /** Get the term name. */
    const std::string& get_term() const { return term; }

    /** Set the term name.
     *
     *  This is used when we optimise a term matching all documents to an
     *  all documents postlist so that postlist reports the correct termname.
     */
    void set_term(std::string_view term_) { term = term_; }
};

#endif // XAPIAN_INCLUDED_LEAFPOSTLIST_H
