/** @file constants.h
 * @brief Constants in the Xapian namespace
 */
/* Copyright (C) 2012,2013 Olly Betts
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

#ifndef XAPIAN_INCLUDED_CONSTANTS_H
#define XAPIAN_INCLUDED_CONSTANTS_H

#if !defined XAPIAN_INCLUDED_XAPIAN_H && !defined XAPIAN_LIB_BUILD
# error "Never use <xapian/constants.h> directly; include <xapian.h> instead."
#endif

namespace Xapian {

/** Create database if it doesn't already exist.
 *
 *  If no opening mode is specified, this is the default.
 */
const int DB_CREATE_OR_OPEN	 = 0x00;

/** Create database if it doesn't already exist, or overwrite if it does. */
const int DB_CREATE_OR_OVERWRITE = 0x01;

/** Create a new database.
 *
 *  If the database already exists, an exception will be thrown.
 */
const int DB_CREATE		 = 0x02;

/** Open an existing database.
 *
 *  If the database doesn't exist, an exception will be thrown.
 */
const int DB_OPEN		 = 0x03;


/** Show a short-format display of the B-tree contents.
 *
 *  For use with Xapian::Database::check().
 */
const int DBCHECK_SHORT_TREE = 1;

/** Show a full display of the B-tree contents.
 *
 *  For use with Xapian::Database::check().
 */
const int DBCHECK_FULL_TREE = 2;

/** Show the bitmap for the B-tree.
 *
 *  For use with Xapian::Database::check().
 */
const int DBCHECK_SHOW_BITMAP = 4;

/** Show statistics for the B-tree.
 *
 *  For use with Xapian::Database::check().
 */
const int DBCHECK_SHOW_STATS = 8;

/** Fix problems.
 *
 *  Currently this is supported for chert, and will:
 *
 *    * regenerate the "iamchert" file if it isn't valid (so if it is lost, you
 *      can just create it empty and then "fix problems").
 *
 *    * regenerate base files (currently the algorithm for finding the root
 *      block may not work if there was a change partly written but not
 *      committed).
 *
 *  For use with Xapian::Database::check().
 */
const int DBCHECK_FIX = 16;

}

#endif /* XAPIAN_INCLUDED_CONSTANTS_H */
