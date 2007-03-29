/* tcpserver.cc: class for TCP/IP-based server.
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2002,2003,2004,2005,2006 Olly Betts
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

#include "tcpserver.h"
#include "stats.h"

#include <netinet/in_systm.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#ifdef __WIN32__
# include <winsock2.h>
#else
# include <sys/socket.h>
#endif
#include <sys/types.h>
#include <netdb.h>
#include <signal.h>
#include <sys/wait.h>

#include <iostream>

using namespace std;

// Handle older systems.
#if !defined SIGCHLD && defined SIGCLD
# define SIGCHLD SIGCLD
#endif

/// The TcpServer constructor, taking a database and a listening port.
TcpServer::TcpServer(Xapian::Database db_, int port_,
		     int msecs_active_timeout_,
		     int msecs_idle_timeout_,
		     bool verbose_)
	: port(port_), writable(false), db(db_), listen_socket(get_listening_socket(port_)),
	  msecs_active_timeout(msecs_active_timeout_),
	  msecs_idle_timeout(msecs_idle_timeout_),
	  verbose(verbose_)
{

}

TcpServer::TcpServer(Xapian::WritableDatabase wdb_, int port_,
		     int msecs_active_timeout_,
		     int msecs_idle_timeout_,
		     bool verbose_)
	: port(port_), writable(true), wdb(wdb_), listen_socket(get_listening_socket(port_)),
	  msecs_active_timeout(msecs_active_timeout_),
	  msecs_idle_timeout(msecs_idle_timeout_),
	  verbose(verbose_)
{

}

int
TcpServer::get_listening_socket(int port)
{
    int socketfd = socket(PF_INET, SOCK_STREAM, 0);

    if (socketfd < 0) {
	throw Xapian::NetworkError("socket", socket_errno());
    }

    int retval;

    {
	int optval = 1;
	retval = setsockopt(socketfd,
			    SOL_SOCKET,
			    SO_REUSEADDR,
			    reinterpret_cast<void *>(&optval),
			    sizeof(optval));

	if (retval >= 0) {
	    retval = setsockopt(socketfd, IPPROTO_TCP, TCP_NODELAY,
				reinterpret_cast<void *>(&optval),
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
    addr.sin_addr.s_addr = INADDR_ANY;

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
TcpServer::get_connected_socket()
{
    struct sockaddr_in remote_address;
    SOCKLEN_T remote_address_size = sizeof(remote_address);
    // accept connections
    int con_socket = accept(listen_socket,
			    reinterpret_cast<sockaddr *>(&remote_address),
			    &remote_address_size);

    if (con_socket < 0) {
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

void
TcpServer::run_once()
{
    int connected_socket = get_connected_socket();
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
