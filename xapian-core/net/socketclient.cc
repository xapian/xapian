/* socketclient.cc: implementation of NetClient using a socket
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
#include "socketclient.h"
#include "om/omerror.h"
#include "omerr_string.h"
#include "utils.h"
#include "netutils.h"
#include "omdebug.h"

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <signal.h>
#include <cstdio>
#include <cerrno>
#include <strstream.h>

SocketClient::SocketClient(int socketfd_,
			   int msecs_timeout_,
			   std::string context_,
			   bool close_socket_)
	: socketfd(socketfd_),
	  close_socket(close_socket_),
	  buf(socketfd, context_),
	  conv_state(state_getquery),
	  remote_stats_valid(false),
	  global_stats_valid(false),
	  context(context_),
	  msecs_timeout(msecs_timeout_),
	  end_time_set(false)
{
    // ignore SIGPIPE - we check return values instead, and that
    // way we can easily throw an exception.
    if (signal(SIGPIPE, SIG_IGN) < 0) {
	throw OmNetworkError("Couldn't install SIGPIPE handler", context, errno);
    }

    std::string received = do_read();

    DEBUGLINE(UNKNOWN, "Read back " << received);
    if (received.substr(0, 3) != "OM ") {
	throw OmNetworkError("Unknown start of conversation", context);
    }

    istrstream is(received.c_str() + 3);

    int version;
    is >> version >> doccount >> avlength;

    if (version != OM_SOCKET_PROTOCOL_VERSION) {
	throw OmNetworkError(std::string("Invalid protocol version: found ") +
			     om_tostring(version) + " expected " +
			     om_tostring(OM_SOCKET_PROTOCOL_VERSION), context);
    }
}

void
SocketClient::keep_alive()
{
    get_requested_docs();  // avoid confusing the protocol
    do_write(std::string("K"));
    std::string message = do_read();
    // ignore message
}

void
SocketClient::init_end_time()
{
    end_time = OmTime::now() + OmTime(msecs_timeout);

    end_time_set = true;
    DEBUGLINE(UNKNOWN, "init_end_time() - set timer to " <<
	      end_time.sec << "." << end_time.usec <<
	      " (" << msecs_timeout << " msecs)");
}

void
SocketClient::close_end_time()
{
    end_time_set = false;
    DEBUGLINE(UNKNOWN, "close_end_time()");
}

NetClient::TermListItem
string_to_tlistitem(const std::string &s)
{
    istrstream is(s.c_str());
    NetClient::TermListItem item;
    std::string tencoded;

    is >> item.wdf >> item.termfreq >> tencoded;
    item.tname = decode_tname(tencoded);

    return item;
}

void
SocketClient::get_tlist(om_docid did,
			std::vector<NetClient::TermListItem> &items) {
    /* avoid confusing the protocol if there are requested documents
     * being returned.
     */
    get_requested_docs();
    do_write(std::string("T") + om_tostring(did));

    init_end_time();

    while (1) {
	std::string message = do_read();
	if (message == "Z") break;
	items.push_back(string_to_tlistitem(message));
    }

    close_end_time();
}

void
SocketClient::request_doc(om_docid did)
{
    std::map<om_docid, unsigned int>::iterator i = request_count.find(did);
    if (i != request_count.end()) {
	/* This document has been requested already - just count it again. */
	(i->second)++;
    } else {
	do_write(std::string("D") + om_tostring(did));
	requested_docs.push_back(did);
	request_count[did] = 1;
    }
}

void
SocketClient::get_requested_docs()
{
    while (!requested_docs.empty()) {
	om_docid cdid = requested_docs.front();

	/* Create new entry, and get a reference to it */
	cached_doc &cdoc = collected_docs[cdid];

	// FIXME check did is correct?
	std::string message = do_read();
	Assert(!message.empty() && message[0] == 'O');
	cdoc.data = decode_tname(message.substr(1));

	while (1) {
	    std::string message = do_read();
	    if (message == "Z") break;
	    istrstream is(message.c_str());
	    om_keyno keyno;
	    std::string omkey;
	    is >> keyno >> omkey;
	    cdoc.keys[keyno] = string_to_omkey(omkey);
	}
	cdoc.users = request_count[cdid];
	request_count.erase(cdid);

	/* Remove this did from the queue */
	requested_docs.pop_front();
    }
}

void
SocketClient::collect_doc(om_docid did, std::string &doc,
			  std::map<om_keyno, OmKey> &keys)
{
    /* First check that the data isn't in our temporary cache */
    std::map<om_docid, cached_doc>::iterator i;
    i = collected_docs.find(did);

    if (i != collected_docs.end()) {
	/* A hit! */
	doc = i->second.data;
	keys = i->second.keys;

	/* remove from cache if necessary */
	i->second.users--;
	if (i->second.users == 0) {
	    collected_docs.erase(i);
	}

	return;
    }
    
    /* FIXME: should the cache be cleared at this point? */

    /* Since we've missed our cache, the did being collected must be
     * in our queue of requested documents.  We now fetch all of them
     * into the cache.  Fetching only until the requested doc would
     * defeat the point of a seperate request_doc to some extent.
     */
    get_requested_docs();

    /* FIXME: it's a bit of a waste doing this outside of the collect
     * loop like this, but probably ok.
     */
    i = collected_docs.find(did);

    if (i != collected_docs.end()) {
	/* A hit! */
	doc = i->second.data;
	keys = i->second.keys;

	i->second.users--;

	if (i->second.users == 0) {
	    /* remove from cache */
	    collected_docs.erase(i);
	}
    } else {
	throw OmInternalError("Failed to collect document " +
			      om_tostring(did) + ", possibly not requested.",
			      context);
    }
}

