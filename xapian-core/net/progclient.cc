/* progclient.cc: implementation of NetClient which spawns a program.
 *
 * ----START-LICENCE----
 * Copyright 1999,2000 Dialog Corporation
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
#include "progclient.h"
#include "om/omerror.h"

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <cstdio>
#include <cerrno>

ProgClient::ProgClient(string progname)
{
    /* socketpair() returns two sockets.  We keep sv[0] and give
     * sv[1] to the child process.
     */
    int sv[2];

    if (socketpair(PF_UNIX, SOCK_STREAM, 0, sv) < 0) {
	throw OmNetworkError(string("socketpair:") + strerror(errno));
    }
    
    pid_t pid = fork();

    if (pid < 0) {
	throw OmNetworkError(string("fork:") + strerror(errno));
    }

    if (pid == 0) {
	/* child process:
	 *   set up file descriptors and exec program
	 */

	// replace stdin and stdout with the socket
	// FIXME: check return values.
	close(0);
	close(1);
	dup2(sv[1], 0);
	dup2(sv[1], 1);

	// close unnecessary file descriptors
	// FIXME: Probably a bit excessive...
	for (int i=3; i<256; ++i) {
	    close(i);
	}

	execlp(progname.c_str(), progname.c_str(), 0);

	// if we get here, then execlp failed.
	/* throwing an exception is a bad idea, since we're
	 * not the original process. */
	_exit(-1);
    } else {
	// parent
	socketfd = sv[0];

	do_write("HELLO!\n");

	string received = do_read();
	cout << "Read back " << received << endl;
    }
}

string
ProgClient::do_read()
{
    string::size_type pos;
    while ((pos = buffer.find_first_of('\n')) == buffer.npos) {
	char buf[256];
	ssize_t received = read(socketfd, buf, sizeof(buf) - 1);

	buffer += string(buf, buf + received);
    }
    string retval(buffer.begin(), buffer.begin() + pos);

    buffer.erase(0, pos);

    return retval;
}

void
ProgClient::do_write(string data)
{
    while (data.length() > 0) {
	ssize_t written = write(socketfd, data.data(), data.length());

	if (written < 0) {
	    throw OmNetworkError(std::string("write:") + strerror(errno));
	}

	data.erase(0, written);
    }
}

void
ProgClient::write_data(string msg)
{
    do_write(msg + '\n');
}

string
ProgClient::read_data()
{
    return do_read();
}

bool
ProgClient::data_is_available()
{
    return true;
}

ProgClient::~ProgClient()
{
    close(socketfd);
}
