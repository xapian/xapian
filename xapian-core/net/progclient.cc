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

ProgClient::ProgClient(string progname, string arg)
	: socketfd(get_spawned_socket(progname, arg)),
	  buf(socketfd),
	  conv_state(state_getquery),
	  remote_stats_valid(false),
	  global_stats_valid(false)
{
	do_write("HELLO!\n");

	string received = do_read();
	cout << "Read back " << received << endl;
}

int
ProgClient::get_spawned_socket(string progname, string arg)
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

	execlp(progname.c_str(), progname.c_str(), arg.c_str(), 0);

	// if we get here, then execlp failed.
	/* throwing an exception is a bad idea, since we're
	 * not the original process. */
	_exit(-1);
    } else {
	// parent
	// close the child's end of the socket
	close(sv[1]);

	return sv[0];
    }
}

string
ProgClient::do_read()
{
    string retval = buf.readline();

    cout << "do_read(): " << retval << endl;

    return retval;
}

void
ProgClient::do_write(string data)
{
    cout << "do_write(): " << data.substr(0, data.find_last_of('\n')) << endl;
    buf.writeline(data);
}

void
ProgClient::write_data(string msg)
{
    do_write(msg);
}

string
ProgClient::read_data()
{
    return do_read();
}

bool
ProgClient::data_is_available()
{
    return buf.data_waiting();
}

ProgClient::~ProgClient()
{
    buf.writeline("QUIT");
    close(socketfd);
}

void
ProgClient::set_weighting(IRWeight::weight_type wt_type)
{
    Assert(conv_state == state_getquery);
    wt_string = inttostring(wt_type);
}

void
ProgClient::set_query(const OmQueryInternal *query_)
{
    Assert(conv_state == state_getquery);
    query_string = query_->serialise();
} 

bool
ProgClient::finish_query()
{
    bool success = false;
    switch (conv_state) {
	case state_getquery:
	    // Message 3 (see README_progprotocol.txt)
	    {
		string message = "SETQUERY " +
		                 wt_string + " \"" +
		       	         query_string + "\"";
		do_write(message);
	    }
	    conv_state = state_sentquery;
	    // fall through...
	case state_sentquery:

	    // Message 4
	    if (!data_is_available()) {
		break;
	    }
	    
	    {
		string response = do_read();
		if (response.substr(0, 7) != "MYSTATS") {
		    throw OmNetworkError("Error getting statistics");
		}
		remote_stats = string_to_stats(response.substr(8, response.npos));
		remote_stats_valid = true;

		success = true;
	    }

	    conv_state = state_getmset;
	    // fall through...
	case state_getmset:
	    ;
    }
    return success;
}

void
ProgClient::wait_for_input()
{
    buf.wait_for_data();
}

bool
ProgClient::get_remote_stats(Stats &out)
{
    Assert(remote_stats_valid && conv_state >= state_sentquery);
    if (!remote_stats_valid && conv_state <= state_getmset) {
	bool finished = finish_query();

	if (!finished) return false;
    }

    out = remote_stats;
    return true;
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

OmMSetItem
string_to_msetitem(string s)
{
    istrstream is(s.c_str());
    om_weight wt;
    om_docid did;

    is >> wt >> did;

    return OmMSetItem(wt, did);
}


void
ProgClient::send_global_stats(const Stats &stats)
{
    Assert(conv_state == state_getmset);
    global_stats = stats;
    global_stats_valid = true;
}

void
ProgClient::get_mset(om_doccount first,
		     om_doccount maxitems,
		     vector<OmMSetItem> &mset,
		     om_doccount *mbound,
		     om_weight *greatest_wt)
{
    Assert(global_stats_valid);
    Assert(conv_state == state_getmset);

    // Message 5 (see README_progprotocol.txt)
    string message = "GLOBSTATS " + stats_to_string(global_stats) + '\n';
    message += "GETMSET " +
	       inttostring(first) + " " +
	       inttostring(maxitems);
    do_write(message);

    // Message 6
    string response = do_read();
    if (response.substr(0, 9) != "MSETITEMS") {
	throw OmNetworkError(string("Expected MSETITEMS, got ") + response);
    }
    response = response.substr(10);

    int numitems;
    {
	istrstream is(response.c_str());

	is >> numitems >> remote_maxweight;
    }

    for (int i=0; i<numitems; ++i) {
	mset.push_back(string_to_msetitem(do_read()));
    }
    response = do_read();
    if (response != "OK") {
	throw OmNetworkError("Error at end of mset");
    }
}

om_weight
ProgClient::get_max_weight()
{
    return remote_maxweight;
}
