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
#include "utils.h"
#include "netutils.h"

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <cstdio>
#include <cerrno>
#include <strstream.h>

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

    //cout << "PreBuffer: [" << buffer << "]" << endl;
    buffer.erase(0, pos+1);
    //cout << "PostBuffer: [" << buffer << "]" << endl;

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

void
ProgClient::set_weighting(IRWeight::weight_type wt_type)
{
    do_simple_transaction(string("SETWEIGHT ") + inttostring(wt_type));
}

void
ProgClient::set_query(const OmQueryInternal *query_)
{
    do_simple_transaction(string("SETQUERY ") + query_->serialise());
} 

Stats
ProgClient::string_to_stats(const string &s)
{
    Stats stat;

    istrstream is(s.c_str(), s.length());

    is >> stat.collection_size;
    is >> stat.average_length;

    string word;
    while (is >> word) {
	if (word.length() == 0) continue;

	if (word[0] == 'T') {
            vector<string> parts;
	    split_words(word.substr(1), parts, '=');

	    if (parts.size() != 2) {
		throw OmNetworkError(string("Invalid stats string word part: ")
				     + word);
	    }

	    stat.termfreq[parts[0]] = atoi(parts[1].c_str());
	} else if (word[0] == 'R') {
            vector<string> parts;
	    split_words(word.substr(1), parts, '=');

	    if (parts.size() != 2) {
		throw OmNetworkError(string("Invalid stats string word part: ")
				     + word);
	    }
	    
	    stat.reltermfreq[parts[0]] = atoi(parts[1].c_str());
	} else {
	    throw OmNetworkError(string("Invalid stats string word: ") + word);
	}
    }

    return stat;
}

Stats
ProgClient::get_remote_stats()
{
    string result = do_transaction_with_result(string("GETSTATS"));

    return string_to_stats(result);
}

void
ProgClient::do_simple_transaction(string msg)
{
    do_write(msg + '\n');
    string response = do_read();

    if (response != "OK") {
	throw OmNetworkError(string("Invalid response: (") +
			     msg + ") -> (" + response + ")");
    }
}

string
ProgClient::do_transaction_with_result(string msg)
{
    do_write(msg + '\n');
    string response = do_read();

    if (response == "ERROR") {
	throw OmNetworkError(string("Error response: (") +
			     msg + ") -> (" + response + ")");
    }
    return response;
}

void
ProgClient::get_mset(om_doccount first,
		     om_doccount maxitems,
		     vector<OmMSetItem> &mset,
		     om_doccount *mbound,
		     om_weight *greatest_wt)
{
    Assert(false);
}
