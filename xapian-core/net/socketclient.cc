/* socketclient.cc: implementation of NetClient using a socket
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2002,2003,2004,2005 Olly Betts
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
#include "socketclient.h"
#include <xapian/error.h>
#include "omerr_string.h"
#include "utils.h"
#include "netutils.h"
#include "omdebug.h"

#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <cerrno>
#ifdef HAVE_SSTREAM
#include <sstream>
#else
#include <strstream.h>
#endif
#include <string>
#include <vector>

using namespace std;

SocketClient::SocketClient(int socketfd_,
			   int msecs_timeout_,
			   string context_,
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
	throw Xapian::NetworkError("Couldn't install SIGPIPE handler", context, errno);
    }

    string received = do_read();

    DEBUGLINE(UNKNOWN, "Read back " << received);
    if (received.substr(0, 3) != "OM ") {
	throw Xapian::NetworkError("Unknown start of conversation", context);
    }

#ifdef HAVE_SSTREAM
    istringstream is(received.c_str() + 3);
#else
    istrstream is(received.c_str() + 3);
#endif

    int version;
    is >> version >> doccount >> avlength;

    if (version != XAPIAN_SOCKET_PROTOCOL_VERSION) {
	throw Xapian::NetworkError(string("Mismatched protocol version: found ") +
			     om_tostring(version) + " expected " +
			     om_tostring(XAPIAN_SOCKET_PROTOCOL_VERSION),
			     context);
    }
}

void
SocketClient::keep_alive()
{
    get_requested_docs();  // avoid confusing the protocol
    do_write(string("K"));
    string message = do_read();
    // ignore message
}

// Timer sentry, used to ensure that the timer is set for a period and
// gets unset even if an exception occurs.
class TimerSentry {
    private:
        // Prevent copying
        TimerSentry(const TimerSentry &);
        TimerSentry & operator=(const TimerSentry &);

	SocketClient *client;
    public:
	TimerSentry(SocketClient *client_) : client(client_) {
	    client->init_end_time();
	}
	~TimerSentry() {
	    client->close_end_time();
	}
};

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
string_to_tlistitem(const string &s)
{
#ifdef HAVE_SSTREAM
    istringstream is(s);
#else
    istrstream is(s.data(), s.length());
#endif
    NetClient::TermListItem item;
    string tencoded;

    is >> item.wdf >> item.termfreq >> tencoded;
    item.tname = decode_tname(tencoded);

    return item;
}

void
SocketClient::get_tlist(Xapian::docid did,
			vector<NetClient::TermListItem> &items) {
    /* avoid confusing the protocol if there are requested documents
     * being returned.
     */
    get_requested_docs();
    do_write(string("T") + om_tostring(did));

    TimerSentry timersentry(this);
    while (1) {
	string message = do_read();
	if (message == "Z") break;
	items.push_back(string_to_tlistitem(message));
    }
}

void
SocketClient::request_doc(Xapian::docid did)
{
    map<Xapian::docid, unsigned int>::iterator i = request_count.find(did);
    if (i != request_count.end()) {
	/* This document has been requested already - just count it again. */
	(i->second)++;
    } else {
	do_write(string("D") + om_tostring(did));
	requested_docs.push_back(did);
	request_count[did] = 1;
    }
}

void
SocketClient::get_requested_docs()
{
    while (!requested_docs.empty()) {
	TimerSentry timersentry(this);

	Xapian::docid cdid = requested_docs.front();

	/* Create new entry, and get a reference to it */
	cached_doc &cdoc = collected_docs[cdid];

	// FIXME check did is correct?
	string message = do_read();
	Assert(!message.empty() && message[0] == 'O');
	cdoc.data = decode_tname(message.substr(1));

	while (true) {
	    message = do_read();
	    if (message == "Z") break;
#ifdef HAVE_SSTREAM
	    istringstream is(message);
#else
	    istrstream is(message.data(), message.length());
#endif
	    Xapian::valueno valueno;
	    string omvalue;
	    is >> valueno >> omvalue;
	    cdoc.values[valueno] = decode_tname(omvalue);
	}
	cdoc.users = request_count[cdid];
	request_count.erase(cdid);

	/* Remove this did from the queue */
	requested_docs.pop_front();
    }
}

void
SocketClient::collect_doc(Xapian::docid did, string &doc,
			  map<Xapian::valueno, string> &values)
{
    /* First check that the data isn't in our temporary cache */
    map<Xapian::docid, cached_doc>::iterator i;
    i = collected_docs.find(did);

    if (i != collected_docs.end()) {
	/* A hit! */
	doc = i->second.data;
	values = i->second.values;

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
	values = i->second.values;

	i->second.users--;

	if (i->second.users == 0) {
	    /* remove from cache */
	    collected_docs.erase(i);
	}
    } else {
	throw Xapian::InternalError("Failed to collect document " +
			      om_tostring(did) + ", possibly not requested.",
			      context);
    }
}

void
SocketClient::get_doc(Xapian::docid did,
		      string &doc,
		      map<Xapian::valueno, string> &values)
{
    request_doc(did);
    collect_doc(did, doc, values);
}

Xapian::doccount
SocketClient::get_doccount() const
{
    return doccount;
}

Xapian::doclength
SocketClient::get_avlength() const
{
    return avlength;
}

bool
SocketClient::term_exists(const string & tname)
{
    do_write(string("t") + encode_tname(tname));
    string message = do_read();
    Assert(!message.empty() && message[0] == 't');
    return message[1] == '1';
}

