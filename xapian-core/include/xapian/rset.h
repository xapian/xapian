/** @file
 *  @brief Set of documents judged as relevant
 */
/* Copyright (C) 2015,2016,2017 Olly Betts
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301
 * USA
 */

#ifndef XAPIAN_INCLUDED_RSET_H
#define XAPIAN_INCLUDED_RSET_H

#if !defined XAPIAN_IN_XAPIAN_H && !defined XAPIAN_LIB_BUILD
# error Never use <xapian/rset.h> directly; include <xapian.h> instead.
#endif

#include <xapian/attributes.h>
#include <xapian/intrusive_ptr.h>
#include <xapian/mset.h>
#include <xapian/types.h>
#include <xapian/visibility.h>

namespace Xapian {

class ESetIterator;

/// Class representing a set of documents judged as relevant.
class XAPIAN_VISIBILITY_DEFAULT RSet {
  public:
    /// Class representing the RSet internals.
    class Internal;
    /// @private @internal Reference counted internals.
    Xapian::Internal::intrusive_ptr<Internal> internal;

    /** Copying is allowed.
     *
     *  The internals are reference counted, so copying is cheap.
     */
    RSet(const RSet & o);

    /** Copying is allowed.
     *
     *  The internals are reference counted, so assignment is cheap.
     */
    RSet & operator=(const RSet & o);

    /// Move constructor.
    RSet(RSet && o);

    /// Move assignment operator.
    RSet & operator=(RSet && o);

    /** Default constructor.
     *
     *  Creates an empty RSet.
     */
    RSet();

    /** @private @internal Wrap an existing Internal. */
    XAPIAN_VISIBILITY_INTERNAL
    explicit RSet(Internal* internal_);

    /// Destructor.
    ~RSet();

    /** Return number of documents in this RSet object. */
    Xapian::doccount size() const;

    /** Return true if this RSet object is empty. */
    bool empty() const { return size() == 0; }

    /** Efficiently swap this RSet object with another. */
    void swap(RSet & o) { internal.swap(o.internal); }

    /** Mark a document as relevant.
     *
     *  If @a did is already marked as relevant, nothing happens.
     */
    void add_document(Xapian::docid did);

    /** Mark a document as relevant.
     *
     *  If @a did is already marked as relevant, nothing happens.
     */
    void add_document(const Xapian::MSetIterator& it) {
	add_document(*it);
    }

    /** Unmark a document as relevant.
     *
     *  If @a did is not marked as relevant, nothing happens.
     */
    void remove_document(Xapian::docid did);

    /** Unmark a document as relevant.
     *
     *  If @a did is not marked as relevant, nothing happens.
     */
    void remove_document(const Xapian::MSetIterator& it) {
	remove_document(*it);
    }

    /// Check if a document is marked as relevant.
    bool contains(Xapian::docid did) const;

    /// Check if a document is marked as relevant.
    bool contains(const Xapian::MSetIterator& it) const {
	return contains(*it);
    }

    /// Return a string describing this object.
    std::string get_description() const;
};

}

#endif // XAPIAN_INCLUDED_RSET_H
