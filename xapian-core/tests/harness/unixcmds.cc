/** @file unixcmds.cc
 *  @brief C++ function versions of useful Unix commands.
 */
/* Copyright (C) 2003,2004,2007 Olly Betts
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

#include "unixcmds.h"

#include <cstring>
#include <string>
#include <cstdlib>
#include <sys/types.h>
#include "safesysstat.h"
#include "safeunistd.h"
#include "safefcntl.h"

#ifdef __WIN32__
# include "safewindows.h"
#endif

#include "stringutils.h"
#include "utils.h"

using namespace std;

/// Append filename argument arg to command cmd with suitable escaping.
static bool
append_filename_argument(string & cmd, const string & arg) {
#ifdef __WIN32__
    cmd.reserve(cmd.size() + arg.size() + 3);
    cmd += " \"";
    for (string::const_iterator i = arg.begin(); i != arg.end(); ++i) {
	if (*i == '/') {
	    // Convert Unix path separators to backslashes.  C library
	    // functions understand "/" in paths, but we are going to
	    // call commands like "deltree" or "rd" which don't.
	    cmd += '\\';
	} else if (*i < 32 || strchr("<>\"|*?", *i)) {
	    // Check for illegal characters in filename.
	    return false;
	} else {
	    cmd += *i;
	}
    }
    cmd += '"';
#else
    // Allow for escaping a few characters.
    cmd.reserve(cmd.size() + arg.size() + 10);

    // Prevent a leading "-" on the filename being interpreted as a command
    // line option.
    if (arg[0] == '-')
	cmd += " ./";
    else
	cmd += ' ';

    for (string::const_iterator i = arg.begin(); i != arg.end(); ++i) {
	// Don't escape a few safe characters which are common in filenames.
	if (!C_isalnum(*i) && strchr("/._-", *i) == NULL) {
	    cmd += '\\';
	}
	cmd += *i;
    }
#endif
    return true;
}

/// Recursively copy a directory.
void cp_R(const std::string &src, const std::string &dest) {
#ifdef __WIN32__
    // xcopy should be available on both NT and 95 derivatives.  We create
    // the target directory first to avoid being prompted as to whether we
    // want to create a directory or a file (which makes no sense when
    // copying a directory, but that's how xcopy seems to work!)
    mkdir(dest.c_str());
    string cmd("xcopy /E /Y");
#else
    string cmd("cp -R");
#endif
    if (!append_filename_argument(cmd, src)) return;
    if (!append_filename_argument(cmd, dest)) return;
    system(cmd);
#ifndef __WIN32__
    // Allow write access to the copy (to deal with builds where srcdir is
    // readonly).
    cmd = "chmod -R +w";
    if (!append_filename_argument(cmd, dest)) return;
    system(cmd);
#endif
}

#ifdef __WIN32__
static bool running_on_win9x() {
    static int win9x = -1;
    if (win9x == -1) {
	OSVERSIONINFO info;
	memset(&info, 0, sizeof(OSVERSIONINFO));
	info.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	if (GetVersionEx(&info)) {
	    win9x = (info.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS);
	}
    }
    return win9x;
}
#endif

/// Remove a directory and contents, just like the Unix "rm -rf" command.
void rm_rf(const string &filename) {
    // Check filename exists and is actually a directory
    struct stat sb;
    if (filename.empty() || stat(filename, &sb) != 0 || !S_ISDIR(sb.st_mode))
	return;

#ifdef __WIN32__
    string cmd;
    if (running_on_win9x()) {
	// For 95-like systems:
	cmd = "deltree /y";
    } else {
	// For NT-like systems:
	cmd = "rd /s /q";
    }
#else
    string cmd("rm -rf");
#endif
    if (!append_filename_argument(cmd, filename)) return;
    system(cmd);
}

/// Touch a file, just like the Unix "touch" command.
void touch(const string &filename) {
    int fd = open(filename.c_str(), O_CREAT|O_WRONLY|O_BINARY, 0644);
    if (fd >= 0) close(fd);
}
