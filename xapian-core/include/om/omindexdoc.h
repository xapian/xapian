/* omindexdoc.h: Representation of document in indexes
 *
 * ----START-LICENCE----
 * Copyright 1999,2000 BrightStation PLC
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

#ifndef OM_HGUARD_OMINDEXDOC_H
#define OM_HGUARD_OMINDEXDOC_H

#include <string>
#include <vector>
#include <map>
#include <om/omtypes.h>
#include <om/omdocument.h>

///////////////////////////////////////////////////////////////////
// OmDocumentContents class
// ========================

/** The information which is stored in a document.
 *
 *  This object contains all the information associated with a document,
 *  and can be used to build up that information and then add it to a
 *  writable database.
 */
struct OmDocumentContents {
    /** The (user defined) data associated with this document. */
    OmData data;

    /** Type to store keys in. */
    typedef std::map<om_keyno, OmKey> document_keys;

    /** The keys associated with this document. */
    document_keys keys;

    /** Type to store terms in. */
    typedef std::map<om_termname, OmDocumentTerm> document_terms;

    /** The terms (and their frequencies and positions) in this document. */
    document_terms terms;

    /** Add an occurrence of a term to the document.
     *
     *  Multiple occurrences of the term at the same position are represented
     *  only once in the positional information, but do increase the wdf.
     *
     *  @param tname  The name of the term.
     *  @param tpos   The position of the term.
     */
    void add_posting(const om_termname & tname, om_termpos tpos = 0);

    /** Returns a string representing the OmDocumentContents.
     *  Introspection method.
     */
    std::string get_description() const;
};

#endif /* OM_HGUARD_OMINDEXDOC_H */
