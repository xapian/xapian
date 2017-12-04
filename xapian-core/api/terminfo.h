/** @file terminfo.h
 * @brief Metadata for a term in a document
 */
/* Copyright 2017 Olly Betts
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

#ifndef XAPIAN_INCLUDED_TERMINFO_H
#define XAPIAN_INCLUDED_TERMINFO_H

#include "api/smallvector.h"
#include <xapian/types.h>

using namespace std;

/// Metadata for a term in a document
class TermInfo {
    Xapian::termcount wdf;

    /** Flag to indicate if this term was deleted from this document.
     *
     *  We flag entries as deleted instead of actually deleting them to avoid
     *  invalidating existing TermIterator objects.
     */
    bool deleted = false;

    /** Positions at which the term occurs.
     *
     *  The entries are sorted in strictly increasing order (so duplicate
     *  entries are not allowed).
     */
    Xapian::VecCOW<Xapian::termpos> positions;

  public:
    /** Constructor.
     *
     *  @param wdf_   Within-document frequency
     */
    explicit TermInfo(Xapian::termcount wdf_) : wdf(wdf_) {}

    /** Constructor which also adds an initial position.
     *
     *  @param wdf_   Within-document frequency
     *  @param termpos	Position to add
     */
    TermInfo(Xapian::termcount wdf_, Xapian::termpos termpos) : wdf(wdf_) {
	positions.push_back(termpos);
    }

    /// Get a pointer to the positions.
    const Xapian::VecCOW<Xapian::termpos>* get_positions() const {
	return &positions;
    }

    /// Get the within-document frequency.
    Xapian::termcount get_wdf() const { return wdf; }

    /** Increase within-document frequency.
     *
     *  @return true if the term was flagged as deleted before the operation.
     */
    bool increase_wdf(Xapian::termcount delta) {
	if (rare(deleted)) {
	    deleted = false;
	    wdf = delta;
	    return true;
	}
	wdf += delta;
	return false;
    }

    /// Decrease within-document frequency.
    void decrease_wdf(Xapian::termcount delta) {
	// Saturating arithmetic - don't let the wdf go below zero.
	if (wdf >= delta) {
	    wdf -= delta;
	} else {
	    wdf = 0;
	}
    }

    bool remove() {
	if (deleted)
	    return false;
	positions.clear();
	deleted = true;
	return true;
    }

    /** Add a position.
     *
     *  If @a termpos is already present, this is a no-op.
     *
     *  @param wdf_inc  wdf increment
     *  @param termpos	Position to add
     *
     *  @return true if the term was flagged as deleted before the operation.
     */
    bool add_position(Xapian::termcount wdf_inc, Xapian::termpos termpos);

    /** Append a position.
     *
     *  The position must be >= the largest currently in the list.
     */
    void append_position(Xapian::termpos termpos) {
	positions.push_back(termpos);
    }

    /** Remove a position.
     *
     *  @param termpos	Position to remove
     *
     *  @return If @a termpos wasn't present, returns false.
     */
    bool remove_position(Xapian::termpos tpos);

    /// Is this term flagged as deleted?
    bool is_deleted() const { return deleted; }
};

#endif // XAPIAN_INCLUDED_TERMINFO_H
