/** \file errortypes.h
 *  \brief Exception subclasses
 */
/* ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2003,2004 Olly Betts
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

/** This file doesn't have header guards, deliberately.  It should only be
 *  included, by files which know what they're doing with it - this means
 *  xapian/error.h and some internal source files.
 */

/** Macros XAPIAN_DEFINE_ERROR_BASECLASS and XAPIAN_DEFINE_ERROR_CLASS must be
 *  defined before this file is included.
 */

/** Base class for errors due to programming errors.
 *  An exception derived from LogicError is thrown when a misuse
 *  of the API is detected.
 */
XAPIAN_DEFINE_ERROR_BASECLASS(LogicError, Error);

/** Base class for errors due to run time problems.
 *  An exception derived from RuntimeError is thrown when an
 *  error is caused by problems with the data or environment rather
 *  than a programming mistake.
 */
XAPIAN_DEFINE_ERROR_BASECLASS(RuntimeError, Error);

/** Thrown if an internal consistency check fails.
 *  This represents a bug in Xapian. */
XAPIAN_DEFINE_ERROR_CLASS(AssertionError, LogicError);

/** Thrown when an attempt to use an unimplemented feature is made. */
XAPIAN_DEFINE_ERROR_CLASS(UnimplementedError, LogicError);

/** Thrown when an invalid argument is supplied to the API. */
XAPIAN_DEFINE_ERROR_CLASS(InvalidArgumentError, LogicError);

/** Thrown when API calls are made in an invalid way. */
XAPIAN_DEFINE_ERROR_CLASS(InvalidOperationError, LogicError);

/** Thrown when an attempt is made to access a document which is not in the
 *  database.  This could occur either due to a programming error, or
 *  because the database has changed since running the query. */
XAPIAN_DEFINE_ERROR_CLASS(DocNotFoundError, RuntimeError);

/** thrown when an element is out of range. */
XAPIAN_DEFINE_ERROR_CLASS(RangeError, RuntimeError);

/** thrown when really weird stuff happens.  If this is thrown something
 *  has gone badly wrong.
 */
XAPIAN_DEFINE_ERROR_CLASS(InternalError, RuntimeError);

/** thrown for miscellaneous database errors. */
XAPIAN_DEFINE_ERROR_CLASS(DatabaseError, RuntimeError);

/** Thrown if a feature is unavailable - usually due to not being compiled
 *  in.*/
XAPIAN_DEFINE_ERROR_CLASS(FeatureUnavailableError, RuntimeError);

/** thrown when there is a communications problem with
 *  a remote database.
 */
XAPIAN_DEFINE_ERROR_CLASS(NetworkError, RuntimeError);

/** Thrown when a network timeout is exceeded
 */
XAPIAN_DEFINE_ERROR_CLASS(NetworkTimeoutError, NetworkError);

/** thrown if the database is corrupt. */
XAPIAN_DEFINE_ERROR_CLASS(DatabaseCorruptError, DatabaseError);

/** Thrown when creating a database fails. */
XAPIAN_DEFINE_ERROR_CLASS(DatabaseCreateError, DatabaseError);

/** Thrown when opening a database fails. */
XAPIAN_DEFINE_ERROR_CLASS(DatabaseOpeningError, DatabaseError);

/** Thrown when gaining a lock on a database fails. */
XAPIAN_DEFINE_ERROR_CLASS(DatabaseLockError, DatabaseError);

/** Thrown when a database has been modified whilst being read. */
XAPIAN_DEFINE_ERROR_CLASS(DatabaseModifiedError, DatabaseError);
