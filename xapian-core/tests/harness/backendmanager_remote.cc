/** @file
 * @brief BackendManager subclass for remote databases.
 */
/* Copyright (C) 2006,2007,2008,2009,2011,2015 Olly Betts
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

#include "backendmanager_glass.h"
#include <cstdlib>
#include <string>
#include "str.h"

std::string
BackendManagerRemote::get_generated_database_path(const std::string& name) {
    return sub_manager->get_writable_database_path(name);
}

Xapian::WritableDatabase
BackendManagerRemote::get_generated_database(const std::string& name)
{
    return sub_manager->get_writable_database(name, std::string());
}

std::string
BackendManagerRemote::get_writable_database_args(const std::string & name,
						 const std::string & file)
{
    // Default to a long (5 minute) timeout so that tests won't fail just
    // because the host is slow or busy.
    std::string args = "-t300000 --writable ";

    sub_manager->get_writable_database(name, file);
    args += sub_manager->get_writable_database_path(name);

    return args;
}

std::string
BackendManagerRemote::get_writable_database_args(const std::string& path,
						 unsigned int timeout)
{
    std::string args = "-t";
    args += str(timeout);
    args += " --writable ";
    args += path;
    return args;
}

std::string
BackendManagerRemote::get_remote_database_args(const std::vector<std::string> & files,
					       unsigned int timeout)
{
    std::string args = "-t";
    args += str(timeout);
    args += ' ';
    args += sub_manager->get_database_path(files);
    return args;
}

std::string
BackendManagerRemote::get_remote_database_args(const std::string& path,
					       unsigned int timeout)
{
    std::string args = "-t";
    args += str(timeout);
    args += ' ';
    args += path;
    return args;
}

std::string
BackendManagerRemote::get_writable_database_as_database_args()
{
    std::string args = "-t300000 ";
    args += sub_manager->get_writable_database_path_again();
    return args;
}

std::string
BackendManagerRemote::get_writable_database_again_args()
{
    std::string args = "-t300000 --writable ";
    args += sub_manager->get_writable_database_path_again();
    return args;
}
