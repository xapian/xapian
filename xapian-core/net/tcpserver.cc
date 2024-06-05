/** @file
 *  @brief Generic TCP/IP socket based server base class.
 */
/* Copyright 2006-2023 Olly Betts
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <config.h>

#include "tcpserver.h"

#include <xapian/error.h>

#include "resolver.h"
#include "socket_utils.h"

#include "safefcntl.h"
#include "safenetdb.h"
#include "safesysexits.h"
#include "safesysselect.h"
#include "safesyssocket.h"
#include "safeunistd.h"

#ifdef __WIN32__
# include <process.h>    /* _beginthreadex, _endthreadex */
#else
# include <netinet/in.h>
# include <netinet/tcp.h>
# include <signal.h>
# include <sys/wait.h>
#endif

#include <iostream>
#include <limits>

#include <cerrno>
#include <cstring>
#include <cstdlib>
#include <sys/types.h>

using namespace std;

// The parent process/main thread sits in a loop which calls accept() and
// then passes the connection off to a new process/threadm so we should accept
// connections promptly and shouldn't need to handle a large backlog.
//
// We've been using 5 for this since 2006 without anyone reporting a problem.
#define LISTEN_BACKLOG 5

/** Create a listening socket ready to accept connections.
 *
 *  @param host		hostname or address to listen on or an empty string to
 *			accept connections on any interface.
 *  @param port		TCP port to listen on.
 *  @param tcp_nodelay	If true, enable TCP_NODELAY option.
 */
static int
create_listener(const std::string& host,
		int port,
		bool tcp_nodelay)
{
    int socketfd = -1;
    int bind_errno = 0;
    for (auto&& r : Resolver(host, port, AI_PASSIVE)) {
	int socktype = r.ai_socktype | SOCK_CLOEXEC;
	int fd = socket(r.ai_family, socktype, r.ai_protocol);
	if (fd == -1)
	    continue;

#if !defined __WIN32__ && defined F_SETFD && defined FD_CLOEXEC
	// We can't use a preprocessor check on the *value* of SOCK_CLOEXEC as
	// on Linux SOCK_CLOEXEC is an enum, with '#define SOCK_CLOEXEC
	// SOCK_CLOEXEC' to allow '#ifdef SOCK_CLOEXEC' to work.
	if (SOCK_CLOEXEC == 0)
	    (void)fcntl(fd, F_SETFD, FD_CLOEXEC);
#endif

	int on = 1;
	if (tcp_nodelay) {
	    // 4th argument might need to be void* or char* - cast it to char*
	    // since C++ allows implicit conversion to void* but not from
	    // void*.
	    if (setsockopt(fd, IPPROTO_TCP, TCP_NODELAY,
			   reinterpret_cast<char*>(&on),
			   sizeof(on)) < 0) {
		int setsockopt_errno = socket_errno();
		CLOSESOCKET(fd);
		throw Xapian::NetworkError("setsockopt TCP_NODELAY failed",
					   setsockopt_errno);
	    }
	}

#ifndef __WIN32__
	// On Unix, we use SO_REUSEADDR so that we can bind to a port which was
	// previously in use and still has old connections in the TIME_WAIT
	// state.  Without this it may not be possible to restart a server
	// without downtime.
	//
	// It seems that the default Microsoft behaviour is to allow bindings
	// to a port with old connections in the TIME_WAIT state, so we don't
	// need to do anything special here.
	//
	// We definitely want to avoid SO_REUSEADDR on Microsoft Windows as
	// it was implemented with stupid and dangerous semantics which allowed
	// binding to a port which was already bound and listening, and even
	// worse is that this affected *any* listening process, even if didn't
	// use SO_REUSEADDR itself.  This serious security problem existed for
	// many years but Microsoft finally addressed it and the situation
	// with Microsoft Windows versions that are still relevant is that we
	// don't need to worry about another process hijacking our listening
	// socket, but we still don't want to use SO_REUSEADDR.
	//
	// There's also the Microsoft-specific SO_EXCLUSIVEADDRUSE, but this
	// doesn't seem to affect the TIME_WAIT situation.
	//
	// We used to try to set it in combination with SO_REUSEADDR thinking
	// this tempered the problems, but you can't set both on the same
	// socket and the attempt to set SO_EXCLUSIVEADDRUSE was failing with
	// WSAEINVAL which we ignored (to support OS versions from before it
	// was added), so we've never actual set SO_EXCLUSIVEADDRUSE in any
	// released version of Xapian.
	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR,
		       reinterpret_cast<char*>(&on),
		       sizeof(on)) < 0) {
	    int setsockopt_errno = socket_errno();
	    CLOSESOCKET(fd);
	    throw Xapian::NetworkError("setsockopt SO_REUSEADDR failed",
				       setsockopt_errno);
	}
