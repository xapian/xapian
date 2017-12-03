/** @file documentterm.h
 * @brief internal class representing a term in a modified document
 */
/* Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2003,2007 Olly Betts
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

using namespace std;

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

    /** Flag to indicate if this term was deleted from this document.
     *
     *  We flag entries as deleted instead of actually deleting them to avoid
     *  invalidating existing TermIterator objects.
     */
    bool deleted = false;

    typedef vector<Xapian::termpos> term_positions;

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
    term_positions positions;

    void remove() {
	positions.clear();
	deleted = true;
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

    /// Get the wdf
    Xapian::termcount get_wdf() const { return wdf; }

    /// Is this term flagged as deleted?
    bool is_deleted() const { return deleted; }

    /// Return a string describing this object.
    string get_description() const;
};

#endif // OM_HGUARD_DOCUMENTTERM_H
