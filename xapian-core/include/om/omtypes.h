/** \file omtypes.h
 * \brief Common types used
 */
/*
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2002 Olly Betts
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

/** A unique id for a document.
 *  Document ids start at 1.  A zero docid isn't valid, and may be used to
 *  indicate "no document".
 */
typedef unsigned int om_docid;

/** Type for counts of documents. */
typedef om_docid     om_doccount;

/** Type for signed difference between counts of documents. */
typedef int	     om_doccount_diff;

/** Type for counts of terms (eg, wdf, wqf). */
typedef unsigned int om_termcount;

/** Type for signed difference between counts of terms. */
typedef int	     om_termcount_diff;

/** Type for term positions within documents.
 *  These start at 1.  A value of 0 means that the positional information
 *  is not available for that term.
 */
typedef unsigned int om_termpos;

/** Type for signed difference between term positions. */
typedef int	     om_termpos_diff;

/** Type for (normalised) lengths of documents. */
typedef double       om_doclength;

/** Type for referring to the number of a value in document. */
typedef unsigned int om_valueno;

/** Type for signed difference between two om_valueno-s. */
typedef int	     om_valueno_diff;

/** A calculated weight, for a term or document. */
typedef double       om_weight;

/** A percentage weight, for a term or document. */
typedef int	     om_percent;

/** Type for specifying a timeout.  This refers to a time in microseconds:
 *  ie. a timeout value of 1000000 corresponds to a timeout of 1 second.
 */
typedef unsigned int om_timeout;

#ifdef __cplusplus
#include <string>
/** A term name.  This is a string representing the term, and will often be
 *  the actual text of the term.
 */
typedef std::string om_termname;
#endif

#endif /* OM_HGUARD_OMTYPES_H */
