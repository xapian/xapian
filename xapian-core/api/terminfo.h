/** @file
 * @brief Metadata for a term in a document
 */
/* Copyright 2017,2018,2019 Olly Betts
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

/// Metadata for a term in a document
class TermInfo {
    Xapian::termcount wdf;

    /** Split point in the position range.
     *
     *  To allow more efficient insertion of positions, we support the
     *  positions being split into two sorted ranges, and if this is the
     *  case, split will be > 0 and there will be two sorted ranges [0, split)
     *  and [split, positions.size()).
     *
     *  If split is 0, then [0, positions.size()) form a single sorted range.
     *
     *  If positions.empty(), then split > 0 indicates that the term has been
     *  deleted (this allows us to delete terms without invalidating existing
     *  TermIterator objects).
     *
     *  Use type unsigned here to avoid bloating this structure.  More than
     *  4 billion positions in one document is not sensible (and not possible
     *  unless termpos is configured to be 64 bit).
     */
    mutable unsigned split = 0;

    /** Positions at which the term occurs.
     *
     *  The entries are sorted in strictly increasing order (so duplicate
     *  entries are not allowed).
     */
    mutable Xapian::VecCOW<Xapian::termpos> positions;

    /** Merge sorted ranges before and after @a split. */
    void merge() const;

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
	if (split) merge();
	return &positions;
    }

    bool has_positions() const { return !positions.empty(); }

    size_t count_positions() const { return positions.size(); }

    /// Get the within-document frequency.
    Xapian::termcount get_wdf() const { return wdf; }

    /** Increase within-document frequency.
     *
     *  @return true if the term was flagged as deleted before the operation.
     */
    bool increase_wdf(Xapian::termcount delta) {
	if (rare(is_deleted())) {
	    split = 0;
	    wdf = delta;
	    return true;
	}
	wdf += delta;
	return false;
    }

    /** Decrease within-document frequency.
     *
     *  @return true If the adjusted wdf is zero and there are no positions.
     */
    bool decrease_wdf(Xapian::termcount delta) {
	// Saturating arithmetic - don't let the wdf go below zero.
	if (wdf >= delta) {
	    wdf -= delta;
	} else {
	    wdf = 0;
	}
	if (wdf == 0 && positions.empty()) {
	    // Flag term as deleted if no wdf or positions.
	    split = 1;
	    return true;
	}
	return false;
    }

    bool remove() {
	if (is_deleted())
	    return false;
	positions.clear();
	split = 1;
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
    bool remove_position(Xapian::termpos termpos);

    /** Remove a range of positions.
     *
     *  @param termpos_first	First position to remove
     *  @param termpos_last	Last position to remove
     *
     *  It's OK if there are no positions in the specified range.
     *
     *  @return the number of positions removed.
     */
    Xapian::termpos remove_positions(Xapian::termpos termpos_first,
				     Xapian::termpos termpos_last);

    /** Has this term been deleted from this document?
     *
     *  We flag entries as deleted instead of actually deleting them to avoid
     *  invalidating existing TermIterator objects.
     */
    bool is_deleted() const { return positions.empty() && split > 0; }
};

#endif // XAPIAN_INCLUDED_TERMINFO_H
