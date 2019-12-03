/** @file remote-database.cc
 *  @brief Remote backend database class
 */
/* Copyright (C) 2006,2007,2008,2009,2010,2011,2012,2013,2014,2015,2017,2018,2019 Olly Betts
 * Copyright (C) 2007,2009,2010 Lemur Consulting Ltd
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#include <config.h>

#include "remote-database.h"

#include <signal.h>

#include "api/msetinternal.h"
#include "api/smallvector.h"
#include "backends/inmemory/inmemory_positionlist.h"
#include "net_postlist.h"
#include "remote-document.h"
#include "omassert.h"
#include "realtime.h"
#include "net/serialise.h"
#include "net/serialise-error.h"
#include "pack.h"
#include "remote_alltermslist.h"
#include "remote_metadatatermlist.h"
#include "remote_termlist.h"
#include "serialise-double.h"
#include "str.h"
#include "stringutils.h" // For STRINGIZE().
#include "weight/weightinternal.h"

#include <cerrno>
#include <memory>
#include <string>
#include <vector>

#include "xapian/constants.h"
#include "xapian/error.h"
#include "xapian/matchspy.h"

using namespace std;
using Xapian::Internal::intrusive_ptr;

/// Return true if further replies should be expected.
static inline bool
is_intermediate_reply(int reply_code)
{
    return reply_code == REPLY_DOCDATA ||
	   reply_code == REPLY_VALUE ||
	   reply_code == REPLY_TERMLISTHEADER ||
	   reply_code == REPLY_POSTLISTHEADER;
}

[[noreturn]]
static void
throw_invalid_operation(const char* message)
{
    throw Xapian::InvalidOperationError(message);
}

[[noreturn]]
static void
throw_handshake_failed(const string & context)
{
    throw Xapian::NetworkError("Handshake failed - is this a Xapian server?",
			       context);
}

[[noreturn]]
static void
throw_connection_closed_unexpectedly()
{
    throw Xapian::NetworkError("Connection closed unexpectedly");
}

RemoteDatabase::RemoteDatabase(int fd, double timeout_,
			       const string & context_, bool writable,
			       int flags)
    : Xapian::Database::Internal(writable ?
				 TRANSACTION_NONE :
				 TRANSACTION_READONLY),
      link(fd, fd, context_),
      context(context_),
      cached_stats_valid(),
      mru_valstats(),
      mru_slot(Xapian::BAD_VALUENO),
      timeout(timeout_)
{
#ifndef __WIN32__
    // It's simplest to just ignore SIGPIPE.  We'll still know if the
    // connection dies because we'll get EPIPE back from write().
    if (signal(SIGPIPE, SIG_IGN) == SIG_ERR) {
	throw Xapian::NetworkError("Couldn't set SIGPIPE to SIG_IGN", errno);
    }
#endif

    update_stats(MSG_MAX);

    if (writable) {
	if (flags & Xapian::DB_RETRY_LOCK) {
	    string message;
	    pack_uint_last(message, unsigned(flags & Xapian::DB_RETRY_LOCK));
	    update_stats(MSG_WRITEACCESS, message);
	} else {
	    update_stats(MSG_WRITEACCESS);
	}
    }
}

Xapian::termcount
RemoteDatabase::positionlist_count(Xapian::docid did,
				   const std::string& term) const
{
    if (cached_stats_valid && !has_positional_info)
	return 0;

    string message;
    pack_uint(message, did);
    message += term;
    send_message(MSG_POSITIONLISTCOUNT, message);

    get_message(message, REPLY_POSITIONLISTCOUNT);
    const char * p = message.data();
    const char * p_end = p + message.size();
    Xapian::termcount count;
    if (!unpack_uint_last(&p, p_end, &count)) {
	throw Xapian::NetworkError("Bad REPLY_POSITIONLISTCOUNT", context);
    }
    return count;
}

void
RemoteDatabase::keep_alive()
{
    send_message(MSG_KEEPALIVE, string());
    string message;
    get_message(message, REPLY_DONE);
}

TermList*
RemoteDatabase::open_metadata_keylist(const std::string& prefix) const
{
    send_message(MSG_METADATAKEYLIST, prefix);
    string message;
    get_message(message, REPLY_METADATAKEYLIST);
    return new RemoteMetadataTermList(prefix, std::move(message));
}

TermList *
RemoteDatabase::open_term_list(Xapian::docid did) const
{
    Assert(did);

    // Ensure that total_length and doccount are up-to-date.
    if (!cached_stats_valid) update_stats();

    string message;
    pack_uint_last(message, did);
    send_message(MSG_TERMLIST, message);

    get_message(message, REPLY_TERMLISTHEADER);
    const char * p = message.c_str();
    const char * p_end = p + message.size();
    Xapian::termcount doclen;
    Xapian::termcount num_entries;
    if (!unpack_uint(&p, p_end, &doclen) ||
	!unpack_uint_last(&p, p_end, &num_entries)) {
	throw Xapian::NetworkError("Bad REPLY_TERMLISTHEADER", context);
    }
    get_message(message, REPLY_TERMLIST);
    return new RemoteTermList(num_entries, doclen, doccount, this, did,
			      std::move(message));
}

TermList *
RemoteDatabase::open_term_list_direct(Xapian::docid did) const
{
    return RemoteDatabase::open_term_list(did);
}

TermList*
RemoteDatabase::open_allterms(const string& prefix) const
{
    send_message(MSG_ALLTERMS, prefix);
    string message;
    get_message(message, REPLY_ALLTERMS);
    return new RemoteAllTermsList(prefix, std::move(message));
}

PostList *
RemoteDatabase::open_post_list(const string& term) const
{
    return RemoteDatabase::open_leaf_post_list(term, false);
}

LeafPostList *
RemoteDatabase::open_leaf_post_list(const string& term, bool) const
{
    send_message(MSG_POSTLIST, term);

    string message;
    get_message(message, REPLY_POSTLISTHEADER);

    const char * p = message.data();
    const char * p_end = p + message.size();
    Xapian::doccount termfreq;
    if (!unpack_uint_last(&p, p_end, &termfreq)) {
	unpack_throw_serialisation_error(p);
    }

    get_message(message, REPLY_POSTLIST);

    return new NetworkPostList(intrusive_ptr<const RemoteDatabase>(this),
			       term,
			       termfreq,
			       std::move(message));
}

PositionList *
RemoteDatabase::open_position_list(Xapian::docid did, const string &term) const
{
    string message;
    pack_uint(message, did);
    message += term;
    send_message(MSG_POSITIONLIST, message);

    Xapian::VecCOW<Xapian::termpos> positions;

    get_message(message, REPLY_POSITIONLIST);
    Xapian::termpos lastpos = static_cast<Xapian::termpos>(-1);
    const char* p = message.data();
    const char* p_end = p + message.size();
    while (p != p_end) {
	Xapian::termpos inc;
	if (!unpack_uint(&p, p_end, &inc)) {
	    unpack_throw_serialisation_error(p);
	}
	lastpos += inc + 1;
	positions.push_back(lastpos);
    }

    return new InMemoryPositionList(std::move(positions));
}

bool
RemoteDatabase::has_positions() const
{
    if (!cached_stats_valid) update_stats();
    return has_positional_info;
}

bool
RemoteDatabase::reopen()
{
    mru_slot = Xapian::BAD_VALUENO;
    return update_stats(MSG_REOPEN);
}

void
RemoteDatabase::close()
{
    do_close();
}

// Currently lazy is used:
//
// * To implement API flag Xapian::DOC_ASSUME_VALID which can be specified when
//   calling method Database::get_document()
//
// * To read values for backends without streamed values in SlowValueList
//
// * If you call get_data(), values_begin() or values_count() on a Document
//   object passed to a KeyMaker, MatchDecider, MatchSpy during the match
//
// The first is relevant to the remote backend, but doesn't happen during
// the match.
//
// SlowValueList is used with the remote backend, but not to read values
// during the match.
//
// KeyMaker and MatchSpy happens on the server with the remote backend, so
// they aren't relevant here.
//
// So the cases which are relevant to the remote backend don't matter during
// the match, and so we can ignore the lazy flag here without affecting matcher
// performance.
Xapian::Document::Internal *
RemoteDatabase::open_document(Xapian::docid did, bool /*lazy*/) const
{
    Assert(did);

    string message;
    pack_uint_last(message, did);
    send_message(MSG_DOCUMENT, message);

    string doc_data;
    get_message(doc_data, REPLY_DOCDATA);

    map<Xapian::valueno, string> values;
    while (get_message_or_done(message, REPLY_VALUE)) {
	const char * p = message.data();
	const char * p_end = p + message.size();
	Xapian::valueno slot;
	if (!unpack_uint(&p, p_end, &slot)) {
	    unpack_throw_serialisation_error(p);
	}
	values.insert(make_pair(slot, string(p, p_end)));
    }

    return new RemoteDocument(this, did, std::move(doc_data),
			      std::move(values));
}

