/* diritor.cc: Iterator through entries in a directory.
 *
 * Copyright (C) 2007,2008,2010 Olly Betts
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

#include "safeunistd.h"
#include <sys/types.h>
#include "safeerrno.h"

#include <cstring>

using namespace std;

uid_t DirectoryIterator::euid = geteuid();

#ifdef HAVE_MAGIC_H
magic_t DirectoryIterator::magic_cookie = NULL;
#endif

void
DirectoryIterator::call_stat()
{
    build_path();
    int retval;
#ifdef HAVE_LSTAT
    if (follow_symlinks) {
#endif
	retval = stat(path.c_str(), &statbuf);
#ifdef HAVE_LSTAT
    } else {
	retval = lstat(path.c_str(), &statbuf);
    }
#endif
    if (retval == -1) {
	string error = "Can't stat \"";
	error += path;
	error += "\" (";
	error += strerror(errno);
	error += ')';
	throw error;
    }
}

void
DirectoryIterator::build_path()
{
    if (path.length() == path_len) {
	path += '/';
	path += leafname();
    }
}

void
DirectoryIterator::start(const std::string & path_)
{
    if (dir) closedir(dir);
    path = path_;
    path_len = path.length();
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

#ifdef HAVE_MAGIC_H
string
DirectoryIterator::get_magic_mimetype()
{
    if (rare(magic_cookie == NULL)) {
	magic_cookie = magic_open(MAGIC_SYMLINK|MAGIC_MIME_TYPE|MAGIC_ERROR);
	if (magic_cookie == NULL) {
	    string m("Failed to initialise the file magic library: ");
	    m += strerror(errno);
	    throw m;
	}
	if (magic_load(magic_cookie, NULL) == -1) {
	    string m("Failed to load the file magic database");
	    const char * err = magic_error();
	    if (err) {
		m += ": ";
		m += err;
	    }
	    throw m;
	}
    }

    // FIXME: handle NOATIME here and share the fd with load_file().
    build_path();
    const char * res = magic_file(magic_cookie, path.c_str());
    if (!res) {
	const char * err = magic_error();
	if (rare(err)) {
	    string m("Failed to use magic on file: ");
	    m += err;
	    throw m;
	}
	return string();
    }
    return res;
}
#endif
