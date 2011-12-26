/** @file remoteserver.cc
 *  @brief Xapian remote backend server base class
 */
/* Copyright (C) 2006,2007,2008,2009,2010,2011 Olly Betts
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

#include "xapian/database.h"
#include "xapian/enquire.h"
#include "xapian/error.h"
#include "xapian/matchspy.h"
#include "xapian/valueiterator.h"

#include "safeerrno.h"
#include <signal.h>
#include <cstdlib>

#include "autoptr.h"
#include "matcher/multimatch.h"
#include "noreturn.h"
#include "omassert.h"
#include "realtime.h"
#include "serialise.h"
#include "serialise-double.h"
#include "str.h"
#include "weight/weightinternal.h"

XAPIAN_NORETURN(static void throw_read_only());
static void
throw_read_only()
{
    throw Xapian::InvalidOperationError("Server is read-only");
}

/// Class to throw when we receive the connection closing message.
struct ConnectionClosed { };

RemoteServer::RemoteServer(const std::vector<std::string> &dbpaths,
			   int fdin_, int fdout_,
			   double active_timeout_, double idle_timeout_,
			   bool writable_)
    : RemoteConnection(fdin_, fdout_, std::string()),
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
	    vector<std::string>::const_iterator i(dbpaths.begin());
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
    unsigned int type = RemoteConnection::get_message(result, end_time);

    // Handle "shutdown connection" message here.
    if (type == MSG_SHUTDOWN) throw ConnectionClosed();
    if (type >= MSG_MAX) {
	string errmsg("Invalid message type ");
	errmsg += str(type);
	throw Xapian::NetworkError(errmsg);
    }
    if (required_type != MSG_MAX && type != unsigned(required_type)) {
	string errmsg("Expecting message type ");
	errmsg += str(int(required_type));
	errmsg += ", got ";
	errmsg += str(int(type));
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
	    /* This list needs to be kept in the same order as the list of
	     * message types in "remoteprotocol.h". Note that messages at the
	     * end of the list in "remoteprotocol.h" can be omitted if they
	     * don't correspond to dispatch actions.
	     */
	    static const dispatch_func dispatch[] = {
		&RemoteServer::msg_allterms,
		&RemoteServer::msg_collfreq,
		&RemoteServer::msg_document,
		&RemoteServer::msg_termexists,
		&RemoteServer::msg_termfreq,
		&RemoteServer::msg_valuestats,
		&RemoteServer::msg_keepalive,
		&RemoteServer::msg_doclength,
		&RemoteServer::msg_query,
		&RemoteServer::msg_termlist,
		&RemoteServer::msg_positionlist,
		&RemoteServer::msg_postlist,
		&RemoteServer::msg_reopen,
		&RemoteServer::msg_update,
		&RemoteServer::msg_adddocument,
		&RemoteServer::msg_cancel,
		&RemoteServer::msg_deletedocumentterm,
		&RemoteServer::msg_commit,
		&RemoteServer::msg_replacedocument,
		&RemoteServer::msg_replacedocumentterm,
		&RemoteServer::msg_deletedocument,
		&RemoteServer::msg_writeaccess,
		&RemoteServer::msg_getmetadata,
		&RemoteServer::msg_setmetadata,
		&RemoteServer::msg_addspelling,
		&RemoteServer::msg_removespelling,
		0, // MSG_GETMSET - used during a conversation.
		0, // MSG_SHUTDOWN - handled by get_message().
		&RemoteServer::msg_openmetadatakeylist,
	    };

	    string message;
	    size_t type = get_message(idle_timeout, message);
	    if (type >= sizeof(dispatch)/sizeof(dispatch[0]) || !dispatch[type]) {
		string errmsg("Unexpected message type ");
		errmsg += str(type);
		throw Xapian::InvalidArgumentError(errmsg);
	    }
	    (this->*(dispatch[type]))(message);
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
    const string & prefix = message;

    const Xapian::TermIterator end = db->allterms_end(prefix);
    for (Xapian::TermIterator t = db->allterms_begin(prefix); t != end; ++t) {
	string item = encode_length(t.get_termfreq());
	item += *t;
	send_message(REPLY_ALLTERMS, item);
    }

    send_message(REPLY_DONE, string());
}

