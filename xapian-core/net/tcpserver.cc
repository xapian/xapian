/* tcpserver.cc: class for TCP/IP-based server.
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2002,2003,2004,2005,2006,2007 Olly Betts
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

#include "safeerrno.h"
#include "safefcntl.h"

#include "remoteserver.h"
#include "tcpserver.h"
#include "stats.h"

#ifdef __WIN32__
# define SOCKOPT_OPTIONS_TYPE char *
# include <process.h>    /* _beginthread, _endthread */
#else
# define SOCKOPT_OPTIONS_TYPE void *
# include <sys/socket.h>
# include <netinet/in_systm.h>
# include <netinet/in.h>
# include <netinet/ip.h>
# include <netinet/tcp.h>
# include <arpa/inet.h>
# include <netdb.h>
# include <signal.h>
# include <sys/wait.h>
#endif

#include <sys/types.h>
#include <iostream>

using namespace std;

// Handle older systems.
#if !defined SIGCHLD && defined SIGCLD
# define SIGCHLD SIGCLD
#endif

/// The TcpServer constructor, taking a database and a listening port.
TcpServer::TcpServer(Xapian::Database db_, const std::string & host, int port,
		     int msecs_active_timeout_,
		     int msecs_idle_timeout_,
		     bool verbose_)
	: writable(false), db(db_), listen_socket(get_listening_socket(host, port)),
	  msecs_active_timeout(msecs_active_timeout_),
	  msecs_idle_timeout(msecs_idle_timeout_),
	  verbose(verbose_)
{

}

TcpServer::TcpServer(Xapian::WritableDatabase wdb_, const std::string & host, int port,
		     int msecs_active_timeout_,
		     int msecs_idle_timeout_,
		     bool verbose_)
	: writable(true), wdb(wdb_), listen_socket(get_listening_socket(host, port)),
	  msecs_active_timeout(msecs_active_timeout_),
	  msecs_idle_timeout(msecs_idle_timeout_),
	  verbose(verbose_)
{

}

int
TcpServer::get_listening_socket(const std::string & host, int port)
{
    int socketfd = socket(PF_INET, SOCK_STREAM, 0);

    if (socketfd < 0) {
	throw Xapian::NetworkError("socket", socket_errno());
    }

    int retval;

    {
	int optval = 1;
	// 4th argument might need to be void* or char* - cast it to char*
	// since C++ allows implicit conversion to void* but not from void*.
	retval = setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR,
			    reinterpret_cast<char *>(&optval),
			    sizeof(optval));

	if (retval >= 0) {
	    retval = setsockopt(socketfd, IPPROTO_TCP, TCP_NODELAY,
				reinterpret_cast<char *>(&optval),
				sizeof(optval));
	}
    }

    if (retval < 0) {
	int saved_errno = socket_errno(); // note down in case close hits an error
	close(socketfd);
	throw Xapian::NetworkError("setsockopt failed", saved_errno);
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    if (host.empty()) {
	addr.sin_addr.s_addr = INADDR_ANY;
    } else {
	// FIXME: timeout on gethostbyname() ?
	struct hostent *hostent = gethostbyname(host.c_str());

	if (hostent == 0) {
	    throw Xapian::NetworkError(string("Couldn't resolve host ") + host,
		"",
#ifdef __WIN32__
		// "socket_errno()" is just errno on UNIX which is
		// inappropriate here - if gethostbyname() returns NULL an
		// error code is available in h_errno (with values
		// incompatible with errno).  FIXME: it would be good to see
		// the h_errno error code though...
		, socket_errno()
#endif
		);
	}

	memcpy(&addr.sin_addr, hostent->h_addr, sizeof(addr.sin_addr));
    }

    retval = bind(socketfd,
		  reinterpret_cast<sockaddr *>(&addr),
		  sizeof(addr));

    if (retval < 0) {
	int saved_errno = socket_errno(); // note down in case close hits an error
	close(socketfd);
	throw Xapian::NetworkError("bind failed", saved_errno);
    }

    retval = listen(socketfd, 5);

    if (retval < 0) {
	int saved_errno = socket_errno(); // note down in case close hits an error
	close(socketfd);
	throw Xapian::NetworkError("listen failed", saved_errno);
    }
    return socketfd;
}

