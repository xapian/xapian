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
	: db(db_), readfd(readfd_),
	  writefd((writefd_ == -1)?readfd_ : writefd_),
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
	    MultiMatch match(db.get(),
			     auto_ptr<StatsGatherer>(gatherer =
						     new NetworkStatsGatherer(this)));

	    string message;

	    // Message 3 (see README_progprotocol.txt)
	    message = buf.readline();

	    if (message == "QUIT") {
		return;
	    };

	    // extract the match options
	    if (message.substr(0, 8) != "MOPTIONS") {
		throw OmNetworkError(string("Expected MOPTIONS, got ") + message);
	    }

	    {
		OmMatchOptions moptions = string_to_moptions(message.substr(9));

		match.set_options(moptions);
	    }

	    // extract the rset
	    message = buf.readline();
	    if (message.substr(0, 4) != "RSET") {
		DebugMsg("Expected RSET, got " << message << endl);
		throw OmNetworkError(string("Invalid message: ") + message);
	    }
	    {
		OmRSet omrset = string_to_omrset(message.substr(5));

		match.set_rset(omrset);
	    }


	    // extract the query
	    message = buf.readline();

	    if (message.substr(0, 8) != "SETQUERY") {
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

	    {
		// extract the weighting type
		IRWeight::weight_type wt_type = 
			static_cast<IRWeight::weight_type>(
							   atol(wt_string.c_str()));
		match.set_weighting(wt_type);
	    }

	    {
		// Extract the query
		if (message[0] != '\"' || message[message.length()-1] != '\"') {
		    throw OmNetworkError("Invalid query specification");
		} else {
		    message = message.substr(1, message.length() - 2);
		}
		OmQueryInternal temp = query_from_string(message);
		match.set_query(&temp);
	    }

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
		os << "MSETITEM: " << i->wt << " " << i->did << ends;
		buf.writeline(charbuf);

		DebugMsg("MSETITEM: " << i->wt << " " << i->did << endl);
	    }
	    //DebugMsg("sent items..." << endl);

	    buf.writeline("OK");

	    //DebugMsg("sent OK..." << endl);
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
SocketServer::read_global_stats()
{
    Assert(conversation_state == conv_getglobal);

    global_stats = string_to_stats(buf.readline());

    conversation_state = conv_sendresult;

    have_global_stats = true;
}
