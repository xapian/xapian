/** @file backendmanager_remotetcp.cc
 * @brief BackendManager subclass for remotetcp databases.
 */
/* Copyright (C) 2006,2007,2008,2009,2013,2015 Olly Betts
 * Copyright (C) 2008 Lemur Consulting Ltd
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

#include "backendmanager_remotetcp.h"

#include <xapian.h>

#include "safeerrno.h"
#include <stdio.h> // For fdopen().
#include <cstring>

#ifdef HAVE_FORK
# include <signal.h>
# include <sys/types.h>
# include "safesyssocket.h"
# include <sys/wait.h>
# include <unistd.h>
// Some older systems had SIGCLD rather than SIGCHLD.
# if !defined SIGCHLD && defined SIGCLD
#  define SIGCHLD SIGCLD
# endif
#endif

#ifdef __WIN32__
# include <io.h> // For _open_osfhandle().
# include "safefcntl.h"
# include "safewindows.h"
# include <cstdlib> // For free().
#endif

#include "noreturn.h"
#include "str.h"

#include <string>
#include <vector>

#ifdef HAVE_VALGRIND
# include <valgrind/memcheck.h>
#endif

using namespace std;

// We've had problems on some hosts which run tinderbox tests with "localhost"
// not being set in /etc/hosts - using the IP address equivalent seems more
// reliable.
#define LOCALHOST "127.0.0.1"

// Start at DEFAULT port and try higher ports until one isn't already in use.
#define DEFAULT_PORT 1239

#ifdef HAVE_FORK

// We can't dynamically allocate memory for this because it confuses the leak
// detector.  We only have 1-3 child fds open at once anyway, so a fixed size
// array isn't a problem, and linear scanning isn't a problem either.
struct pid_fd {
    pid_t pid;
    int fd;
};

static pid_fd pid_to_fd[16];

extern "C" {

static void
on_SIGCHLD(int /*sig*/)
{
    int status;
    pid_t child;
    while ((child = waitpid(-1, &status, WNOHANG)) > 0) {
	for (unsigned i = 0; i < sizeof(pid_to_fd) / sizeof(pid_fd); ++i) {
	    if (pid_to_fd[i].pid == child) {
		int fd = pid_to_fd[i].fd;
		pid_to_fd[i].fd = 0;
		pid_to_fd[i].pid = 0;
		// NB close() *is* safe to use in a signal handler.
		close(fd);
		break;
	    }
	}
    }
}

}

