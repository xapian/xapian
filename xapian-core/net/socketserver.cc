/* socketserver.cc: class for socket-based server.
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
#include "socketserver.h"
#include "database.h"
#include "stats.h"
#include "localmatch.h"
#include "netutils.h"
#include "socketcommon.h"
#include "utils.h"
#include "om/omerror.h"
#include "omerr_string.h"
#include "termlist.h"
#include "document.h"
#include <memory>
#include <strstream.h>
#include <signal.h>
#include <cerrno>
#include <cstring>
#include <unistd.h>

/// The SocketServer constructor, taking two filedescriptors and a database.
SocketServer::SocketServer(OmRefCntPtr<MultiDatabase> db_,
			   int readfd_,
			   int writefd_)
	: db(db_),
	  readfd(readfd_),
	  writefd((writefd_ == -1) ? readfd_ : writefd_),
	  buf(readfd, writefd),
	  conversation_state(conv_ready),
	  gatherer(0),
	  have_global_stats(0)
{
    // ignore SIGPIPE - we check return values instead, and that
    // way we can easily throw an exception.
    if (signal(SIGPIPE, SIG_IGN) < 0) {
	throw OmNetworkError(string("signal: ") + strerror(errno));
    }
    buf.readline();
    buf.writeline("HELLO " +
		  inttostring(OM_SOCKET_PROTOCOL_VERSION) + " " +
		  inttostring(db->get_doccount()) + " " +
		  doubletostring(db->get_avlength()));
}

/// The SocketServer destructor
SocketServer::~SocketServer()
{
}

void
SocketServer::send_local_stats(Stats stats)
{
    string mystatstr = string("MYSTATS ") + stats_to_string(stats);

    buf.writeline(mystatstr);
    DebugMsg("SocketServer::send_local_stats(): wrote " << mystatstr);
}

Stats
SocketServer::get_global_stats()
{
    Assert(have_global_stats);

    return global_stats;
}

void
SocketServer::run()
{
    try {
	while (1) {
	    string message;

	    // Message 3 (see README_progprotocol.txt)
	    message = buf.readline();

	    if (message == "QUIT") {
		return;
	    } else if (startswith(message, "MOPTIONS ")) {
		run_match(message);
	    } else if (startswith(message, "GETTLIST ")) {
		run_gettermlist(message);
	    } else if (startswith(message, "GETDOC ")) {
		run_getdocument(message);
	    } else {
		throw OmInvalidArgumentError(string("Unexpected message:") +
					     message);
	    }

	}
    } catch (OmNetworkError &e) {
	// _Don't_ send network errors over, since they're likely to have
	// been caused by an error talking to the other end.
	throw;
    } catch (OmError &e) {
	buf.writeline(string("ERROR ") + omerror_to_string(e));
	throw;
    } catch (...) {
	buf.writeline(string("ERROR UNKNOWN"));
	throw;
    }
}

void
SocketServer::run_match(const string &firstmessage)
{
    string message = firstmessage;
    // extract the match options
    if (!startswith(message, "MOPTIONS ")) {
	throw OmNetworkError(string("Expected MOPTIONS, got ") + message);
    }

    OmMatchOptions moptions = string_to_moptions(message.substr(9));

    // extract the rset
    message = buf.readline();
    if (!startswith(message, "RSET ")) {
	DebugMsg("Expected RSET, got " << message << endl);
	throw OmNetworkError(string("Invalid message: ") + message);
    }
    OmRSet omrset = string_to_omrset(message.substr(5));

    // extract the query
    message = buf.readline();

    if (!startswith(message, "SETQUERY ")) {
	DebugMsg("Expected SETQUERY, got " << message << endl);
	throw OmNetworkError("Invalid message");
    }

    message = message.substr(9, message.npos);

    string wt_string;
    // FIXME: use an iterator or something.
    while (message.length() > 0 && isdigit(message[0])) {
	wt_string += message[0];
	message = message.substr(1);
    }
    message = message.substr(1);

    // extract the weighting type
    IRWeight::weight_type wt_type = 
	    static_cast<IRWeight::weight_type>(
					       atol(wt_string.c_str()));

    // Extract the query
    if (message[0] != '\"' || message[message.length()-1] != '\"') {
	throw OmNetworkError("Invalid query specification");
    } else {
	message = message.substr(1, message.length() - 2);
    }
    OmQueryInternal query = query_from_string(message);

    MultiMatch match(db.get(),
		     &query,
		     omrset,
		     wt_type,
		     moptions,
		     auto_ptr<StatsGatherer>(gatherer =
					     new NetworkStatsGatherer(this)));

#if 0
    DebugMsg("Adding artificial delay for statistics" << endl);
    sleep(1);
#endif

    // Message 4
    send_local_stats(gatherer->get_local_stats());

    // Message 5, part 1
    message = buf.readline();

    if (message.substr(0, 9) != "GLOBSTATS") {
	throw OmNetworkError(string("Expected GLOBSTATS, got ") + message);
    }

    global_stats = string_to_stats(message.substr(10));
    have_global_stats = true;

    // Message 5, part 2
    message = buf.readline();

    if (message.substr(0, 7) != "GETMSET") {
	throw OmNetworkError(string("Expected GETMSET, got ") + message);
    }
    message = message.substr(8);

#if 0
    DebugMsg("Adding artificial delay..." << endl);
    sleep(2);
#endif

    om_doccount first;
    om_doccount maxitems;
    {
	// extract first,maxitems
	istrstream is(message.c_str());
	is >> first >> maxitems;
    }

    OmMSet mset;

    DebugMsg("About to get_mset(" << first
	     << ", " << maxitems << "..." << endl);

    match.match(first,
		maxitems,
		mset,
		0);

    DebugMsg("done get_mset..." << endl);

    buf.writeline(string("MSETITEMS ") +
		  inttostring(mset.items.size()) + " "
		  + doubletostring(mset.max_possible)
		  + doubletostring(mset.max_attained));

    DebugMsg("sent size, maxweight..." << endl);

    for (vector<OmMSetItem>::iterator i=mset.items.begin();
	 i != mset.items.end();
	 ++i) {
	char charbuf[100];
	ostrstream os(charbuf, 100);
	os << "MSETITEM: " << i->wt << " " << i->did << 
		" " << omkey_to_string(i->collapse_key) << ends;
	buf.writeline(charbuf);

	DebugMsg("MSETITEM: " << i->wt << " " << i->did << endl);
    }
    //DebugMsg("sent items..." << endl);

    buf.writeline("OK");

    //DebugMsg("sent OK..." << endl);
}

void
SocketServer::run_gettermlist(const string &firstmessage)
{
    string message = firstmessage;
    // extract the match options
    if (!startswith(message, "GETTLIST ")) {
	throw OmNetworkError(string("Expected GETTLIST, got ") + message);
    }

    om_docid did = atoi(message.c_str() + 9);

    auto_ptr<LeafTermList> tl(db->open_term_list(did));

    tl->next();

    while (!tl->at_end()) {
	string item = "TLISTITEM ";
	item = item + encode_tname(tl->get_termname()) + " ";
	item = item + inttostring(tl->get_wdf()) + " ";
	item = item + inttostring(tl->get_termfreq());
	buf.writeline(item);

	tl->next();
    }

    buf.writeline("END");
}

void
SocketServer::run_getdocument(const string &firstmessage)
{
    string message = firstmessage;
    // extract the match options
    if (!startswith(message, "GETDOC ")) {
	throw OmNetworkError(string("Expected GETDOC, got ") + message);
    }

    om_docid did = atoi(message.c_str() + 7);

    auto_ptr<LeafDocument> doc(db->open_document(did));

    buf.writeline(string("DOC ") + encode_tname(doc->get_data().value));

    map<om_keyno, OmKey> keys = doc->get_all_keys();

    map<om_keyno, OmKey>::const_iterator i = keys.begin();
    while (i != keys.end()) {
	string item = string("KEY ") +
		inttostring(i->first) + " " +
		omkey_to_string(i->second);
	buf.writeline(item);
	++i;
    }

    buf.writeline("END");
}

void
SocketServer::read_global_stats()
{
    Assert(conversation_state == conv_getglobal);

    global_stats = string_to_stats(buf.readline());

    conversation_state = conv_sendresult;

    have_global_stats = true;
}
