/** @file flint_lock.cc
 * @brief Flint-compatible database locking.
 */
/* Copyright (C) 2005,2006,2007,2008,2009,2010,2011,2012,2013,2014,2015 Olly Betts
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

#include "flint_lock.h"

#ifndef __WIN32__
#include "safeerrno.h"

#include "safefcntl.h"
#include <unistd.h>
#include <cstdlib>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <signal.h>
#include <cstring>
#endif

#include "closefrom.h"
#include "errno_to_string.h"
#include "omassert.h"

#ifdef __CYGWIN__
# include <cygwin/version.h>
# include <sys/cygwin.h>
#endif

#include "xapian/error.h"

using namespace std;

#ifndef F_OFD_SETLK
# ifdef __linux__
// Apparently defining _GNU_SOURCE should get us F_OFD_SETLK, etc, but that
// doesn't actually seem to work, so hard-code the known values.
#  define F_OFD_GETLK	36
#  define F_OFD_SETLK	37
#  define F_OFD_SETLKW	38
# endif
#endif

FlintLock::reason
FlintLock::lock(bool exclusive, string & explanation) {
    // Currently we only support exclusive locks.
    (void)exclusive;
    Assert(exclusive);
#if defined __CYGWIN__ || defined __WIN32__
    Assert(hFile == INVALID_HANDLE_VALUE);
#ifdef __CYGWIN__
    char fnm[MAX_PATH];
#if CYGWIN_VERSION_API_MAJOR == 0 && CYGWIN_VERSION_API_MINOR < 181
    cygwin_conv_to_win32_path(filename.c_str(), fnm);
#else
    if (cygwin_conv_path(CCP_POSIX_TO_WIN_A|CCP_RELATIVE, filename.c_str(),
			 fnm, MAX_PATH) < 0) {
	explanation.assign("cygwin_conv_path failed: ");
	errno_to_string(errno, explanation);
	return UNKNOWN;
    }
#endif
#else
    const char *fnm = filename.c_str();
#endif
    hFile = CreateFile(fnm, GENERIC_WRITE, FILE_SHARE_READ,
		       NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) return SUCCESS;
    if (GetLastError() == ERROR_ALREADY_EXISTS) return INUSE;
    explanation = string();
    return UNKNOWN;
#elif defined __EMX__
    APIRET rc;
    ULONG ulAction;
    rc = DosOpen((PCSZ)filename.c_str(), &hFile, &ulAction, 0, FILE_NORMAL,
		 OPEN_ACTION_OPEN_IF_EXISTS | OPEN_ACTION_CREATE_IF_NEW,
		 OPEN_SHARE_DENYWRITE | OPEN_ACCESS_WRITEONLY,
		 NULL);
    if (rc == NO_ERROR) return SUCCESS;
    if (rc == ERROR_ACCESS_DENIED) return INUSE;
    explanation = string();
    return UNKNOWN;
#else
    Assert(fd == -1);
    int lockfd = open(filename.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (lockfd < 0) {
	// Couldn't open lockfile.
	explanation.assign("Couldn't open lockfile: ");
	errno_to_string(errno, explanation);
	return ((errno == EMFILE || errno == ENFILE) ? FDLIMIT : UNKNOWN);
    }

#ifdef F_OFD_SETLK
    // F_OFD_SETLK has exactly the semantics we want, so use it if it's
    // available.  Support was added in Linux 3.15, and there's work on
    // getting it standardised via POSIX:
    // http://austingroupbugs.net/view.php?id=768

    // Use a static flag so we don't repeatedly try F_OFD_SETLK when
    // the kernel in use doesn't support it.  This should be safe in a
    // threaded context - at worst multiple threads might end up trying
    // F_OFD_SETLK and then setting f_ofd_setlk_fails to true.
    static bool f_ofd_setlk_fails = false;
    if (!f_ofd_setlk_fails) {
	struct flock fl;
	fl.l_type = F_WRLCK;
	fl.l_whence = SEEK_SET;
	fl.l_start = 0;
	fl.l_len = 1;
	fl.l_pid = 0;
	while (fcntl(lockfd, F_OFD_SETLK, &fl) == -1) {
	    if (errno != EINTR) {
		if (errno == EINVAL) {
		    // F_OFD_SETLK not supported by this kernel.
		    goto no_ofd_support;
		}
		// Lock failed - translate known errno values into a reason
		// code.
		int e = errno;
		close(lockfd);
		switch (e) {
		    case EACCES: case EAGAIN:
			return INUSE;
		    case ENOLCK:
			return UNSUPPORTED;
		    default:
			return UNKNOWN;
		}
	    }
	}
	fd = lockfd;
	pid = 0;
	return SUCCESS;
no_ofd_support:
	f_ofd_setlk_fails = true;
    }
#endif

    // If stdin and/or stdout have been closed, it is possible that lockfd could
    // be 0 or 1.  We need fds 0 and 1 to be available in the child process to
    // be stdin and stdout, and we can't use dup() on lockfd after locking it,
    // as the lock won't be transferred, so we handle this corner case here by
    // using F_DUPFD or by calling dup() once or twice so that lockfd >= 2.
    if (rare(lockfd < 2)) {
	// Note this temporarily requires one or two spare fds to work, but
	// then we need two spare for socketpair() to succeed below anyway.
#ifdef F_DUPFD
	// Where available, F_DUPFD allows us to directly get the first unused
	// fd which is at least 2.
	int lockfd_dup = fcntl(lockfd, F_DUPFD, 2);
	int eno = errno;
	close(lockfd);
	if (lockfd_dup < 0) {
	    return ((eno == EMFILE || eno == ENFILE) ? FDLIMIT : UNKNOWN);
	}
	lockfd = lockfd_dup;
#else
	// Otherwise we have to call dup() until we get one, though at least
	// that's only at most twice.
	int lockfd_dup = dup(lockfd);
	if (rare(lockfd_dup < 2)) {
	    int eno = 0;
	    if (lockfd_dup < 0) {
		eno = errno;
		close(lockfd);
	    } else {
		int lockfd_dup2 = dup(lockfd);
		if (lockfd_dup2 < 0) {
		    eno = errno;
		}
		close(lockfd);
		close(lockfd_dup);
		lockfd = lockfd_dup2;
	    }
	    if (eno) {
		return ((eno == EMFILE || eno == ENFILE) ? FDLIMIT : UNKNOWN);
	    }
	} else {
	    close(lockfd);
	    lockfd = lockfd_dup;
	}
#endif
    }

    int fds[2];
    if (socketpair(AF_UNIX, SOCK_STREAM, PF_UNSPEC, fds) < 0) {
	// Couldn't create socketpair.
	explanation.assign("Couldn't create socketpair: ");
	errno_to_string(errno, explanation);
	reason why = ((errno == EMFILE || errno == ENFILE) ? FDLIMIT : UNKNOWN);
	(void)close(lockfd);
	return why;
    }

    pid_t child = fork();

    if (child == 0) {
	// Child process.
	close(fds[0]);

	// Connect pipe to stdin and stdout.
	dup2(fds[1], 0);
	dup2(fds[1], 1);

	// Make sure we don't hang on to open files which may get deleted but
	// not have their disk space released until we exit.  Close these
	// before we try to get the lock because if one of them is open on
	// the lock file then closing it after obtaining the lock would release
	// the lock, which would be really bad.
	for (int i = 2; i < lockfd; ++i) {
	    // Retry on EINTR; just ignore other errors (we'll get
	    // EBADF if the fd isn't open so that's OK).
	    while (close(i) < 0 && errno == EINTR) { }
	}
	closefrom(lockfd + 1);

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
	    while (write(1, &ch, 1) < 0) {
		// EINTR means a signal interrupted us, so retry.
		// Otherwise we're DOOMED!  The best we can do is just exit
		// and the parent process should get EOF and know the lock
		// failed.
		if (errno != EINTR) _exit(1);
	    }
	    if (why != SUCCESS) _exit(0);
	}

	// Make sure we don't block unmount() of partition holding the current
	// directory.
	if (chdir("/") < 0) {
	    // We can't usefully do anything in response to an error, so just
	    // ignore it - the worst harm it can do is make it impossible to
	    // unmount a partition.
	    //
	    // We need the if statement because glibc's _FORTIFY_SOURCE mode
	    // gives a warning even if we cast the result to void.
	}

	// FIXME: use special statically linked helper instead of cat.
	execl("/bin/cat", "/bin/cat", static_cast<void*>(NULL));
	// Emulate cat ourselves (we try to avoid this to reduce VM overhead).
	char ch;
	while (read(0, &ch, 1) != 0) { /* Do nothing */ }
	_exit(0);
    }

    close(lockfd);
    close(fds[1]);

    if (child == -1) {
	// Couldn't fork.
	explanation.assign("Couldn't fork: ");
	errno_to_string(errno, explanation);
	close(fds[0]);
	return UNKNOWN;
    }

    reason why = UNKNOWN;

    // Parent process.
    while (true) {
	char ch;
	ssize_t n = read(fds[0], &ch, 1);
	if (n == 1) {
	    why = static_cast<reason>(ch);
	    if (why != SUCCESS) break;
	    // Got the lock.
	    fd = fds[0];
	    pid = child;
	    return SUCCESS;
	}
	if (n == 0) {
	    // EOF means the lock failed.
	    explanation.assign("Got EOF reading from child process");
	    break;
	}
	if (errno != EINTR) {
	    // Treat unexpected errors from read() as failure to get the lock.
	    explanation.assign("Error reading from child process: ");
	    errno_to_string(errno, explanation);
	    break;
	}
    }

    close(fds[0]);

    int status;
    while (waitpid(child, &status, 0) < 0) {
	if (errno != EINTR) break;
    }

    return why;