int
TcpServer::accept_connection()
{
    struct sockaddr_in remote_address;
    SOCKLEN_T remote_address_size = sizeof(remote_address);
    // accept connections
    int con_socket = accept(listen_socket,
			    reinterpret_cast<sockaddr *>(&remote_address),
			    &remote_address_size);

    if (con_socket < 0) {
#ifdef __WIN32__
	if (WSAGetLastError() == WSAEINTR)
	    // Our CtrlHandler function closed the socket.
	    return -1;
#endif
	throw Xapian::NetworkError("accept failed", socket_errno());
    }

    if (remote_address_size != sizeof(remote_address)) {
	throw Xapian::NetworkError("accept: unexpected remote address size");
    }

    if (verbose) {
	cout << "Connection from " << inet_ntoa(remote_address.sin_addr)
	     << ", port " << remote_address.sin_port << endl;
    }

    return con_socket;
}

TcpServer::~TcpServer()
{
    close(listen_socket);
}

#ifdef HAVE_FORK
// A fork() based implementation
void
TcpServer::run_once()
{
    int connected_socket = accept_connection();
    int pid = fork();
    if (pid == 0) {
	// child code
	close(listen_socket);
	try {
	    if (writable) {
		RemoteServer sserv(&wdb, connected_socket, connected_socket,
				   msecs_active_timeout,
				   msecs_idle_timeout);
		sserv.run();
	    } else {
		RemoteServer sserv(&db, connected_socket, connected_socket,
				   msecs_active_timeout,
				   msecs_idle_timeout);
		sserv.run();
	    }
	} catch (const Xapian::Error &err) {
	    cerr << "Got exception " << err.get_type()
		 << ": " << err.get_msg() << endl;
	} catch (...) {
	    // ignore other exceptions
	}
	close(connected_socket);

	if (verbose) cout << "Closing connection.\n";
	exit(0);
    } else if (pid > 0) {
	// parent code
	close(connected_socket);
    } else {
	// fork() failed
	int saved_errno = socket_errno(); // note down in case close hits an error
	close(connected_socket);
	throw Xapian::NetworkError("fork failed", saved_errno);
    }
}

extern "C" void
on_SIGTERM(int /*sig*/)
{
    signal(SIGTERM, SIG_DFL);
    /* terminate all processes in my process group */
#ifdef HAVE_KILLPG
    killpg(0, SIGTERM);
#else
    kill(0, SIGTERM);
#endif
    exit (0);
}

#ifdef HAVE_WAITPID
extern "C" void
on_SIGCHLD(int /*sig*/)
{
    int status;
    while (waitpid(-1, &status, WNOHANG) > 0);
}
#endif

void
TcpServer::run()
{
    // set up signal handlers
#ifdef HAVE_WAITPID
    signal(SIGCHLD, on_SIGCHLD);
#else
    signal(SIGCHLD, SIG_IGN);
#endif
    signal(SIGTERM, on_SIGTERM);
    while (true) {
	try {
	    run_once();
	} catch (const Xapian::DatabaseModifiedError &) {
	    cerr << "Database modified - calling db.reopen()" << endl;
	    db.reopen();
	} catch (const Xapian::Error &err) {
	    // FIXME: better error handling.
	    cerr << "Caught " << err.get_type()
		 << ": " << err.get_msg() << endl;
	} catch (...) {
	    // FIXME: better error handling.
	    cerr << "Caught exception." << endl;
	}
    }
}

#endif
#ifdef __WIN32__

// A threaded, Windows specific, implementation

/** The socket which will be closed by CtrlHandler.
 *
 *  FIXME - is there any way to avoid using a global variable here?
 */
static const int *pShutdownSocket = NULL;