#endif

	if (::bind(fd, r.ai_addr, r.ai_addrlen) == 0) {
	    socketfd = fd;
	    break;
	}

	// Note down the error code for the first address we try, which seems
	// likely to be more helpful than the last in the case where they
	// differ.
	if (bind_errno == 0)
	    bind_errno = socket_errno();

	CLOSESOCKET(fd);
    }

    if (socketfd < 0) {
#ifdef __WIN32__
	// On __WIN32__ we can get WSAEACCESS instead of EADDRINUSE in
	// some cases, but there are no privileged ports so we can just
	// treat it the same as EADDRINUSE.
	if (bind_errno == WSAEACCES) bind_errno = EADDRINUSE;
#endif
	if (bind_errno == EADDRINUSE) {
address_in_use:
	    cerr << host << ':' << port << " already in use\n";
	    // EX_UNAVAILABLE is 69.  Scripts can use this to detect that the
	    // requested port was already in use.
	    exit(EX_UNAVAILABLE);
	}
#ifndef __WIN32__
	// No privileged ports on __WIN32__.
	if (bind_errno == EACCES) {
	    cerr << "Can't bind to privileged port " << port << '\n';
	    // EX_NOPERM is 77.  Scripts can use this to detect if
	    // xapian-tcpsrv failed to bind to the requested port.
	    exit(EX_NOPERM);
	}
#endif
	throw Xapian::NetworkError("bind failed", bind_errno);
    }

    if (listen(socketfd, LISTEN_BACKLOG) < 0) {
	int listen_errno = socket_errno();
	CLOSESOCKET(socketfd);
	if (listen_errno == EADDRINUSE) {
	    // The Linux listen(2) man page documents:
	    //
	    // EADDRINUSE  Another socket is already listening on the same port
	    //
	    // I'm not sure this is actually possible, but it's not hard to
	    // handle it suitably in case it is.
	    goto address_in_use;
	}
	throw Xapian::NetworkError("listen failed", listen_errno);
    }

    return socketfd;
}

TcpServer::TcpServer(const std::string& host,
		     int port,
		     bool tcp_nodelay,
		     bool verbose_)
    : listener(create_listener(host, port, tcp_nodelay)),
      verbose(verbose_)
{ }

int
TcpServer::accept_connection()
{
#ifndef __WIN32__
    // We want to be able to report when a client disconnects, but it's better
    // to do so in the main process to avoid problems with output getting
    // interleaved.  That means we need to be able to take action if either
    // a new connection is made or an existing connection goes away.
    //
    // To achieve this we use select() to wait until there's a connection to
    // accept(), or we're interrupted by a signal (we'll get SIGCHLD when a
    // child process servicing a client exits).
    //
    // We can rely on socketfd being small and so <= FD_SETSIZE so select()
    // should work fine here.
    fd_set fds;
    FD_ZERO(&fds);
    while (true) {
	FD_SET(listener, &fds);
	if (select(listener + 1, &fds, nullptr, nullptr, nullptr) > 0) {
	    // There's a connection waiting to be accepted.
	    break;
	}

	// Reap and report any zombie children.
	int status;
	while (waitpid(-1, &status, WNOHANG) > 0) {
	    if (verbose) cout << "Connection closed.\n";
	}
    }
#endif

    struct sockaddr_storage client_address;
    SOCKLEN_T client_address_size = sizeof(client_address);
    int connection = accept(listener,
			    reinterpret_cast<sockaddr*>(&client_address),
			    &client_address_size);

    if (connection < 0) {
	int accept_errno = socket_errno();
#ifdef __WIN32__
	if (accept_errno == WSAEINTR) {
	    // Our CtrlHandler function closed the socket.
	    return -1;
	}
#endif
	throw Xapian::NetworkError("accept failed", accept_errno);
    }

    if (verbose) {
	char host[PRETTY_IP6_LEN];
	int port = pretty_ip6(&client_address, host);
	if (port >= 0) {
	    cout << host << ':' << port << " connected\n";
	} else {
	    cout << "Unknown host connected\n";
	}
    }

    return connection;
}

TcpServer::~TcpServer()
{
    CLOSESOCKET(listener);
}

#ifdef HAVE_FORK
// A fork() based implementation.

extern "C" {

static void
sigterm_handler(int)
{
    signal(SIGTERM, SIG_DFL);
    // Terminate all processes in the same process group.
    kill(0, SIGTERM);
}

}

