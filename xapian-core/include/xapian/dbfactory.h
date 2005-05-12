/** \file dbfactory.h
 * \brief Factory functions for constructing Database and WritableDatabase objects
 */
/* Copyright (C) 2005 Olly Betts
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
 */

#ifndef XAPIAN_INCLUDED_DBFACTORY_H
#define XAPIAN_INCLUDED_DBFACTORY_H

#include <string>

#include <xapian/types.h>
#include <xapian/database.h>
#include <xapian/version.h>

namespace Xapian {

namespace Auto {

/** Construct a Database object for a stub database file.
 *
 * The stub database file contains serialised parameters for one
 * or more databases.
 *
 * @param file  pathname of the stub database file.
 */
Database open_stub(const std::string &file);

/** Construct a Database object for read-only access to a database.
 *
 * The appropriate database backend is chosen automatically.
 *
 * @deprecated	This function is deprecated - use the Xapian::Database(path)
 *		constructor instead.
 *
 * @param path  pathname of the file or directory containing the database.
 */
inline Database open(const std::string &path) {
    return Database(path);
}

/** Construct a WritableDatabase object for update access to a database.
 *
 * The appropriate database backend is chosen automatically.
 *
 * @deprecated	This function is deprecated - use the
 *		Xapian::WritableDatabase(path, action) constructor instead.
 *
 * @param path  pathname of the file or directory containing the database.
 */
inline WritableDatabase open(const std::string &path, int action) {
    return WritableDatabase(path, action);
}

}

#ifdef XAPIAN_HAS_INMEMORY_BACKEND
namespace InMemory {

/** Construct a Database object for update access to an InMemory database.
 *
 * A new, empty database is created for each call.
 */
WritableDatabase open();

}
#endif

#ifdef XAPIAN_HAS_MUSCAT36_BACKEND
namespace Muscat36 {

/** Construct a Database object for read-only access to a Muscat36 DA database.
 *
 * @param R		pathname for the Record file.
 * @param T		pathname for the Term file.
 * @param heavy_duty	true if this DA database is "heavy-duty" (3 byte
 *			lengths); false if it's "flimsy" (2 byte lengths).
 */
Database open_da(const std::string &R, const std::string &T, bool heavy_duty = true);

/** Construct a Database object for read-only access to a Muscat36 DA database.
 *
 * This variants allows you to open a DA database with a values file.  Muscat36
 * didn't support values - support for them was added to Xapian for mostly to
 * allow testing during early development work.
 *
 * @param R		pathname for the Record file.
 * @param T		pathname for the Term file.
 * @param values	pathname for the Values file.
 * @param heavy_duty	true if this DA database is "heavy-duty" (3 byte
 *			lengths); false if it's "flimsy" (2 byte lengths).
 */
Database open_da(const std::string &R, const std::string &T, const std::string &values, bool heavy_duty = true);

/** Construct a Database object for read-only access to a Muscat36 DB database.
 *
 * It's easy to tell if a DB database is heavy-duty or flimsy (unlike with a
 * DA database), so this is automatically detected.
 *
 * @param DB		pathname for the DB file.
 * @param cache_size	size of the Btree block cache.
 */
Database open_db(const std::string &DB, size_t cache_size = 30);

/** Construct a Database object for read-only access to a Muscat36 DB database.
 *
 * This variants allows you to open a DB database with a values file.  Muscat36
 * didn't support values - support for them was added to Xapian for mostly to
 * allow testing during early development work.
 *
 * It's easy to tell if a DB database is heavy-duty or flimsy (unlike with a
 * DA database), so this is automatically detected.
 *
 * @param DB		pathname for the DB file.
 * @param values	pathname for the Values file.
 * @param cache_size	size of the Btree block cache.
 */
Database open_db(const std::string &DB, const std::string &values, size_t cache_size = 30);

}
#endif

#ifdef XAPIAN_HAS_QUARTZ_BACKEND
namespace Quartz {

/** Construct a Database object for read-only access to a Quartz database.
 *
 * @param dir  pathname of the directory containing the database.
 */
Database open(const std::string &dir);

/** Construct a Database object for update access to a Quartz database.
 *
 * @param dir		pathname of the directory containing the database.
 * @param action  	determines handling of existing/non-existing database:
 *  - Xapian::DB_CREATE			fail if database already exist,
 *					otherwise create new database.
 *  - Xapian::DB_CREATE_OR_OPEN 	open existing database, or create new
 *					database if none exists.
 *  - Xapian::DB_CREATE_OR_OVERWRITE	overwrite existing database, or create
 *					new database if none exists.
 *  - Xapian::DB_OPEN			open existing database, failing if none
 *					exists.
 * @param block_size	the Btree blocksize to use (in bytes), which must be a
 * 			power of two between 2048 and 65536 (inclusive).  The
 * 			default (also used if an invalid value if passed) is
 * 			8192 bytes.  This parameter is ignored when opening an
 * 			existing database.
 */
WritableDatabase
open(const std::string &dir, int action, int block_size = 8192);

}
#endif

#ifdef XAPIAN_HAS_REMOTE_BACKEND
namespace Remote {

/** Construct a Database object for read-only access to a remote database
 *  accessed via a TCP connection.
 *
 * Access to the remote database is via a TCP connection to the specified
 * host and port.
 *
 * @param host		hostname to connect to.
 * @param port		port number to connect to.
 * @param timeout	timeout in milliseconds.  If this timeout is exceeded
 *			for any individual operation on the remote database
 *			then Xapian::NetworkTimeoutError is thrown.  (Default
 *			is 10000ms, which is 10 seconds).
 * @param connect_timeout	timeout to use when connecting to the server.
 *				If this timeout is exceeded then
 *				Xapian::NetworkTimeoutError is throw.  (Default
 *				is to be the same as timeout).
 */
Database open(const std::string &host, unsigned int port, Xapian::timeout timeout = 10000, Xapian::timeout connect_timeout = 0);

/** Construct a Database object for read-only access to a remote database
 *  accessed via a program.
 *
 * Access to the remote database is done by running an external program and
 * communicating with it on stdin/stdout.
 *
 * @param program	the external program to run.
 * @param arguments	space-separated list of arguments to pass to program.
 * @param timeout	timeout in milliseconds.  If this timeout is exceeded
 *			for any individual operation on the remote database
 *			then Xapian::NetworkTimeoutError is thrown (default is
 *			10000ms, which is 10 seconds).
 */
Database open(const std::string &program, const std::string &args, Xapian::timeout timeout = 10000);

}
#endif

}

#endif /* XAPIAN_INCLUDED_DBFACTORY_H */
