/** @file
 * @brief Flint-compatible database locking.
 */
/* Copyright (C) 2005,2006,2007,2008,2009,2010,2011,2012,2013,2014,2015,2016,2017 Olly Betts
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
#include <cerrno>

#include "safefcntl.h"
#include <unistd.h>
#include <cstdlib>
#include <sys/types.h>
#include "safesyssocket.h"
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

#ifdef FLINTLOCK_USE_FLOCK
# include <sys/file.h>
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

[[noreturn]]
static void
throw_cannot_test_lock()
{
    throw Xapian::FeatureUnavailableError("Can't test lock without trying to "
					  "take it");
}

bool
FlintLock::test() const
{
    // A database which doesn't support update can't be locked for update.
    if (filename.empty()) return false;

#if defined __CYGWIN__ || defined __WIN32__
    if (hFile != INVALID_HANDLE_VALUE) return true;
    // Doesn't seem to be possible to check if the lock is held without briefly
    // taking the lock.
    throw_cannot_test_lock();
#elif defined FLINTLOCK_USE_FLOCK
    if (fd != -1) return true;
    // Doesn't seem to be possible to check if the lock is held without briefly
    // taking the lock.
    throw_cannot_test_lock();
#else
    if (fd != -1) return true;
    int lockfd = open(filename.c_str(), O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, 0666);
    if (lockfd < 0) {
	// Couldn't open lockfile.
	reason why = ((errno == EMFILE || errno == ENFILE) ? FDLIMIT : UNKNOWN);
	throw_databaselockerror(why, filename, "Testing lock");
    }

    struct flock fl;
    fl.l_type = F_WRLCK;
    fl.l_whence = SEEK_SET;
    fl.l_start = 0;
    fl.l_len = 1;
    fl.l_pid = 0;
    while (fcntl(lockfd, F_GETLK, &fl) == -1) {
	if (errno != EINTR) {
	    // Translate known errno values into a reason code.
	    int e = errno;
	    close(lockfd);
	    if (e == ENOSYS) {
		// F_GETLK always failed with ENOSYS on older GNU Hurd libc
		// versions: https://bugs.debian.org/190367
		throw_cannot_test_lock();
	    }
	    reason why = (e == ENOLCK ? UNSUPPORTED : UNKNOWN);
	    throw_databaselockerror(why, filename, "Testing lock");
	}
    }
    close(lockfd);
    return fl.l_type != F_UNLCK;
#endif
}

FlintLock::reason
FlintLock::lock(bool exclusive, bool wait, string & explanation) {
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
retry:
    // FIXME: Use LockFileEx() for locking, which would allow proper blocking
    // and also byte-range locking for when we implement MVCC.  But is there a
    // way to interwork with the CreateFile()-based locking while doing so?
    hFile = CreateFile(fnm, GENERIC_WRITE, FILE_SHARE_READ,
		       NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) return SUCCESS;
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
	if (wait) {
	    Sleep(1000);
	    goto retry;
	}
	return INUSE;
    }
    explanation = string();
    return UNKNOWN;
#elif defined FLINTLOCK_USE_FLOCK
    // This is much simpler than using fcntl() due to saner semantics around
    // releasing locks when closing other descriptors on the same file (at
    // least on platforms where flock() isn't just a compatibility wrapper
    // around fcntl()).  We can't simply switch to this without breaking
    // locking compatibility with previous releases, though it might be useful
    // for porting to platforms without fcntl() locking.
    //
    // Also, flock() is problematic over NFS at least on Linux - it's been
    // supported since Linux 2.6.12 but it's actually emulated by taking out an
    // fcntl() byte-range lock on the entire file, which means that a process
    // on the NFS server can get a (genuine) flock() lock on the same file a
    // process on an NFS client has locked by flock() emulated as an fcntl()
    // lock.
    Assert(fd == -1);
    int lockfd = open(filename.c_str(), O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, 0666);
    if (lockfd < 0) {
	// Couldn't open lockfile.
	explanation.assign("Couldn't open lockfile: ");
	errno_to_string(errno, explanation);
	return ((errno == EMFILE || errno == ENFILE) ? FDLIMIT : UNKNOWN);
    }

    int op = LOCK_EX;
    if (!wait) op |= LOCK_NB;
    while (flock(lockfd, op) == -1) {
	if (errno != EINTR) {
	    // Lock failed - translate known errno values into a reason code.
	    close(lockfd);
	    switch (errno) {
		case EWOULDBLOCK:
		    return INUSE;
		case ENOLCK:
		    return UNSUPPORTED; // FIXME: what do we get for NFS?
		default:
		    return UNKNOWN;
	    }
	}
    }

    fd = lockfd;
    return SUCCESS;
#else
    Assert(fd == -1);
    // Set the close-on-exec flag.  If we don't have OFD locks then our child
    // will clear it after we fork() but before the child exec()s so that
    // there's no window where another thread in the parent process could
    // fork()+exec() and end up with these fds still open.
    int lockfd = open(filename.c_str(), O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, 0666);
    if (lockfd < 0) {
	// Couldn't open lockfile.
	explanation.assign("Couldn't open lockfile: ");
	errno_to_string(errno, explanation);
	return ((errno == EMFILE || errno == ENFILE) ? FDLIMIT : UNKNOWN);
    }

#ifdef F_OFD_SETLK
    // F_OFD_SETLK has exactly the semantics we want, so use it if it's
    // available.  Support was added in Linux 3.15, and it was accepted
    // for POSIX issue 8 on 2022-12-15:
    // https://austingroupbugs.net/view.php?id=768

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
	while (fcntl(lockfd, wait ? F_OFD_SETLKW : F_OFD_SETLK, &fl) == -1) {
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

    int fds[2];
    if (socketpair(AF_UNIX, SOCK_STREAM|SOCK_CLOEXEC, PF_UNSPEC, fds) < 0) {
	// Couldn't create socketpair.
	explanation.assign("Couldn't create socketpair: ");
	errno_to_string(errno, explanation);
	reason why = ((errno == EMFILE || errno == ENFILE) ? FDLIMIT : UNKNOWN);
	(void)close(lockfd);
	return why;
    }
    // "The two sockets are indistinguishable" so we can just swap the fds
    // if we want, and being able to assume fds[1] != 2 is useful in the child
    // code below.
    if (rare(fds[1] == 2)) swap(fds[0], fds[1]);

    pid_t child = fork();

    if (child == 0) {
	// Child process.

	// Close the other socket which ensures we have at least one free fd.
	// That means that we don't expect failure due to not having a free
	// file descriptor, but we still check for it as it might be possible
	// e.g. if the process open fd limit was reduced by another thread
	// between socketpair() and fork().
	close(fds[0]);
	int parentfd = fds[1];

	// Closing *ANY* file descriptor open on the lock file will release the
	// lock.  Therefore before we attempt to take the lock, any other fds
	// which could be open on the lock file must have been closed.
	//
	// We also need to arrange that fds 0 and 1 are the socket back to our
	// parent (so that exec-ing /bin/cat does what we want) and both need
	// to have their close-on-exec flag cleared (we can achieve all this
	// part with two dup2() calls with a little care).  This means that
	// we need lockfd >= 2, but we actually arrange that lockfd == 2 since
	// then we can call closefrom(3) to close any other open fds in a
	// single call.
	//
	// If we need to use dup() to clear the close-on-exec flag on lockfd,
	// that must also have been done (because otherwise we can't close
	// the original, and since it has close-on-exec set, that means we
	// can't call exec()).

	bool lockfd_cloexec_cleared = false;
	int dup_parent_to_first = parentfd == 0 ? 1 : 0;
	if (rare(lockfd < 2)) {
	    int oldlockfd = lockfd;
	    // This dup2() will clear the close-on-exec flag for the new lockfd.
	    // Note that we ensured above that parentfd != 2.
	    lockfd = dup2(lockfd, 2);
	    if (rare(lockfd < 0)) goto report_dup_failure;
	    lockfd_cloexec_cleared = true;
	    // Ensure we reuse an already open fd as we just used up our spare.
	    dup_parent_to_first = oldlockfd;
	}

	// Connect our stdin and stdout to our parent via the socket.  With
	// a little care here we ensure that both dup2() calls actually
	// duplicate the fd and so the close-on-exec flag should be clear for
	// both fds 0 and 1.
	if (rare(dup2(parentfd, dup_parent_to_first) < 0)) {
report_dup_failure:
	    _exit((errno == EMFILE || errno == ENFILE) ? FDLIMIT : UNKNOWN);
	}
	close(parentfd);
	if (rare(dup2(dup_parent_to_first, dup_parent_to_first ^ 1)))
	    goto report_dup_failure;

	// Ensure lockfd is fd 2, and clear close-on-exec if necessary.
	if (lockfd != 2) {
	    // This dup2() will clear the close-on-exec flag for the new lockfd.
	    lockfd = dup2(lockfd, 2);
	    if (rare(lockfd < 0)) goto report_dup_failure;
	} else if (!lockfd_cloexec_cleared && O_CLOEXEC != 0) {
#if defined F_SETFD && defined FD_CLOEXEC
	    (void)fcntl(lockfd, F_SETFD, 0);
#else
	    // We use dup2() twice to clear the close-on-exec flag but keep
	    // lockfd == 2.
	    if (rare(dup2(lockfd, 3) < 0 || dup2(3, lockfd) < 0))
		goto report_dup_failure;
#endif
	}

	closefrom(3);

	{
	    struct flock fl;
	    fl.l_type = F_WRLCK;
	    fl.l_whence = SEEK_SET;
	    fl.l_start = 0;
	    fl.l_len = 1;
	    while (fcntl(lockfd, wait ? F_SETLKW : F_SETLK, &fl) == -1) {
		if (errno != EINTR) {
		    // Lock failed - translate known errno values into a reason
		    // code.
		    if (errno == EACCES || errno == EAGAIN) {
			_exit(INUSE);
		    } else if (errno == ENOLCK) {
			_exit(UNSUPPORTED);
		    } else {
			_exit(UNKNOWN);
		    }
		    break;
		}
	    }
	}

	{
	    // Tell the parent if we got the lock by writing a byte.
	    while (write(1, "", 1) < 0) {
		// EINTR means a signal interrupted us, so retry.
		//
		// Otherwise we can't tell our parent that we got the lock so
		// we just exit and our parent will think that the locking
		// attempt failed for "UNKNOWN" reasons.
		if (errno != EINTR) _exit(UNKNOWN);
	    }
	}

	// Make sure we don't block unmount of partition holding the current
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
	while (read(0, &ch, 1) != 0) {
	    /* Do nothing */
	}
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

    // Parent process.
    while (true) {
	char ch;
	ssize_t n = read(fds[0], &ch, 1);
	if (n == 1) {
	    // Got the lock.
	    fd = fds[0];
	    pid = child;
	    return SUCCESS;
	}
	if (n == 0) {
	    // EOF means the lock failed.  The child's exit status should be a
	    // reason code.
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
	if (errno != EINTR) return UNKNOWN;
    }

    reason why = UNKNOWN;
    if (WIFEXITED(status)) {
	int exit_status = WEXITSTATUS(status);
	if (usual(exit_status > 0 && exit_status <= UNKNOWN))
	    why = static_cast<reason>(exit_status);
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
#elif defined FLINTLOCK_USE_FLOCK
    if (fd < 0) return;
    close(fd);
    fd = -1;
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
				   const string & explanation) const
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
