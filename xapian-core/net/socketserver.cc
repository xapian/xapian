/* socketserver.cc: class for socket-based server.
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
#include "socketserver.h"
#include "database.h"
#include "stats.h"
#include "netutils.h"
#include "socketcommon.h"
#include "utils.h"
#include "om/omerror.h"
#include "omerr_string.h"
#include "termlist.h"
#include "document.h"
#include "omdebug.h"
#include "om/autoptr.h"
#include "../api/omdatabaseinternal.h"
#include "omdatabaseinterface.h"
#include <strstream.h>
#include <signal.h>
#include <cerrno>
#include <cstring>
#include <unistd.h>
#ifdef TIMING_PATCH
#include <sys/time.h>

#define uint64_t unsigned long long
#endif /* TIMING_PATCH */

/// The SocketServer constructor, taking two filedescriptors and a database.
SocketServer::SocketServer(OmDatabase db_, int readfd_, int writefd_,
#ifndef TIMING_PATCH
			   int msecs_timeout_)
#else /* TIMING_PATCH */
			   int msecs_timeout_, bool timing_)
#endif /* TIMING_PATCH */
	: db(db_),
	  readfd(readfd_),
	  writefd((writefd_ == -1) ? readfd_ : writefd_),
	  msecs_timeout(msecs_timeout_),
#ifdef TIMING_PATCH
	  timing(timing_),
#endif /* TIMING_PATCH */
	  buf(new OmSocketLineBuf(readfd, writefd)),
	  conversation_state(conv_ready),
	  gatherer(0),
	  have_global_stats(0)
{
    // ignore SIGPIPE - we check return values instead, and that
    // way we can easily throw an exception.
    if (signal(SIGPIPE, SIG_IGN) < 0) {
	throw OmNetworkError(std::string("signal: ") + strerror(errno));
    }
    buf->writeline("OM "STRINGIZE(OM_SOCKET_PROTOCOL_VERSION)" " +
		   om_tostring(db.get_doccount()) + " " +
		   om_tostring(db.get_avlength()));
}

SocketServer::SocketServer(OmDatabase db_, AutoPtr<OmLineBuf> buf_,
#ifndef TIMING_PATCH
			   int msecs_timeout_)
#else /* TIMING_PATCH */
			   int msecs_timeout_, bool timing_)
#endif /* TIMING_PATCH */
	: db(db_), readfd(-1), writefd(-1), msecs_timeout(msecs_timeout_),
#ifdef TIMING_PATCH
	  timing(timing_),
#endif /* TIMING_PATCH */
	  buf(buf_), conversation_state(conv_ready), gatherer(0),
	  have_global_stats(0)
{
    Assert(buf.get() != 0);
}

/// The SocketServer destructor
SocketServer::~SocketServer()
{
}

void
SocketServer::send_local_stats(Stats stats)
{
    buf->writeline("L" + stats_to_string(stats));
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
#ifdef TIMING_PATCH
	struct timeval stp, etp;
	uint64_t time = 0;
	uint64_t total = 0;
	uint64_t totalidle = 0;
	int returnval = 0;
#endif /* TIMING_PATCH */
	while (1) {
	    std::string message;

	    // Message 3 (see README_progprotocol.txt)
#ifdef TIMING_PATCH
	    returnval = gettimeofday(&stp, NULL);
#endif /* TIMING_PATCH */
	    message = buf->readline(msecs_timeout);
#ifndef TIMING_PATCH
	    
#else /* TIMING_PATCH */
	    returnval = gettimeofday(&etp, NULL);
	    time = ((1000000 * etp.tv_sec) + etp.tv_usec) - ((1000000 * stp.tv_sec) + stp.tv_usec);
	    totalidle += time;
#endif /* TIMING_PATCH */
	    switch (message.empty() ? '\0' : message[0]) {
#ifndef TIMING_PATCH
		case 'X': return;
		case 'Q': run_match(message.substr(1)); break;
		case 'T': run_gettermlist(message.substr(1)); break;
		case 'D': run_getdocument(message.substr(1)); break;
#else /* TIMING_PATCH */
	        case 'X': {
		    if (timing) {
			cout << "Total working time = " << total << " usecs. (socketserver.cc)\n";
			cout << "Total waiting time = " << totalidle << " usecs. (socketserver.cc)\n";
		    }
		    return;
		}
		case 'Q': {
		    returnval = gettimeofday(&stp, NULL);
		    run_match(message.substr(1));
		    returnval = gettimeofday(&etp, NULL);
		    time = ((1000000 * etp.tv_sec) + etp.tv_usec) - ((1000000 * stp.tv_sec) + stp.tv_usec);
		    total += time;
		    if (timing) cout << "Match time = " << time << " usecs. (socketserver.cc)\n";
		}
		break;
		case 'T': {
		    returnval = gettimeofday(&stp, NULL);
		    run_gettermlist(message.substr(1));
		    returnval = gettimeofday(&etp, NULL);
		    time = ((1000000 * etp.tv_sec) + etp.tv_usec) - ((1000000 * stp.tv_sec) + stp.tv_usec);
		    total += time;
		    if (timing) cout << "Get Term List time = " << time << " usecs. (socketserver.cc)\n";
		}
		break;
		case 'D': {
		    returnval = gettimeofday(&stp, NULL);
		    run_getdocument(message.substr(1));
		    gettimeofday(&etp, NULL);
		    time = ((1000000 * etp.tv_sec) + etp.tv_usec) - ((1000000 * stp.tv_sec) + stp.tv_usec);
		    total += time;
		    if (timing) cout << "Get Doc time = " << time << " usecs. (socketserver.cc)\n";
		}
		break;
#endif /* TIMING_PATCH */
		case 'm': break; // ignore min weight message left over from postlist
		case 'S': break; // ignore skip_to message left over from postlist
		default:
		    throw OmInvalidArgumentError(std::string("Unexpected message:") +
						 message);
	    }
	}
    } catch (const OmNetworkError &e) {
	// _Don't_ send network errors over, since they're likely to have
	// been caused by an error talking to the other end.
	throw;
    } catch (const OmError &e) {
	buf->writeline(std::string("E") + omerror_to_string(e));
	throw;
    } catch (...) {
	buf->writeline(std::string("EUNKNOWN"));
	throw;
    }
}

