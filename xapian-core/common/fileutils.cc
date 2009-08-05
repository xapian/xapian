/** @file fileutils.cc
 *  @brief File and path manipulation routines.
 */
/* Copyright (C) 2008 Lemur Consulting Ltd
 * Copyright (C) 2008,2009 Olly Betts
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

#include "stringutils.h"
#include <string>

using namespace std;

static bool
isabspath(const string & path)
{
    // Empty paths are never absolute.
    if (path.empty()) return false;
#ifdef __WIN32__
    // On windows, paths may begin with a drive letter - but the part after the
    // drive letter may still be relative.
    if (path.size() >= 2 && path[1] == ':') {
	if (path.size() == 2) return false;
	if (path[2] != '/' && path[2] != '\\') return false;
	return true;
    }
    if (path[0] != '/' && path[0] != '\\') return false;
    return true;
#else
    // Assume we're on a unix system.
    if (path[0] != '/') return false;
    return true;
#endif
}

string
calc_dirname(const string & filename)
{
    string::size_type slash = filename.rfind('/');
#ifdef __WIN32__
    string::size_type backslash = filename.rfind('\\');
    if (backslash != string::npos && backslash > slash) slash = backslash;
#endif
    if (slash == string::npos) return "./";
    return (filename.substr(0, slash) + "/");
}

#ifdef __WIN32__
/// Return true iff a path starts with a drive letter.
static bool
has_drive(const string &path)
{
    return (path.size() >= 2 && path[1] == ':');
}

/// Return true iff a path consists only of a drive letter.
static bool
is_drive(const string &path)
{
    if (!has_drive(path)) return false;
    if (path.size() == 2) return true;
    if (path.size() == 3 && (path[2] == '\\' || path[2] == '/')) return true;
    return false;
}
#endif

string
join_paths(const string & path1, const string & path2)
{
    if (path1.empty()) return path2;
#ifdef __WIN32__
    if (isabspath(path2)) {
	// If path2 has a drive, just return it.
	// Otherwise, if path1 is only a drive specification, prepend that
	// drive specifier to path2.
	if (has_drive(path2)) return path2;
	if (is_drive(path1)) return path1.substr(0, 2) + path2;
	return path2;
    }
    if (has_drive(path2)) return path2;
    if (endswith(path1, '/') || endswith(path1, '\\'))
	return path1 + path2;
    return path1 + "\\" + path2;
#else
    // Assume we're on a unix system.
    if (isabspath(path2)) return path2;
    if (path1[path1.size() - 1] == '/') return path1 + path2;
    return path1 + "/" + path2;
#endif
}
