/** @file postingsource.h
 *  @brief PostingSource class
 */
/* Copyright (C) 2007,2008 Olly Betts
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

#ifndef XAPIAN_INCLUDED_POSTINGSOURCE_H
#define XAPIAN_INCLUDED_POSTINGSOURCE_H

#include <xapian/types.h>
#include <xapian/visibility.h>

#include <string>

namespace Xapian {

/// Base class which provides an "external" source of postings.
class XAPIAN_VISIBILITY_DEFAULT PostingSource {
    /// Don't allow assignment.
    void operator=(const PostingSource &);

    /// Don't allow copying.
    PostingSource(const PostingSource &);

  public:
    // Destructor.
    virtual ~PostingSource();

    /// A lower bound on the number of documents this object can return.
    virtual Xapian::doccount get_termfreq_min() const = 0;

    /** An estimate of the number of documents this object can return.
     *
     *  It should always be true that:
     *  get_termfreq_min() <= get_termfreq_est() <= get_termfreq_max()
     */
    virtual Xapian::doccount get_termfreq_est() const = 0;

    /// A upper bound on the number of documents this object can return.
    virtual Xapian::doccount get_termfreq_max() const = 0;

    /** Return an upper bound on what get_weight() can return.
     *
     *  This default implementation always returns 0, for convenience when
     *  implementing "weight-less" PostingSource subclasses.
     */
    virtual Xapian::weight get_maxweight() const;

    /** Return the weight contribution for the current document.
     *
     *  This default implementation always returns 0, for convenience when
     *  implementing "weight-less" PostingSource subclasses.
     */
    virtual Xapian::weight get_weight() const;

    /** Advance the current position to the next matching document.
     *
     *  The PostingSource starts before the first entry in the list, so next()
     *  must be called before any methods which need the context of
     *  the current position.
     *
     *  @param w_min	The minimum weight contribution that is needed (this is
     *			just a hint which subclasses may ignore).
     */
    virtual void next(Xapian::weight) = 0;

    /** Skip forward to the specified docid.
     *
     *  If the specified docid isn't in the list, position ourselves on the
     *  first document after it (or at_end() if no greater docids are present).
     *
     *  @param w_min	The minimum weight contribution that is needed (this is
     *			just a hint which subclasses may ignore).
     *
     *  The default implementation calls next() repeatedly, which works but
     *  skip_to() can often be implemented much more efficiently.
     */
    virtual void skip_to(Xapian::docid, Xapian::weight);

    /** Check if the specified docid occurs.
     *
     *  The caller is required to ensure that the specified @a docid actually
     *  exists in the database.
     *
     *  This method acts like skip_to() if that can be done at little extra
     *  cost, in which case it then sets @a valid to true.
     *
     *  Otherwise it simply checks if a particular docid is present.  If it
     *  is, @a valid is set to true.  If it isn't, it sets @a valid to
     *  false, and leaves the position unspecified (and hence the result of
     *  calling methods which depends on the current position, such as
     *  get_docid(), are also unspecified).  In this state, next() will
     *  advance to the first matching position after @a docid, and skip_to()
     *  will act as it would if the position was the first matching position
     *  after @a docid.
     *
     *  The default implementation calls skip_to() and always sets valid to
     *  true.
     */
    virtual void check(Xapian::docid, Xapian::weight, bool&);

    /// Return true if the current position is past the last entry in this list.
    virtual bool at_end() const = 0;

    /// Return the current docid.
    virtual Xapian::docid get_docid() const = 0;

    /** Reset this PostingSource to its freshly constructed state.
     *
     *  This is called automatically by the matcher prior to each query being
     *  processed.
     */
    virtual void reset() = 0;

    /** Return a string describing this object.
     *
     *  This default implementation returns a generic answer.  This default
     *  it provided to avoid forcing those deriving their own PostingSource
     *  subclass from having to implement this (they may not care what
     *  get_description() gives for their subclass).
     */
    virtual std::string get_description() const;
};

}

#endif // XAPIAN_INCLUDED_POSTINGSOURCE_H