#endif
}

void
FlintLock::release() {
#if defined __CYGWIN__ || defined __WIN32__
    if (hFile == INVALID_HANDLE_VALUE) return;
    CloseHandle(hFile);
    hFile = INVALID_HANDLE_VALUE;
#elif defined __EMX__
    if (hFile == NULLHANDLE) return;
    DosClose(hFile);
    hFile = NULLHANDLE;
#else
    if (fd < 0) return;
    close(fd);
    fd = -1;
#ifdef F_OFD_SETLK
    if (pid == 0) return;
#endif
    // Kill the child process which is holding the lock.  Use SIGKILL since
    // that can't be caught or ignored (we used to use SIGHUP, but if the
    // application has set that to SIG_IGN, the child process inherits that
    // setting, which sometimes results in the child process not exiting -
    // noted on Linux).
    //
    // The only likely error from kill is ESRCH (pid doesn't exist).  The other
    // possibilities (according to the Linux man page) are EINVAL (invalid
    // signal) and EPERM (don't have permission to SIGKILL the process) but in
    // none of the cases does calling waitpid do us any good!
    if (kill(pid, SIGKILL) == 0) {
	int status;
	while (waitpid(pid, &status, 0) < 0) {
	    if (errno != EINTR) break;
	}
    }
#endif
}

void
FlintLock::throw_databaselockerror(FlintLock::reason why,
				   const string & db_dir,
				   const string & explanation)
{
    string msg("Unable to get write lock on ");
    msg += db_dir;
    if (why == FlintLock::INUSE) {
	msg += ": already locked";
    } else if (why == FlintLock::UNSUPPORTED) {
	msg += ": locking probably not supported by this FS";
    } else if (why == FlintLock::FDLIMIT) {
	msg += ": too many open files";
    } else if (why == FlintLock::UNKNOWN) {
	if (!explanation.empty())
	    msg += ": " + explanation;
    }
    throw Xapian::DatabaseLockError(msg);
}
