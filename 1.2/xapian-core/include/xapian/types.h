/** @file xapian/types.h
 *  @brief typedefs for Xapian
 */
/* Copyright (C) 2007,2010 Olly Betts
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

#ifndef XAPIAN_INCLUDED_TYPES_H
#define XAPIAN_INCLUDED_TYPES_H

namespace Xapian {

/** A count of documents.
 *
 *  This is used to hold values such as the number of documents in a database
 *  and the frequency of a term in the database.
 */
typedef unsigned doccount;

/** A signed difference between two counts of documents.
 *
 *  This is used by the Xapian classes which are STL containers of documents
 *  for "difference_type".
 */
typedef int doccount_diff; /* FIXME: can overflow with more than 2^31 docs. */

/** A unique identifier for a document.
 *
 *  Docid 0 is invalid, providing an "out of range" value which can be
 *  used to mean "not a valid document".
 */
typedef unsigned docid;

/** A normalised document length.
 *
 *  The normalised document length is the document length divided by the
 *  average document length in the database.
 */
typedef double doclength;

/** The percentage score for a document in an MSet. */
typedef int percent;

/** A counts of terms.
 *
 *  This is used to hold values such as the Within Document Frequency (wdf).
 */
typedef unsigned termcount;

/** A signed difference between two counts of terms.
 *
 *  This is used by the Xapian classes which are STL containers of terms
 *  for "difference_type".
 */
typedef int termcount_diff; /* FIXME: can overflow with more than 2^31 terms. */

/** A term position within a document or query.
 */
typedef unsigned termpos;

/** A signed difference between two term positions.
 *
 *  This is used by the Xapian classes which are STL containers of positions
 *  for "difference_type".
 */
typedef int termpos_diff; /* FIXME: can overflow. */

/** A timeout value in milliseconds.
 *
 *  There are 1000 milliseconds in a second, so for example, to set a
 *  timeout of 5 seconds use 5000.
 */
typedef unsigned timeout;

/** The number for a value slot in a document.
 *
 *  Value slot numbers are unsigned and (currently) a 32-bit quantity, with
 *  Xapian::BAD_VALUENO being represented by the largest possible value.
 *  Therefore value slots 0 to 0xFFFFFFFE are available for use.
 */
typedef unsigned valueno;

/** A signed difference between two value slot numbers.
 *
 *  This is used by the Xapian classes which are STL containers of values
 *  for "difference_type".
 */
typedef int valueno_diff; /* FIXME: can overflow. */

/** The weight of a document or term. */
typedef double weight;

/** Reserved value to indicate "no valueno". */
const valueno BAD_VALUENO = static_cast<valueno>(-1);

}

#endif /* XAPIAN_INCLUDED_TYPES_H */
