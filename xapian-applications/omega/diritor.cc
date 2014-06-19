/* diritor.cc: Iterator through entries in a directory.
 *
 * Copyright (C) 2007,2008,2010,2014 Olly Betts
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

#if defined O_NOATIME && O_NOATIME != 0
uid_t DirectoryIterator::euid = geteuid();
#endif

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
#ifdef MAGIC_MIME_TYPE
	magic_cookie = magic_open(MAGIC_SYMLINK|MAGIC_MIME_TYPE|MAGIC_ERROR);
#else
	// MAGIC_MIME_TYPE was added in 4.22, released 2007-12-27.  If we don't
	// have it then use MAGIC_MIME instead and trim any encoding off below.
	magic_cookie = magic_open(MAGIC_SYMLINK|MAGIC_MIME|MAGIC_ERROR);
#endif
	if (magic_cookie == NULL) {
	    string m("Failed to initialise the file magic library: ");
	    m += strerror(errno);
	    throw m;
	}
	if (magic_load(magic_cookie, NULL) == -1) {
	    string m("Failed to load the file magic database");
	    const char * err = magic_error(magic_cookie);
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
	const char * err = magic_error(magic_cookie);
	if (rare(err)) {
	    string m("Failed to use magic on file: ");
	    m += err;
	    throw m;
	}
	return string();
    }

    // Sometimes libmagic returns this string instead of a mime-type for some
    // Microsoft documents, so pick a suitable MIME content-type based on the
    // extension.  Newer versions seem to return "application/CDFV2-corrupt"
    // instead for this case (on Debian, file 5.11 gives the former and file
    // 5.18 the latter).
#define COMPOSITE_DOC "Composite Document File V2 Document"
    if (strncmp(res, COMPOSITE_DOC, sizeof(COMPOSITE_DOC) - 1) == 0 ||
	strcmp(res, "application/CDFV2-corrupt") == 0) {
	// Default to something self-explanatory.
	res = "application/x-compound-document-file";
	const char * leaf = leafname();
	const char * ext = strrchr(leaf, '.');
	if (ext && strlen(++ext) == 3) {
	    char e[3];
	    for (int i = 0; i != 3; ++i) {
		if (ext[i] <= 'Z' && ext[i] >= 'A')
		    e[i] = ext[i] + ('a' - 'A');
		else
		    e[i] = ext[i];
	    }
	    switch (e[0]) {
		case 'd':
		    if (e[1] == 'o')
			res = "application/msword";
		    break;
		case 'm':
		    if (e[1] == 's' && e[2] == 'g')
			res = "application/vnd.ms-outlook";
		    break;
		case 'p':
		    if (e[1] == 'p' || e[1] == 'o')
			res = "application/vnd.ms-powerpoint";
		    else if (e[1] == 'u' && e[2] == 'b')
			res = "application/x-mspublisher";
		    break;
		case 'x':
		    if (e[1] == 'l')
			res = "application/vnd.ms-excel";
		    break;
		case 'w':
		    if (e[1] == 'p' && e[2] != 'd')
			res = "application/vnd.ms-works";
		    break;
	    }
	}
    } else {
#ifndef MAGIC_MIME_TYPE
	// Discard any encoding returned.
	char * spc = strchr(res, ' ');
	if (spc)
	    *spc = '\0';
#endif
    }

    return res;
}
#endif