static OmLineBuf *snooper_buf; // FIXME FIXME FIXME
static snooper_do_collapse;

om_docid
match_snooper(const OmMSetItem &i, om_weight &w_min)
{
    om_docid new_did = 0;
    while (snooper_buf->data_waiting()) {
	std::string m = snooper_buf->readline(0);
	switch (m.empty() ? 0 : m[0]) {
	    case 'm': {
		// min weight has dropped
		istrstream is(m.c_str() + 1);
		om_weight w;
		is >> w;
		if (w < w_min) w_min = w;
		DEBUGLINE(UNKNOWN, "w_min now " << w_min);
		break;
	    }
	    case 'S': {
		// skip to
		istrstream is(m.c_str() + 1);
		is >> new_did;
		DEBUGLINE(UNKNOWN, "skip_to now " << new_did);
		break;
	    }
	    default:
	    Assert(false);
	}
    }
    if (i.did >= new_did && i.wt >= w_min) {
        std::string msg = om_tostring(i.did);
	if (i.wt != 0) msg += " " + om_tostring(i.wt);
	if (snooper_do_collapse) msg += ";" + omkey_to_string(i.collapse_key);
	snooper_buf->writeline(msg);
	return 0;
    }
    return (new_did > i.did + 1 ? new_did : 0);
}

