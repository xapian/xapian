/* socketserver.cc: class for socket-based server.
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2001,2002 Ananova Ltd
 * Copyright 2002,2003,2004,2005,2006 Olly Betts
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301
 * USA
 */

#include <config.h>

#include "safeerrno.h"

#include "socketserver.h"
#include "database.h"
#include "networkstats.h"
#include "netutils.h"
#include "socketcommon.h"
#include "utils.h"
#include <xapian/error.h>
#include "omerr_string.h"
#include "document.h"
#include "omdebug.h"
#include "autoptr.h"
#include <xapian/enquire.h>
#include "multimatch.h"
#include <signal.h>
#include <cstring>
#ifdef HAVE_SSTREAM
#include <sstream>
using std::istringstream;
#else
#include <strstream.h>
#endif
#ifdef TIMING_PATCH
#include <iostream>
using std::cout;

#include <sys/time.h>

#define uint64_t unsigned long long
#endif /* TIMING_PATCH */

/// An object used for "close down" exceptions
struct SocketServerFinished { };

/// The SocketServer constructor, taking two filedescriptors and a database.
SocketServer::SocketServer(const Xapian::Database &db_, int readfd_, int writefd_,
			   int msecs_active_timeout_, int msecs_idle_timeout_
#ifdef TIMING_PATCH
			   , bool timing_
#endif /* TIMING_PATCH */
			   )
	: db(db_),
	  readfd(readfd_),
	  writefd((writefd_ == -1) ? readfd_ : writefd_),
	  msecs_active_timeout(msecs_active_timeout_),
	  msecs_idle_timeout(msecs_idle_timeout_),
#ifdef TIMING_PATCH
	  timing(timing_),
#endif /* TIMING_PATCH */
	  buf(new OmSocketLineBuf(readfd, writefd, "socketserver(" + db.get_description() + ')')),
	  conversation_state(conv_ready),
	  gatherer(0),
	  have_global_stats(0)
{
    // ignore SIGPIPE - we check return values instead, and that
    // way we can easily throw an exception.
    if (signal(SIGPIPE, SIG_IGN) < 0) {
	throw Xapian::NetworkError("Couldn't install SIGPIPE handler", errno);
    }
    writeline("OM "STRINGIZE(XAPIAN_SOCKET_PROTOCOL_VERSION)" " +
	      om_tostring(db.get_doccount()) + ' ' +
	      om_tostring(db.get_avlength()));
    // FIXME: these registrations duplicated below - refactor into method
    Xapian::Weight *wt = new Xapian::BoolWeight();
    wtschemes[wt->name()] = wt;
    wt = new Xapian::TradWeight();
    wtschemes[wt->name()] = wt;
    wt = new Xapian::BM25Weight();
    wtschemes[wt->name()] = wt;
}

SocketServer::SocketServer(const Xapian::Database &db_, AutoPtr<OmLineBuf> buf_,
			   int msecs_active_timeout_, int msecs_idle_timeout_
#ifdef TIMING_PATCH
			   , bool timing_
#endif /* TIMING_PATCH */
			   )
	: db(db_), readfd(-1), writefd(-1),
	  msecs_active_timeout(msecs_active_timeout_),
	  msecs_idle_timeout(msecs_idle_timeout_),
#ifdef TIMING_PATCH
	  timing(timing_),
#endif /* TIMING_PATCH */
	  buf(buf_), conversation_state(conv_ready), gatherer(0),
	  have_global_stats(0)
{
    Assert(buf.get() != 0);
    // FIXME: these registrations duplicated above - refactor into method
    Xapian::Weight *wt = new Xapian::BoolWeight();
    wtschemes[wt->name()] = wt;
    wt = new Xapian::TradWeight();
    wtschemes[wt->name()] = wt;
    wt = new Xapian::BM25Weight();
    wtschemes[wt->name()] = wt;
}

/// The SocketServer destructor
SocketServer::~SocketServer()
{
    map<string, Xapian::Weight *>::const_iterator i;
    for (i = wtschemes.begin(); i != wtschemes.end(); ++i) {
	delete i->second;
    }
    wtschemes.clear();
}