bool
RemoteDatabase::update_stats(message_type msg_code, const string & body) const
{
    // MSG_MAX signals that we're handling the opening greeting, which isn't in
    // response to an explicit message.
    if (msg_code != MSG_MAX)
	send_message(msg_code, body);

    string message;
    if (!get_message_or_done(message, REPLY_UPDATE)) {
	// The database was already open at the latest revision.
	return false;
    }

    if (message.size() < 3) {
	throw_handshake_failed(context);
    }
    const char *p = message.c_str();
    const char *p_end = p + message.size();

    // The protocol major versions must match.  The protocol minor version of
    // the server must be >= that of the client.
    int protocol_major = static_cast<unsigned char>(*p++);
    int protocol_minor = static_cast<unsigned char>(*p++);
    if (protocol_major != XAPIAN_REMOTE_PROTOCOL_MAJOR_VERSION ||
	protocol_minor < XAPIAN_REMOTE_PROTOCOL_MINOR_VERSION) {
	string errmsg("Server supports protocol version");
	if (protocol_minor) {
	    errmsg += "s ";
	    errmsg += str(protocol_major);
	    errmsg += ".0 to ";
	} else {
	    errmsg += ' ';
	}
	errmsg += str(protocol_major);
	errmsg += '.';
	errmsg += str(protocol_minor);
	errmsg +=
	    " - client is using "
	    STRINGIZE(XAPIAN_REMOTE_PROTOCOL_MAJOR_VERSION)
	    "."
	    STRINGIZE(XAPIAN_REMOTE_PROTOCOL_MINOR_VERSION);
	throw Xapian::NetworkError(errmsg, context);
    }

    if (!unpack_uint(&p, p_end, &doccount) ||
	!unpack_uint(&p, p_end, &lastdocid) ||
	!unpack_uint(&p, p_end, &doclen_lbound) ||
	!unpack_uint(&p, p_end, &doclen_ubound) ||
	!unpack_bool(&p, p_end, &has_positional_info) ||
	!unpack_uint(&p, p_end, &total_length)) {
	throw Xapian::NetworkError("Bad stats update message received", context);
    }
    lastdocid += doccount;
    doclen_ubound += doclen_lbound;
    uuid.assign(p, p_end);
    cached_stats_valid = true;
    return true;
}