void
SocketServer::run_match(const std::string &firstmessage)
{
    std::string message = firstmessage;
    
    gatherer = new NetworkStatsGatherer(this);
    
    OmQuery::Internal query = query_from_string(message);

    // extract the match options
    message = buf->readline(msecs_timeout);
    OmSettings moptions = string_to_moptions(message);

    // extract the rset
    message = buf->readline(msecs_timeout);
    OmRSet omrset = string_to_omrset(message);

    MultiMatch match(db, &query, omrset, moptions,
		     AutoPtr<StatsGatherer>(gatherer));

#if 0
    DEBUGLINE(UNKNOWN, "Adding artificial delay for statistics");
    sleep(1);
#endif

    // Message 4
    send_local_stats(gatherer->get_local_stats());

    // Message 5, part 1
    message = buf->readline(msecs_timeout);

    if (!startswith(message, "G")) {
	throw OmNetworkError(std::string("Expected 'G', got ") + message);
    }

    global_stats = string_to_stats(message.substr(1));
    have_global_stats = true;

    // Message 5, part 2
    message = buf->readline(msecs_timeout);

    if (message.substr(0, 1) != "M") {
	if (message.substr(0, 1) != "P") {
	    throw OmNetworkError(std::string("Expected M or P, got ") + message);
	}
	message = message.substr(1);
	om_doccount first;
	om_doccount maxitems;
	{
	    // extract first,maxitems
	    istrstream is(message.c_str());
	    is >> first >> maxitems;
	}
#if 1
	snooper_do_collapse = (moptions.get_int("match_collapse_key", -1) >= 0);
	PostList *pl;
	{
	    std::map<om_termname, OmMSet::TermFreqAndWeight> terminfo;
	    pl = match.get_postlist(first, maxitems, terminfo, 0);
	    buf->writeline(om_tostring(pl->get_termfreq()) + " " +
			   om_tostring(pl->recalc_maxweight()));
	    buf->writeline("O" + ommset_termfreqwts_to_string(terminfo));
	    snooper_buf = buf.get();
	    OmMSet mset;
	    match.get_mset_2(pl, terminfo, first, maxitems, mset, 0,
			     match_snooper);
	}
	buf->writeline("Z");
	return;
#else
	PostList *pl;
	{
	    std::map<om_termname, OmMSet::TermFreqAndWeight> terminfo;
	    // not sure we really need these numbers...
	    pl = match.get_postlist(first, maxitems, terminfo, 0);
	    buf->writeline(om_tostring(pl->get_termfreq()) + " " +
			   om_tostring(pl->recalc_maxweight())));
	    buf->writeline("O" + ommset_termfreqwts_to_string(terminfo));
	}
	om_docid did = 0;
	om_weight w_min = 0;
	om_keyno collapse_key;
	bool do_collapse = true;
	{
	    int val = moptions.get_int("match_collapse_key", -1);
	    if (val >= 0) {
		do_collapse = true;
		collapse_key = val;
	    }
	}
	while (1) {
	    om_docid new_did = 0;
	    while (buf->data_waiting()) {
		std::string m = buf->readline(0);
		switch (m.empty() ? 0 : m[0]) {
		    case 'm': {
			// min weight has dropped
			istrstream is(message.c_str() + 1);
			is >> w_min;
			DEBUGLINE(UNKNOWN, "w_min now " << w_min);
			break;
		    }
		    case 'S': {
			// skip to
			istrstream is(message.c_str() + 1);
			is >> new_did;
			DEBUGLINE(UNKNOWN, "skip_to now " << new_did);
			break;
		    }
		default:
		    Assert(false);
		}
	    }
	    PostList *p;
	    if (new_did > did + 1) {
		DEBUGLINE(UNKNOWN, "skip_to(" << new_did << ", " << w_min << ")");
		p = pl->skip_to(new_did, w_min);
	    } else {
		DEBUGLINE(UNKNOWN, "next(" << w_min << ")");
		p = pl->next(w_min);
	    }
	    if (p) {
		delete pl;
		pl = p;
	    }
	    if (pl->at_end()) break;
	    did = pl->get_docid();
	    om_weight w = pl->get_weight();
	    if (w >= w_min) {
		DEBUGLINE(UNKNOWN, "Returning did " << did << " wt " << w);
		std::string msg = om_tostring(did);
		if (w != 0) msg += " " + om_tostring(w);
		if (do_collapse) {
		    AutoPtr<LeafDocument> doc(OmDatabase::InternalInterface::get(match.db)->open_document(did));
		    msg += ";" + omkey_to_string(doc->get_key(collapse_key));		    
		}
		buf->writeline(msg);
	    } else {
		DEBUGLINE(UNKNOWN, "Ignoring did " << did << " wt " << w << " (since wt < " << w_min << ")");
	    }
	}
	delete pl;
	buf->writeline("Z");
	return;
#endif
    }
    message = message.substr(1);

#if 0
    DEBUGLINE(UNKNOWN, "Adding artificial delay...");
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

    DEBUGLINE(UNKNOWN, "About to get_mset(" << first
	      << ", " << maxitems << "...");

    match.get_mset(first, maxitems, mset, 0, 0);

    DEBUGLINE(UNKNOWN, "done get_mset...");

    buf->writeline("O" + ommset_to_string(mset));

    DEBUGLINE(UNKNOWN, "sent mset...");
}

void
SocketServer::run_gettermlist(const std::string &firstmessage)
{
    std::string message = firstmessage;

    om_docid did = atoi(message.c_str());

    OmTermListIterator tl = db.termlist_begin(did);
    OmTermListIterator tlend = db.termlist_end(did);

    while (tl != tlend) {
	std::string item = om_tostring(tl.get_wdf())
	    + " " + om_tostring(tl.get_termfreq()) + " " + encode_tname(*tl);
	buf->writeline(item);
	tl++;
    }

    buf->writeline("Z");
}

void
SocketServer::run_getdocument(const std::string &firstmessage)
{
    std::string message = firstmessage;

    om_docid did = atoi(message.c_str());

    AutoPtr<LeafDocument> doc(OmDatabase::InternalInterface::get(db)->open_document(did));

    buf->writeline("O" + encode_tname(doc->get_data().value));

    std::map<om_keyno, OmKey> keys = doc->get_all_keys();

    std::map<om_keyno, OmKey>::const_iterator i = keys.begin();
    while (i != keys.end()) {
	std::string item = om_tostring(i->first) + " "
	    + omkey_to_string(i->second);
	buf->writeline(item);
	++i;
    }

    buf->writeline("Z");
}

void
SocketServer::read_global_stats()
{
    Assert(conversation_state == conv_getglobal);

    global_stats = string_to_stats(buf->readline(msecs_timeout));

    conversation_state = conv_sendresult;

    have_global_stats = true;
}