void
SocketServer::send_local_stats(const Stats &stats)
{
    writeline("L" + stats_to_string(stats));
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
#ifdef TIMING_PATCH
    struct timeval stp, etp;
    uint64_t time = 0;
    uint64_t total = 0;
    uint64_t totalidle = 0;
    int returnval = 0;
#endif /* TIMING_PATCH */
    while (true) {
	try {
	    string message;

	    // Message 2 (see remote_protocol.html)
#ifdef TIMING_PATCH
	    returnval = gettimeofday(&stp, NULL);
#endif /* TIMING_PATCH */
	    message = readline(msecs_idle_timeout);

#ifdef TIMING_PATCH
	    returnval = gettimeofday(&etp, NULL);
	    time = ((1000000 * etp.tv_sec) + etp.tv_usec)
		- ((1000000 * stp.tv_sec) + stp.tv_usec);
	    totalidle += time;
#endif /* TIMING_PATCH */
	    switch (message.empty() ? '\0' : message[0]) {
#ifndef TIMING_PATCH
		case 'Q': run_match(message.substr(1)); break;
		case 'T': run_gettermlist(message.substr(1)); break;
		case 'D': run_getdocument(message.substr(1)); break;
		case 'K': run_keepalive(); break;
#else /* TIMING_PATCH */
		case 'Q': {
		    returnval = gettimeofday(&stp, NULL);
		    run_match(message.substr(1));
		    returnval = gettimeofday(&etp, NULL);
		    time = ((1000000 * etp.tv_sec) + etp.tv_usec)
			- ((1000000 * stp.tv_sec) + stp.tv_usec);
		    total += time;
		    if (timing) cout << "Match time = " << time
			<< " usecs. (socketserver.cc)\n";
		    break;
		}
		case 'T': {
		    returnval = gettimeofday(&stp, NULL);
		    run_gettermlist(message.substr(1));
		    returnval = gettimeofday(&etp, NULL);
		    time = ((1000000 * etp.tv_sec) + etp.tv_usec)
			- ((1000000 * stp.tv_sec) + stp.tv_usec);
		    total += time;
		    if (timing) cout << "Get Term List time = " << time
			<< " usecs. (socketserver.cc)\n";
		    break;
		}
		case 'D': {
		    returnval = gettimeofday(&stp, NULL);
		    run_getdocument(message.substr(1));
		    gettimeofday(&etp, NULL);
		    time = ((1000000 * etp.tv_sec) + etp.tv_usec)
			- ((1000000 * stp.tv_sec) + stp.tv_usec);
		    total += time;
		    if (timing) cout << "Get Doc time = " << time
			<< " usecs. (socketserver.cc)\n";
		    break;
		}
		case 'K': {
		    returnval = gettimeofday(&stp, NULL);
		    run_keepalive();
		    gettimeofday(&etp, NULL);
		    time = ((1000000 * etp.tv_sec) + etp.tv_usec)
			- ((1000000 * stp.tv_sec) + stp.tv_usec);
		    total += time;
		    if (timing) cout << "Keep-alive time = " << time
			<< " usecs. (socketserver.cc)\n";
		    break;
		}
#endif /* TIMING_PATCH */
		case 'm': // ignore min weight message left over from postlist
		    break;
		case 'S': // ignore skip_to message left over from postlist
		    break;
		case 'F': // get_termfreq
		    writeline("F" +
			      om_tostring(db.get_termfreq(decode_tname(message.substr(1)))));
		    break;
		case 't': // term_exists
		    if (db.term_exists(decode_tname(message.substr(1))))
			writeline("t1");
		    else
			writeline("t0");
		    break;
		default:
		    throw Xapian::InvalidArgumentError(string("Unexpected message:") +
						       message);
	    }
	} catch (const SocketServerFinished &) {
	    // received close message, just return.
#ifdef TIMING_PATCH
	    if (timing) {
		cout << "Total working time = " << total
		    << " usecs. (socketserver.cc)\n";
		cout << "Total waiting time = " << totalidle
		    << " usecs. (socketserver.cc)\n";
	    }
#endif
	    return;
	} catch (const Xapian::NetworkError &e) {
	    // _Don't_ send network errors over, since they're likely
	    // to have been caused by an error talking to the other end.
	    // (This isn't necessarily true with cascaded remote
	    // databases, though...)
	    throw;
	} catch (const Xapian::Error &e) {
	    /* Pass the error across the link, and continue. */
	    writeline(string("E") + omerror_to_string(e));
	} catch (...) {
	    /* Do what we can reporting the error, and then propagate
	     * the exception.
	     */
	    writeline(string("EUNKNOWN"));
	    throw;
	}
    }
}

