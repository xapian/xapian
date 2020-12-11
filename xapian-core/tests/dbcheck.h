/** @file
 * @brief test database contents and consistency.
 */
/* Copyright (C) 2010 Richard Boulton
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
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

#ifndef XAPIAN_INCLUDED_DBCHECK_H
#define XAPIAN_INCLUDED_DBCHECK_H

#include <xapian.h>
#include <string>

/** Convert the list of positions in a positionlist to a string.
 *
 *  @param it The start iterator for the positionlist.  This will be moved.
 *  @param end The end iterator for the positionlist.
 *  @param count If not NULL, a pointer to a value which will be set to the
 *               number of positions found.
 */
std::string
positions_to_string(Xapian::PositionIterator & it,
		    const Xapian::PositionIterator & end,
		    Xapian::termcount * count = NULL);

/** Convert the list of postings in a postlist to a string.
 *
 *  This will include positional information when such information is present.
 */
std::string
postlist_to_string(const Xapian::Database & db, const std::string & tname);

/** Convert the list of terms in a document to a string.
 *
 *  This will include positional information when such information is present.
 */
std::string
docterms_to_string(const Xapian::Database & db, Xapian::docid did);

/// Convert statistics about a document to a string.
std::string
docstats_to_string(const Xapian::Database & db, Xapian::docid did);

/// Convert statistics about a term to a string.
std::string
termstats_to_string(const Xapian::Database & db, const std::string & term);

/** Check consistency of database and statistics.
 *
 *  Raises a TestFail exception if the database is inconsistent.
 */
void
dbcheck(const Xapian::Database & db,
	Xapian::doccount expected_doccount,
	Xapian::docid expected_lastdocid);

#endif /* XAPIAN_INCLUDED_DBCHECK_H */