Xapian::doccount
RemoteDatabase::get_doccount() const
{
    if (!cached_stats_valid) update_stats();
    return doccount;
}

Xapian::docid
RemoteDatabase::get_lastdocid() const
{
    if (!cached_stats_valid) update_stats();
    return lastdocid;
}

Xapian::totallength
RemoteDatabase::get_total_length() const
{
    if (!cached_stats_valid) update_stats();
    return total_length;
}

bool
RemoteDatabase::term_exists(const string & tname) const
{
    if (tname.empty()) {
	return get_doccount() != 0;
    }
    send_message(MSG_TERMEXISTS, tname);
    string message;
    reply_type type = get_message(message,
				  REPLY_TERMEXISTS,
				  REPLY_TERMDOESNTEXIST);
    return (type == REPLY_TERMEXISTS);
}

void
RemoteDatabase::get_freqs(const string & term,
			  Xapian::doccount * termfreq_ptr,
			  Xapian::termcount * collfreq_ptr) const
{
    Assert(!term.empty());
    string message;
    if (termfreq_ptr && collfreq_ptr) {
	send_message(MSG_FREQS, term);
	get_message(message, REPLY_FREQS);
	const char* p = message.data();
	const char* p_end = p + message.size();
	if (unpack_uint(&p, p_end, termfreq_ptr) &&
	    unpack_uint_last(&p, p_end, collfreq_ptr)) {
	    return;
	}
    } else if (termfreq_ptr) {
	send_message(MSG_TERMFREQ, term);
	get_message(message, REPLY_TERMFREQ);
	const char* p = message.data();
	const char* p_end = p + message.size();
	if (unpack_uint_last(&p, p_end, termfreq_ptr)) {
	    return;
	}
    } else if (collfreq_ptr) {
	send_message(MSG_COLLFREQ, term);
	get_message(message, REPLY_COLLFREQ);
	const char* p = message.data();
	const char* p_end = p + message.size();
	if (unpack_uint_last(&p, p_end, collfreq_ptr)) {
	    return;
	}
    } else {
	Assert(false);
	return;
    }
    throw Xapian::NetworkError("Bad REPLY_FREQS/REPLY_TERMFREQ/REPLY_COLLFREQ",
			       context);
}

