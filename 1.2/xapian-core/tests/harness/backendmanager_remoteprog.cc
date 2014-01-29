/** @file backendmanager_remoteprog.cc
 * @brief BackendManager subclass for remoteprog databases.
 */
/* Copyright (C) 2007,2008,2009 Olly Betts
 * Copyright (C) 2008 Lemur Consulting Ltd
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

#include <config.h>

#include "backendmanager_remoteprog.h"

#include <xapian.h>

#include "utils.h"

#ifdef HAVE_VALGRIND
# include <valgrind/memcheck.h>
#endif

using namespace std;

std::string
BackendManagerRemoteProg::get_dbtype() const
{
    return "remoteprog_" + remote_type;
}

Xapian::Database
BackendManagerRemoteProg::do_get_database(const vector<string> & files)
{
    // Default to a long (5 minute) timeout so that tests won't fail just
    // because the host is slow or busy.
    return BackendManagerRemoteProg::get_remote_database(files, 300000);
}

Xapian::WritableDatabase
BackendManagerRemoteProg::get_writable_database(const string & name,
						const string & file)
{
    string args = get_writable_database_args(name, file);

#ifdef HAVE_VALGRIND
    if (RUNNING_ON_VALGRIND) {
	args.insert(0, XAPIAN_PROGSRV" ");
	return Xapian::Remote::open_writable("./runsrv", args);
    }
#endif
    return Xapian::Remote::open_writable(XAPIAN_PROGSRV, args);
}

Xapian::Database
BackendManagerRemoteProg::get_remote_database(const vector<string> & files,
					      unsigned int timeout)
{
    string args = get_remote_database_args(files, timeout);

#ifdef HAVE_VALGRIND
    if (RUNNING_ON_VALGRIND) {
	args.insert(0, XAPIAN_PROGSRV" ");
	return Xapian::Remote::open("./runsrv", args);
    }
#endif
    return Xapian::Remote::open(XAPIAN_PROGSRV, args);
}

Xapian::Database
BackendManagerRemoteProg::get_writable_database_as_database()
{
    string args = get_writable_database_as_database_args();

#ifdef HAVE_VALGRIND
    if (RUNNING_ON_VALGRIND) {
	args.insert(0, XAPIAN_PROGSRV" ");
	return Xapian::Remote::open("./runsrv", args);
    }
#endif
    return Xapian::Remote::open(XAPIAN_PROGSRV, args);
}

Xapian::WritableDatabase
BackendManagerRemoteProg::get_writable_database_again()
{
    string args = get_writable_database_again_args();

#ifdef HAVE_VALGRIND
    if (RUNNING_ON_VALGRIND) {
	args.insert(0, XAPIAN_PROGSRV" ");
	return Xapian::Remote::open_writable("./runsrv", args);
    }
#endif
    return Xapian::Remote::open_writable(XAPIAN_PROGSRV, args);
}