void
SocketClient::get_doc(om_docid did,
		      std::string &doc,
		      std::map<om_keyno, OmKey> &keys)
{
    request_doc(did);
    collect_doc(did, doc, keys);
}

om_doccount
SocketClient::get_doccount()
{
    return doccount;
}

om_doclength
SocketClient::get_avlength()
{
    return avlength;
}

std::string
SocketClient::do_read()
{
    std::string retval;
    if (end_time_set) {
	retval = buf.readline(end_time);
    } else {
	init_end_time();
	retval = buf.readline(end_time);
	close_end_time();
    }

    DEBUGLINE(UNKNOWN, "do_read(): " << retval);

    if (retval.substr(0, 1) == "E") {
	string_to_omerror(retval.substr(1), "REMOTE:", context);
    }

    return retval;
}

void
SocketClient::do_write(std::string data)
{
    DEBUGLINE(UNKNOWN, "do_write(): " << data.substr(0, data.find_last_of('\n')));
    if (end_time_set) {
	buf.writeline(data, end_time);
    } else {
	init_end_time();
	buf.writeline(data, end_time);
	close_end_time();
    }
}

void
SocketClient::write_data(std::string msg)
{
    do_write(msg);
}

std::string
SocketClient::read_data()
{
    return do_read();
}

bool
SocketClient::data_is_available()
{
    return buf.data_waiting();
}

void
SocketClient::do_close()
{
    // mustn't let any exception escape or else we
    // abort immediately if called from a destructor.
    try {
	/* Don't wait for a timeout to expire while writing
	 * the close-down message.
	 * FIXME: come up with a better way of not waiting.
	 * FIXME: 1000 is an arbitrary parameter
	 */
	OmTime endtime = OmTime::now() + OmTime(1000);
	buf.writeline("X", endtime);
    } catch (...) {
    }
    close(socketfd);
}

SocketClient::~SocketClient()
{
    if (close_socket) {
	do_close();
    }
}

void
SocketClient::set_query(const OmQuery::Internal *query_,
			const OmSettings &moptions_, const OmRSet &omrset_)
{
    /* no actual communication performed in this method */

    init_end_time();
    Assert(conv_state == state_getquery);
    query_string = query_->serialise();
    moptions = moptions_;
    omrset = omrset_;
}

bool
SocketClient::finish_query()
{
    /* avoid confusing the protocol if there are requested documents
     * being returned.
     */
    get_requested_docs();

    bool success = false;
    switch (conv_state) {
	case state_getquery:
	    // Message 3 (see README_progprotocol.txt)
	    do_write("Q" + query_string + '\n'
		     + moptions_to_string(moptions) + '\n'
		     + omrset_to_string(omrset));		
	    conv_state = state_sentquery;
	    // fall through...
	case state_sentquery:

	    // Message 4
	    if (!buf.data_waiting()) {
		break;
	    }

	    {
		std::string response = do_read();
		if (response.substr(0, 1) != "L") {
		    throw OmNetworkError("Error getting statistics", context);
		}
		remote_stats = string_to_stats(response.substr(1));
		remote_stats_valid = true;

		success = true;
	    }

	    conv_state = state_sendglobal;
	    // fall through...
	case state_sendglobal:
	case state_getmset:
	case state_getresult:
	    ;
    }
    return success;
}

void
SocketClient::wait_for_input()
{
    buf.wait_for_data(msecs_timeout);
}

bool
SocketClient::get_remote_stats(Stats &out)
{
    Assert(remote_stats_valid && conv_state >= state_sentquery);
    if (!remote_stats_valid && conv_state <= state_sendglobal) {
	bool finished = finish_query();

	if (!finished) return false;
    }

    out = remote_stats;

    conv_state = state_sendglobal;
    return true;
}

void
SocketClient::send_global_stats(const Stats &stats)
{
    Assert(conv_state >= state_sendglobal);
    if (conv_state == state_sendglobal) {
	Assert(end_time_set);
	global_stats = stats;
	global_stats_valid = true;
	conv_state = state_getmset;
    }
}

