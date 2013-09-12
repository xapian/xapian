/* diritor.cc: Iterator through entries in a directory.
 *
 * Copyright (C) 2007,2008,2010,2011,2012 Olly Betts
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

CommitAndExit::CommitAndExit(const char * msg_, const std::string & path,
			     int errno_)
{
    msg = msg_;
    msg += " \"";
    msg += path;
    msg += "\" (";
    msg += strerror(errno_);
    msg += ")";
}

CommitAndExit::CommitAndExit(const char * msg_, int errno_)
{
    msg = msg_;
    msg += " (";
    msg += strerror(errno_);
    msg += ")";
}

CommitAndExit::CommitAndExit(const char * msg_, const char * error)
{
    msg = msg_;
    msg += " (";
    msg += error;
    msg += ")";
}

#if defined O_NOATIME && O_NOATIME != 0
uid_t DirectoryIterator::euid = geteuid();
#endif

magic_t DirectoryIterator::magic_cookie = NULL;

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
	if (errno == ENOENT)
	    throw FileNotFound();
	// Commit changes to files processed so far.
	throw CommitAndExit("Can't stat", path, errno);
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
	if (errno == ENOENT)
	    throw FileNotFound();
	// Commit changes to files processed so far.
	throw CommitAndExit("Can't open directory", path, errno);
    }
}

void
DirectoryIterator::next_failed() const
{
    throw CommitAndExit("Can't read next entry from directory", path, errno);
}

string
DirectoryIterator::get_magic_mimetype()
{
    if (rare(magic_cookie == NULL)) {
#ifdef MAGIC_MIME_TYPE
	magic_cookie = magic_open(MAGIC_SYMLINK|MAGIC_MIME_TYPE|MAGIC_ERROR);
#else
	// MAGIC_MIME_TYPE was added in 4.22, released 2007-12-27.  If we don't
	// have it then use MAGIC_MIME instead and trim any encoding off below.
	magic_cookie = magic_open(MAGIC_SYMLINK|MAGIC_MIME|MAGIC_ERROR);
#endif
	if (magic_cookie == NULL) {
	    // Commit changes to files processed so far.
	    throw CommitAndExit("Failed to initialise the file magic library",
				errno);
	}
	if (magic_load(magic_cookie, NULL) == -1) {
	    // Commit changes to files processed so far.
	    const char * err = magic_error(magic_cookie);
	    throw CommitAndExit("Failed to load the file magic database", err);
	}
    }

    const char * res = NULL;
#ifdef HAVE_MAGIC_DESCRIPTOR
    if (fd >= 0) {
	if (lseek(fd, 0, SEEK_SET) == 0)
	    res = magic_descriptor(magic_cookie, fd);
    } else
#endif
    {
	build_path();
	res = magic_file(magic_cookie, path.c_str());
    }
    if (!res) {
	const char * err = magic_error(magic_cookie);
	if (rare(err)) {
	    if (magic_errno(magic_cookie) == ENOENT)
		throw FileNotFound();
	    string m("Failed to use magic on file: ");
	    m += err;
	    throw m;
	}
	return string();
    }

#ifndef MAGIC_MIME_TYPE
    // Discard any encoding returned.
    char * spc = strchr(res, ' ');
    if (spc)
	*spc = '\0';
#endif

    return res;
}
