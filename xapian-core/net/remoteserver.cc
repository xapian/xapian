/** @file remoteserver.cc
 *  @brief Xapian remote backend server base class
 */
/* Copyright (C) 2006,2007,2008,2009,2010,2011,2012,2013,2014,2015,2016,2017 Olly Betts
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

#include "safeerrno.h"
#include <signal.h>
#include <cstdlib>
#include <memory>

#include "api/msetinternal.h"
#include "length.h"
#include "matcher/matcher.h"
#include "omassert.h"
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
RemoteServer::msg_allterms(const string &message)
{
    string prev = message;
    string reply;

    const string & prefix = message;
    const Xapian::TermIterator end = db->allterms_end(prefix);
    for (Xapian::TermIterator t = db->allterms_begin(prefix); t != end; ++t) {
	if (rare(prev.size() > 255))
	    prev.resize(255);
	const string & v = *t;
	size_t reuse = common_prefix_length(prev, v);
	reply = encode_length(t.get_termfreq());
	reply.append(1, char(reuse));
	reply.append(v, reuse, string::npos);
	send_message(REPLY_ALLTERMS, reply);
	prev = v;
    }

    send_message(REPLY_DONE, string());
}

void
RemoteServer::msg_termlist(const string &message)
{
    const char *p = message.data();
    const char *p_end = p + message.size();
    Xapian::docid did;
    decode_length(&p, p_end, did);

    send_message(REPLY_DOCLENGTH, encode_length(db->get_doclength(did)));
    string prev;
    const Xapian::TermIterator end = db->termlist_end(did);
    for (Xapian::TermIterator t = db->termlist_begin(did); t != end; ++t) {
	if (rare(prev.size() > 255))
	    prev.resize(255);
	const string & v = *t;
	size_t reuse = common_prefix_length(prev, v);
	string reply = encode_length(t.get_wdf());
	reply += encode_length(t.get_termfreq());
	reply.append(1, char(reuse));
	reply.append(v, reuse, string::npos);
	send_message(REPLY_TERMLIST, reply);
	prev = v;
    }

    send_message(REPLY_DONE, string());
}

void
RemoteServer::msg_positionlist(const string &message)
{
    const char *p = message.data();
    const char *p_end = p + message.size();
    Xapian::docid did;
    decode_length(&p, p_end, did);
    string term(p, p_end - p);

    Xapian::termpos lastpos = static_cast<Xapian::termpos>(-1);
    const Xapian::PositionIterator end = db->positionlist_end(did, term);
    for (Xapian::PositionIterator i = db->positionlist_begin(did, term);
	 i != end; ++i) {
	Xapian::termpos pos = *i;
	send_message(REPLY_POSITIONLIST, encode_length(pos - lastpos - 1));
	lastpos = pos;
    }

    send_message(REPLY_DONE, string());
}

void
RemoteServer::msg_positionlistcount(const string &message)
{
    const char *p = message.data();
    const char *p_end = p + message.size();
    Xapian::docid did;
    decode_length(&p, p_end, did);

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
    send_message(REPLY_POSITIONLISTCOUNT, encode_length(result));
}

void
RemoteServer::msg_postlist(const string &message)
{
    const string & term = message;

    Xapian::doccount termfreq = db->get_termfreq(term);
    Xapian::termcount collfreq = db->get_collection_freq(term);
    send_message(REPLY_POSTLISTSTART, encode_length(termfreq) + encode_length(collfreq));

    Xapian::docid lastdocid = 0;
    const Xapian::PostingIterator end = db->postlist_end(term);
    for (Xapian::PostingIterator i = db->postlist_begin(term);
	 i != end; ++i) {

	Xapian::docid newdocid = *i;
	string reply = encode_length(newdocid - lastdocid - 1);
	reply += encode_length(i.get_wdf());

	send_message(REPLY_POSTLISTITEM, reply);
	lastdocid = newdocid;
    }

    send_message(REPLY_DONE, string());
}

void
RemoteServer::msg_writeaccess(const string & msg)
{
    if (!writable)
	throw_read_only();

    int flags = Xapian::DB_OPEN;
    const char *p = msg.c_str();
    const char *p_end = p + msg.size();
    if (p != p_end) {
	unsigned flag_bits;
	decode_length(&p, p_end, flag_bits);
	flags |= flag_bits &~ Xapian::DB_ACTION_MASK_;
	if (p != p_end) {
	    throw Xapian::NetworkError("Junk at end of MSG_WRITEACCESS");
	}
    }

    wdb = new Xapian::WritableDatabase(context, flags);
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
    message += encode_length(num_docs);
    message += encode_length(db->get_lastdocid() - num_docs);
    Xapian::termcount doclen_lb = db->get_doclength_lower_bound();
    message += encode_length(doclen_lb);
    message += encode_length(db->get_doclength_upper_bound() - doclen_lb);
    message += (db->has_positions() ? '1' : '0');
    message += encode_length(db->get_total_length());
    string uuid = db->get_uuid();
    message += uuid;
    send_message(REPLY_UPDATE, message);
}

void
RemoteServer::msg_query(const string &message_in)
{
    const char *p = message_in.c_str();
    const char *p_end = p + message_in.size();

    // Unserialise the Query.
    size_t len;
    decode_length_and_check(&p, p_end, len);
    Xapian::Query query(Xapian::Query::unserialise(string(p, len), reg));
    p += len;

    // Unserialise assorted Enquire settings.
    Xapian::termcount qlen;
    decode_length(&p, p_end, qlen);

    Xapian::valueno collapse_max;
    decode_length(&p, p_end, collapse_max);

    Xapian::valueno collapse_key = Xapian::BAD_VALUENO;
    if (collapse_max)
	decode_length(&p, p_end, collapse_key);

    if (p_end - p < 4 || *p < '0' || *p > '2') {
	throw Xapian::NetworkError("bad message (docid_order)");
    }
    Xapian::Enquire::docid_order order;
    order = static_cast<Xapian::Enquire::docid_order>(*p++ - '0');

    Xapian::valueno sort_key;
    decode_length(&p, p_end, sort_key);

    if (*p < '0' || *p > '3') {
	throw Xapian::NetworkError("bad message (sort_by)");
    }
    Xapian::Enquire::Internal::sort_setting sort_by;
    sort_by = static_cast<Xapian::Enquire::Internal::sort_setting>(*p++ - '0');

    if (*p < '0' || *p > '1') {
	throw Xapian::NetworkError("bad message (sort_value_forward)");
    }
    bool sort_value_forward(*p++ != '0');

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
    decode_length_and_check(&p, p_end, len);
    string wtname(p, len);
    p += len;

    const Xapian::Weight * wttype = reg.get_weighting_scheme(wtname);
    if (wttype == NULL) {
	// Note: user weighting schemes should be registered by adding them to
	// a Registry, and setting the context using
	// RemoteServer::set_registry().
	throw Xapian::InvalidArgumentError("Weighting scheme " +
					   wtname + " not registered");
    }

    decode_length_and_check(&p, p_end, len);
    unique_ptr<Xapian::Weight> wt(wttype->unserialise(string(p, len)));
    p += len;

    // Unserialise the RSet object.
    decode_length_and_check(&p, p_end, len);
    Xapian::RSet rset = unserialise_rset(string(p, len));
    p += len;

    // Unserialise any MatchSpy objects.
    vector<Xapian::Internal::opt_intrusive_ptr<Xapian::MatchSpy>> matchspies;
    while (p != p_end) {
	decode_length_and_check(&p, p_end, len);
	string spytype(p, len);
	const Xapian::MatchSpy * spyclass = reg.get_match_spy(spytype);
	if (spyclass == NULL) {
	    throw Xapian::InvalidArgumentError("Match spy " + spytype +
					       " not registered");
	}
	p += len;

	decode_length_and_check(&p, p_end, len);
	matchspies.push_back(spyclass->unserialise(string(p, len), reg)->release());
	p += len;
    }

    Xapian::Weight::Internal local_stats;
    Matcher matcher(*db, query, qlen, &rset, local_stats, wt.get(),
		    false, false,
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
    decode_length(&p, p_end, first);
    Xapian::termcount maxitems;
    decode_length(&p, p_end, maxitems);

    Xapian::termcount check_at_least;
    decode_length(&p, p_end, check_at_least);

    message.erase(0, message.size() - (p_end - p));
    unique_ptr<Xapian::Weight::Internal> total_stats(new Xapian::Weight::Internal);
    unserialise_stats(message, *total_stats);
    total_stats->set_bounds_from_db(*db);

    Xapian::MSet mset = matcher.get_mset(first, maxitems, check_at_least,
					 *total_stats, 0, 0,
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
	string spy_results = i->serialise_results();
	message += encode_length(spy_results.size());
	message += spy_results;
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
    decode_length(&p, p_end, did);

    Xapian::Document doc = db->get_document(did);

    send_message(REPLY_DOCDATA, doc.get_data());

    Xapian::ValueIterator i;
    for (i = doc.values_begin(); i != doc.values_end(); ++i) {
	string item = encode_length(i.get_valueno());
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
    send_message(REPLY_COLLFREQ, encode_length(db->get_collection_freq(term)));
}

void
RemoteServer::msg_termfreq(const string &term)
{
    send_message(REPLY_TERMFREQ, encode_length(db->get_termfreq(term)));
}

void
RemoteServer::msg_freqs(const string &term)
{
    string msg = encode_length(db->get_termfreq(term));
    msg += encode_length(db->get_collection_freq(term));
    send_message(REPLY_FREQS, msg);
}

void
RemoteServer::msg_valuestats(const string & message)
{
    const char *p = message.data();
    const char *p_end = p + message.size();
    while (p != p_end) {
	Xapian::valueno slot;
	decode_length(&p, p_end, slot);
	string message_out;
	message_out += encode_length(db->get_value_freq(slot));
	string bound = db->get_value_lower_bound(slot);
	message_out += encode_length(bound.size());
	message_out += bound;
	bound = db->get_value_upper_bound(slot);
	message_out += encode_length(bound.size());
	message_out += bound;

	send_message(REPLY_VALUESTATS, message_out);
    }
}

void
RemoteServer::msg_doclength(const string &message)
{
    const char *p = message.data();
    const char *p_end = p + message.size();
    Xapian::docid did;
    decode_length(&p, p_end, did);
    send_message(REPLY_DOCLENGTH, encode_length(db->get_doclength(did)));
}

void
RemoteServer::msg_uniqueterms(const string &message)
{
    const char *p = message.data();
    const char *p_end = p + message.size();
    Xapian::docid did;
    decode_length(&p, p_end, did);
    send_message(REPLY_UNIQUETERMS, encode_length(db->get_unique_terms(did)));
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
}

void
RemoteServer::msg_adddocument(const string & message)
{
    if (!wdb)
	throw_read_only();

    Xapian::docid did = wdb->add_document(unserialise_document(message));

    send_message(REPLY_ADDDOCUMENT, encode_length(did));
}

void
RemoteServer::msg_deletedocument(const string & message)
{
    if (!wdb)
	throw_read_only();

    const char *p = message.data();
    const char *p_end = p + message.size();
    Xapian::docid did;
    decode_length(&p, p_end, did);

    wdb->delete_document(did);

    send_message(REPLY_DONE, string());
}

void
RemoteServer::msg_deletedocumentterm(const string & message)
{
    if (!wdb)
	throw_read_only();

    wdb->delete_document(message);
}

void
RemoteServer::msg_replacedocument(const string & message)
{
    if (!wdb)
	throw_read_only();

    const char *p = message.data();
    const char *p_end = p + message.size();
    Xapian::docid did;
    decode_length(&p, p_end, did);

    wdb->replace_document(did, unserialise_document(string(p, p_end)));
}

void
RemoteServer::msg_replacedocumentterm(const string & message)
{
    if (!wdb)
	throw_read_only();

    const char *p = message.data();
    const char *p_end = p + message.size();
    size_t len;
    decode_length_and_check(&p, p_end, len);
    string unique_term(p, len);
    p += len;

    Xapian::docid did = wdb->replace_document(unique_term, unserialise_document(string(p, p_end)));

    send_message(REPLY_ADDDOCUMENT, encode_length(did));
}

void
RemoteServer::msg_getmetadata(const string & message)
{
    send_message(REPLY_METADATA, db->get_metadata(message));
}

void
RemoteServer::msg_metadatakeylist(const string & message)
{
    string prev = message;
    string reply;

    const string & prefix = message;
    const Xapian::TermIterator end = db->metadata_keys_end(prefix);
    Xapian::TermIterator t = db->metadata_keys_begin(prefix);
    for (; t != end; ++t) {
	if (rare(prev.size() > 255))
	    prev.resize(255);
	const string & v = *t;
	size_t reuse = common_prefix_length(prev, v);
	reply.assign(1, char(reuse));
	reply.append(v, reuse, string::npos);
	send_message(REPLY_METADATAKEYLIST, reply);
	prev = v;
    }
    send_message(REPLY_DONE, string());
}

void
RemoteServer::msg_setmetadata(const string & message)
{
    if (!wdb)
	throw_read_only();
    const char *p = message.data();
    const char *p_end = p + message.size();
    size_t keylen;
    decode_length_and_check(&p, p_end, keylen);
    string key(p, keylen);
    p += keylen;
    string val(p, p_end - p);
    wdb->set_metadata(key, val);
}

void
RemoteServer::msg_addspelling(const string & message)
{
    if (!wdb)
	throw_read_only();
    const char *p = message.data();
    const char *p_end = p + message.size();
    Xapian::termcount freqinc;
    decode_length(&p, p_end, freqinc);
    wdb->add_spelling(string(p, p_end - p), freqinc);
}

void
RemoteServer::msg_removespelling(const string & message)
{
    if (!wdb)
	throw_read_only();
    const char *p = message.data();
    const char *p_end = p + message.size();
    Xapian::termcount freqdec;
    decode_length(&p, p_end, freqdec);
    auto result = wdb->remove_spelling(string(p, p_end - p), freqdec);
    send_message(REPLY_REMOVESPELLING, encode_length(result));
}