void
RemoteDatabase::read_value_stats(Xapian::valueno slot) const
{
    if (mru_slot == slot)
	return;

    string message;
    pack_uint_last(message, slot);
    send_message(MSG_VALUESTATS, message);

    get_message(message, REPLY_VALUESTATS);
    const char* p = message.data();
    const char* p_end = p + message.size();
    mru_slot = slot;
    if (!unpack_uint(&p, p_end, &mru_valstats.freq) ||
	!unpack_string(&p, p_end, mru_valstats.lower_bound)) {
	throw Xapian::NetworkError("Bad REPLY_VALUESTATS", context);
    }
    mru_valstats.upper_bound.assign(p, p_end);
}

Xapian::doccount
RemoteDatabase::get_value_freq(Xapian::valueno slot) const
{
    read_value_stats(slot);
    return mru_valstats.freq;
}

std::string
RemoteDatabase::get_value_lower_bound(Xapian::valueno slot) const
{
    read_value_stats(slot);
    return mru_valstats.lower_bound;
}

std::string
RemoteDatabase::get_value_upper_bound(Xapian::valueno slot) const
{
    read_value_stats(slot);
    return mru_valstats.upper_bound;
}

Xapian::termcount
RemoteDatabase::get_doclength_lower_bound() const
{
    return doclen_lbound;
}

Xapian::termcount
RemoteDatabase::get_doclength_upper_bound() const
{
    return doclen_ubound;
}

Xapian::termcount
RemoteDatabase::get_wdf_upper_bound(const string &) const
{
    // The default implementation returns get_collection_freq(), but we
    // don't want the overhead of a remote message and reply per query
    // term, and we can get called in the middle of a remote exchange
    // too.  FIXME: handle this bound in the stats local/remote code...
    return doclen_ubound;
}

Xapian::termcount
RemoteDatabase::get_doclength(Xapian::docid did) const
{
    Assert(did != 0);
    string message;
    pack_uint_last(message, did);
    send_message(MSG_DOCLENGTH, message);

    get_message(message, REPLY_DOCLENGTH);
    const char* p = message.c_str();
    const char* p_end = p + message.size();
    Xapian::termcount doclen;
    if (!unpack_uint_last(&p, p_end, &doclen)) {
	throw Xapian::NetworkError("Bad REPLY_DOCLENGTH", context);
    }
    return doclen;
}

Xapian::termcount
RemoteDatabase::get_unique_terms(Xapian::docid did) const
{
    Assert(did != 0);
    string message;
    pack_uint_last(message, did);
    send_message(MSG_UNIQUETERMS, message);

    get_message(message, REPLY_UNIQUETERMS);
    const char* p = message.c_str();
    const char* p_end = p + message.size();
    Xapian::termcount doclen;
    if (!unpack_uint_last(&p, p_end, &doclen)) {
	throw Xapian::NetworkError("Bad REPLY_DOCLENGTH", context);
    }
    return doclen;
}

reply_type
RemoteDatabase::get_message(string &result,
			    reply_type required_type,
			    reply_type required_type2) const
{
    double end_time = RealTime::end_time(timeout);
    int type = link.get_message(result, end_time);
    if (pending_reply && !is_intermediate_reply(type)) {
	pending_reply = false;
    }
    if (type < 0)
	throw_connection_closed_unexpectedly();
    if (rare(type) >= REPLY_MAX) {
	if (required_type == REPLY_UPDATE)
	    throw_handshake_failed(context);
	string errmsg("Invalid reply type ");
	errmsg += str(type);
	throw Xapian::NetworkError(errmsg);
    }
    if (type == REPLY_EXCEPTION) {
	unserialise_error(result, "REMOTE:", context);
    }
    if (type != required_type && type != required_type2) {
	string errmsg("Expecting reply type ");
	errmsg += str(int(required_type));
	if (required_type2 != required_type) {
	    errmsg += " or ";
	    errmsg += str(int(required_type2));
	}
	errmsg += ", got ";
	errmsg += str(type);
	throw Xapian::NetworkError(errmsg);
    }

    return static_cast<reply_type>(type);
}

