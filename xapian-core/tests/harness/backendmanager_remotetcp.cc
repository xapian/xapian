/** @file
 * @brief BackendManager subclass for remotetcp databases.
 */
/* Copyright (C) 2006,2007,2008,2009,2013,2015,2023 Olly Betts
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

#include <stdio.h> // For fdopen().
#include <cerrno>
#include <cstring>

#include "safesysexits.h"

#ifdef HAVE_FORK
# include <signal.h>
# include <sys/types.h>
# include "safesyssocket.h"
# include <sys/wait.h>
# include <unistd.h>
#endif

#ifdef __WIN32__
# include <io.h> // For _open_osfhandle().
# include "safefcntl.h"
# include "safewindows.h"
# include <cstdlib> // For free().
#endif

#include "errno_to_string.h"
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

// Start at port DEFAULT_PORT and increment until one isn't already in use.
#define DEFAULT_PORT 1239

class ServerData {
#ifdef HAVE_FORK
    /// Value of pid which indicates an unused entry.
    static constexpr pid_t UNUSED_PID = 0;

    typedef pid_t pid_type;
#elif defined __WIN32__
    /** Value of pid which indicates an unused entry.
     *
     *  This is a #define because it's a pointer type which doesn't work as a
     *  `static const` or `static constexpr` class member.
     */
#define UNUSED_PID INVALID_HANDLE_VALUE

    typedef HANDLE pid_type;
#else
# error Neither HAVE_FORK nor __WIN32__ is defined
#endif

    /** The remote server process ID.
     *
     *  Under Unix, this will actually be the /bin/sh process.
     */
    pid_type pid;

    /** The internal pointer of the Database object.
     *
     *  We use this to find the entry for a given Xapian::Database object (in
     *  kill_remote()).
     */
    const void* db_internal;

  public:
    void set_pid(pid_type pid_) { pid = pid_; }

    void set_db_internal(const void* dbi) { db_internal = dbi; }

    void clean_up() {
	if (pid == UNUSED_PID) return;
#ifdef HAVE_FORK
	int status;
	while (waitpid(pid, &status, 0) == -1 && errno == EINTR) { }
	// Other possible error from waitpid is ECHILD, which it seems can
	// only mean that the child has already exited and SIGCHLD was set
	// to SIG_IGN.  If we did somehow see that, it seems reasonable to
	// treat the child as successfully cleaned up.
#elif defined __WIN32__
	WaitForSingleObject(pid, INFINITE);
	CloseHandle(pid);
#endif
    }

    bool kill_remote(const void* dbi) {
	if (pid == UNUSED_PID || dbi != db_internal) return false;
#ifdef HAVE_FORK
	// Kill the process group that we put the server in so that we kill
	// the server itself and not just the /bin/sh that launched it.
	if (kill(-pid, SIGKILL) < 0) {
	    throw Xapian::DatabaseError("Couldn't kill remote server",
					errno);
	}
#elif defined __WIN32__
	if (!TerminateProcess(pid, 0)) {
	    throw Xapian::DatabaseError("Couldn't kill remote server",
					-int(GetLastError()));
	}
#endif
	clean_up();
	pid = UNUSED_PID;
	return true;
    }
};

// We can't dynamically resize this on demand (e.g. by using a std::vector)
// because it would confuse the leak detector.  We clean up after each testcase
// so this only needs to store the children launched by a single testcase,
// which is at most 6 currently, but the entries are tiny so allocate enough
// that future testcases shouldn't hit the limit either.
//
// We need to linear scan up to first_unused_server_data entries to implement
// BackendManagerRemoteTcp::kill_remote(), but with a small fixed bound on the
// number of entries that's effectively O(1); also very few testcases use this
// feature anyway.
static ServerData server_data[16];

static unsigned first_unused_server_data = 0;

#ifdef HAVE_FORK

