/* document_contents.h
 *
 * ----START-LICENCE----
 * Copyright 1999,2000 Dialog Corporation
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

#ifndef OM_HGUARD_DOCUMENT_CONTENTS_H
#define OM_HGUARD_DOCUMENT_CONTENTS_H

#include <string>
#include "om/omtypes.h"

/** A term in a document. */
struct DocumentTerm {
    /** The name of this term.
     */
    om_termname tname;

    /** Term frequency of this item.  The term frequency is the number of
     *  documents which contain the term. 
     */
    om_termcount wdf;

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
    vector<om_termpos> positions
};

/** The information which is stored in a document.
 *
 *  This object contains all the information associated with a document,
 *  and can be used to build up that information and then add it to a
 *  writable database.
 */
struct DocumentContents {

    string data;
    map<om_keyno, string> keys;
    vector<DocumentTerm> terms;
};

#endif /* OM_HGUARD_DOCUMENT_CONTENTS_H */
