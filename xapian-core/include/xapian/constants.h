/** @file constants.h
 * @brief Constants in the Xapian namespace
 */
/* Copyright (C) 2012,2013,2014 Olly Betts
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

/** @internal Bit mask for action codes. */
const int DB_ACTION_MASK_	 = 0x03;

/** Don't attempt to ensure changes have hit disk.
 *
 *  By default, Xapian ask the OS to ensure changes have hit disk (by calling
 *  fdatasync(), fsync() or similar functions).  If these calls do their job,
 *  this should mean that when WritableDatabase::commit() returns, the changes
 *  are durable, but this comes at a performance cost, and if you don't mind
 *  losing changes in the case of a crash, power failure, etc, then this option
 *  can speed up indexing significantly.
 */
const int DB_NO_SYNC		 = 0x04;

/** Try to ensure changes are really written to disk.
 *
 *  Generally fsync() and similar functions only ensure that data has been sent
 *  to the drive.  Modern drives have large write-back caches for performance,
 *  and a power failure could still lose data which is in the write-back cache
 *  waiting to be written.
 *
 *  Some platforms provide a way to ensure data has actually been written and
 *  setting DB_FULL_SYNC will attempt to do so where possible.  The downside is
 *  that committing changes takes longer, and other I/O to the same disk may be
 *  delayed too.
 *
 *  Currently only Mac OS X is supported, and only on some filing system types
 *  - if not supported, Xapian will use fsync() or similar instead.
 */
const int DB_FULL_SYNC		 = 0x08;

/** Update the database in-place.
 *
 *  Xapian's disk-based backends use block-based storage, with copy-on-write
 *  to allow the previous revision to be searched while a new revision forms.
 *
 *  This option means changed blocks get written back over the top of the
 *  old version.  The benefits of this are that less I/O is required during
 *  indexing, and the result of indexing is more compact.  The downsides are
 *  that you can't concurrently search while indexing, transactions can't be
 *  cancelled, and if indexing ends uncleanly (i.e. without commit() or
 *  WritableDatabase's destructor being called) then the database won't be
 *  usable.
 *
 *  Currently all the base files will be removed upon the first modification,
 *  and new base files will be written upon commit.  This prevents new
 *  readers from opening the database while it unsafe to do so, but there's
 *  not currently a mechanism in Xapian to handle notifying existing readers.
 */
const int DB_DANGEROUS		 = 0x10;

/** When creating a database, don't create a termlist table.
 *
 *  For backends which support it (currently brass), this will prevent creation
 *  of a termlist table.  This saves on the disk space that would be needed to
 *  store it, and the CPU and I/O needed to update it, but some features either
 *  inherently need the termlist table, or the current implementation of them
 *  requires it.
 *
 *  The following probably can't be sensibly implemented without it:
 *
 *   - Database::termlist_begin()
 *   - Document::termlist_begin()
 *   - Document::termlist_count()
 *   - Enquire::get_eset()
 *
 *  And the following currently require it:
 *
 *   - Enquire::matching_terms_begin() - we could record this information
 *     during the match, though it might be hard to do without a speed penalty.
 *   - WritableDatabase::delete_document() - we could allow this with inexact
 *     statistics (like how Lucene does).
 *   - WritableDatabase::replace_document() if the document exists already
 *     (again, possible with inexact statistics).
 *   - Currently the list of which values are used in each document is stored
 *     in the termlist table, so things like iterating the values in a document
 *     require it (which is probably reasonably since iterating the terms in
 *     a document requires it).
 *
 *  You can also convert an existing database to not have a termlist table
 *  by simply deleting termlist.*.
 */
const int DB_NO_TERMLIST	 = 0x20;

/** Use the brass backend.
 *
 *  When opening a WritableDatabase, this means create a brass database if a
 *  new database is created.  If there's an existing database (of any type)
 *  at the specified path, this flag has no effect.
 *
 *  When opening a Database, this flag means to only open it if it's a brass
 *  database.  There's rarely a good reason to do this - it's mostly provided
 *  as equivalent functionality to Xapian::Brass::open() in Xapian 1.2.
 */
const int DB_BACKEND_BRASS	 = 0x100;

/** Use the chert backend.
 *
 *  When opening a WritableDatabase, this means create a chert database if a
 *  new database is created.  If there's an existing database (of any type)
 *  at the specified path, this flag has no effect.
 *
 *  When opening a Database, this flag means to only open it if it's a chert
 *  database.  There's rarely a good reason to do this - it's mostly provided
 *  as equivalent functionality to Xapian::Chert::open() in Xapian 1.2.
 */
const int DB_BACKEND_CHERT	 = 0x200;

/** Open a stub database file.
 *
 *  When opening a Database, this flag means to only open it if it's a stub
 *  database file.  There's rarely a good reason to do this - it's mostly
 *  provided as equivalent functionality to Xapian::Auto::open_stub() in
 *  Xapian 1.2.
 */
const int DB_BACKEND_STUB	 = 0x300;

/** @internal Bit mask for backend codes. */
const int DB_BACKEND_MASK_	 = 0x300;

/** @internal Used internally to signify opening read-only. */
const int DB_READONLY_		 = -1;


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
