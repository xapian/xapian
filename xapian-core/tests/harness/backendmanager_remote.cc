/** @file backendmanager_remote.cc
 * @brief BackendManager subclass for remote databases.
 */
/* Copyright (C) 2006,2007,2008,2009 Olly Betts
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
#include "backendmanager_remote.h"
#include "str.h"
#include <cstdlib>
#include <string>

BackendManagerRemote::BackendManagerRemote(const std::string & remote_type_)
	: remote_type(remote_type_)
{
    if (!(false
#ifdef XAPIAN_HAS_BRASS_BACKEND
	  || remote_type == "brass"
#endif
#ifdef XAPIAN_HAS_CHERT_BACKEND
	  || remote_type == "chert"
#endif
#ifdef XAPIAN_HAS_FLINT_BACKEND
	  || remote_type == "flint"
#endif
	 )) {
	throw ("Unknown backend type \"" + remote_type + "\" specified for remote database");
    }
}

std::string
BackendManagerRemote::get_writable_database_args(const std::string & name,
						 const std::string & file)
{
    last_wdb_name = name;

    // Default to a long (5 minute) timeout so that tests won't fail just
    // because the host is slow or busy.
    std::string args = "-t300000 --writable ";

#ifdef XAPIAN_HAS_BRASS_BACKEND
    if (remote_type == "brass") {
	(void)getwritedb_brass(name, std::vector<std::string>(1, file));
	args += ".brass/";
    }
#endif
#ifdef XAPIAN_HAS_CHERT_BACKEND
    if (remote_type == "chert") {
	(void)getwritedb_chert(name, std::vector<std::string>(1, file));
	args += ".chert/";
    }
#endif
#ifdef XAPIAN_HAS_FLINT_BACKEND
    if (remote_type == "flint") {
	(void)getwritedb_flint(name, std::vector<std::string>(1, file));
	args += ".flint/";
    }
#endif
    args += name;

    return args;
}

std::string
BackendManagerRemote::get_remote_database_args(const std::vector<std::string> & files,
					       unsigned int timeout)
{
    std::string args = "-t";
    args += str(timeout);
    args += ' ';
#ifdef XAPIAN_HAS_BRASS_BACKEND
	if (remote_type == "brass") {
	    args += createdb_brass(files);
	}
#endif
#ifdef XAPIAN_HAS_CHERT_BACKEND
    if (remote_type == "chert") {
	args += createdb_chert(files);
    }
#endif
#ifdef XAPIAN_HAS_FLINT_BACKEND
    if (remote_type == "flint") {
	args += createdb_flint(files);
    }
#endif

    return args;
}

std::string
BackendManagerRemote::get_writable_database_as_database_args()
{
    std::string args = "-t300000 ";
#ifdef XAPIAN_HAS_BRASS_BACKEND
    if (remote_type == "brass") {
	args += ".brass/";
    }
#endif
#ifdef XAPIAN_HAS_CHERT_BACKEND
    if (remote_type == "chert") {
	args += ".chert/";
    }
#endif
#ifdef XAPIAN_HAS_FLINT_BACKEND
    if (remote_type == "flint") {
	args += ".flint/";
    }
#endif
    args += last_wdb_name;

    return args;
}

std::string
BackendManagerRemote::get_writable_database_again_args()
{
    std::string args = "-t300000 --writable ";
#ifdef XAPIAN_HAS_BRASS_BACKEND
    if (remote_type == "brass") {
	args += ".brass/";
    }
#endif
#ifdef XAPIAN_HAS_CHERT_BACKEND
    if (remote_type == "chert") {
	args += ".chert/";
    }
#endif
#ifdef XAPIAN_HAS_FLINT_BACKEND
    if (remote_type == "flint") {
	args += ".flint/";
    }
#endif
    args += last_wdb_name;

    return args;
}
