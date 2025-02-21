/** @file
 * @brief Class representing worker process.
 */
/* Copyright (C) 2005-2023 Olly Betts
 * Copyright (C) 2019 Bruno Baruffaldi
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

#include "worker.h"
#include "worker_comms.h"

#include <cinttypes>
#include <csignal>
#include <cstring>
#include <cerrno>
#include "safefcntl.h"
#include "safeunistd.h"
#include "safesyssocket.h"
#include "safesyswait.h"
#include "safesysexits.h"
#include <utility>

#include "closefrom.h"
#include "freemem.h"
#include "handler.h"
#include "parseint.h"
#include "pkglibbindir.h"
#include "str.h"

#ifdef __WIN32__
# include "safewindows.h"
#endif

using namespace std;

bool Worker::ignoring_sigpipe = false;

int
Worker::start_worker_subprocess()
{
    static bool keep_stderr = (getenv("XAPIAN_OMEGA_DEBUG_WORKERS") != nullptr);

    if (error_prefix.empty()) {
	// The first time we're called, set error_prefix to the leafname of
	// filter_module followed by ": ", and qualify filter_module if it's
	// just a leafname.
	auto slash = filter_module.rfind('/');
	if (slash == string::npos) {
	    error_prefix = std::move(filter_module);
	    // Look for unqualified filters in pkglibbindir.
	    filter_module = get_pkglibbindir();
	    filter_module += '/';
	    filter_module += error_prefix;
	} else {
	    error_prefix.assign(filter_module, slash + 1);
	}
	error_prefix += ": ";
    }

#if defined HAVE_SOCKETPAIR && defined HAVE_FORK
    int fds[2];
    if (socketpair(AF_UNIX, SOCK_STREAM, PF_UNSPEC, fds) < 0) {
	error = string("socketpair failed: ") + strerror(errno);
	return 1;
    }

    child = fork();
    if (child == 0) {
	// Child process.
	close(fds[0]);

	// Connect pipe to fd 3.  Don't use stdin and stdout in case the
	// filter library tries to read from stdin or write debug or progress
	// info to stdout.
	dup2(fds[1], 3);

	// Make sure we don't hang on to open files which may get deleted but
	// not have their disk space released until we exit.
	closefrom(4);

	// Connect stdin, stdout and (conditionally) stderr to /dev/null.
	int devnull = open("/dev/null", O_RDWR);
	dup2(devnull, 0);
	dup2(devnull, 1);
	if (!keep_stderr)
	    dup2(devnull, 2);
	if (devnull > 3) close(devnull);

	// FIXME: For filters which support a file descriptor as input, we
	// could open the file here, and pass the file descriptor across the
	// socket to the worker process using sendmsg().  Then the worker
	// process could be chroot()-ed to a sandbox directory, which means
	// we're reasonably protected from security bugs in the filter.

#ifdef HAVE_SETRLIMIT
#if defined RLIMIT_AS || defined RLIMIT_VMEM || defined RLIMIT_DATA
	// Set a memory limit if it is possible
	long mem = get_free_physical_memory();
	if (mem > 0) {
	    struct rlimit ram_limit = {
		static_cast<rlim_t>(mem),
		RLIM_INFINITY
	    };
#ifdef RLIMIT_AS
	    // Limit size of the process's virtual memory
	    // (address space) in bytes.
	    setrlimit(RLIMIT_AS, &ram_limit);
#elif defined RLIMIT_VMEM
	    // Limit size of a process's mapped address space in bytes.
	    setrlimit(RLIMIT_VMEM, &ram_limit);
#else
	    // Limit size of the process's data segment.
	    setrlimit(RLIMIT_DATA, &ram_limit);
#endif
	}
#endif
#endif
	// Replacing the current process image with a new process image
	const char* mod = filter_module.c_str();
	execl(mod, mod, static_cast<void*>(NULL));
	_exit(EX_OSERR);
    }

    // Main process
    int fork_errno = errno;
    close(fds[1]);
    if (child == -1) {
	close(fds[0]);
	error = string("fork failed: ") + strerror(fork_errno);
	return 1;
    }

    sockt = fdopen(fds[0], "r+");

    if (!ignoring_sigpipe) {
	ignoring_sigpipe = true;
	signal(SIGPIPE, SIG_IGN);
    }

    return 0;
#elif defined __WIN32__
    LARGE_INTEGER counter;
    // QueryPerformanceCounter() will always succeed on XP and later
    // and gives us a counter which increments each CPU clock cycle
    // on modern hardware (Pentium or newer).
    QueryPerformanceCounter(&counter);
    char pipename[256];
    snprintf(pipename, sizeof(pipename),
	     "\\\\.\\pipe\\xapian-omega-worker-%lx-%lx_%" PRIx64,
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
	error = "CreateNamedPipe failed: " + str(GetLastError());
	return 1;
    }

    HANDLE hClient = CreateFile(pipename,
				GENERIC_READ|GENERIC_WRITE, 0, NULL,
				OPEN_EXISTING,
				FILE_FLAG_OVERLAPPED, NULL);

    if (hClient == INVALID_HANDLE_VALUE) {
	error = "CreateFile failed: " + str(GetLastError());
	return 1;
    }

    if (!ConnectNamedPipe(hPipe, NULL) &&
	GetLastError() != ERROR_PIPE_CONNECTED) {
	error = "ConnectNamedPipe failed: " + str(GetLastError());
	return 1;
    }

    // Set the appropriate handles to be inherited by the child process.
    SetHandleInformation(hClient, HANDLE_FLAG_INHERIT, 1);

    // Create the child process.
    PROCESS_INFORMATION procinfo;
    memset(&procinfo, 0, sizeof(PROCESS_INFORMATION));

    STARTUPINFO startupinfo;
    memset(&startupinfo, 0, sizeof(STARTUPINFO));
    startupinfo.cb = sizeof(STARTUPINFO);
    // FIXME: Is NULL the way to say "/dev/null"?
    // It's what GetStdHandle() is documented to return if "an application does
    // not have associated standard handles"...
    startupinfo.hStdError = keep_stderr ? GetStdHandle(STD_ERROR_HANDLE) : NULL;
    startupinfo.hStdOutput = hClient;
    startupinfo.hStdInput = hClient;
    startupinfo.dwFlags |= STARTF_USESTDHANDLES;

    string cmdline{filter_module};
    // For some reason Windows wants a modifiable command line so we
    // pass `&cmdline[0]` rather than `cmdline.c_str()`.
    BOOL ok = CreateProcess(filter_module.c_str(), &cmdline[0],
			    0, 0, TRUE, 0, 0, 0,
			    &startupinfo, &procinfo);
    if (!ok) {
	if (GetLastError() == ERROR_FILE_NOT_FOUND) {
	    error = error_prefix + "failed to run helper";
	    filter_module = string();
	    return -1;
	}
	error = "CreateProcess failed: " + str(GetLastError());
	return 1;
    }

    CloseHandle(hClient);
    CloseHandle(procinfo.hThread);
    child = procinfo.hProcess;

    sockt = _fdopen(_open_osfhandle(intptr_t(hPipe), O_RDWR|O_BINARY), "r+");

    return 0;
#else
# error Omega needs porting to this platform
#endif
}

int
Worker::extract(const std::string& filename,
		const std::string& mimetype,
		std::string& dump,
		std::string& title,
		std::string& keywords,
		std::string& author,
		std::string& to,
		std::string& cc,
		std::string& bcc,
		std::string& message_id,
		int& pages,
		time_t& created)
{
    if (filter_module.empty()) {
	error = error_prefix + "hard failed earlier in the current run";
	return -1;
    }

    if (sockt) {
#ifdef HAVE_WAITPID
	// Check if the worker process is still alive - if it is, waitpid()
	// with WNOHANG returns 0.
	int status;
	if (waitpid(child, &status, WNOHANG) != 0) {
	    fclose(sockt);
	    sockt = NULL;
	}
#elif defined __WIN32__
	// Check if the worker process is still alive by trying to wait for it
	// with a timeout of 0ms.
	if (WaitForSingleObject(child, 0) != WAIT_TIMEOUT) {
	    fclose(sockt);
	    sockt = NULL;
	}
#else
# error Omega needs porting to this platform
#endif
    }
    if (!sockt) {
	int r = start_worker_subprocess();
	if (r != 0) return r;
    }

    string attachment_filename;
    // Send a filename and wait for the reply.
    if (write_string(sockt, filename) && write_string(sockt, mimetype)) {
	while (true) {
	    int field_code = GETC(sockt);
	    string* value;
	    switch (field_code) {
	      case FIELD_PAGE_COUNT: {
		unsigned u_pages;
		if (!read_unsigned(sockt, u_pages)) {
		    goto comms_error;
		}
		pages = int(u_pages);
		continue;
	      }
	      case FIELD_CREATED_DATE: {
		unsigned u_created;
		if (!read_unsigned(sockt, u_created)) {
		    goto comms_error;
		}
		created = time_t(long(u_created));
		continue;
	      }
	      case FIELD_BODY:
		value = &dump;
		break;
	      case FIELD_TITLE:
		value = &title;
		break;
	      case FIELD_KEYWORDS:
		value = &keywords;
		break;
	      case FIELD_AUTHOR:
		value = &author;
		break;
	      case FIELD_TO:
		value = &to;
		break;
	      case FIELD_CC:
		value = &cc;
		break;
	      case FIELD_BCC:
		value = &bcc;
		break;
	      case FIELD_MESSAGE_ID:
		value = &message_id;
		break;
	      case FIELD_ERROR:
		if (!read_string(sockt, error)) goto comms_error;
		// Fields shouldn't be empty but the protocol allows them to be.
		if (error.empty())
		    error = error_prefix + "Couldn't extract text";
		else
		    error.insert(0, error_prefix);
		return 1;
	      case FIELD_END:
		return 0;
	      case EOF:
		goto comms_error;
	      case FIELD_ATTACHMENT:
		value = &attachment_filename;
		break;
	      default:
		error = error_prefix + "Unknown field code ";
		error += str(field_code);
		return 1;
	    }
	    if (value->empty()) {
		// First instance of this field for this document.
		if (!read_string(sockt, *value)) break;
	    } else {
		// Repeat instance of this field.
		string s;
		if (!read_string(sockt, s)) break;
		*value += ' ';
		*value += s;
	    }
	    if (field_code == FIELD_ATTACHMENT) {
		// FIXME: Do something more useful with attachment_filename!
		attachment_filename.clear();
	    }
	}
    }

comms_error:
    error = error_prefix;
    // Check if the worker process already died, so we can report if it
    // was killed by a segmentation fault, etc.
#ifdef HAVE_WAITPID
    int status;
    int result = waitpid(child, &status, WNOHANG);
    int waitpid_errno = errno;

    fclose(sockt);
    sockt = NULL;

    if (result == 0) {
	// The worker is still alive, so terminate it.
	kill(child, SIGTERM);
	waitpid(child, &status, 0);
	error += "failed";
    } else if (result < 0) {
	// waitpid() failed with an error.
	error += "failed: ";
	error += strerror(waitpid_errno);
    } else if (WIFEXITED(status)) {
	int rc = WEXITSTATUS(status);
	switch (rc) {
	  case OMEGA_EX_SOCKET_READ_ERROR:
	    error += "subprocess failed to read data from us";
	    break;
	  case OMEGA_EX_SOCKET_WRITE_ERROR:
	    error += "subprocess failed to write data to us";
	    break;
	  case EX_TEMPFAIL:
	    error += "timed out";
	    break;
	  case EX_UNAVAILABLE:
	    error += "failed to initialise";
	    filter_module = string();
	    return -1;
	  case EX_OSERR:
	    error += "failed to run helper";
	    filter_module = string();
	    return -1;
	  default:
	    error += "exited with status ";
	    error += str(rc);
	    break;
	}
    } else {
	error += "killed by signal ";
	error += str(WTERMSIG(status));
    }
#elif defined __WIN32__
    DWORD result = WaitForSingleObject(child, 0);

    fclose(sockt);
    sockt = NULL;

    if (result == WAIT_TIMEOUT) {
	// The worker is still alive, so terminate it.  We need to specify an
	// exit code here, 255 is an arbitrary choice.
	TerminateProcess(child, 255);
	WaitForSingleObject(child, INFINITE);
	error += "failed";
	CloseHandle(child);
    } else if (result == WAIT_FAILED) {
	// WaitForSingleObject() failed in an unexpected way.
	error += "failed: ";
	error += str(GetLastError());
	CloseHandle(child);
    } else {
	DWORD rc;
	while (GetExitCodeProcess(child, &rc) && rc == STILL_ACTIVE) {
	    Sleep(100);
	}
	switch (rc) {
	  case OMEGA_EX_SOCKET_READ_ERROR:
	    error += "subprocess failed to read data from us";
	    break;
	  case OMEGA_EX_SOCKET_WRITE_ERROR:
	    error += "subprocess failed to write data to us";
	    break;
	  case EX_TEMPFAIL:
	    error += "timed out";
	    break;
	  case EX_UNAVAILABLE:
	    error += "failed to initialise";
	    filter_module = string();
	    return -1;
	  case EX_OSERR:
	    error += "failed to run helper";
	    filter_module = string();
	    return -1;
	  default:
	    error += "exited with status ";
	    error += str(rc);
	    break;
	}
    }
#else
# error Omega needs porting to this platform
#endif
    return 1;
}
