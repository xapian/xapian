/* flint_lock.cc: database locking for flint backend.
 *
 * Copyright (C) 2005,2006,2007 Olly Betts
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301
 * USA
 */

#include <config.h>

#ifndef __WIN32__
#include "safeerrno.h"

#include "safefcntl.h"
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <signal.h>
#endif

#include "flint_lock.h"

#include "omassert.h"

#ifdef __CYGWIN__
#include <sys/cygwin.h>
#endif

FlintLock::reason
FlintLock::lock(bool exclusive) {
    // Currently we only support exclusive locks.
    (void)exclusive;
    Assert(exclusive);
#if defined __CYGWIN__ || defined __WIN32__
    Assert(hFile == INVALID_HANDLE_VALUE);
#ifdef __CYGWIN__
    char fnm[MAX_PATH];
    cygwin_conv_to_win32_path(filename.c_str(), fnm);
#else
    const char *fnm = filename.c_str();
#endif
    hFile = CreateFile(fnm, GENERIC_WRITE, FILE_SHARE_READ,
		       NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) return SUCCESS;
    if (GetLastError() == ERROR_ALREADY_EXISTS) return INUSE;
    return UNKNOWN;
#else
    Assert(fd == -1);
    int lockfd = open(filename.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0600);
    if (lockfd < 0) return UNKNOWN; // Couldn't open lockfile.

    int fds[2];
    if (socketpair(AF_UNIX, SOCK_STREAM, PF_UNSPEC, fds) < 0) {
	// Couldn't create socketpair.
	close(lockfd);
	return UNKNOWN;
    }

    pid_t child = fork();
    if (child == 0) {
	// Child process.
	close(fds[0]);

	reason why = SUCCESS;
	{
	    struct flock fl;
	    fl.l_type = F_WRLCK;
	    fl.l_whence = SEEK_SET;
	    fl.l_start = 0;
	    fl.l_len = 1;
	    while (fcntl(lockfd, F_SETLK, &fl) == -1) {
		if (errno != EINTR) {
		    // Lock failed - translate known errno values into a reason
		    // code.
		    if (errno == EACCES || errno == EAGAIN) {
			why = INUSE;
		    } else if (errno == ENOLCK) {
			why = UNSUPPORTED;
		    } else {
			_exit(0);
		    }
		    break;
		}
	    }
	}

	{
	    // Tell the parent if we got the lock, and if not, why not.
	    char ch = static_cast<char>(why);
	    while (write(fds[1], &ch, 1) < 0) {
		// EINTR means a signal interrupted us, so retry.
		// Otherwise we're DOOMED!  The best we can do is just exit
		// and the parent process should get EOF and know the lock
		// failed.
		if (errno != EINTR) _exit(1);
	    }
	    if (why != SUCCESS) _exit(0);
	}

	//shutdown(fds[1], 1); // Disable further sends.
	// Connect pipe to stdin.
	dup2(fds[1], 0);
	// FIXME: use special statically linked helper instead of cat.
	execl("/bin/cat", "/bin/cat", (void*)NULL);
	// Emulate cat ourselves (we try to avoid this to reduce VM overhead).
	char ch;
	while (read(0, &ch, 1) != 0) { /* Do nothing */ }
	_exit(0);
    }

    close(lockfd);

    if (child == -1) {
	// Couldn't fork.
	close(fds[0]);
	close(fds[1]);
	return UNKNOWN;
    }

    // Parent process.
    close(fds[1]);
    while (true) {
	char ch;
	int n = read(fds[0], &ch, 1);
	if (n == 1) {
	    reason why = static_cast<reason>(ch);
	    if (why == SUCCESS) break; // Got the lock.
	    close(fds[0]);
	    return why;
	}
	if (n == 0 || errno != EINTR) {
	    // EOF means the lock failed; we also treat unexpected errors from
	    // read() the same way.
	    close(fds[0]);
	    return UNKNOWN;
	}
    }
    //shutdown(fds[0], 0); // Disable further receives.
    fd = fds[0];
    pid = child;
    return SUCCESS;
#endif
}

void
FlintLock::release() {
#if defined __CYGWIN__ || defined __WIN32__
    if (hFile == INVALID_HANDLE_VALUE) return;
    CloseHandle(hFile);
    hFile = INVALID_HANDLE_VALUE;
#else
    if (fd < 0) return;
    close(fd);
    fd = -1;
    // The only likely error from kill is ESRCH.  The other possibilities
    // (according to the Linux man page) are EINVAL (invalid signal) and EPERM
    // (don't have permission to SIGHUP the process) but in none of the cases
    // does calling waitpid do us any good!
    if (kill(pid, SIGHUP) == 0) {
	int status;
	while (waitpid(pid, &status, 0) < 0) {
	    if (errno != EINTR) break;
	}
    }
#endif
}