void
TcpServer::run()
{
    // Handle connections until shutdown.

    // Put this process into its own process group so that we can easily kill
    // any child processes on SIGTERM.
    setpgid(0, 0);

    // Set up signal handler.
    signal(SIGTERM, sigterm_handler);

    while (true) {
	try {
	    int connected_socket = accept_connection();
	    pid_t pid = fork();
	    if (pid == 0) {
		// Child process.
		close(listener);

		handle_one_connection(connected_socket);
		close(connected_socket);
		_exit(0);
	    }

	    // Parent process.

	    if (pid < 0) {
		// fork() failed.
		int fork_errno = errno;
		close(connected_socket);
		throw Xapian::NetworkError("fork failed", fork_errno);
	    }

	    close(connected_socket);
	} catch (const Xapian::Error& e) {
	    cerr << "Caught " << e.get_description() << '\n';
	} catch (...) {
	    cerr << "Caught unknown exception\n";
	}
    }
}

#elif defined __WIN32__

// A threaded, Windows specific, implementation.

/** The socket which will be closed by CtrlHandler.
 *
 *  FIXME - is there any way to avoid using a global variable here?
 */
static const int* pShutdownSocket = NULL;

/// Console interrupt handler.
static BOOL
CtrlHandler(DWORD fdwCtrlType)
{
    switch (fdwCtrlType) {
	case CTRL_C_EVENT:
	case CTRL_CLOSE_EVENT:
	    //  Console is about to die.
	    // CTRL_CLOSE_EVENT gives us 5 seconds before displaying a
	    // confirmation dialog asking if we really are sure.
	case CTRL_LOGOFF_EVENT:
	case CTRL_SHUTDOWN_EVENT:
	    // These 2 will probably need to change when we get service
	    // support - the service will prevent these being seen, so only
	    // apply interactively.
	    cout << "Shutting down...\n";
	    break; // default behaviour
	case CTRL_BREAK_EVENT:
	    // This (probably) means the developer is struggling to get
	    // things to behave, and really wants to shutdown so let the OS
	    // handle Ctrl+Break in the default way.
	    cout << "Ctrl+Break: aborting process\n";
	    return FALSE;
	default:
	    cerr << "unexpected CtrlHandler: " << fdwCtrlType << '\n';
	    return FALSE;
    }

    // Note: close() does not cause a blocking accept() call to terminate.
    // However, it appears closesocket() does.  This is much easier than trying
    // to setup a non-blocking accept().
    if (!pShutdownSocket || closesocket(*pShutdownSocket) == SOCKET_ERROR) {
	// We failed to close the socket, so just let the OS handle the
	// event in the default way.
	return FALSE;
    }

    pShutdownSocket = NULL;
    return TRUE; // Tell the OS that we've handled the event.
}

/// Structure which is used to pass parameters to the new threads.
struct thread_param
{
    thread_param(TcpServer* s, int c) : server(s), connected_socket(c) {}
    TcpServer* server;
    int connected_socket;
};

/// The thread entry-point.
static unsigned __stdcall
run_thread(void* param_)
{
    thread_param* param(reinterpret_cast<thread_param*>(param_));
    int socket = param->connected_socket;

    TcpServer* tcp_server = param->server;
    tcp_server->handle_one_connection(socket);
    closesocket(socket);

    if (tcp_server->get_verbose()) cout << "Connection closed.\n";

    delete param;

    _endthreadex(0);
    return 0;
}

void
TcpServer::run()
{
    // Handle connections until shutdown.

    // Set up the shutdown handler - this is a bit hacky, and sadly involves
    // a global variable.
    pShutdownSocket = &listener;
    if (!::SetConsoleCtrlHandler((PHANDLER_ROUTINE)CtrlHandler, TRUE))
	throw Xapian::NetworkError("Failed to install shutdown handler");

    while (true) {
	try {
	    int connected_socket = accept_connection();
	    if (connected_socket == -1)
	       return; // Shutdown has happened

	    // Spawn a new thread to handle the connection.
	    // (This seems like lots of hoops just to end up calling
	    // this->handle_one_connection() on a new thread. There might be a
	    // better way...)
	    thread_param* param = new thread_param(this, connected_socket);
	    HANDLE hthread = (HANDLE)_beginthreadex(NULL, 0, ::run_thread,
						    param, 0, NULL);
	    if (hthread == 0) {
		// errno holds the error code from _beginthreadex, and
		// closesocket() doesn't set errno.
		closesocket(connected_socket);
		throw Xapian::NetworkError("_beginthreadex failed", errno);
	    }

	    // FIXME: keep track of open thread handles so we can gracefully
	    // close each thread down.  OTOH, when we want to kill them all its
	    // likely to mean the process is on its way down, so it doesn't
	    // really matter...
	    CloseHandle(hthread);
	} catch (const Xapian::Error& e) {
	    cerr << "Caught " << e.get_description() << '\n';
	} catch (...) {
	    cerr << "Caught unknown exception\n";
	}
    }
}

#else
# error Neither HAVE_FORK nor __WIN32__ are defined.
#endif

void
TcpServer::run_once()
{
    // Run a single request in the current process/thread.
    int fd = accept_connection();
    handle_one_connection(fd);
    CLOSESOCKET(fd);
}