void
RemoteDatabase::send_message(message_type type, const string &message) const
{
    double end_time = RealTime::end_time(timeout);
    while (pending_reply) {
	string dummy;
	int reply_code = link.get_message(dummy, end_time);
	if (reply_code < 0)
	    throw_connection_closed_unexpectedly();
	if (!is_intermediate_reply(reply_code)) {
	    pending_reply = false;
	}
    }
    link.send_message(static_cast<unsigned char>(type), message, end_time);
    pending_reply = true;
}

void
RemoteDatabase::do_close()
{
    // The dtor hasn't really been called!  FIXME: This works, but means any
    // exceptions from end_transaction()/commit() are swallowed, which is
    // not entirely desirable.
    dtor_called();

    if (!is_read_only()) {
	// If we're writable, send a shutdown message to the server and wait
	// for it to close its end of the connection so we know that changes
	// have been written and flushed, and the database write lock released.
	// For the non-writable case, there's no need to wait - it would just
	// slow down searching needlessly.
	link.shutdown();
    }
    link.do_close();
}

void
RemoteDatabase::set_query(const Xapian::Query& query,
			  Xapian::termcount qlen,
			  Xapian::valueno collapse_key,
			  Xapian::doccount collapse_max,
			  Xapian::Enquire::docid_order order,
			  Xapian::valueno sort_key,
			  Xapian::Enquire::Internal::sort_setting sort_by,
			  bool sort_value_forward,
			  double time_limit,
			  int percent_threshold, double weight_threshold,
			  const Xapian::Weight& wtscheme,
			  const Xapian::RSet &omrset,
			  const vector<opt_ptr_spy>& matchspies,
			  bool full_db_has_positions) const
{
    string message;
    pack_string(message, query.serialise());

    // Serialise assorted Enquire settings.
    pack_uint(message, qlen);
    pack_uint(message, collapse_max);
    if (collapse_max) pack_uint(message, collapse_key);
    message += char(order);
    message += char(sort_by);
    if (sort_by != Xapian::Enquire::Internal::REL) {
	pack_uint(message, sort_key);
    }
    pack_bool(message, sort_value_forward);
    pack_bool(message, full_db_has_positions);
    message += serialise_double(time_limit);
    message += char(percent_threshold);
    message += serialise_double(weight_threshold);

    pack_string(message, wtscheme.name());

    pack_string(message, wtscheme.serialise());

    pack_string(message, serialise_rset(omrset));

    for (auto i : matchspies) {
	const string& name = i->name();
	if (name.empty()) {
	    throw Xapian::UnimplementedError("MatchSpy subclass not suitable for use with remote searches - name() method returned empty string");
	}
	pack_string(message, name);
	pack_string(message, i->serialise());
    }

    send_message(MSG_QUERY, message);
}

void
RemoteDatabase::get_remote_stats(Xapian::Weight::Internal& out) const
{
    string message;
    get_message(message, REPLY_STATS);
    unserialise_stats(message, out);
}

void
RemoteDatabase::send_global_stats(Xapian::doccount first,
				  Xapian::doccount maxitems,
				  Xapian::doccount check_at_least,
				  const Xapian::KeyMaker* sorter,
				  const Xapian::Weight::Internal &stats) const
{
    string message;
    pack_uint(message, first);
    pack_uint(message, maxitems);
    pack_uint(message, check_at_least);
    if (!sorter) {
	pack_string_empty(message);
    } else {
	const string& name = sorter->name();
	if (name.empty()) {
	    throw_invalid_operation("sorter reported empty name");
	}
	pack_string(message, name);
	pack_string(message, sorter->serialise());
    }
    message += serialise_stats(stats);
    send_message(MSG_GETMSET, message);
}

Xapian::MSet
RemoteDatabase::get_mset(const vector<opt_ptr_spy>& matchspies) const
{
    string message;
    get_message(message, REPLY_RESULTS);
    const char * p = message.data();
    const char * p_end = p + message.size();

    string spyresults;
    for (auto i : matchspies) {
	if (!unpack_string(&p, p_end, spyresults)) {
	    throw Xapian::NetworkError("Expected serialised matchspy");
	}
	i->merge_results(spyresults);
    }
    Xapian::MSet mset;
    mset.internal->unserialise(p, p_end);
    return mset;
}

void
RemoteDatabase::commit()
{
    send_message(MSG_COMMIT, string());

    // We need to wait for a response to ensure documents have been committed.
    string message;
    get_message(message, REPLY_DONE);
}

void
RemoteDatabase::cancel()
{
    cached_stats_valid = false;
    mru_slot = Xapian::BAD_VALUENO;

    send_message(MSG_CANCEL, string());
    string dummy;
    get_message(dummy, REPLY_DONE);
}