static int
launch_xapian_tcpsrv(const string & args)
{
    int port = DEFAULT_PORT;

    // We want to be able to get the exit status of the child process we fork
    // if xapian-tcpsrv doesn't start listening successfully.
    signal(SIGCHLD, SIG_DFL);
try_next_port:
    string cmd = XAPIAN_TCPSRV " --one-shot --interface " LOCALHOST " --port ";
    cmd += str(port);
    cmd += " ";
    cmd += args;
#ifdef HAVE_VALGRIND
    if (RUNNING_ON_VALGRIND) cmd = "./runsrv " + cmd;
#endif
    int fds[2];
    if (socketpair(AF_UNIX, SOCK_STREAM|SOCK_CLOEXEC, PF_UNSPEC, fds) < 0) {
	string msg("Couldn't create socketpair: ");
	msg += strerror(errno);
	throw msg;
    }

    pid_t child = fork();
    if (child == 0) {
	// Child process.
	close(fds[0]);
	// Connect stdout and stderr to the socket.
	//
	// Make sure the socket isn't fd 1 or 2.  We need to ensure that
	// FD_CLOEXEC isn't set for stdout or stderr (which creating them with
	// dup2() achieves), and that we close fds[1].  The cleanest way to
	// address this seems to be to turn the unusual situation into the
	// usual one.
	if (fds[1] == 1 || fds[1] == 2) {
	    dup2(fds[1], 3);
	    fds[1] = 3;
	}
	dup2(fds[1], 1);
	dup2(fds[1], 2);
	close(fds[1]);
	execl("/bin/sh", "/bin/sh", "-c", cmd.c_str(), static_cast<void*>(0));
	_exit(-1);
    }

    close(fds[1]);
    if (child == -1) {
	// Couldn't fork.
	int fork_errno = errno;
	close(fds[0]);
	string msg("Couldn't fork: ");
	msg += strerror(fork_errno);
	throw msg;
    }

    // Parent process.

    // Wrap the file descriptor in a FILE * so we can read lines using fgets().
    FILE * fh = fdopen(fds[0], "r");
    if (fh == NULL) {
	string msg("Failed to run command '");
	msg += cmd;
	msg += "': ";
	msg += strerror(errno);
	throw msg;
    }

    string output;
    while (true) {
	char buf[256];
	if (fgets(buf, sizeof(buf), fh) == NULL) {
	    fclose(fh);
	    // Wait for the child to exit.
	    int status;
	    if (waitpid(child, &status, 0) == -1) {
		string msg("waitpid failed: ");
		msg += strerror(errno);
		throw msg;
	    }
	    if (++port < 65536 && status != 0) {
		if (WIFEXITED(status) && WEXITSTATUS(status) == 69) {
		    // 69 is EX_UNAVAILABLE which xapian-tcpsrv exits
		    // with if (and only if) the port specified was
		    // in use.
		    goto try_next_port;
		}
	    }
	    string msg("Failed to get 'Listening...' from command '");
	    msg += cmd;
	    msg += "' (output: ";
	    msg += output;
	    msg += ")";
	    throw msg;
	}
	if (strcmp(buf, "Listening...\n") == 0) break;
	output += buf;
    }

    // dup() the fd we wrapped with fdopen() so we can keep it open so the
    // xapian-tcpsrv keeps running.
    int tracked_fd = dup(fds[0]);

    // We must fclose() the FILE* to avoid valgrind detecting memory leaks from
    // its buffers.
    fclose(fh);

    // Find a slot to track the pid->fd mapping in.  If we can't find a slot
    // it just means we'll leak the fd, so don't worry about that too much.
    for (unsigned i = 0; i < sizeof(pid_to_fd) / sizeof(pid_fd); ++i) {
	if (pid_to_fd[i].pid == 0) {
	    pid_to_fd[i].fd = tracked_fd;
	    pid_to_fd[i].pid = child;
	    break;
	}
    }

    // Set a signal handler to clean up the xapian-tcpsrv child process when it
    // finally exits.
    signal(SIGCHLD, on_SIGCHLD);

    return port;
}

#elif defined __WIN32__

XAPIAN_NORETURN(static void win32_throw_error_string(const char * str));
static void win32_throw_error_string(const char * str)
{
    string msg(str);
    char * error = 0;
    DWORD len;
    len = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_ALLOCATE_BUFFER,
			0, GetLastError(), 0, (CHAR*)&error, 0, 0);
    if (error) {
	// Remove any trailing \r\n from output of FormatMessage.
	if (len >= 2 && error[len - 2] == '\r' && error[len - 1] == '\n')
	    len -= 2;
	if (len) {
	    msg += ": ";
	    msg.append(error, len);
	}
	LocalFree(error);
    }
    throw msg;
}

