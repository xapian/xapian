/* tcpserver.cc: class for TCP/IP-based server.
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2002,2003 Olly Betts
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 * -----END-LICENCE-----
 */

#include <config.h>
#include "tcpserver.h"
#include "stats.h"
#include "netutils.h"
#include "socketcommon.h"
#include "utils.h"

#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in_systm.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <signal.h>
#include <sys/wait.h>
#ifdef TIMING_PATCH
#include <sys/time.h>

#define uint64_t unsigned long long
#endif /* TIMING_PATCH */

#include <iostream>
using namespace std;

/// The TcpServer constructor, taking a database and a listening port.
TcpServer::TcpServer(Xapian::Database db_, int port_,
		     int msecs_active_timeout_,
		     int msecs_idle_timeout_,
#ifndef TIMING_PATCH
		     bool verbose_)
#else /* TIMING_PATCH */
		     bool verbose_, bool timing_)
#endif /* TIMING_PATCH */
	: port(port_), db(db_), listen_socket(get_listening_socket(port_)),
	  msecs_active_timeout(msecs_active_timeout_),
	  msecs_idle_timeout(msecs_idle_timeout_),
	  verbose(verbose_)
#ifdef TIMING_PATCH
          , timing(timing_)
#endif /* TIMING_PATCH */
{

}

int
TcpServer::get_listening_socket(int port)
{
    int socketfd = socket(PF_INET, SOCK_STREAM, 0);

    if (socketfd < 0) {
	throw Xapian::NetworkError("socket", errno);
    }

    int retval;

    {
	int optval = 1;
	retval = setsockopt(socketfd,
			    SOL_SOCKET,
			    SO_REUSEADDR,
			    reinterpret_cast<void *>(&optval),
			    sizeof(optval));
    }

    if (retval < 0) {
	int saved_errno = errno; // note down in case close hits an error
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
	int saved_errno = errno; // note down in case close hits an error
	close(socketfd);
	throw Xapian::NetworkError("bind failed", saved_errno);
    }

    // FIXME: backlog argument should perhaps be larger.
    retval = listen(socketfd, 1);

    if (retval < 0) {
	int saved_errno = errno; // note down in case close hits an error
	close(socketfd);
	throw Xapian::NetworkError("listen failed", saved_errno);
    }
    return socketfd;
}

int
TcpServer::get_connected_socket()
{
    struct sockaddr_in remote_address;
    socklen_t remote_address_size = sizeof(remote_address);
    // accept connections
    int con_socket = accept(listen_socket,
			    reinterpret_cast<sockaddr *>(&remote_address),
			    &remote_address_size);

    if (con_socket < 0) {
	throw Xapian::NetworkError("accept failed", errno);
    }

    if (remote_address_size != sizeof(remote_address)) {
	throw Xapian::NetworkError("accept: unexpected remote address size");
    }

    struct in_addr address = remote_address.sin_addr;
    struct hostent *hent = gethostbyaddr(reinterpret_cast<char *>(&address),
					 sizeof(address),
					 AF_INET);

    if (hent == 0) {
	close(con_socket);
	string errmsg = "gethostbyaddr: ";
	switch(h_errno) {
	    case HOST_NOT_FOUND:
		errmsg += "Unknown host";
		break;
	    case NO_DATA:
#if NO_DATA != NO_ADDRESS
		/* Is this ever different? */
	    case NO_ADDRESS:
#endif
		errmsg += "No address for hostname";
		break;
	    case NO_RECOVERY:
		errmsg += "Unrecoverable name server error";
		break;
	    case TRY_AGAIN:
		errmsg += "Temporary nameserver error";
		break;
	    default:
		errmsg += "Unknown error.";
	}
	throw Xapian::NetworkError(errmsg);
    }

    if (verbose) {
	cout << "Connection from " << hent->h_name << ", port " <<
#ifndef TIMING_PATCH
	    remote_address.sin_port << endl;
#else /* TIMING_PATCH */
	    remote_address.sin_port << ". (tcpserver.cc)" << endl;
#endif /* TIMING_PATCH */
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
#ifdef TIMING_PATCH
    struct timeval stp, etp;
    // record start time
    int returnval = gettimeofday(&stp,NULL);
    if (returnval != 0) {
	cerr << "Could not get time of day...\n";
    }
#endif /* TIMING_PATCH */
    int pid = fork();
    if (pid == 0) {
	// child code
	close(listen_socket);
	try {
#ifndef TIMING_PATCH
	    SocketServer sserv(db, connected_socket, -1,
			       msecs_active_timeout,
			       msecs_idle_timeout);
#else /* TIMING_PATCH */
	    SocketServer sserv(db, connected_socket, -1,
			       msecs_active_timeout,
			       msecs_idle_timeout, timing);
#endif /* TIMING_PATCH */
	    sserv.run();
	} catch (const Xapian::Error &err) {
	    cerr << "Got exception " << err.get_type()
		 << ": " << err.get_msg() << endl;
	} catch (...) {
	    // ignore other exceptions
	}
	close(connected_socket);

#ifndef TIMING_PATCH
	if (verbose) cout << "Closing connection.\n";
#else /* TIMING_PATCH */
	// record end time
	returnval = gettimeofday(&etp, NULL);
	if (returnval != 0) {
	    cerr << "Could not get time of day...\n";
	}
	uint64_t total = ((1000000 * etp.tv_sec) + etp.tv_usec) - ((1000000 * stp.tv_sec) + stp.tv_usec);
	if (verbose) cout << "Connection held open for " <<  total << " usecs. (tcpserver.cc)\n\n";
#endif /* TIMING_PATCH */
	exit(0);
    } else if (pid > 0) {
	// parent code
	close(connected_socket);
    } else {
	// fork() failed
	int saved_errno = errno; // note down in case close hits an error
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

extern "C" void 
on_SIGCHLD(int /*sig*/)
{    
    int status;
    while (waitpid(-1, &status, WNOHANG) > 0);
}

void
TcpServer::run()
{
    // set up signal handlers
    /* NOTE: Changed from SIGCLD to SIGCHLD, as I believe it to be
     * more portable.  If any systems only understand SIGCLD, then
     * we'll have to add a define, but it may not be necessary.
     */
#ifndef HAVE_WAITPID
    signal(SIGCHLD, SIG_IGN);
#else
    signal(SIGCHLD, on_SIGCHLD);
#endif
    signal(SIGTERM, on_SIGTERM);
    while (1) {
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
