/** @file
 *  @brief Implementation of RemoteDatabase using a spawned server.
 */
/* Copyright (C) 2007-2023 Olly Betts
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

#include "progclient.h"

#include <xapian/error.h>

#include <cerrno>
#include <string>
#include <vector>

#include "safefcntl.h"

#include <sys/types.h>
#ifndef __WIN32__
# include "safesyssocket.h"
# include "safeunistd.h"
# include <sys/wait.h>
#else
# include <cinttypes> // For PRIx64
# include <cstdio> // For snprintf().
# include <io.h>
#endif

#include "closefrom.h"
#include "debuglog.h"

using namespace std;

pair<int, string>
ProgClient::run_program(const string& progname,
			const string& args,
#ifndef __WIN32__
			pid_t& child
#else
			HANDLE& child
#endif
			)
{
    LOGCALL_STATIC(DB, RETURN_TYPE(pair<int, string>), "ProgClient::run_program", progname | args | Literal("[&child]"));

    string context{"remote:prog("};
    context += progname;
    context += ' ';
    context += args;
    context += ')';

#if defined HAVE_SOCKETPAIR && defined HAVE_FORK
    int fds[2];

    if (socketpair(PF_UNIX, SOCK_STREAM|SOCK_CLOEXEC, 0, fds) < 0) {
	throw Xapian::NetworkError("socketpair failed", context, errno);
    }

    // Make a copy of args - we replace spaces with zero bytes and build
    // a char* array that points into this which we pass to execvp() as argv.
    //
    // We do this before we call fork() because we shouldn't do anything
    // which calls malloc() in the child process.
    string args_buf = args;
    vector<char*> argv;
    argv.push_back(const_cast<char*>(progname.c_str()));
    if (!args_buf.empty()) {
	// Split argument list on spaces.
	argv.push_back(&args_buf[0]);
	for (char& ch : args_buf) {
	    if (ch == ' ') {
		// Drop the previous element if it's empty (either due to leading
		// spaces or multiple consecutive spaces).
		if (&ch == argv.back()) argv.pop_back();
		ch = '\0';
		argv.push_back(&ch + 1);
	    }
	}
	// Drop final element if it's empty (due to trailing space(s)).
	if (&args_buf.back() == argv.back()) argv.pop_back();
    }
    argv.push_back(nullptr);

    child = fork();

    if (child != 0) {
	// Not the child process.

	// Close the child's end of the pipe.
	::close(fds[1]);
	if (child < 0) {
	    // Couldn't fork.
	    ::close(fds[0]);
	    throw Xapian::NetworkError("fork failed", context, errno);
	}

	// Parent process.
	RETURN({fds[0], context});
    }

    // Child process.

    // Close the parent's end of the pipe.
    ::close(fds[0]);

# if defined F_SETFD && defined FD_CLOEXEC
    // Clear close-on-exec flag, if we set it when we called socketpair().
    // Clearing it here means there's no window where another thread in the
    // parent process could fork()+exec() and end up with this fd still
    // open (assuming close-on-exec is supported).
    //
    // We can't use a preprocessor check on the *value* of SOCK_CLOEXEC as
    // on Linux SOCK_CLOEXEC is an enum, with '#define SOCK_CLOEXEC
    // SOCK_CLOEXEC' to allow '#ifdef SOCK_CLOEXEC' to work.
    if (SOCK_CLOEXEC != 0)
	(void)fcntl(fds[1], F_SETFD, 0);
# endif

    // Connect pipe to stdin and stdout.
    dup2(fds[1], 0);
    dup2(fds[1], 1);

    // Make sure we don't hang on to open files which may get deleted but
    // not have their disk space released until we exit.
    closefrom(2);

    // Redirect stderr to /dev/null
    int devnull = open("/dev/null", O_WRONLY);
    if (devnull == -1) {
	// We can't throw an exception here or usefully flag the failure.
	// Best option seems to be to continue with stderr closed.
    } else if (rare(devnull != 2)) {
	// We expect to get fd 2 as that's the first free one, but handle
	// if we don't for some reason.
	dup2(devnull, 2);
	::close(devnull);
    }

    execvp(progname.c_str(), argv.data());

    // execvp() failed - all we can usefully do is exit.
    _exit(-1);
#elif defined __WIN32__
    LARGE_INTEGER counter;
    // QueryPerformanceCounter() will always succeed on XP and later
    // and gives us a counter which increments each CPU clock cycle
    // on modern hardware (Pentium or newer).
    QueryPerformanceCounter(&counter);
    char pipename[256];
    snprintf(pipename, sizeof(pipename),
	     "\\\\.\\pipe\\xapian-remote-%lx-%lx_%" PRIx64,
	     static_cast<unsigned long>(GetCurrentProcessId()),
	     static_cast<unsigned long>(GetCurrentThreadId()),
	     static_cast<unsigned long long>(counter.QuadPart));
    pipename[sizeof(pipename) - 1] = '\0';
    // Create a pipe so we can read stdout from the child process.
    HANDLE hPipe = CreateNamedPipe(pipename,
				   PIPE_ACCESS_DUPLEX|FILE_FLAG_OVERLAPPED,
				   0,
				   1, 4096, 4096, NMPWAIT_USE_DEFAULT_WAIT,
				   NULL);

    if (hPipe == INVALID_HANDLE_VALUE) {
	throw Xapian::NetworkError("CreateNamedPipe failed",
				   context,
				   -int(GetLastError()));
    }

    HANDLE hClient = CreateFile(pipename,
				GENERIC_READ|GENERIC_WRITE, 0, NULL,
				OPEN_EXISTING,
				FILE_FLAG_OVERLAPPED, NULL);

    if (hClient == INVALID_HANDLE_VALUE) {
	throw Xapian::NetworkError("CreateFile failed",
				   context,
				   -int(GetLastError()));
    }

    if (!ConnectNamedPipe(hPipe, NULL) &&
	GetLastError() != ERROR_PIPE_CONNECTED) {
	throw Xapian::NetworkError("ConnectNamedPipe failed",
				   context,
				   -int(GetLastError()));
    }

    // Set the appropriate handles to be inherited by the child process.
    SetHandleInformation(hClient, HANDLE_FLAG_INHERIT, 1);

    // Create the child process.
    PROCESS_INFORMATION procinfo;
    memset(&procinfo, 0, sizeof(PROCESS_INFORMATION));

    STARTUPINFO startupinfo;
    memset(&startupinfo, 0, sizeof(STARTUPINFO));
    startupinfo.cb = sizeof(STARTUPINFO);
    startupinfo.hStdError = hClient;
    startupinfo.hStdOutput = hClient;
    startupinfo.hStdInput = hClient;
    startupinfo.dwFlags |= STARTF_USESTDHANDLES;

    string cmdline{progname};
    cmdline += ' ';
    cmdline += args;
    // For some reason Windows wants a modifiable command line so we
    // pass `&cmdline[0]` rather than `cmdline.c_str()`.
    BOOL ok = CreateProcess(progname.c_str(), &cmdline[0], 0, 0, TRUE, 0, 0, 0,
			    &startupinfo, &procinfo);
    if (!ok) {
	throw Xapian::NetworkError("CreateProcess failed",
				   context,
				   -int(GetLastError()));
    }

    CloseHandle(hClient);
    CloseHandle(procinfo.hThread);
    child = procinfo.hProcess;
    RETURN({_open_osfhandle(intptr_t(hPipe), O_RDWR|O_BINARY), context});
#else
// This should have been detected at configure time.
# error ProgClient needs porting to this platform
#endif
}

ProgClient::~ProgClient()
{
    // Close the pipe.
    try {
	do_close();
    } catch (...) {
    }

    // Wait for the child process to exit.
#ifndef __WIN32__
    waitpid(child, 0, 0);
#else
    WaitForSingleObject(child, INFINITE);
#endif
}