/// Console interrupt handler.
BOOL
CtrlHandler(DWORD fdwCtrlType)
{
    bool shutdown = true;
    BOOL rc = TRUE;
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
	    cout << "Shutting down..." << endl;
	    break; // default behaviour
	case CTRL_BREAK_EVENT:
	    // This (probably) means the developer is struggling to get
	    // things to behave, and really wants to shutdown.
	    shutdown = false;
	    rc = FALSE;
	    cout << "Ctrl+Break: aborting process" << endl;
	    break;
	default:
	    shutdown = false;
	    rc = FALSE;
	    cerr << "unexpected CtrlHandler: " << fdwCtrlType << endl;
	    break;
    }
    if (shutdown) {
	// We must have a valid pointer!
	Assert(pShutdownSocket);
	// Note: close() does not cause a blocking accept() call to terminate.
	// However, it appears closesocket() does.  This is much easier
	// than trying to setup a non-blocking accept().
	closesocket(*pShutdownSocket);
    }
    return rc;
}

/** A Win32 thread based implementation of run_once.
 * 
 *  This method contains the actual implementation, and is called by the "C"
 *  thread entry-point function "run_thread".
 */
void
TcpServer::handle_one_request(int connected_socket)
{
    try {
	if (writable) {
	    RemoteServer sserv(&wdb, connected_socket, connected_socket,
			       msecs_active_timeout,
			       msecs_idle_timeout);
	    sserv.run();
	} else {
	    RemoteServer sserv(&db, connected_socket, connected_socket,
			       msecs_active_timeout,
			       msecs_idle_timeout);
	    sserv.run();
	}
    } catch (const Xapian::Error &err) {
	cerr << "Got exception " << err.get_type()
	     << ": " << err.get_msg() << endl;
    } catch (...) {
	// ignore other exceptions
    }
    close(connected_socket);
}

/// Structure which is used to pass parameters to the new threads.
struct thread_param
{
    thread_param(TcpServer *s, int c) : server(s), connected_socket(c) {}
    TcpServer *server;
    int connected_socket;
};

/// The thread entry-point.
static unsigned __stdcall
run_thread(void * param_)
{
    thread_param * param(reinterpret_cast<thread_param *>(param_));
    param->server->handle_one_request(param->connected_socket);
    delete param;
    _endthreadex(0);
    return 0;
}

void
TcpServer::run()
{
    // Handle requests until shutdown.

    // Set up the shutdown handler - this is a bit hacky, and sadly involves
    // a global variable.
    pShutdownSocket = &listen_socket;
    if (!::SetConsoleCtrlHandler((PHANDLER_ROUTINE) CtrlHandler, TRUE))
	throw Xapian::NetworkError("Failed to install shutdown handler");

    while (true) {
	try {
	    int connected_socket = accept_connection();
	    if (connected_socket == -1) // Shutdown has happened
		break;
	    // Spawn a new thread to handle the connection.
	    // (This seems like lots of hoops just to end up calling
	    // this->handle_one_request() on a new thread. There might be a
	    // better way...)
	    thread_param *param = new thread_param(this, connected_socket);
	    HANDLE hthread = (HANDLE)_beginthreadex(NULL, 0, ::run_thread, param, 0, NULL);
	    if (hthread == 0) {
		// errno holds the error here (it's not a socket error!)
		int saved_errno = errno; // note down in case close hits an error
		close(connected_socket);
		throw Xapian::NetworkError("_beginthreadex failed", saved_errno);
	    }

	    // FIXME: keep track of open thread handles so we can gracefully
	    // close each thread down.  OTOH, when we want to kill them all its
	    // likely to mean the process is on its way down, so it doesn't
	    // really matter...
	    CloseHandle(hthread);
	
	} catch (const Xapian::DatabaseModifiedError &) {
	    cerr << "Database modified - calling db.reopen()" << endl;
	    db.reopen();
	} catch (const Xapian::Error &err) {
	    // FIXME: better error handling.
	    cerr << "Caught " << err.get_type()
		 << ": " << err.get_msg() << endl;
	} catch (...) {
	    // FIXME: better error handling.
	    cerr << "Caught exception." << endl;
	}
    }
}

void
TcpServer::run_once()
{
    // Runs a single request on the current thread.
    int connected_socket = accept_connection();
    handle_one_request(connected_socket);
}

#endif // __WIN32__
