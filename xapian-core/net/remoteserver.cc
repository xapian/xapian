/** @file remoteserver.cc
 *  @brief Xapian remote backend server base class
 */
/* Copyright (C) 2006,2007,2008,2009,2010,2011,2012,2013,2014,2015,2016,2017,2018,2019 Olly Betts
 * Copyright (C) 2006,2007,2009,2010 Lemur Consulting Ltd
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#include <config.h>
#include "remoteserver.h"

#include "xapian/constants.h"
#include "xapian/database.h"
#include "xapian/enquire.h"
#include "xapian/error.h"
#include "xapian/matchspy.h"
#include "xapian/query.h"
#include "xapian/rset.h"
#include "xapian/valueiterator.h"

#include <signal.h>
#include <cerrno>
#include <cstdlib>
#include <memory>

#include "api/msetinternal.h"
#include "api/termlist.h"
#include "matcher/matcher.h"
#include "omassert.h"
#include "pack.h"
#include "realtime.h"
#include "serialise.h"
#include "serialise-double.h"
#include "serialise-error.h"
#include "str.h"
#include "stringutils.h"
#include "weight/weightinternal.h"

using namespace std;

[[noreturn]]
static void
throw_read_only()
{
    throw Xapian::InvalidOperationError("Server is read-only");
}

/// Class to throw when we receive the connection closing message.
struct ConnectionClosed { };

RemoteServer::RemoteServer(const vector<string>& dbpaths,
			   int fdin_, int fdout_,
			   double active_timeout_, double idle_timeout_,
			   bool writable_)
    : RemoteConnection(fdin_, fdout_, string()),
      db(NULL), wdb(NULL), writable(writable_),
      active_timeout(active_timeout_), idle_timeout(idle_timeout_)
{
    // Catch errors opening the database and propagate them to the client.
    try {
	Assert(!dbpaths.empty());
	// We always open the database read-only to start with.  If we're
	// writable, the client can ask to be upgraded to write access once
	// connected if it wants it.
	db = new Xapian::Database(dbpaths[0]);
	// Build a better description than Database::get_description() gives
	// in the variable context.  FIXME: improve Database::get_description()
	// and then just use that instead.
	context = dbpaths[0];

	if (!writable) {
	    vector<string>::const_iterator i(dbpaths.begin());
	    for (++i; i != dbpaths.end(); ++i) {
		db->add_database(Xapian::Database(*i));
		context += ' ';
		context += *i;
	    }
	} else {
	    AssertEq(dbpaths.size(), 1); // Expecting exactly one database.
	}
    } catch (const Xapian::Error &err) {
	// Propagate the exception to the client.
	send_message(REPLY_EXCEPTION, serialise_error(err));
	// And rethrow it so our caller can log it and close the connection.
	throw;
    }

#ifndef __WIN32__
    // It's simplest to just ignore SIGPIPE.  We'll still know if the
    // connection dies because we'll get EPIPE back from write().
    if (signal(SIGPIPE, SIG_IGN) == SIG_ERR)
	throw Xapian::NetworkError("Couldn't set SIGPIPE to SIG_IGN", errno);
#endif

    // Send greeting message.
    msg_update(string());
}

RemoteServer::~RemoteServer()
{
    delete db;
    // wdb is either NULL or equal to db, so we shouldn't delete it too!
}

message_type
RemoteServer::get_message(double timeout, string & result,
			  message_type required_type)
{
    double end_time = RealTime::end_time(timeout);
    int type = RemoteConnection::get_message(result, end_time);

    // Handle "shutdown connection" message here.  Treat EOF here for a read-only
    // database the same way since a read-only client just closes the
    // connection when done.
    if (type == MSG_SHUTDOWN || (type < 0 && wdb == NULL))
	throw ConnectionClosed();
    if (type < 0)
	throw Xapian::NetworkError("Connection closed unexpectedly");
    if (type >= MSG_MAX) {
	string errmsg("Invalid message type ");
	errmsg += str(type);
	throw Xapian::NetworkError(errmsg);
    }
    if (required_type != MSG_MAX && type != int(required_type)) {
	string errmsg("Expecting message type ");
	errmsg += str(int(required_type));
	errmsg += ", got ";
	errmsg += str(type);
	throw Xapian::NetworkError(errmsg);
    }
    return static_cast<message_type>(type);
}

void
RemoteServer::send_message(reply_type type, const string &message)
{
    double end_time = RealTime::end_time(active_timeout);
    unsigned char type_as_char = static_cast<unsigned char>(type);
    RemoteConnection::send_message(type_as_char, message, end_time);
}

typedef void (RemoteServer::* dispatch_func)(const string &);

void
RemoteServer::run()
{
    while (true) {
	try {
	    string message;
	    size_t type = get_message(idle_timeout, message);
	    switch (type) {
		case MSG_ALLTERMS:
		    msg_allterms(message);
		    continue;
		case MSG_COLLFREQ:
		    msg_collfreq(message);
		    continue;
		case MSG_DOCUMENT:
		    msg_document(message);
		    continue;
		case MSG_TERMEXISTS:
		    msg_termexists(message);
		    continue;
		case MSG_TERMFREQ:
		    msg_termfreq(message);
		    continue;
		case MSG_VALUESTATS:
		    msg_valuestats(message);
		    continue;
		case MSG_KEEPALIVE:
		    msg_keepalive(message);
		    continue;
		case MSG_DOCLENGTH:
		    msg_doclength(message);
		    continue;
		case MSG_QUERY:
		    msg_query(message);
		    continue;
		case MSG_TERMLIST:
		    msg_termlist(message);
		    continue;
		case MSG_POSITIONLIST:
		    msg_positionlist(message);
		    continue;
		case MSG_POSTLIST:
		    msg_postlist(message);
		    continue;
		case MSG_REOPEN:
		    msg_reopen(message);
		    continue;
		case MSG_UPDATE:
		    msg_update(message);
		    continue;
		case MSG_ADDDOCUMENT:
		    msg_adddocument(message);
		    continue;
		case MSG_CANCEL:
		    msg_cancel(message);
		    continue;
		case MSG_DELETEDOCUMENTTERM:
		    msg_deletedocumentterm(message);
		    continue;
		case MSG_COMMIT:
		    msg_commit(message);
		    continue;
		case MSG_REPLACEDOCUMENT:
		    msg_replacedocument(message);
		    continue;
		case MSG_REPLACEDOCUMENTTERM:
		    msg_replacedocumentterm(message);
		    continue;
		case MSG_DELETEDOCUMENT:
		    msg_deletedocument(message);
		    continue;
		case MSG_WRITEACCESS:
		    msg_writeaccess(message);
		    continue;
		case MSG_GETMETADATA:
		    msg_getmetadata(message);
		    continue;
		case MSG_SETMETADATA:
		    msg_setmetadata(message);
		    continue;
		case MSG_ADDSPELLING:
		    msg_addspelling(message);
		    continue;
		case MSG_REMOVESPELLING:
		    msg_removespelling(message);
		    continue;
		case MSG_METADATAKEYLIST:
		    msg_metadatakeylist(message);
		    continue;
		case MSG_FREQS:
		    msg_freqs(message);
		    continue;
		case MSG_UNIQUETERMS:
		    msg_uniqueterms(message);
		    continue;
		case MSG_POSITIONLISTCOUNT:
		    msg_positionlistcount(message);
		    continue;
		case MSG_RECONSTRUCTTEXT:
		    msg_reconstructtext(message);
		    continue;
		default: {
		    // MSG_GETMSET - used during a conversation.
		    // MSG_SHUTDOWN - handled by get_message().
		    string errmsg("Unexpected message type ");
		    errmsg += str(type);
		    throw Xapian::InvalidArgumentError(errmsg);
		}
	    }
	} catch (const Xapian::NetworkTimeoutError & e) {
	    try {
		// We've had a timeout, so the client may not be listening, so
		// set the end_time to 1 and if we can't send the message right
		// away, just exit and the client will cope.
		send_message(REPLY_EXCEPTION, serialise_error(e), 1.0);
	    } catch (...) {
	    }
	    // And rethrow it so our caller can log it and close the
	    // connection.
	    throw;
	} catch (const Xapian::NetworkError &) {
	    // All other network errors mean we are fatally confused and are
	    // unlikely to be able to communicate further across this
	    // connection.  So we don't try to propagate the error to the
	    // client, but instead just rethrow the exception so our caller can
	    // log it and close the connection.
	    throw;
	} catch (const Xapian::Error &e) {
	    // Propagate the exception to the client, then return to the main
	    // message handling loop.
	    send_message(REPLY_EXCEPTION, serialise_error(e));
	} catch (ConnectionClosed &) {
	    return;
	} catch (...) {
	    // Propagate an unknown exception to the client.
	    send_message(REPLY_EXCEPTION, string());
	    // And rethrow it so our caller can log it and close the
	    // connection.
	    throw;
	}
    }
}

void
RemoteServer::msg_allterms(const string& message)
{
    string reply;
    string prev = message;
    const string& prefix = message;
    for (Xapian::TermIterator t = db->allterms_begin(prefix);
	 t != db->allterms_end(prefix);
	 ++t) {
	if (rare(prev.size() > 255))
	    prev.resize(255);
	const string& term = *t;
	size_t reuse = common_prefix_length(prev, term);
	reply.append(1, char(reuse));
	pack_uint(reply, term.size() - reuse);
	reply.append(term, reuse, string::npos);
	pack_uint(reply, t.get_termfreq());
	prev = term;
    }
    send_message(REPLY_ALLTERMS, reply);
}

void
RemoteServer::msg_termlist(const string &message)
{
    const char *p = message.data();
    const char *p_end = p + message.size();
    Xapian::docid did;
    if (!unpack_uint_last(&p, p_end, &did)) {
	throw Xapian::NetworkError("Bad MSG_TERMLIST");
    }
    Xapian::TermIterator t = db->termlist_begin(did);
    Xapian::termcount num_terms = 0;
    if (t.internal)
	num_terms = t.internal->get_approx_size();
    string reply;
    pack_uint(reply, db->get_doclength(did));
    pack_uint_last(reply, num_terms);
    send_message(REPLY_TERMLISTHEADER, reply);

    reply.resize(0);
    string prev;
    while (t != db->termlist_end(did)) {
	if (rare(prev.size() > 255))
	    prev.resize(255);
	const string& term = *t;
	size_t reuse = common_prefix_length(prev, term);
	reply.append(1, char(reuse));
	pack_uint(reply, term.size() - reuse);
	reply.append(term, reuse, string::npos);
	pack_uint(reply, t.get_wdf());
	pack_uint(reply, t.get_termfreq());
	prev = term;
	++t;
    }
    send_message(REPLY_TERMLIST, reply);
}

void
RemoteServer::msg_positionlist(const string &message)
{
    const char *p = message.data();
    const char *p_end = p + message.size();
    Xapian::docid did;
    if (!unpack_uint(&p, p_end, &did)) {
	throw Xapian::NetworkError("Bad MSG_POSITIONLIST");
    }
    string term(p, p_end - p);

    string reply;
    Xapian::termpos lastpos = static_cast<Xapian::termpos>(-1);
    for (Xapian::PositionIterator i = db->positionlist_begin(did, term);
	 i != db->positionlist_end(did, term);
	 ++i) {
	Xapian::termpos pos = *i;
	pack_uint(reply, pos - lastpos - 1);
	lastpos = pos;
    }
    send_message(REPLY_POSITIONLIST, reply);
}

void
RemoteServer::msg_positionlistcount(const string &message)
{
    const char *p = message.data();
    const char *p_end = p + message.size();
    Xapian::docid did;
    if (!unpack_uint(&p, p_end, &did)) {
	throw Xapian::NetworkError("Bad MSG_POSITIONLISTCOUNT");
    }

    // This is kind of clumsy, but what the public API requires.
    Xapian::termcount result = 0;
    Xapian::TermIterator termit = db->termlist_begin(did);
    if (termit != db->termlist_end(did)) {
	string term(p, p_end - p);
	termit.skip_to(term);
	if (termit != db->termlist_end(did)) {
	    result = termit.positionlist_count();
	}
    }
    string reply;
    pack_uint_last(reply, result);
    send_message(REPLY_POSITIONLISTCOUNT, reply);
}

void
RemoteServer::msg_postlist(const string &message)
{
    const string & term = message;

    Xapian::doccount termfreq = db->get_termfreq(term);
    string reply;
    pack_uint_last(reply, termfreq);
    send_message(REPLY_POSTLISTHEADER, reply);

    reply.resize(0);
    Xapian::docid lastdocid = 0;
    for (Xapian::PostingIterator i = db->postlist_begin(term);
	 i != db->postlist_end(term);
	 ++i) {
	Xapian::docid newdocid = *i;
	pack_uint(reply, newdocid - lastdocid - 1);
	pack_uint(reply, i.get_wdf());

	lastdocid = newdocid;
    }

    send_message(REPLY_POSTLIST, reply);
}

void
RemoteServer::msg_writeaccess(const string & msg)
{
    if (!writable)
	throw_read_only();

    int flags = 0;
    const char *p = msg.c_str();
    const char *p_end = p + msg.size();
    if (p != p_end) {
	unsigned flag_bits;
	if (!unpack_uint_last(&p, p_end, &flag_bits)) {
	    throw Xapian::NetworkError("Bad flags in MSG_WRITEACCESS");
	}
	flags = flag_bits &~ Xapian::DB_ACTION_MASK_;
    }

    wdb = new Xapian::WritableDatabase(db->lock(flags));
    delete db;
    db = wdb;
    msg_update(msg);
}

void
RemoteServer::msg_reopen(const string & msg)
{
    if (!db->reopen()) {
	send_message(REPLY_DONE, string());
	return;
    }
    msg_update(msg);
}

void
RemoteServer::msg_update(const string &)
{
    static const char protocol[2] = {
	char(XAPIAN_REMOTE_PROTOCOL_MAJOR_VERSION),
	char(XAPIAN_REMOTE_PROTOCOL_MINOR_VERSION)
    };
    string message(protocol, 2);
    Xapian::doccount num_docs = db->get_doccount();
    pack_uint(message, num_docs);
    pack_uint(message, db->get_lastdocid() - num_docs);
    Xapian::termcount doclen_lb = db->get_doclength_lower_bound();
    pack_uint(message, doclen_lb);
    pack_uint(message, db->get_doclength_upper_bound() - doclen_lb);
    pack_bool(message, db->has_positions());
    pack_uint(message, db->get_total_length());
    message += db->get_uuid();
    send_message(REPLY_UPDATE, message);
}

void
RemoteServer::msg_query(const string &message_in)
{
    const char *p = message_in.c_str();
    const char *p_end = p + message_in.size();

    // Unserialise the Query.
    string serialisation;
    if (!unpack_string(&p, p_end, serialisation)) {
	throw Xapian::NetworkError("Bad MSG_QUERY");
    }

    Xapian::Query query(Xapian::Query::unserialise(serialisation, reg));

    // Unserialise assorted Enquire settings.
    Xapian::termcount qlen;
    Xapian::valueno collapse_max;
    if (!unpack_uint(&p, p_end, &qlen) ||
	!unpack_uint(&p, p_end, &collapse_max)) {
	throw Xapian::NetworkError("Bad MSG_QUERY");
    }

    Xapian::valueno collapse_key = Xapian::BAD_VALUENO;
    if (collapse_max) {
	if (!unpack_uint(&p, p_end, &collapse_key)) {
	    throw Xapian::NetworkError("Bad MSG_QUERY");
	}
    }

    if (p_end - p < 4 || static_cast<unsigned char>(*p) > 2) {
	throw Xapian::NetworkError("bad message (docid_order)");
    }
    Xapian::Enquire::docid_order order;
    order = static_cast<Xapian::Enquire::docid_order>(*p++);

    if (static_cast<unsigned char>(*p) > 3) {
	throw Xapian::NetworkError("bad message (sort_by)");
    }
    Xapian::Enquire::Internal::sort_setting sort_by;
    sort_by = static_cast<Xapian::Enquire::Internal::sort_setting>(*p++);

    Xapian::valueno sort_key = Xapian::BAD_VALUENO;
    if (sort_by != Xapian::Enquire::Internal::REL) {
	if (!unpack_uint(&p, p_end, &sort_key)) {
	    throw Xapian::NetworkError("Bad MSG_QUERY");
	}
    }

    bool sort_value_forward;
    if (!unpack_bool(&p, p_end, &sort_value_forward)) {
	throw Xapian::NetworkError("bad message (sort_value_forward)");
    }

    bool full_db_has_positions;
    if (!unpack_bool(&p, p_end, &full_db_has_positions)) {
	throw Xapian::NetworkError("bad message (full_db_has_positions)");
    }

    double time_limit = unserialise_double(&p, p_end);

    int percent_threshold = *p++;
    if (percent_threshold < 0 || percent_threshold > 100) {
	throw Xapian::NetworkError("bad message (percent_threshold)");
    }

    double weight_threshold = unserialise_double(&p, p_end);
    if (weight_threshold < 0) {
	throw Xapian::NetworkError("bad message (weight_threshold)");
    }

    // Unserialise the Weight object.
    string wtname;
    if (!unpack_string(&p, p_end, wtname)) {
	throw Xapian::NetworkError("Bad MSG_QUERY");
    }

    const Xapian::Weight * wttype = reg.get_weighting_scheme(wtname);
    if (wttype == NULL) {
	// Note: user weighting schemes should be registered by adding them to
	// a Registry, and setting the context using
	// RemoteServer::set_registry().
	throw Xapian::InvalidArgumentError("Weighting scheme " +
					   wtname + " not registered");
    }

    if (!unpack_string(&p, p_end, serialisation)) {
	throw Xapian::NetworkError("Bad MSG_QUERY");
    }
    unique_ptr<Xapian::Weight> wt(wttype->unserialise(serialisation));

    // Unserialise the RSet object.
    if (!unpack_string(&p, p_end, serialisation)) {
	throw Xapian::NetworkError("Bad MSG_QUERY");
    }
    Xapian::RSet rset = unserialise_rset(serialisation);

    // Unserialise any MatchSpy objects.
    vector<Xapian::Internal::opt_intrusive_ptr<Xapian::MatchSpy>> matchspies;
    while (p != p_end) {
	string spytype;
	if (!unpack_string(&p, p_end, spytype)) {
	    throw Xapian::NetworkError("Bad MSG_QUERY");
	}
	const Xapian::MatchSpy * spyclass = reg.get_match_spy(spytype);
	if (spyclass == NULL) {
	    throw Xapian::InvalidArgumentError("Match spy " + spytype +
					       " not registered");
	}

	if (!unpack_string(&p, p_end, serialisation)) {
	    throw Xapian::NetworkError("Bad MSG_QUERY");
	}
	matchspies.push_back(spyclass->unserialise(serialisation,
						   reg)->release());
    }

    Xapian::Weight::Internal local_stats;
    Matcher matcher(*db, full_db_has_positions,
		    query, qlen, &rset, local_stats, *wt,
		    false,
		    collapse_key, collapse_max,
		    percent_threshold, weight_threshold,
		    order, sort_key, sort_by, sort_value_forward, time_limit,
		    matchspies);

    send_message(REPLY_STATS, serialise_stats(local_stats));

    string message;
    get_message(active_timeout, message, MSG_GETMSET);
    p = message.c_str();
    p_end = p + message.size();

    Xapian::termcount first;
    Xapian::termcount maxitems;
    Xapian::termcount check_at_least;
    string sorter_type;
    if (!unpack_uint(&p, p_end, &first) ||
	!unpack_uint(&p, p_end, &maxitems) ||
	!unpack_uint(&p, p_end, &check_at_least) ||
	!unpack_string(&p, p_end, sorter_type)) {
	throw Xapian::NetworkError("Bad MSG_GETMSET");
    }
    unique_ptr<Xapian::KeyMaker> sorter;
    if (!sorter_type.empty()) {
	const Xapian::KeyMaker* sorterclass = reg.get_key_maker(sorter_type);
	if (sorterclass == NULL) {
	    throw Xapian::InvalidArgumentError("KeyMaker " + sorter_type +
					       " not registered");
	}

	string serialised_sorter;
	if (!unpack_string(&p, p_end, serialised_sorter)) {
	    throw Xapian::NetworkError("Bad MSG_GETMSET");
	}
	sorter.reset(sorterclass->unserialise(serialised_sorter, reg));
    }

    message.erase(0, message.size() - (p_end - p));
    unique_ptr<Xapian::Weight::Internal> total_stats(new Xapian::Weight::Internal);
    unserialise_stats(message, *total_stats);
    total_stats->set_bounds_from_db(*db);

    Xapian::MSet mset = matcher.get_mset(first, maxitems, check_at_least,
					 *total_stats, *wt, 0, sorter.get(),
					 collapse_key, collapse_max,
					 percent_threshold, weight_threshold,
					 order,
					 sort_key, sort_by, sort_value_forward,
					 time_limit, matchspies);
    // FIXME: The local side already has these stats, except for the maxpart
    // information.
    mset.internal->set_stats(total_stats.release());

    message.resize(0);
    for (auto i : matchspies) {
	pack_string(message, i->serialise_results());
    }
    message += mset.internal->serialise();
    send_message(REPLY_RESULTS, message);
}

void
RemoteServer::msg_document(const string &message)
{
    const char *p = message.data();
    const char *p_end = p + message.size();
    Xapian::docid did;
    if (!unpack_uint_last(&p, p_end, &did)) {
	throw Xapian::NetworkError("Bad MSG_DOCUMENT");
    }

    Xapian::Document doc = db->get_document(did);

    send_message(REPLY_DOCDATA, doc.get_data());

    Xapian::ValueIterator i;
    for (i = doc.values_begin(); i != doc.values_end(); ++i) {
	string item;
	pack_uint(item, i.get_valueno());
	item += *i;
	send_message(REPLY_VALUE, item);
    }
    send_message(REPLY_DONE, string());
}

void
RemoteServer::msg_keepalive(const string &)
{
    // Ensure *our* database stays alive, as it may contain remote databases!
    db->keep_alive();
    send_message(REPLY_DONE, string());
}

void
RemoteServer::msg_termexists(const string &term)
{
    send_message((db->term_exists(term) ? REPLY_TERMEXISTS : REPLY_TERMDOESNTEXIST), string());
}

void
RemoteServer::msg_collfreq(const string &term)
{
    string reply;
    pack_uint_last(reply, db->get_collection_freq(term));
    send_message(REPLY_COLLFREQ, reply);
}

void
RemoteServer::msg_termfreq(const string &term)
{
    string reply;
    pack_uint_last(reply, db->get_termfreq(term));
    send_message(REPLY_TERMFREQ, reply);
}

void
RemoteServer::msg_freqs(const string &term)
{
    string msg;
    pack_uint(msg, db->get_termfreq(term));
    pack_uint_last(msg, db->get_collection_freq(term));
    send_message(REPLY_FREQS, msg);
}

void
RemoteServer::msg_valuestats(const string & message)
{
    const char *p = message.data();
    const char *p_end = p + message.size();
    Xapian::valueno slot;
    if (!unpack_uint_last(&p, p_end, &slot)) {
	throw Xapian::NetworkError("Bad MSG_VALUESTATS");
    }
    string message_out;
    pack_uint(message_out, db->get_value_freq(slot));
    pack_string(message_out, db->get_value_lower_bound(slot));
    message_out += db->get_value_upper_bound(slot);

    send_message(REPLY_VALUESTATS, message_out);
}

void
RemoteServer::msg_doclength(const string &message)
{
    const char *p = message.data();
    const char *p_end = p + message.size();
    Xapian::docid did;
    if (!unpack_uint_last(&p, p_end, &did)) {
	throw Xapian::NetworkError("Bad MSG_DOCLENGTH");
    }
    string reply;
    pack_uint_last(reply, db->get_doclength(did));
    send_message(REPLY_DOCLENGTH, reply);
}

void
RemoteServer::msg_uniqueterms(const string &message)
{
    const char *p = message.data();
    const char *p_end = p + message.size();
    Xapian::docid did;
    if (!unpack_uint_last(&p, p_end, &did)) {
	throw Xapian::NetworkError("Bad MSG_UNIQUETERMS");
    }
    string reply;
    pack_uint_last(reply, db->get_unique_terms(did));
    send_message(REPLY_UNIQUETERMS, reply);
}

void
RemoteServer::msg_reconstructtext(const string& message)
{
    const char* p = message.data();
    const char* p_end = p + message.size();
    Xapian::docid did;
    size_t length;
    Xapian::termpos start_pos, end_pos;
    if (!unpack_uint(&p, p_end, &did) ||
	!unpack_uint(&p, p_end, &length) ||
	!unpack_uint(&p, p_end, &start_pos) ||
	!unpack_uint(&p, p_end, &end_pos)) {
	throw Xapian::NetworkError("Bad MSG_RECONSTRUCTTEXT");
    }
    send_message(REPLY_RECONSTRUCTTEXT,
		 db->reconstruct_text(did, length, string(p, p_end),
				      start_pos, end_pos));
}

void
RemoteServer::msg_commit(const string &)
{
    if (!wdb)
	throw_read_only();

    wdb->commit();

    send_message(REPLY_DONE, string());
}

void
RemoteServer::msg_cancel(const string &)
{
    if (!wdb)
	throw_read_only();

    // We can't call cancel since that's an internal method, but this
    // has the same effect with minimal additional overhead.
    wdb->begin_transaction(false);
    wdb->cancel_transaction();

    send_message(REPLY_DONE, string());
}

void
RemoteServer::msg_adddocument(const string & message)
{
    if (!wdb)
	throw_read_only();

    Xapian::docid did = wdb->add_document(unserialise_document(message));

    string reply;
    pack_uint_last(reply, did);
    send_message(REPLY_ADDDOCUMENT, reply);
}

void
RemoteServer::msg_deletedocument(const string & message)
{
    if (!wdb)
	throw_read_only();

    const char *p = message.data();
    const char *p_end = p + message.size();
    Xapian::docid did;
    if (!unpack_uint_last(&p, p_end, &did)) {
	throw Xapian::NetworkError("Bad MSG_DELETEDOCUMENT");
    }

    wdb->delete_document(did);

    send_message(REPLY_DONE, string());
}

void
RemoteServer::msg_deletedocumentterm(const string & message)
{
    if (!wdb)
	throw_read_only();

    wdb->delete_document(message);

    send_message(REPLY_DONE, string());
}

void
RemoteServer::msg_replacedocument(const string & message)
{
    if (!wdb)
	throw_read_only();

    const char *p = message.data();
    const char *p_end = p + message.size();
    Xapian::docid did;
    if (!unpack_uint(&p, p_end, &did)) {
	throw Xapian::NetworkError("Bad MSG_REPLACEDOCUMENT");
    }

    wdb->replace_document(did, unserialise_document(string(p, p_end)));

    send_message(REPLY_DONE, string());
}

void
RemoteServer::msg_replacedocumentterm(const string & message)
{
    if (!wdb)
	throw_read_only();

    const char *p = message.data();
    const char *p_end = p + message.size();
    string unique_term;
    if (!unpack_string(&p, p_end, unique_term)) {
	throw Xapian::NetworkError("Bad MSG_REPLACEDOCUMENTTERM");
    }
    Xapian::docid did = wdb->replace_document(unique_term, unserialise_document(string(p, p_end)));

    string reply;
    pack_uint_last(reply, did);
    send_message(REPLY_ADDDOCUMENT, reply);
}

void
RemoteServer::msg_getmetadata(const string & message)
{
    send_message(REPLY_METADATA, db->get_metadata(message));
}

void
RemoteServer::msg_metadatakeylist(const string& message)
{
    string reply;
    string prev = message;
    const string& prefix = message;
    for (Xapian::TermIterator t = db->metadata_keys_begin(prefix);
	 t != db->metadata_keys_end(prefix);
	 ++t) {
	if (rare(prev.size() > 255))
	    prev.resize(255);
	const string& term = *t;
	size_t reuse = common_prefix_length(prev, term);
	reply.append(1, char(reuse));
	pack_uint(reply, term.size() - reuse);
	reply.append(term, reuse, string::npos);
	prev = term;
    }
    send_message(REPLY_METADATAKEYLIST, reply);
}

void
RemoteServer::msg_setmetadata(const string & message)
{
    if (!wdb)
	throw_read_only();
    const char *p = message.data();
    const char *p_end = p + message.size();
    string key;
    if (!unpack_string(&p, p_end, key)) {
	throw Xapian::NetworkError("Bad MSG_SETMETADATA");
    }
    string val(p, p_end - p);
    wdb->set_metadata(key, val);

    send_message(REPLY_DONE, string());
}

void
RemoteServer::msg_addspelling(const string & message)
{
    if (!wdb)
	throw_read_only();
    const char *p = message.data();
    const char *p_end = p + message.size();
    Xapian::termcount freqinc;
    if (!unpack_uint(&p, p_end, &freqinc)) {
	throw Xapian::NetworkError("Bad MSG_ADDSPELLING");
    }
    wdb->add_spelling(string(p, p_end - p), freqinc);

    send_message(REPLY_DONE, string());
}

void
RemoteServer::msg_removespelling(const string & message)
{
    if (!wdb)
	throw_read_only();
    const char *p = message.data();
    const char *p_end = p + message.size();
    Xapian::termcount freqdec;
    if (!unpack_uint(&p, p_end, &freqdec)) {
	throw Xapian::NetworkError("Bad MSG_REMOVESPELLING");
    }
    string reply;
    pack_uint_last(reply, wdb->remove_spelling(string(p, p_end - p), freqdec));
    send_message(REPLY_REMOVESPELLING, reply);
}
