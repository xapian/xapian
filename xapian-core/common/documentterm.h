/* documentterm.h: internal class representing a term in a modified document
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2003 Olly Betts
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 * -----END-LICENCE-----
 */

#ifndef OM_HGUARD_DOCUMENTTERM_H
#define OM_HGUARD_DOCUMENTTERM_H

#include <string>
#include <xapian/types.h>

using namespace std;

/// A term in a document.
class OmDocumentTerm {
    public:
    /** Make a new term.
     *
     *  @param tname_ The name of the new term.
     */
    OmDocumentTerm(const string & tname_) : tname(tname_), wdf(0), termfreq(0)
    {
	DEBUGAPICALL(void, "OmDocumentTerm::OmDocumentTerm", tname_);
    }

    /** The name of this term.
     */
    string tname;

    /** Within document frequency of the term.
     *  This is the number of occurrences of the term in the document.
     */
    Xapian::termcount wdf;

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

    /** Term frequency information.
     *
     *  This is the number of documents indexed by the term.
     *
     *  If the information is not available, the value will be 0.
     */
    Xapian::doccount termfreq;

    /** Add a position to the posting list.
     *
     *  This adds an entry to the list of positions, unless
     *  there is already one for the specified position.
     *
     *  This does not change the value of the wdf.
     *
     *  @param tpos The position within the document at which the term
     *              occurs.
     */
    void add_position(Xapian::termpos tpos);

    /** Remove an entry from the posting list.
     *
     *  This removes an entry from the list of positions.
     *
     *  This does not change the value of the wdf.
     *
     *  @exception Xapian::InvalidArgumentError is thrown if the 
     */
    void remove_position(Xapian::termpos tpos);

    /// Set the wdf
    void set_wdf(Xapian::termcount wdf_) { wdf = wdf_; }

    /// Get the wdf
    Xapian::termcount get_wdf() { return wdf; }

    /** Returns a string representing the OmDocumentTerm.
     *  Introspection method.
     */
    string get_description() const;
};

#endif // OM_HGUARD_DOCUMENTTERM_H
