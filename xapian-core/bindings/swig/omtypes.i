/* omtypes.i : Common types used
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
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

/// A term id.  Internal use only.
typedef unsigned int termid;

/** A unique id for a document.
 *  Start at 1.  A value of 0 should never occur.
 */
typedef unsigned int docid;

/// Type for counts of documents
typedef docid     doccount;

/// Type for counts of terms (eg, wdf, wqf, termfreq).
typedef termid    termcount;

/** Type for term positions within documents.
 *  These start at 1.  A value of 0 means that the positional information
 *  is not available for that term.
 */
typedef unsigned int termpos;

/// Type for (normalised) lengths of documents
typedef double       doclength;

/// Type for sum of lengths of documents
typedef double       totlength;

/// Type for referring to value in document
typedef unsigned int valueno;

/// A calculated weight, for a term or document
typedef double       weight;

/** Type for specifying a timeout.  This refers to a time in microseconds:
 *  ie. a timeout value of 1000000 corresponds to a timeout of 1 second.
 */
typedef unsigned int timeout;

/// A term name.  This is a string representing the term, and will often be the actual text of the term.
typedef string termname;

/// A list of terms.  This is a container of term names.
//typedef std::list<termname> termname_list;

/// A document name.  This is used when making a new document.
typedef std::string docname;

typedef int	percent;
