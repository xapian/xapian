/* omtypes.h: Common types used
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

#ifndef OM_HGUARD_OMTYPES_H
#define OM_HGUARD_OMTYPES_H

/// A term id.  Internal use only.
typedef unsigned int om_termid;

/** A unique id for a document.
 *  Start at 1.  A value of 0 should never occur.
 */
typedef unsigned int om_docid;

/** Type for counts of documents. */
typedef om_docid     om_doccount;

/** Type for counts of terms (eg, wdf, wqf). */
typedef om_termid    om_termcount;

/** Type for term positions within documents.
 *  These start at 1.  A value of 0 means that the positional information
 *  is not available for that term.
 */
typedef unsigned int om_termpos;

/** Type for (normalised) lengths of documents. */
typedef double       om_doclength;

/** Type for sum of lengths of documents. */
typedef double       om_totlength;

/** Type for referring to key in document. */
typedef unsigned int om_keyno;

/** A calculated weight, for a term or document. */
typedef double       om_weight;

/** Type for specifying a timeout.  This refers to a time in microseconds:
 *  ie. a timeout value of 1000000 corresponds to a timeout of 1 second.
 */
typedef unsigned int om_timeout;

#ifdef __cplusplus
#include <string>
#include <list>
/** A term name.  This is a string representing the term, and will often be
 *  the actual text of the term.
 */
typedef std::string om_termname;

/** A list of terms.  This is a container of term names. */
typedef std::list<om_termname> om_termname_list;

/** A document name.  This is used when making a new document. */
typedef std::string om_docname;
#endif

#endif /* OM_HGUARD_OMTYPES_H */
