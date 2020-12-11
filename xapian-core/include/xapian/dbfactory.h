/** @file
 * @brief Factory functions for constructing Database and WritableDatabase objects
 */
/* Copyright (C) 2005,2006,2007,2008,2009,2011,2013,2014,2016 Olly Betts
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

#ifndef XAPIAN_INCLUDED_DBFACTORY_H
#define XAPIAN_INCLUDED_DBFACTORY_H

#if !defined XAPIAN_IN_XAPIAN_H && !defined XAPIAN_LIB_BUILD
# error Never use <xapian/dbfactory.h> directly; include <xapian.h> instead.
#endif

#include <string>

#include <xapian/constants.h>
#include <xapian/database.h>
#include <xapian/deprecated.h>
#include <xapian/types.h>
#include <xapian/version.h>
#include <xapian/visibility.h>

namespace Xapian {

#ifdef XAPIAN_HAS_REMOTE_BACKEND
/// Database factory functions for the remote backend.
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
 *			then Xapian::NetworkTimeoutError is thrown.  A timeout
 *			of 0 means don't timeout.  (Default is 10000ms, which
 *			is 10 seconds).
 * @param connect_timeout	timeout to use when connecting to the server.
 *				If this timeout is exceeded then
 *				Xapian::NetworkTimeoutError is thrown.  A
 *				timeout of 0 means don't timeout.  (Default is
 *				10000ms, which is 10 seconds).
 */
XAPIAN_VISIBILITY_DEFAULT
Database open(const std::string &host, unsigned int port, unsigned timeout = 10000, unsigned connect_timeout = 10000);

/** Construct a WritableDatabase object for update access to a remote database
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
 *			is 0, which means don't timeout).
 * @param connect_timeout	timeout to use when connecting to the server.
 *				If this timeout is exceeded then
 *				Xapian::NetworkTimeoutError is thrown.  A
 *				timeout of 0 means don't timeout.  (Default is
 *				10000ms, which is 10 seconds).
 * @param flags		Xapian::DB_RETRY_LOCK or 0.
 */
XAPIAN_VISIBILITY_DEFAULT
WritableDatabase open_writable(const std::string &host, unsigned int port, unsigned timeout = 0, unsigned connect_timeout = 10000, int flags = 0);

/** Construct a Database object for read-only access to a remote database
 *  accessed via a program.
 *
 * Access to the remote database is done by running an external program and
 * communicating with it on stdin/stdout.
 *
 * @param program	the external program to run.
 * @param args		space-separated list of arguments to pass to program.
 * @param timeout	timeout in milliseconds.  If this timeout is exceeded
 *			for any individual operation on the remote database
 *			then Xapian::NetworkTimeoutError is thrown.  A timeout
 *			of 0 means don't timeout.  (Default is 10000ms, which
 *			is 10 seconds).
 */
XAPIAN_VISIBILITY_DEFAULT
Database open(const std::string &program, const std::string &args, unsigned timeout = 10000);

/** Construct a WritableDatabase object for update access to a remote database
 *  accessed via a program.
 *
 * Access to the remote database is done by running an external program and
 * communicating with it on stdin/stdout.
 *
 * @param program	the external program to run.
 * @param args		space-separated list of arguments to pass to program.
 * @param timeout	timeout in milliseconds.  If this timeout is exceeded
 *			for any individual operation on the remote database
 *			then Xapian::NetworkTimeoutError is thrown.  (Default
 *			is 0, which means don't timeout).
 * @param flags		Xapian::DB_RETRY_LOCK or 0.
 */
XAPIAN_VISIBILITY_DEFAULT
WritableDatabase open_writable(const std::string &program, const std::string &args, unsigned timeout = 0, int flags = 0);

}
#endif

}

#endif /* XAPIAN_INCLUDED_DBFACTORY_H */
