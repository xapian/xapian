/** @file postlist.h
 * @brief Abstract base class for postlists.
 */
/* Copyright (C) 2007 Olly Betts
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

#ifndef XAPIAN_INCLUDED_POSTLIST_H
#define XAPIAN_INCLUDED_POSTLIST_H

#include <string>

#include <xapian/base.h>
#include <xapian/types.h>
#include <xapian/postingiterator.h>

#include "positionlist.h"

/// Abstract base class for postlists.
class Xapian::PostingIterator::Internal : public Xapian::Internal::RefCntBase {
    /// Don't allow assignment.
    void operator=(const Internal &);

    /// Don't allow copying.
    Internal(const Internal &);

  protected:
    /// Only constructable as a base class for derived classes.
    Internal() { }

  public:
    /** We have virtual methods and want to be able to delete derived classes
     *  using a pointer to the base class, so we need a virtual destructor.
     */
    virtual ~Internal();

    /// Get a lower bound on the number of documents indexed by this term.
    virtual Xapian::doccount get_termfreq_min() const = 0;

    /// Get an upper bound on the number of documents indexed by this term.
    virtual Xapian::doccount get_termfreq_max() const = 0;

    /** Get an estimate of the number of documents indexed by this term.
     *
     *  It should always be true that:
     *  get_termfreq_min() <= get_termfreq_est() <= get_termfreq_max()
     */
    virtual Xapian::doccount get_termfreq_est() const = 0;

    /// Return an upper bound on what get_weight() can return.
    virtual Xapian::weight get_maxweight() const = 0;

    /// Return the current docid.
    virtual Xapian::docid get_docid() const = 0;

    /// Return the length of current document.
    virtual Xapian::doclength get_doclength() const = 0;

    /// Return the wdf for the document at the current position.
    virtual Xapian::termcount get_wdf() const;

    /// Return the weight contribution for the current position.
    virtual Xapian::weight get_weight() const = 0;

    /** If the collapse key is already known, return it.
     *
     *  This is implemented by MSetPostList (and MergePostList).  Other
     *  subclasses rely on the default implementation which just returns
     *  NULL.
     */
    virtual const std::string * get_collapse_key() const;

    /// Return true if the current position is past the last entry in this list.
    virtual bool at_end() const = 0;

    /** Recalculate the upper bound on what get_weight() can return.
     *
     *  If the tree has pruned, get_maxweight() may use cached values.  Calling
     *  this method instead forces a full recalculation.
     */
    virtual Xapian::weight recalc_maxweight() = 0;

    /** Read the position list for the term in the current document and
     *  return a pointer to it (owned by the PostList).
     */
    virtual PositionList * read_position_list() = 0;

    /** Read the position list for the term in the current document and
     *  return a pointer to it (not owned by the PostList).
     */
    virtual PositionList * open_position_list() const = 0;

    /** Advance the current position to the next document in the postlist.
     *
     *  The list starts before the first entry in the list, so next()
     *  must be called before any methods which need the context of
     *  the current position.
     *
     *  @param w_min	The minimum weight contribution that is needed (this is
     *			just a hint which PostList subclasses may ignore).
     *
     *  @return	If a non-NULL pointer is returned, then the caller should
     *		substitute the returned pointer for its pointer to us, and then
     *		delete us.  This "pruning" can only happen for a non-leaf
     *		subclass of this class.
     */
    virtual Internal *next(Xapian::weight w_min) = 0;

    /** Skip forward to the specified docid.
     *
     *  If the specified docid isn't in the list, position ourselves on the
     *  first term after it (or at_end() if no greater docids are present).
     *
     *  @param w_min	The minimum weight contribution that is needed (this is
     *			just a hint which PostList subclasses may ignore).
     *
     *  @return	If a non-NULL pointer is returned, then the caller should
     *		substitute the returned pointer for its pointer to us, and then
     *		delete us.  This "pruning" can only happen for a non-leaf
     *		subclass of this class.
     */
    virtual Internal *skip_to(Xapian::docid, Xapian::weight w_min) = 0;

    /** Advance the current position to the next document in the postlist.
     *
     *  Any weight contribution is acceptable.
     */
    Internal *next() { return next(0.0); }

    /** Skip forward to the specified docid.
     *
     *  Any weight contribution is acceptable.
     */
    Internal *skip_to(Xapian::docid did) { return skip_to(did, 0.0); }

    /// Return a string description of this object.
    virtual std::string get_description() const = 0;
};

// In the external API headers, this class is Xapian::PostingIterator::Internal,
// but in the library code it's still know as "PostList" in most places.
typedef Xapian::PostingIterator::Internal PostList;

#endif // XAPIAN_INCLUDED_POSTLIST_H