void
RemoteServer::msg_termlist(const string &message)
{
    const char *p = message.data();
    const char *p_end = p + message.size();
    Xapian::docid did = decode_length(&p, p_end, false);

    send_message(REPLY_DOCLENGTH, encode_length(db->get_doclength(did)));
    const Xapian::TermIterator end = db->termlist_end(did);
    for (Xapian::TermIterator t = db->termlist_begin(did); t != end; ++t) {
	string item = encode_length(t.get_wdf());
	item += encode_length(t.get_termfreq());
	item += *t;
	send_message(REPLY_TERMLIST, item);
    }

    send_message(REPLY_DONE, string());
}

void
RemoteServer::msg_positionlist(const string &message)
{
    const char *p = message.data();
    const char *p_end = p + message.size();
    Xapian::docid did = decode_length(&p, p_end, false);
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

    wdb = new Xapian::WritableDatabase(context, Xapian::DB_OPEN);
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
    // FIXME: clumsy to reverse calculate total_len like this:
    totlen_t total_len = totlen_t(db->get_avlength() * db->get_doccount() + .5);
    message += encode_length(total_len);
    //message += encode_length(db->get_total_length());
    string uuid = db->get_uuid();
    message += uuid;
    send_message(REPLY_UPDATE, message);
}

/** Structure holding a list of match spies.
 *
 *  The main reason for the existence of this structure is to make it easy to
 *  ensure that the match spies are all deleted after use.
 */
struct MatchSpyList {
    vector<Xapian::MatchSpy *> spies;

    ~MatchSpyList() {
	vector<Xapian::MatchSpy *>::const_iterator i;
	for (i = spies.begin(); i != spies.end(); ++i) {
	    delete *i;
	}
    }
};

