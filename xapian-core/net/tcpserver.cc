/* tcpserver.cc: class for TCP/IP-based server.
 *
 * ----START-LICENCE----
 * Copyright 1999,2000 BrightStation PLC
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

#include "config.h"
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

/// The TcpServer constructor, taking a database and a listening port.
TcpServer::TcpServer(OmDatabase db_, int port_, int msecs_timeout_,
#ifndef TIMING_PATCH
		     bool verbose_)
#else /* TIMING_PATCH */
		     bool verbose_, bool timing_)
#endif /* TIMING_PATCH */
	: port(port_), db(db_), listen_socket(get_listening_socket(port_)),
	  msecs_timeout(msecs_timeout_), verbose(verbose_)
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
	throw OmNetworkError("socket", errno);
    }

    int retval;

    {
	int optval = 1;
	retval = setsockopt(socketfd,
			    SOL_SOCKET,
			    SO_REUSEADDR,
			    (void *)&optval,
			    sizeof(optval));
    }

    if (retval < 0) {
	int saved_errno = errno; // note down in case close hits an error
	close(socketfd);
	throw OmNetworkError("setsockopt failed", saved_errno);
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
	throw OmNetworkError("bind failed", saved_errno);
    }

    // FIXME: backlog argument should perhaps be larger.
    retval = listen(socketfd, 1);

    if (retval < 0) {
	int saved_errno = errno; // note down in case close hits an error
	close(socketfd);
	throw OmNetworkError("listen failed", saved_errno);
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
	throw OmNetworkError("accept failed", errno);
    }

    if (remote_address_size != sizeof(remote_address)) {
	throw OmNetworkError("accept: unexpected remote address size");
    }

    struct in_addr address = remote_address.sin_addr;
    struct hostent *hent = gethostbyaddr(reinterpret_cast<char *>(&address),
					 sizeof(address),
					 AF_INET);

    if (hent == 0) {
	close(con_socket);
	std::string errmsg = "gethostbyaddr: ";
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
	throw OmNetworkError(errmsg);
    }

    if (verbose) {
	std::cout << "Connection from " << hent->h_name << ", port " <<
#ifndef TIMING_PATCH
	    remote_address.sin_port << std::endl;
#else /* TIMING_PATCH */
	    remote_address.sin_port << ". (tcpserver.cc)" << std::endl;
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
	std::cerr << "Could not get time of day...\n";
    }
#endif /* TIMING_PATCH */
    int pid = fork();
    if (pid == 0) {
	// child code
	close(listen_socket);
	try {
#ifndef TIMING_PATCH
	    SocketServer sserv(db, connected_socket, -1, msecs_timeout);
#else /* TIMING_PATCH */
	    SocketServer sserv(db, connected_socket, -1, msecs_timeout, timing);
#endif /* TIMING_PATCH */
	    sserv.run();
	} catch (const OmError &err) {
	    std::cerr << "Got exception " << err.get_type()
		 << ": " << err.get_msg() << std::endl;
	} catch (...) {
	    // ignore other exceptions
	}
	close(connected_socket);

#ifndef TIMING_PATCH
	if (verbose) std::cout << "Closing connection.\n";
#else /* TIMING_PATCH */
	// record end time
	returnval = gettimeofday(&etp, NULL);
	if (returnval != 0) {
	    std::cerr << "Could not get time of day...\n";
	}
	uint64_t total = ((1000000 * etp.tv_sec) + etp.tv_usec) - ((1000000 * stp.tv_sec) + stp.tv_usec);
	if (verbose) std::cout << "Connection held open for " <<  total << " usecs. (tcpserver.cc)\n\n";
#endif /* TIMING_PATCH */
	exit(0);
    } else if (pid > 0) {
	// parent code
	close(connected_socket);
    } else {
	// fork() failed
	int saved_errno = errno; // note down in case close hits an error
	close(connected_socket);
	throw OmNetworkError("fork failed", saved_errno);
    }
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
	} catch (const OmError &err) {
	    // FIXME: better error handling.
	    std::cerr << "Caught " << err.get_type()
		      << ": " << err.get_msg() << std::endl;
	} catch (...) {
	    // FIXME: better error handling.
	    std::cerr << "Caught exception." << std::endl;
	}
    }
}

//////////////////////////////////////////////////////////////
void
TcpServer::on_SIGTERM (int sig)
{
    signal (SIGTERM, SIG_DFL);
    /* terminate all processes in my process group */
#ifdef HAVE_KILLPG
    killpg(0, SIGTERM);
#else
    kill(0, SIGTERM);
#endif
    exit (0);
}

//////////////////////////////////////////////////////////////
void 
TcpServer::on_SIGCHLD (int sig)
{    
    int status;
    while (waitpid(-1, &status, WNOHANG) > 0);
}
