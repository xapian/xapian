/** @file
 * @brief internal class representing a term in a modified document
 */
/* Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2003,2007,2018 Olly Betts
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

#ifndef OM_HGUARD_DOCUMENTTERM_H
#define OM_HGUARD_DOCUMENTTERM_H

#include "debuglog.h"

#include <string>
#include <vector>

#include <xapian/types.h>

using std::string;
using std::vector;

/// A term in a document.
class OmDocumentTerm {
    public:
    /** Make a new term.
     *
     *  @param wdf_   Initial wdf.
     */
    explicit OmDocumentTerm(Xapian::termcount wdf_)
	: wdf(wdf_)
    {
	LOGCALL_CTOR(DB, "OmDocumentTerm", wdf_);
    }

    /** Within document frequency of the term.
     *  This is the number of occurrences of the term in the document.
     */
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

    /** Merge sorted ranges before and after @a split. */
    void merge() const;

    typedef vector<Xapian::termpos> term_positions;

  private:
    /** Positional information.
     *
     *  This is a list of positions at which the term occurs in the
     *  document. The list is in strictly increasing order of term
     *  position.
     *
     *  The positions start at 1.
     *
     *  Note that, even if positional information is present, the WDF might
     *  not be equal to the length of the position list, since a term might
     *  occur multiple times at a single position, but will only have one
     *  entry in the position list for each position.
     */
    mutable term_positions positions;

  public:
    const term_positions* get_vector_termpos() const {
	merge();
	return &positions;
    }

    Xapian::termcount positionlist_count() const {
	return positions.size();
    }

    void remove() {
	positions.clear();
	split = 1;
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

    /** Remove an entry from the position list.
     *
     *  This removes an entry from the list of positions.
     *
     *  This does not change the value of the wdf.
     *
     *  @exception Xapian::InvalidArgumentError is thrown if the position does
     *  not occur in the position list.
     */
    void remove_position(Xapian::termpos tpos);

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

    /// Decrease within-document frequency.
    void decrease_wdf(Xapian::termcount delta) {
	// Saturating arithmetic - don't let the wdf go below zero.
	if (wdf >= delta) {
	    wdf -= delta;
	} else {
	    wdf = 0;
	}
    }

    /// Get the wdf
    Xapian::termcount get_wdf() const { return wdf; }

    /** Has this term been deleted from this document?
     *
     *  We flag entries as deleted instead of actually deleting them to avoid
     *  invalidating existing TermIterator objects.
     */
    bool is_deleted() const { return positions.empty() && split > 0; }

    /// Return a string describing this object.
    string get_description() const;
};

#endif // OM_HGUARD_DOCUMENTTERM_H