void
RemoteServer::msg_query(const string &message_in)
{
    const char *p = message_in.c_str();
    const char *p_end = p + message_in.size();
    size_t len;

    // Unserialise the Query.
    len = decode_length(&p, p_end, true);
    AutoPtr<Xapian::Query::Internal> query(Xapian::Query::Internal::unserialise(string(p, len), reg));
    p += len;

    // Unserialise assorted Enquire settings.
    Xapian::termcount qlen = decode_length(&p, p_end, false);

    Xapian::valueno collapse_max = decode_length(&p, p_end, false);

    Xapian::valueno collapse_key = Xapian::BAD_VALUENO;
    if (collapse_max) collapse_key = decode_length(&p, p_end, false);

    if (p_end - p < 4 || *p < '0' || *p > '2') {
	throw Xapian::NetworkError("bad message (docid_order)");
    }
    Xapian::Enquire::docid_order order;
    order = static_cast<Xapian::Enquire::docid_order>(*p++ - '0');

    Xapian::valueno sort_key = decode_length(&p, p_end, false);

    if (*p < '0' || *p > '3') {
	throw Xapian::NetworkError("bad message (sort_by)");
    }
    Xapian::Enquire::Internal::sort_setting sort_by;
    sort_by = static_cast<Xapian::Enquire::Internal::sort_setting>(*p++ - '0');

    if (*p < '0' || *p > '1') {
	throw Xapian::NetworkError("bad message (sort_value_forward)");
    }
    bool sort_value_forward(*p++ != '0');

    int percent_cutoff = *p++;
    if (percent_cutoff < 0 || percent_cutoff > 100) {
	throw Xapian::NetworkError("bad message (percent_cutoff)");
    }

    double weight_cutoff = unserialise_double(&p, p_end);
    if (weight_cutoff < 0) {
	throw Xapian::NetworkError("bad message (weight_cutoff)");
    }

    // Unserialise the Weight object.
    len = decode_length(&p, p_end, true);
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

    len = decode_length(&p, p_end, true);
    AutoPtr<Xapian::Weight> wt(wttype->unserialise(string(p, len)));
    p += len;

    // Unserialise the RSet object.
    len = decode_length(&p, p_end, true);
    Xapian::RSet rset = unserialise_rset(string(p, len));
    p += len;

    // Unserialise any MatchSpy objects.
    MatchSpyList matchspies;
    while (p != p_end) {
	len = decode_length(&p, p_end, true);
	string spytype(p, len);
	const Xapian::MatchSpy * spyclass = reg.get_match_spy(spytype);
	if (spyclass == NULL) {
	    throw Xapian::InvalidArgumentError("Match spy " + spytype +
					       " not registered");
	}
	p += len;

	len = decode_length(&p, p_end, true);
	matchspies.spies.push_back(spyclass->unserialise(string(p, len), reg));
	p += len;
    }

    Xapian::Weight::Internal local_stats;
    MultiMatch match(*db, query.get(), qlen, &rset, collapse_max, collapse_key,
		     percent_cutoff, weight_cutoff, order,
		     sort_key, sort_by, sort_value_forward, NULL,
		     local_stats, wt.get(), matchspies.spies, false, false);

    send_message(REPLY_STATS, serialise_stats(local_stats));

    string message;
    get_message(active_timeout, message, MSG_GETMSET);
    p = message.c_str();
    p_end = p + message.size();

    Xapian::termcount first = decode_length(&p, p_end, false);
    Xapian::termcount maxitems = decode_length(&p, p_end, false);

    Xapian::termcount check_at_least = 0;
    check_at_least = decode_length(&p, p_end, false);

    message.erase(0, message.size() - (p_end - p));
    Xapian::Weight::Internal total_stats(unserialise_stats(message));
    total_stats.set_bounds_from_db(*db);

    Xapian::MSet mset;
    match.get_mset(first, maxitems, check_at_least, mset, total_stats, 0, 0);

    message.resize(0);
    vector<Xapian::MatchSpy *>::const_iterator i;
    for (i = matchspies.spies.begin(); i != matchspies.spies.end(); ++i) {
	string spy_results = (*i)->serialise_results();
	message += encode_length(spy_results.size());
	message += spy_results;
    }
    message += serialise_mset(mset);
    send_message(REPLY_RESULTS, message);
}

void
RemoteServer::msg_document(const string &message)
{
    const char *p = message.data();
    const char *p_end = p + message.size();
    Xapian::docid did = decode_length(&p, p_end, false);

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
RemoteServer::msg_valuestats(const string & message)
{
    const char *p = message.data();
    const char *p_end = p + message.size();
    while (p != p_end) {
	Xapian::valueno slot = decode_length(&p, p_end, false);
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
    Xapian::docid did = decode_length(&p, p_end, false);
    send_message(REPLY_DOCLENGTH, encode_length(db->get_doclength(did)));
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
    Xapian::docid did = decode_length(&p, p_end, false);

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
    Xapian::docid did = decode_length(&p, p_end, false);

    wdb->replace_document(did, unserialise_document(string(p, p_end)));
}

void
RemoteServer::msg_replacedocumentterm(const string & message)
{
    if (!wdb)
	throw_read_only();

    const char *p = message.data();
    const char *p_end = p + message.size();
    size_t len = decode_length(&p, p_end, true);
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
RemoteServer::msg_openmetadatakeylist(const string & message)
{
    const Xapian::TermIterator end = db->metadata_keys_end(message);
    Xapian::TermIterator t = db->metadata_keys_begin(message);
    for (; t != end; ++t) {
	send_message(REPLY_METADATAKEYLIST, *t);
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
    size_t keylen = decode_length(&p, p_end, false);
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
    Xapian::termcount freqinc = decode_length(&p, p_end, false);
    wdb->add_spelling(string(p, p_end - p), freqinc);
}

void
RemoteServer::msg_removespelling(const string & message)
{
    if (!wdb)
	throw_read_only();
    const char *p = message.data();
    const char *p_end = p + message.size();
    Xapian::termcount freqdec = decode_length(&p, p_end, false);
    wdb->remove_spelling(string(p, p_end - p), freqdec);
}
