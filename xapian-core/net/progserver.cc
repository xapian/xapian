/* progserver.cc: class for fork()-based server.
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

#include "progserver.h"
#include "database.h"
#include "stats.h"
#include "localmatch.h"
#include "netutils.h"
#include "progcommon.h"
#include "utils.h"
#include <unistd.h>
#include <memory>
#include <strstream.h>

/// The ProgServer constructor, taking two filedescriptors and a database.
ProgServer::ProgServer(auto_ptr<MultiDatabase> db_,
		       int readfd_,
		       int writefd_)
	: db(db_), readfd(readfd_), writefd(writefd_),
	  buf(readfd, writefd),
	  conversation_state(conv_ready),
	  gatherer(0),
	  match(db_.get(),
		auto_ptr<StatsGatherer>(gatherer = new NetworkStatsGatherer(this))),
	  have_global_stats(0)
{
}

/// The ProgServer destructor
ProgServer::~ProgServer()
{
}

void
ProgServer::send_local_stats(Stats stats)
{
    string mystatstr = string("MYSTATS ") + stats_to_string(stats);

    buf.writeline(mystatstr);
    DebugMsg("ProgServer::send_local_stats(): wrote " << mystatstr);
}

Stats
ProgServer::get_global_stats()
{
    Assert(have_global_stats);

    return global_stats;
}

void
ProgServer::run()
{
    while (1) {
	string message;

	// Message 3 (see README_progprotocol.txt)
	message = buf.readline();

	if (message == "QUIT") {
	    return;
	};

	if (message.substr(0, 8) != "SETQUERY") {
	    cerr << "Expected SETQUERY, got " << message << endl;
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

	cerr << "Adding artificial delay for statistics" << endl;
	sleep(3);

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

	cerr << "Adding artificial delay..." << endl;
	sleep(5);

	om_doccount first;
	om_doccount maxitems;
	{
	    // extract first,maxitems
	    istrstream is(message.c_str());
	    is >> first >> maxitems;
	}

	vector<OmMSetItem> mset;
	om_doccount mbound;
	om_weight greatest_wt;

	cerr << "About to get_mset(" << first
		<< ", " << maxitems << "..." << endl;

	match.match(first,
		    maxitems,
		    mset,
		    msetcmp_forward,
		    &mbound,
		    &greatest_wt,
		    0);

	cerr << "done get_mset..." << endl;

	buf.writeline(string("MSETITEMS ") +
		      inttostring(mset.size()) + " "
		      + doubletostring(match.get_max_weight()));

	cerr << "sent size, maxweight..." << endl;

	for (vector<OmMSetItem>::iterator i=mset.begin();
	     i != mset.end();
	     ++i) {
	    char charbuf[100];
	    ostrstream os(charbuf, 100);
	    os << "MSETITEM: " << i->wt << " " << i->did << ends;
	    buf.writeline(charbuf);

	    cerr << "MSETITEM: " << i->wt << " " << i->did << endl;
	}
	//cerr << "sent items..." << endl;

	buf.writeline("OK");

	//cerr << "sent OK..." << endl;
    }
}

void
ProgServer::read_global_stats()
{
    Assert(conversation_state == conv_getglobal);

    global_stats = string_to_stats(buf.readline());

    conversation_state = conv_sendresult;

    have_global_stats = true;
}
