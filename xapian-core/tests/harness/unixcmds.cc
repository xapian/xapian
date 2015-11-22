/** @file unixcmds.cc
 *  @brief C++ function versions of useful Unix commands.
 */
/* Copyright (C) 2003,2004,2007,2012,2014,2015 Olly Betts
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
#include "safeunistd.h"
#include "safefcntl.h"

#ifdef __WIN32__
# include "safewindows.h"
#endif

#include "append_filename_arg.h"
#include "filetests.h"
#include "str.h"

using namespace std;

/// Call system() and throw exception if it fails.
static void
checked_system(const string & cmd)
{
    int res = system(cmd.c_str());
    if (res) {
	string msg = "system(\"";
	msg += cmd;
	msg += "\") failed, returning ";
	msg += str(res);
	throw msg;
    }
}

/// Recursively copy a directory.
void cp_R(const std::string &src, const std::string &dest) {
#ifdef __WIN32__
    // We create the target directory first to avoid being prompted as to
    // whether we want to create a directory or a file (which makes no sense
    // when copying a directory, but that's how xcopy seems to work!)
    mkdir(dest.c_str());
    string cmd("xcopy /E /Y");
#else
    string cmd("cp -R");
#endif
    if (!append_filename_argument(cmd, src)) return;
    if (!append_filename_argument(cmd, dest)) return;
    checked_system(cmd);
#ifndef __WIN32__
    // Allow write access to the copy (to deal with builds where srcdir is
    // readonly).
    cmd = "chmod -R +w";
    if (!append_filename_argument(cmd, dest)) return;
    checked_system(cmd);
#endif
}

/// Remove a directory and contents, just like the Unix "rm -rf" command.
void rm_rf(const string &filename) {
    // Check filename exists and is actually a directory
    if (filename.empty() || !dir_exists(filename))
	return;

#ifdef __WIN32__
    string cmd("rd /s /q");
#else
    string cmd("rm -rf");
#endif
    if (!append_filename_argument(cmd, filename)) return;
    checked_system(cmd);
}

/// Touch a file, just like the Unix "touch" command.
void touch(const string &filename) {
    int fd = open(filename.c_str(), O_CREAT|O_WRONLY|O_BINARY, 0644);
    if (fd >= 0) close(fd);
}
