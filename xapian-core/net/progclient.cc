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
	  buf(socketfd)
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
    cout << "do_write(): " << data.substr(0, data.length() - 1) << endl;
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

	    stat.termfreq[decode_tname(parts[0])] = atoi(parts[1].c_str());
	} else if (word[0] == 'R') {
            vector<string> parts;
	    split_words(word.substr(1), parts, '=');

	    if (parts.size() != 2) {
		throw OmNetworkError(string("Invalid stats string word part: ")
				     + word);
	    }
	    
	    stat.reltermfreq[decode_tname(parts[0])] = atoi(parts[1].c_str());
	} else {
	    throw OmNetworkError(string("Invalid stats string word: ") + word);
	}
    }

    return stat;
}

void
ProgClient::finish_query()
{
    do_write(string("ENDQUERY"));
}

Stats
ProgClient::get_remote_stats()
{
    string result = do_read();

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
ProgClient::set_global_stats(const Stats &stats)
{
    do_simple_transaction("SETSTATS " + stats_to_string(stats));
}

void
ProgClient::get_mset(om_doccount first,
		     om_doccount maxitems,
		     vector<OmMSetItem> &mset,
		     om_doccount *mbound,
		     om_weight *greatest_wt)
{
    do_write(string("GET_MSET ") +
	     inttostring(first) + " " +
	     inttostring(maxitems) + "\n");
    
    string response = do_read();
    if (response == "ERROR") {
	throw OmNetworkError("Error getting mset");
    }
    int numitems = atoi(response.c_str());

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
    string response = do_transaction_with_result("GETMAXWEIGHT");

    return atof(response.c_str());
}