void
SocketServer::run_match(const string &firstmessage)
{
    string message = firstmessage;

    gatherer = new NetworkStatsGatherer(this);

    Xapian::Query::Internal * query;
    query = Xapian::Query::Internal::unserialise(message);

    // extract the match options
    message = readline(msecs_active_timeout);

    Xapian::termcount qlen;
    Xapian::Enquire::docid_order order;
    Xapian::valueno sort_key;
    bool sort_by_relevance, sort_value_forward;
    Xapian::valueno collapse_key;
    int percent_cutoff;
    Xapian::weight weight_cutoff;
    string weighting_scheme;

    {
#ifdef HAVE_SSTREAM
	istringstream is(message);
#else
	istrstream is(message.data(), message.length());
#endif
	int order_int;
	is >> qlen >> collapse_key >> order_int
	   >> sort_key >> sort_by_relevance >> sort_value_forward
	   >> percent_cutoff >> weight_cutoff;
	order = Xapian::Enquire::docid_order(order_int);
    }

    // extract the weight object
    message = readline(msecs_active_timeout);
    map<string, Xapian::Weight *>::const_iterator i = wtschemes.find(message);
    if (i == wtschemes.end()) {
	throw Xapian::InvalidArgumentError("Weighting scheme " + message + " not registered");
    }
    message = readline(msecs_active_timeout);
    AutoPtr<Xapian::Weight> wt(i->second->unserialise(message));

    // extract the rset
    message = readline(msecs_active_timeout);
    Xapian::RSet omrset = string_to_omrset(message);

    MultiMatch match(db, query, qlen, omrset, collapse_key, percent_cutoff,
		     weight_cutoff, order,
		     sort_key, sort_by_relevance, sort_value_forward,
		     0, 0, NULL, gatherer, wt.get());

#if 0
    DEBUGLINE(UNKNOWN, "Adding artificial delay for statistics");
    sleep(1);
#endif

    // Message 3
    send_local_stats(gatherer->get_local_stats());

    // Message 4, part 1
    message = readline(msecs_active_timeout);

    if (message.empty() || message[0] != 'G') {
	throw Xapian::NetworkError(string("Expected 'G', got ") + message);
    }

    global_stats = string_to_stats(message.substr(1));
    have_global_stats = true;

    // Message 4, part 2
    message = readline(msecs_active_timeout);

    if (message.empty() || message[0] != 'M') {
	throw Xapian::NetworkError(string("Expected 'M', got ") + message);
    }

    message = message.substr(1);

#if 0
    DEBUGLINE(UNKNOWN, "Adding artificial delay...");
    sleep(2);
#endif

    Xapian::doccount first;
    Xapian::doccount maxitems;
    {
	// extract first,maxitems
#ifdef HAVE_SSTREAM
	istringstream is(message);
#else
	istrstream is(message.data(), message.length());
#endif
	is >> first >> maxitems;
    }

    Xapian::MSet mset;

    DEBUGLINE(UNKNOWN, "About to get_mset(" << first
	      << ", " << maxitems << "...");

    match.get_mset(first, maxitems, 0, mset, 0);

    DEBUGLINE(UNKNOWN, "done get_mset...");

    writeline("O" + ommset_to_string(mset));

    DEBUGLINE(UNKNOWN, "sent mset...");
}

string
SocketServer::readline(int msecs_timeout)
{
    string result = buf->readline(OmTime::now() + OmTime(msecs_timeout));
    DEBUGLINE(UNKNOWN, "READ[" << result << "]");
    // intercept 'X' messages.
    if (!result.empty() && result[0] == 'X') {
	throw SocketServerFinished();
    }
    return result;
}

void
SocketServer::writeline(const string &message,
			int milliseconds_timeout)
{
    if (milliseconds_timeout == 0) {
	// default to our normal timeout
	milliseconds_timeout = msecs_active_timeout;
    }
    buf->writeline(message, OmTime::now() + OmTime(milliseconds_timeout));
}

void
SocketServer::run_gettermlist(const string &firstmessage)
{
    string message = firstmessage;

    Xapian::docid did = atoi(message);

    Xapian::TermIterator tl = db.termlist_begin(did);
    Xapian::TermIterator tlend = db.termlist_end(did);

    while (tl != tlend) {
	string item = om_tostring(tl.get_wdf());
	item += ' ';
	item += om_tostring(tl.get_termfreq());
	item += ' ';
	item += encode_tname(*tl);
	writeline(item);
	tl++;
    }

    writeline("Z");
}

void
SocketServer::run_getdocument(const string &firstmessage)
{
    string message = firstmessage;

    Xapian::docid did = atoi(message);

    unsigned int multiplier = db.internal.size();
    Assert(multiplier != 0);
    Xapian::doccount n = (did - 1) % multiplier; // which actual database
    Xapian::docid m = (did - 1) / multiplier + 1; // real docid in that database
    AutoPtr<Xapian::Document::Internal> doc(db.internal[n]->open_document(m));

    writeline("O" + encode_tname(doc->get_data()));

    map<Xapian::valueno, string> values = doc->get_all_values();

    map<Xapian::valueno, string>::const_iterator i = values.begin();
    while (i != values.end()) {
	string item = om_tostring(i->first);
	item += ' ';
	item += encode_tname(i->second);
	writeline(item);
	++i;
    }

    writeline("Z");
}

void
SocketServer::run_keepalive()
{
    /* Transmit to any of our own remote databases */
    db.keep_alive();
    writeline("OK");
}