// This implementation uses the WIN32 API to start xapian-tcpsrv as a child
// process and read its output using a pipe.
static int
launch_xapian_tcpsrv(const string & args)
{
    int port = DEFAULT_PORT;

try_next_port:
    string cmd = XAPIAN_TCPSRV " --one-shot --interface " LOCALHOST " --port ";
    cmd += str(port);
    cmd += " ";
    cmd += args;

    // Create a pipe so we can read stdout/stderr from the child process.
    HANDLE hRead, hWrite;
    if (!CreatePipe(&hRead, &hWrite, 0, 0))
	win32_throw_error_string("Couldn't create pipe");

    // Set the write handle to be inherited by the child process.
    SetHandleInformation(hWrite, HANDLE_FLAG_INHERIT, 1);

    // Create the child process.
    PROCESS_INFORMATION procinfo;
    memset(&procinfo, 0, sizeof(PROCESS_INFORMATION));

    STARTUPINFO startupinfo;
    memset(&startupinfo, 0, sizeof(STARTUPINFO));
    startupinfo.cb = sizeof(STARTUPINFO);
    startupinfo.hStdError = hWrite;
    startupinfo.hStdOutput = hWrite;
    startupinfo.hStdInput = INVALID_HANDLE_VALUE;
    startupinfo.dwFlags |= STARTF_USESTDHANDLES;

    // For some reason Windows wants a modifiable copy!
    BOOL ok;
    char * cmdline = strdup(cmd.c_str());
    ok = CreateProcess(0, cmdline, 0, 0, TRUE, 0, 0, 0, &startupinfo, &procinfo);
    free(cmdline);
    if (!ok)
	win32_throw_error_string("Couldn't create child process");

    CloseHandle(hWrite);
    CloseHandle(procinfo.hThread);

    string output;
    FILE *fh = fdopen(_open_osfhandle(intptr_t(hRead), O_RDONLY), "r");
    while (true) {
	char buf[256];
	if (fgets(buf, sizeof(buf), fh) == NULL) {
	    fclose(fh);
	    DWORD rc;
	    // This doesn't seem to be necessary on the machine I tested on,
	    // but I guess it could be on a slow machine...
	    while (GetExitCodeProcess(procinfo.hProcess, &rc) && rc == STILL_ACTIVE) {
		Sleep(100);
	    }
	    CloseHandle(procinfo.hProcess);
	    if (++port < 65536 && rc == 69) {
		// 69 is EX_UNAVAILABLE which xapian-tcpsrv exits
		// with if (and only if) the port specified was
		// in use.
		goto try_next_port;
	    }
	    string msg("Failed to get 'Listening...' from command '");
	    msg += cmd;
	    msg += "' (output: ";
	    msg += output;
	    msg += ")";
	    throw msg;
	}
	if (strcmp(buf, "Listening...\r\n") == 0) break;
	output += buf;
    }
    fclose(fh);

    return port;
}

#else
# error Neither HAVE_FORK nor __WIN32__ is defined
#endif

BackendManagerRemoteTcp::~BackendManagerRemoteTcp() {
    BackendManagerRemoteTcp::clean_up();
}

std::string
BackendManagerRemoteTcp::get_dbtype() const
{
    return "remotetcp_" + remote_type;
}

Xapian::Database
BackendManagerRemoteTcp::do_get_database(const vector<string> & files)
{
    // Default to a long (5 minute) timeout so that tests won't fail just
    // because the host is slow or busy.
    return BackendManagerRemoteTcp::get_remote_database(files, 300000);
}

Xapian::WritableDatabase
BackendManagerRemoteTcp::get_writable_database(const string & name,
					       const string & file)
{
    string args = get_writable_database_args(name, file);
    int port = launch_xapian_tcpsrv(args);
    return Xapian::Remote::open_writable(LOCALHOST, port);
}

Xapian::Database
BackendManagerRemoteTcp::get_remote_database(const vector<string> & files,
					     unsigned int timeout)
{
    string args = get_remote_database_args(files, timeout);
    int port = launch_xapian_tcpsrv(args);
    return Xapian::Remote::open(LOCALHOST, port);
}

Xapian::Database
BackendManagerRemoteTcp::get_writable_database_as_database()
{
    string args = get_writable_database_as_database_args();
    int port = launch_xapian_tcpsrv(args);
    return Xapian::Remote::open(LOCALHOST, port);
}

Xapian::WritableDatabase
BackendManagerRemoteTcp::get_writable_database_again()
{
    string args = get_writable_database_again_args();
    int port = launch_xapian_tcpsrv(args);
    return Xapian::Remote::open_writable(LOCALHOST, port);
}

void
BackendManagerRemoteTcp::clean_up()
{
#ifdef HAVE_FORK
    signal(SIGCHLD, SIG_DFL);
    for (unsigned i = 0; i < sizeof(pid_to_fd) / sizeof(pid_fd); ++i) {
	pid_t child = pid_to_fd[i].pid;
	if (child) {
	    int status;
	    while (waitpid(child, &status, 0) == -1 && errno == EINTR) { }
	    // Other possible error from waitpid is ECHILD, which it seems can
	    // only mean that the child has already exited and SIGCHLD was set
	    // to SIG_IGN.  If we did somehow see that, the sanest response
	    // seems to be to close the fd and move on.
	    int fd = pid_to_fd[i].fd;
	    pid_to_fd[i].fd = 0;
	    pid_to_fd[i].pid = 0;
	    close(fd);
	}
    }
#endif
}
