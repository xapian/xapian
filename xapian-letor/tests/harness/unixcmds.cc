/** @file
 *  @brief C++ function versions of useful Unix commands.
 */
/* Copyright (C) 2003,2004,2007,2012,2014,2015,2018 Olly Betts
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

#include <string>
#include <cerrno>
#include <cstdlib>
#include <sys/types.h>
#include "safeunistd.h"
#include "safefcntl.h"

#include "append_filename_arg.h"
#include "errno_to_string.h"
#include "filetests.h"
#include "str.h"

// mingw-w64 added an implementation of nftw() but it seems to be buggy and
// garble paths, though I can't see where in the code things go wrong and
// their code seems to work when built on Linux.
//
// For now, we just blacklist nftw() here and on mingw32 (which doesn't
// currently have it, but let's be defensive in case somebody copies over the
// buggy version).  Using nftw() is just a minor optimisation which only
// makes a real difference for developers running the testsuite a lot.
#if defined HAVE_NFTW && !defined __MINGW32__
# include <ftw.h>
# include <unistd.h>
#endif

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
    string cmd("xcopy /E /q /Y");
#else
    string cmd("cp -R");
#endif
    if (!append_filename_argument(cmd, src)) return;
    if (!append_filename_argument(cmd, dest)) return;
#ifdef __WIN32__
    // xcopy reports how many files it copied, even with /q.
    cmd += " >nul";
#endif
    checked_system(cmd);
#ifndef __WIN32__
    // Allow write access to the copy (to deal with builds where srcdir is
    // readonly).
    cmd = "chmod -R +w";
    if (!append_filename_argument(cmd, dest)) return;
    checked_system(cmd);
#endif
}

#if defined HAVE_NFTW && !defined __MINGW32__
extern "C" {
static int
rm_rf_nftw_helper(const char* path,
		  const struct stat*,
		  int type,
		  struct FTW*)
{
    int r = (type == FTW_DP ? rmdir(path) : unlink(path));
    // Return the errno value if deletion fails as the nftw() function might
    // overwrite errno during clean-up.  Any non-zero return value will end
    // the walk.
    return r < 0 ? errno : 0;
}
}
#endif

/// Remove a directory and contents, just like the Unix "rm -rf" command.
void rm_rf(const string &filename) {
    // Check filename exists and is actually a directory
    if (filename.empty() || !dir_exists(filename))
	return;

#if defined HAVE_NFTW && !defined __MINGW32__
    auto flags = FTW_DEPTH | FTW_PHYS;
    int retries = 5;
    while (true) {
	int eno = nftw(filename.c_str(), rm_rf_nftw_helper, 10, flags);
	if (eno == 0)
	    return;

	// nftw() either returns 0 for OK, -1 for error, or the non-zero return
	// value of the helper (which in our case is an errno value).
	if (eno < 0)
	    eno = errno;
	if (!(eno == EEXIST || eno == ENOTEMPTY) || --retries == 0) {
	    string msg = "recursive delete of \"";
	    msg += filename;
	    msg += "\") failed, errno = ";
	    errno_to_string(eno, msg);
	    throw msg;
	}

	// On NFS, rmdir() can fail with EEXIST or ENOTEMPTY (POSIX allows
	// either) due to .nfs* files which are used by NFS clients to
	// implement the Unix semantics of a deleted but open file continuing
	// to exist.  We sleep and retry a few times in this situation to give
	// the NFS client a chance to process the closing of the open handle.
	sleep(5);
    }
#else
# ifdef __WIN32__
    string cmd("rd /s /q");
# else
    string cmd("rm -rf");
# endif
    if (!append_filename_argument(cmd, filename)) return;
    checked_system(cmd);
#endif
}

/// Touch a file, just like the Unix "touch" command.
void touch(const string &filename) {
    int fd = open(filename.c_str(), O_CREAT|O_WRONLY|O_BINARY, 0644);
    if (fd >= 0) close(fd);
}
