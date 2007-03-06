/** @file rmdir.cc
 *  @brief rmdir(DIR) recursively deletes directory DIR.
 */
/* Copyright (C) 2004,2007 Olly Betts
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

#include "rmdir.h"

#include <string>
#include <stdlib.h>
#include <sys/types.h>
#include "safesysstat.h"
#include "safeunistd.h"

#ifdef __WIN32__
# include "safewindows.h"
#endif

#include "utils.h"

using namespace std;

/// Remove a directory and contents.
void
rmdir(const string &filename)
{
    // Check filename exists and is actually a directory
    struct stat sb;
    if (filename.empty() || stat(filename, &sb) != 0 || !S_ISDIR(sb.st_mode))
	return;

#ifdef __WIN32__
    string safefile = filename;
    string::iterator i;
    for (i = safefile.begin(); i != safefile.end(); ++i) {
	if (*i == '/') {
	    // Convert Unix path separators to backslashes.  C library
	    // functions understand "/" in paths, but we are going to
	    // call "deltree" or "rd" which don't.
	    *i = '\\';
	} else if (*i < 32 || strchr("<>\"|*?", *i)) {
	    // Check for illegal characters in filename.
	    return;
	}
    }

    static int win95 = -1;
    if (win95 == -1) {
	OSVERSIONINFO info;
	memset(&info, 0, sizeof(OSVERSIONINFO));
	info.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	if (GetVersionEx(&info)) {
	    win95 = (info.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS);
	}
    }

    if (win95) {
	// for 95 like systems:
	system("deltree /y \"" + safefile + "\"");
    } else {
	// for NT like systems:
	system("rd /s /q \"" + safefile + "\"");
    }
#else
    string cmd("rm -rf ");
    cmd.reserve(filename.size() + 16);

    // Prevent a leading "-" on the filename being interpreted as a command line
    // option.
    if (filename[0] == '-') cmd += "./";

    string::const_iterator i;
    for (i = filename.begin(); i != filename.end(); ++i) {
	// Don't escape a few safe characters which are common in filenames
	if (!C_isalnum(*i) && strchr("/._-", *i) == NULL) {
	    cmd += '\\';
	}
	cmd += *i;
    }

    system(cmd);
#endif
}