Xapian::docid
RemoteDatabase::add_document(const Xapian::Document & doc)
{
    cached_stats_valid = false;
    mru_slot = Xapian::BAD_VALUENO;

    send_message(MSG_ADDDOCUMENT, serialise_document(doc));

    string message;
    get_message(message, REPLY_ADDDOCUMENT);

    const char* p = message.data();
    const char* p_end = p + message.size();
    Xapian::docid did;
    if (!unpack_uint_last(&p, p_end, &did)) {
	unpack_throw_serialisation_error(p);
    }
    return did;
}

void
RemoteDatabase::delete_document(Xapian::docid did)
{
    cached_stats_valid = false;
    mru_slot = Xapian::BAD_VALUENO;

    string message;
    pack_uint_last(message, did);
    send_message(MSG_DELETEDOCUMENT, message);

    get_message(message, REPLY_DONE);
}

void
RemoteDatabase::delete_document(const std::string & unique_term)
{
    cached_stats_valid = false;
    mru_slot = Xapian::BAD_VALUENO;

    send_message(MSG_DELETEDOCUMENTTERM, unique_term);
    string dummy;
    get_message(dummy, REPLY_DONE);
}

void
RemoteDatabase::replace_document(Xapian::docid did,
				 const Xapian::Document & doc)
{
    cached_stats_valid = false;
    mru_slot = Xapian::BAD_VALUENO;

    string message;
    pack_uint(message, did);
    message += serialise_document(doc);

    send_message(MSG_REPLACEDOCUMENT, message);

    get_message(message, REPLY_DONE);
}

Xapian::docid
RemoteDatabase::replace_document(const std::string & unique_term,
				 const Xapian::Document & doc)
{
    cached_stats_valid = false;
    mru_slot = Xapian::BAD_VALUENO;

    string message;
    pack_string(message, unique_term);
    message += serialise_document(doc);

    send_message(MSG_REPLACEDOCUMENTTERM, message);

    get_message(message, REPLY_ADDDOCUMENT);

    const char* p = message.data();
    const char* p_end = p + message.size();
    Xapian::docid did;
    if (!unpack_uint_last(&p, p_end, &did)) {
	unpack_throw_serialisation_error(p);
    }
    return did;
}

string
RemoteDatabase::get_uuid() const
{
    return uuid;
}

string
RemoteDatabase::get_metadata(const string & key) const
{
    send_message(MSG_GETMETADATA, key);
    string metadata;
    get_message(metadata, REPLY_METADATA);
    return metadata;
}

void
RemoteDatabase::set_metadata(const string & key, const string & value)
{
    string message;
    pack_string(message, key);
    message += value;
    send_message(MSG_SETMETADATA, message);

    get_message(message, REPLY_DONE);
}

void
RemoteDatabase::add_spelling(const string & word,
			     Xapian::termcount freqinc) const
{
    string message;
    pack_uint(message, freqinc);
    message += word;
    send_message(MSG_ADDSPELLING, message);

    get_message(message, REPLY_DONE);
}

Xapian::termcount
RemoteDatabase::remove_spelling(const string & word,
				Xapian::termcount freqdec) const
{
    string message;
    pack_uint(message, freqdec);
    message += word;
    send_message(MSG_REMOVESPELLING, message);

    get_message(message, REPLY_REMOVESPELLING);
    const char * p = message.data();
    const char * p_end = p + message.size();
    Xapian::termcount result;
    if (!unpack_uint_last(&p, p_end, &result)) {
	throw Xapian::NetworkError("Bad REPLY_REMOVESPELLING", context);
    }
    return result;
}

bool
RemoteDatabase::locked() const
{
    throw Xapian::UnimplementedError("Database::locked() not implemented for remote backend");
}

string
RemoteDatabase::reconstruct_text(Xapian::docid did,
				 size_t length,
				 const string& prefix,
				 Xapian::termpos start_pos,
				 Xapian::termpos end_pos) const
{
    string message;
    pack_uint(message, did);
    pack_uint(message, length);
    pack_uint(message, start_pos);
    pack_uint(message, end_pos);
    message += prefix;
    send_message(MSG_RECONSTRUCTTEXT, message);

    get_message(message, REPLY_RECONSTRUCTTEXT);
    return message;
}

string
RemoteDatabase::get_description() const
{
    string desc = "Remote(context=";
    desc += context;
    desc += ')';
    return desc;
}