static std::pair<int, ServerData&>
launch_xapian_tcpsrv(const string & args)
{
    int port = DEFAULT_PORT;

try_next_port:
    string cmd = XAPIAN_TCPSRV " --one-shot --interface " LOCALHOST " --port ";
    cmd += str(port);
    cmd += " ";
    cmd += args;
#ifdef HAVE_VALGRIND
    if (RUNNING_ON_VALGRIND) cmd = "./runsrv " + cmd;
#endif
    int fds[2];
    int type = SOCK_STREAM | SOCK_CLOEXEC;
#ifdef SOCK_NOSIGPIPE
    // Avoids remotefailure* testcases causing apitest to die on NetBSD. Not
    // entirely clear why this happens as we close both of the sockets created
    // by socketpair() on the parent side after we have confirmed that
    // xapian-tcpsrv has successfully started.
    type |= SOCK_NOSIGPIPE;
#endif
    if (socketpair(AF_UNIX, type, PF_UNSPEC, fds) < 0) {
	string msg("Couldn't create socketpair: ");
	errno_to_string(errno, msg);
	throw msg;
    }

    pid_t child = fork();
    if (child == 0) {
	// Put this process into its own process group so that we can kill the
	// server itself easily by killing the process group.  Just killing
	// `child` only kills the /bin/sh and leaves the server running.
	setpgid(0, 0);
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
	errno_to_string(fork_errno, msg);
	throw msg;
    }

    // Parent process.

    // Wrap the file descriptor in a FILE * so we can read lines using fgets().
    FILE * fh = fdopen(fds[0], "r");
    if (fh == NULL) {
	string msg("Failed to run command '");
	msg += cmd;
	msg += "': ";
	errno_to_string(errno, msg);
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
		errno_to_string(errno, msg);
		throw msg;
	    }
	    if (++port < 65536 && status != 0) {
		if (WIFEXITED(status) &&
		    WEXITSTATUS(status) == EX_UNAVAILABLE) {
		    // Exit code EX_UNAVAILABLE from xapian-tcpsrv means the
		    // specified port was in use.
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
    fclose(fh);

    if (first_unused_server_data >= std::size(server_data)) {
	// We used to quietly ignore not finding a slot, but it's helpful to
	// know if we haven't allocated enough.
	throw Xapian::DatabaseError("Not enough ServerData slots");
    }

    auto& data = server_data[first_unused_server_data++];
    data.set_pid(child);
    return {port, data};
}

#elif defined __WIN32__

[[noreturn]]
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
static std::pair<int, ServerData&>
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

    // For some reason Windows wants a modifiable command line string
    // so pass a pointer to the first character rather than using c_str().
    if (!CreateProcess(XAPIAN_TCPSRV, &cmd[0], 0, 0, TRUE, 0, 0, 0,
		       &startupinfo, &procinfo)) {
	win32_throw_error_string("Couldn't create child process");
    }

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
	    if (++port < 65536 && rc == EX_UNAVAILABLE) {
		// Exit code EX_UNAVAILABLE from xapian-tcpsrv means the
		// specified port was in use.
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

    if (first_unused_server_data >= std::size(server_data)) {
	// We used to quietly ignore not finding a slot, but it's helpful to
	// know if we haven't allocated enough.
	throw Xapian::DatabaseError("Not enough ServerData slots");
    }

    auto& data = server_data[first_unused_server_data++];
    data.set_pid(procinfo.hProcess);
    return {port, data};
}

#else
# error Neither HAVE_FORK nor __WIN32__ is defined
#endif

static Xapian::Database
get_remotetcp_db(const string& args, int* port_ptr = nullptr)
{
    auto [port, server] = launch_xapian_tcpsrv(args);
    if (port_ptr) *port_ptr = port;
    auto db = Xapian::Remote::open(LOCALHOST, port);
    server.set_db_internal(db.internal.get());
    return db;
}

static Xapian::WritableDatabase
get_remotetcp_writable_db(const string& args)
{
    auto [port, server] = launch_xapian_tcpsrv(args);
    auto db = Xapian::Remote::open_writable(LOCALHOST, port);
    server.set_db_internal(db.internal.get());
    return db;
}

BackendManagerRemoteTcp::~BackendManagerRemoteTcp() {
    BackendManagerRemoteTcp::clean_up();
}

Xapian::Database
BackendManagerRemoteTcp::do_get_database(const vector<string> & files)
{
    // Default to a long (5 minute) timeout so that tests won't fail just
    // because the host is slow or busy.
    return BackendManagerRemoteTcp::get_remote_database(files, 300000,
							nullptr);
}

Xapian::WritableDatabase
BackendManagerRemoteTcp::get_writable_database(const string & name,
					       const string & file)
{
    return get_remotetcp_writable_db(get_writable_database_args(name, file));
}

Xapian::Database
BackendManagerRemoteTcp::get_remote_database(const vector<string> & files,
					     unsigned int timeout,
					     int* port_ptr)
{
    return get_remotetcp_db(get_remote_database_args(files, timeout), port_ptr);
}

Xapian::Database
BackendManagerRemoteTcp::get_database_by_path(const string& path)
{
    return get_remotetcp_db(get_remote_database_args(path, 300000));
}

Xapian::Database
BackendManagerRemoteTcp::get_writable_database_as_database()
{
    return get_remotetcp_db(get_writable_database_as_database_args());
}

Xapian::WritableDatabase
BackendManagerRemoteTcp::get_writable_database_again()
{
    return get_remotetcp_writable_db(get_writable_database_again_args());
}

void
BackendManagerRemoteTcp::kill_remote(const Xapian::Database& db)
{
    const void* db_internal = db.internal.get();
    for (unsigned i = 0; i != first_unused_server_data; ++i) {
	if (server_data[i].kill_remote(db_internal))
	    return;
    }
    throw Xapian::DatabaseError("No known server for remote DB");
}

void
BackendManagerRemoteTcp::clean_up()
{
    while (first_unused_server_data) {
	server_data[--first_unused_server_data].clean_up();
    }
}
