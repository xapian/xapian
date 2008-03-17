/** @file backendmanager_remoteprog.cc
 * @brief BackendManager subclass for remoteprog databases.
 */
/* Copyright (C) 2007 Olly Betts
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

BackendManagerRemoteProg::~BackendManagerRemoteProg() { }

const char *
BackendManagerRemoteProg::get_dbtype() const
{
    return "remoteprog";
}

Xapian::Database
BackendManagerRemoteProg::get_database(const vector<string> & files)
{
    // Default to a long (5 minute) timeout so that tests won't fail just
    // because the host is slow or busy.
    return BackendManagerRemoteProg::get_remote_database(files, 300000);
}

Xapian::Database
BackendManagerRemoteProg::get_database(const string & file)
{
    return BackendManagerRemoteProg::get_database(vector<string>(1, file));
}

Xapian::WritableDatabase
BackendManagerRemoteProg::get_writable_database(const string & name,
						const string & file)
{
    last_wdb_name = name;

    vector<string> files(1, file);
    // Default to a long (5 minute) timeout so that tests won't fail just
    // because the host is slow or busy.
    string args = "-t300000 --writable ";

#ifdef XAPIAN_HAS_FLINT_BACKEND
    (void)getwritedb_flint(name, files);
    args += ".flint/";
#else
    (void)getwritedb_quartz(name, files);
    args += ".quartz/";
#endif
    args += name;

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
    string args = "-t";
    args += om_tostring(timeout);
    args += ' ';
#ifdef XAPIAN_HAS_FLINT_BACKEND
    args += createdb_flint(files);
#else
    args += createdb_quartz(files);
#endif
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
    string args = "-t300000 ";
#ifdef XAPIAN_HAS_FLINT_BACKEND
    args += ".flint/";
#else
    args += ".quartz/";
#endif
    args += last_wdb_name;

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
    string args = "-t300000 --writable ";
#ifdef XAPIAN_HAS_FLINT_BACKEND
    args += ".flint/";
#else
    args += ".quartz/";
#endif
    args += last_wdb_name;

#ifdef HAVE_VALGRIND
    if (RUNNING_ON_VALGRIND) {
	args.insert(0, XAPIAN_PROGSRV" ");
	return Xapian::Remote::open_writable("./runsrv", args);
    }
#endif
    return Xapian::Remote::open_writable(XAPIAN_PROGSRV, args);
}