Xapian::doccount
SocketClient::get_termfreq(const string & tname)
{
    do_write(string("F") + encode_tname(tname));
    string message = do_read();
    Assert(!message.empty() && message[0] == 'F');
#ifdef HAVE_SSTREAM
    istringstream is(message.substr(1));
#else
    istrstream is(message.data() + 1, message.length() - 1);
#endif
    Xapian::doccount freq;
    is >> freq;
    return freq;
}

string
SocketClient::do_read()
{
    string retval;
    if (end_time_set) {
	retval = buf.readline(end_time);
    } else {
	TimerSentry timersentry(this);
	retval = buf.readline(end_time);
    }

    DEBUGLINE(UNKNOWN, "do_read(): " << retval);

    if (retval[0] == 'E') {
	string_to_omerror(retval.substr(1), "REMOTE:", context);
    }

    return retval;
}

void
SocketClient::do_write(string data)
{
    DEBUGLINE(UNKNOWN, "do_write(): " << data.substr(0, data.find_last_of('\n')));
    if (end_time_set) {
	buf.writeline(data, end_time);
    } else {
	TimerSentry timersentry(this);
	buf.writeline(data, end_time);
    }
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
SocketClient::set_query(const Xapian::Query::Internal *query_,
			Xapian::termcount qlen,
			Xapian::valueno collapse_key,
			Xapian::Enquire::docid_order order,
			int percent_cutoff, Xapian::weight weight_cutoff,
			const Xapian::Weight *wtscheme,
			const Xapian::RSet &omrset_)
{
    /* no actual communication performed in this method */

    // This timer will be sorted out by RemoteSubMatch's destructor, if
    // neccessary, otherwise it will stop at the end of get_mset()
    init_end_time();
    Assert(conv_state == state_getquery);
    // FIXME: no point carefully serialising these all separately...
    query_string = query_->serialise();
    optstring = om_tostring(qlen) + ' ' + om_tostring(collapse_key) +
	' ' + om_tostring(int(order)) + ' ' +
	om_tostring(percent_cutoff) + ' ' + om_tostring(weight_cutoff);
    wtstring = wtscheme->name() + '\n' + wtscheme->serialise();
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
	    // Message 2 (see remote_protocol.html)
	    do_write("Q" + query_string + '\n'
		     + optstring + '\n'
		     + wtstring + '\n'
		     + omrset_to_string(omrset));		
	    conv_state = state_sentquery;
	    // fall through...
	case state_sentquery:

	    // Message 3
	    if (!buf.data_waiting()) {
		break;
	    }

	    {
		string response = do_read();
		if (response[0] != 'L') {
		    throw Xapian::NetworkError("Error getting statistics", context);
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
SocketClient::get_mset(Xapian::doccount first,
		       Xapian::doccount maxitems,
		       Xapian::MSet &mset)
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
	    throw Xapian::InvalidArgumentError("get_mset called before global stats given", context);
	    break;
	case state_getmset:

	    // Message 4 (see remote_protocol.html)
	    do_write("G" + stats_to_string(global_stats) + '\n' +
		     "M" + om_tostring(first) + " " + om_tostring(maxitems));
	    conv_state = state_getresult;
	    return false; // FIXME icky

	    // fall through...
	case state_getresult:

	    if (!buf.data_waiting()) {
		return false;
	    }

	    // Message 5
	    {
		string response = do_read();
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
    omrset = Xapian::RSet();

    // disable the timeout, now that the mset has been retrieved.
    close_end_time();

    return true;
}

bool
SocketClient::get_posting(Xapian::docid &did, Xapian::weight &w, string &value)
{
    DEBUGCALL(MATCH, bool, "SocketClient::get_posting", "");
    Assert(global_stats_valid);
    Assert(conv_state >= state_getresult);
    switch (conv_state) {
	case state_getquery:
	case state_sentquery:
	case state_sendglobal:
	case state_getmset:
	    throw Xapian::InvalidArgumentError("get_posting called too soon", context);
	    break;
	case state_getresult: {

	    DEBUGLINE(MATCH, "about to see if data is waiting");
	    if (!buf.data_waiting()) {
		RETURN(false);
	    }
	
	    DEBUGLINE(MATCH, "data is waiting");
	    string message = do_read();
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
		optstring = wtstring = "";
		omrset = Xapian::RSet();
	    } else {
		did = atoi(message);
		string::size_type i = message.find(';');
		string::size_type j = message.find(' ');
		if (j != message.npos && (i == message.npos || j < i)) {
#ifdef HAVE_SSTREAM
		    istringstream is(message.substr(j + 1, i - j - 1));
#else
		    istrstream is(message.substr(j + 1, i - j - 1).c_str());
#endif
		    is >> w;
		} else {
		    w = 0;
		}
		if (i != message.npos) {
		    value = decode_tname(message.substr(i + 1));
		}
	    }
	}
    } // switch (conv_state)

    RETURN(true);
}

void
SocketClient::next(Xapian::weight w_min, Xapian::docid &did, Xapian::weight &w, string &value)
{
    if (w_min > minw) {
	minw = w_min;
	do_write("m" + om_tostring(w_min));
    }

    while (!get_posting(did, w, value)) wait_for_input();
}

void
SocketClient::skip_to(Xapian::docid new_did, Xapian::weight w_min, Xapian::docid &did, Xapian::weight &w, string &value)
{
    do_write("S" + om_tostring(new_did));
    if (w_min > minw) {
	minw = w_min;
	do_write("m" + om_tostring(w_min));
    }

    while (!get_posting(did, w, value) || (did && (did < new_did || w < w_min)))
	wait_for_input();
}
