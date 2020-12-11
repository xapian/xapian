/** @file
 *  @brief File and path manipulation routines.
 */
/* Copyright (C) 2008 Lemur Consulting Ltd
 * Copyright (C) 2008,2009,2010,2012 Olly Betts
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

#include "fileutils.h"

#include "xapian/error.h"
#include "safedirent.h"
#include "safeunistd.h"

#include <cerrno>
#include <cstring>
#include <string>
#include <sys/types.h>

using namespace std;

class dircloser {
    DIR * dir;
  public:
    dircloser(DIR * dir_) : dir(dir_) {}
    ~dircloser() {
	if (dir != NULL) {
	    closedir(dir);
	    dir = NULL;
	}
    }
};

void
removedir(const string &dirname)
{
    DIR * dir;

    dir = opendir(dirname.c_str());
    if (dir == NULL) {
	if (errno == ENOENT) return;
	throw Xapian::DatabaseError("Cannot open directory '" + dirname + "'", errno);
    }

    {
	dircloser dc(dir);
	while (true) {
	    errno = 0;
	    struct dirent * entry = readdir(dir);
	    if (entry == NULL) {
		if (errno == 0)
		    break;
		throw Xapian::DatabaseError("Cannot read entry from directory at '" + dirname + "'", errno);
	    }
	    string name(entry->d_name);
	    if (name == "." || name == "..")
		continue;
	    if (unlink((dirname + "/" + name).c_str())) {
		throw Xapian::DatabaseError("Cannot remove file '" + string(entry->d_name) + "'", errno);
	    }
	}
    }
    if (rmdir(dirname.c_str())) {
	throw Xapian::DatabaseError("Cannot remove directory '" + dirname + "'", errno);
    }
}

#ifdef __WIN32__
/// Return true iff a path starts with a drive letter.
static bool
has_drive(const string &path)
{
    return (path.size() >= 2 && path[1] == ':');
}

/// Return true iff path is a UNCW path.
static bool
uncw_path(const string & path)
{
    return (path.size() >= 4 && memcmp(path.data(), "\\\\?\\", 4) == 0);
}

static inline bool slash(char ch)
{
    return ch == '/' || ch == '\\';
}
#endif

void
resolve_relative_path(string & path, const string & base)
{
#ifndef __WIN32__
    if (path.empty() || path[0] != '/') {
	// path is relative.
	string::size_type last_slash = base.rfind('/');
	if (last_slash != string::npos)
	    path.insert(0, base, 0, last_slash + 1);
    }
#else
    // Microsoft Windows paths may begin with a drive letter but still be
    // relative within that drive.
    bool drive = has_drive(path);
    string::size_type p = (drive ? 2 : 0);
    bool absolute = (p != path.size() && slash(path[p]));

    if (absolute) {
	// If path is absolute and has a drive specifier, just return it.
	if (drive)
	    return;

	// If base has a drive specifier prepend that to path.
	if (has_drive(base)) {
	    path.insert(0, base, 0, 2);
	    return;
	}

	// If base has a UNC (\\SERVER\\VOLUME) or \\?\ prefix, prepend that
	// to path.
	if (uncw_path(base)) {
	    string::size_type sl = 0;
	    if (base.size() >= 7 && memcmp(base.data() + 5, ":\\", 2) == 0) {
		// "\\?\X:\"
		sl = 6;
	    } else if (base.size() >= 8 &&
		       memcmp(base.data() + 4, "UNC\\", 4) == 0) {
		// "\\?\UNC\server\volume\"
		sl = base.find('\\', 8);
		if (sl != string::npos)
		    sl = base.find('\\', sl + 1);
	    }
	    if (sl) {
		// With the \\?\ prefix, '/' isn't recognised so change it
		// to '\' in path.
		string::iterator i;
		for (i = path.begin(); i != path.end(); ++i) {
		    if (*i == '/')
			*i = '\\';
		}
		path.insert(0, base, 0, sl);
	    }
	} else if (base.size() >= 5 && slash(base[0]) && slash(base[1])) {
	    // Handle UNC base.
	    string::size_type sl = base.find_first_of("/\\", 2);
	    if (sl != string::npos) {
		sl = base.find_first_of("/\\", sl + 1);
		path.insert(0, base, 0, sl);
	    }
	}
	return;
    }

    // path is relative, so if it has no drive specifier or the same drive
    // specifier as base, then we want to qualify it using base.
    bool base_drive = has_drive(base);
    if (!drive || (base_drive && (path[0] | 32) == (base[0] | 32))) {
	string::size_type last_slash = base.find_last_of("/\\");
	if (last_slash == string::npos && !drive && base_drive)
	    last_slash = 1;
	if (last_slash != string::npos) {
	    string::size_type b = (drive && base_drive ? 2 : 0);
	    if (uncw_path(base)) {
		// With the \\?\ prefix, '/' isn't recognised so change it
		// to '\' in path.
		string::iterator i;
		for (i = path.begin(); i != path.end(); ++i) {
		    if (*i == '/')
			*i = '\\';
		}
	    }
	    path.insert(b, base, b, last_slash + 1 - b);
	}
    }
#endif
}
