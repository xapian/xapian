/* diritor.cc: Iterator through entries in a directory.
 *
 * Copyright (C) 2007,2008 Olly Betts
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

#include <config.h>

#include "diritor.h"

#include "safeerrno.h"

#include <cstring>

using namespace std;

void
DirectoryIterator::call_stat()
{
    string file = path;
    file += '/';
    file += leafname();
    int retval;
#ifdef HAVE_LSTAT
    if (follow_symlinks) {
#endif
	retval = stat(file.c_str(), &statbuf);
#ifdef HAVE_LSTAT
    } else {
	retval = lstat(file.c_str(), &statbuf);
    }
#endif
    if (retval == -1) {
	string error = "Can't stat \"";
	error += file;
	error += "\" (";
	error += strerror(errno);
	error += ')';
	throw error;
    }

    statbuf_valid = true;
}

void
DirectoryIterator::start(const std::string & path_)
{
    if (dir) closedir(dir);
    path = path_;
    dir = opendir(path.c_str());
    if (dir == NULL) {
	string error = "Can't open directory \"";
	error += path;
	error += "\" (";
	error += strerror(errno);
	error += ')';
	throw error;
    }
}

void
DirectoryIterator::next_failed() const
{
    string error = "Can't read next entry from directory \"";
    error += path;
    error += "\" (";
    error += strerror(errno);
    error += ')';
    throw error;
}