bool
SocketClient::get_mset(om_doccount first,
		       om_doccount maxitems,
		       OmMSet &mset)
{
    /* avoid confusing the protocol if there are requested documents
     * being returned.
     */
    get_requested_docs();

    Assert(global_stats_valid);
    Assert(conv_state >= state_getmset);
    switch (conv_state) {
	case state_getquery:
	case state_sentquery:
	case state_sendglobal:
	    throw OmInvalidArgumentError("get_mset called before global stats given", context);
	    break;
	case state_getmset:

	    // Message 5 (see README_progprotocol.txt)
	    do_write("G" + stats_to_string(global_stats) + '\n' +
		     "M" + om_tostring(first) + " " + om_tostring(maxitems));
	    conv_state = state_getresult;
	    return false; // FIXME icky

	    // fall through...
	case state_getresult:

	    if (!buf.data_waiting()) {
		return false;
	    }

	    // Message 6
	    {
		std::string response = do_read();
		Assert(!response.empty() && response[0] == 'O');
		mset = string_to_ommset(response.substr(1));
	    }
    } // switch (conv_state)

    // reset the state
    conv_state = state_getquery;
    query_string = "";
    remote_stats = Stats();
    remote_stats_valid = false;
    global_stats = Stats();
    global_stats_valid = false;
    moptions = OmSettings();
    omrset = OmRSet();

    // disable the timeout, now that the mset has been retrieved.
    close_end_time();

    return true;
}

bool
SocketClient::open_postlist(om_doccount first, om_doccount maxitems,
			    om_doccount &termfreq_max,
			    om_doccount &termfreq_min,
			    om_doccount &termfreq_est,
			    om_weight &maxw,
			    std::map<om_termname, OmMSet::Internal::Data::TermFreqAndWeight> &term_info)
{
    /* avoid confusing the protocol if there are requested documents
     * being returned.
     */
    get_requested_docs();

    DEBUGCALL(MATCH, bool, "SocketClient::open_postlist", first << ", " << maxitems);
    Assert(global_stats_valid);
    Assert(conv_state >= state_getmset);
    switch (conv_state) {
	case state_getquery:
	case state_sentquery:
	case state_sendglobal:
	    throw OmInvalidArgumentError("open_postlist called before global stats given", context);
	    break;
	case state_getmset: {

	    // Message 5 (see README_progprotocol.txt)
	    do_write("G" + stats_to_string(global_stats) + '\n' +
		     "P" + om_tostring(first) + " " + om_tostring(maxitems));

	    // FIXME: new state here...

	    while (!buf.data_waiting()) {
		wait_for_input();
	    }
	
	    std::string message = do_read();
	    {
		// extract term frequency and max weight
		istrstream is(message.c_str());
		is >> termfreq_max >> termfreq_min >> termfreq_est >> maxw;
		minw = maxw;
	    }
	    message = do_read();
	    DEBUGLINE(UNKNOWN, "term_info = `" << message << "'");
	    Assert(!message.empty() && message[0] == 'O');
	    term_info = string_to_ommset_termfreqwts(message.substr(1));
	    conv_state = state_getresult;
	}
	case state_getresult: // avoid compiler warning
	    break;
    } // switch (conv_state)
    RETURN(false);
}

bool
SocketClient::get_posting(om_docid &did, om_weight &w, OmKey &key)
{
    DEBUGCALL(MATCH, bool, "SocketClient::get_posting", "");
    Assert(global_stats_valid);
    Assert(conv_state >= state_getresult);
    switch (conv_state) {
	case state_getquery:
	case state_sentquery:
	case state_sendglobal:
	case state_getmset:
	    throw OmInvalidArgumentError("get_posting called too soon", context);
	    break;
	case state_getresult: {

	    DEBUGLINE(MATCH, "about to see if data is waiting");
	    if (!buf.data_waiting()) {
		RETURN(false);
	    }
	
	    DEBUGLINE(MATCH, "data is waiting");
	    std::string message = do_read();
	    DEBUGLINE(MATCH, "read `" << message << "'");
	    if (message == "Z") {
		did = 0;

		// reset the state
		conv_state = state_getquery;
		query_string = "";
		remote_stats = Stats();
		remote_stats_valid = false;
		global_stats = Stats();
		global_stats_valid = false;
		moptions = OmSettings();
		omrset = OmRSet();
	    } else {
		did = atoi(message.c_str());
		std::string::size_type i = message.find(';');
		std::string::size_type j = message.find(' ');
		if (j != message.npos && (i == message.npos || j < i)) {
		    istrstream is(message.substr(j + 1, i - j - 1).c_str());
		    is >> w;
		} else {
		    w = 0;
		}
		if (i != message.npos) {
		    key = string_to_omkey(message.substr(i + 1));
		}
	    }
	}
    } // switch (conv_state)

    RETURN(true);
}

void
SocketClient::next(om_weight w_min, om_docid &did, om_weight &w, OmKey &key)
{
    if (w_min > minw) {
	minw = w_min;
	do_write("m" + om_tostring(w_min));
    }

    while (!get_posting(did, w, key)) wait_for_input();
}

void
SocketClient::skip_to(om_docid new_did, om_weight w_min, om_docid &did, om_weight &w, OmKey &key)
{
    do_write("S" + om_tostring(new_did));
    if (w_min > minw) {
	minw = w_min;
	do_write("m" + om_tostring(w_min));
    }

    while (!get_posting(did, w, key) || (did && (did < new_did || w < w_min)))
	wait_for_input();
}
