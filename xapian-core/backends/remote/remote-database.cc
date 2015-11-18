/** @file remote-database.cc
 *  @brief Remote backend database class
 */
/* Copyright (C) 2006,2007,2008,2009,2010,2015 Olly Betts
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

#include "safeerrno.h"
#include <signal.h>

#include "autoptr.h"
#include "emptypostlist.h"
#include "inmemory_positionlist.h"
#include "net/length.h"
#include "net_postlist.h"
#include "net_termlist.h"
#include "remote-document.h"
#include "omassert.h"
#include "realtime.h"
#include "serialise.h"
#include "serialise-double.h"
#include "str.h"
#include "stringutils.h" // For STRINGIZE().
#include "weightinternal.h"

#include <string>
#include <vector>

#include "xapian/error.h"
#include "xapian/matchspy.h"

using namespace std;

XAPIAN_NORETURN(static void throw_connection_closed_unexpectedly());
static void
throw_connection_closed_unexpectedly()
{
    throw Xapian::NetworkError("Connection closed unexpectedly");
}

RemoteDatabase::RemoteDatabase(int fd, double timeout_,
			       const string & context_, bool writable)
	: link(fd, fd, context_),
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

    if (!writable) {
	// Transactions only make sense when writing, so flag them as
	// "unimplemented" so that our destructor doesn't call dtor_called()
	// since that might try to call commit() which will cause a message to
	// be sent to the remote server and probably an InvalidOperationError
	// exception message to be returned.
	transaction_state = TRANSACTION_UNIMPLEMENTED;
    }

    string message;
    char type = get_message(message);

    if (reply_type(type) != REPLY_GREETING || message.size() < 3) {
	if (type == 'O' && message.size() == size_t('M') && message[0] == ' ') {
	    // The server reply used to start "OM ", which will now be
	    // interpreted as a type 'O' message of length size_t('M')
	    // with first character ' '.
	    throw Xapian::NetworkError("Server protocol version too old", context);
	}
	throw Xapian::NetworkError("Handshake failed - is this a Xapian server?", context);
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

    apply_stats_update(p, p_end);

    if (writable) update_stats(MSG_WRITEACCESS);
}

RemoteDatabase *
RemoteDatabase::as_remotedatabase()
{
    return this;
}

void
RemoteDatabase::keep_alive()
{
    send_message(MSG_KEEPALIVE, string());
    string message;
    get_message(message, REPLY_DONE);
}

TermList *
RemoteDatabase::open_metadata_keylist(const std::string &prefix) const
{
    // Ensure that total_length and doccount are up-to-date.
    if (!cached_stats_valid) update_stats();

    send_message(MSG_METADATAKEYLIST, prefix);

    string message;
    AutoPtr<NetworkTermList> tlist(
	new NetworkTermList(0, doccount,
			    Xapian::Internal::RefCntPtr<const RemoteDatabase>(this),
			    0));
    vector<NetworkTermListItem> & items = tlist->items;

    char type;
    while ((type = get_message(message)) == REPLY_METADATAKEYLIST) {
	NetworkTermListItem item;
	item.tname = message;
	items.push_back(item);
    }
    if (type != REPLY_DONE) {
	throw Xapian::NetworkError("Bad message received", context);
    }

    tlist->current_position = tlist->items.begin();
    return tlist.release();
}

TermList *
RemoteDatabase::open_term_list(Xapian::docid did) const
{
    Assert(did);

    // Ensure that total_length and doccount are up-to-date.
    if (!cached_stats_valid) update_stats();

    send_message(MSG_TERMLIST, encode_length(did));

    string message;
    get_message(message, REPLY_DOCLENGTH);
    const char * p = message.c_str();
    const char * p_end = p + message.size();
    Xapian::termcount doclen;
    decode_length(&p, p_end, doclen);
    if (p != p_end) {
	throw Xapian::NetworkError("Bad REPLY_DOCLENGTH message received", context);
    }

    AutoPtr<NetworkTermList> tlist(
	new NetworkTermList(doclen, doccount,
			    Xapian::Internal::RefCntPtr<const RemoteDatabase>(this),
			    did));
    vector<NetworkTermListItem> & items = tlist->items;

    char type;
    while ((type = get_message(message)) == REPLY_TERMLIST) {
	NetworkTermListItem item;
	p = message.data();
	p_end = p + message.size();
	decode_length(&p, p_end, item.wdf);
	decode_length(&p, p_end, item.termfreq);
	item.tname.assign(p, p_end);
	items.push_back(item);
    }
    if (type != REPLY_DONE) {
	throw Xapian::NetworkError("Bad message received", context);
    }

    tlist->current_position = tlist->items.begin();
    return tlist.release();
}

TermList *
RemoteDatabase::open_allterms(const string & prefix) const {
    // Ensure that total_length and doccount are up-to-date.
    if (!cached_stats_valid) update_stats();

    send_message(MSG_ALLTERMS, prefix);

    AutoPtr<NetworkTermList> tlist(
	new NetworkTermList(0, doccount,
			    Xapian::Internal::RefCntPtr<const RemoteDatabase>(this),
			    0));
    vector<NetworkTermListItem> & items = tlist->items;

    string message;
    char type;
    while ((type = get_message(message)) == REPLY_ALLTERMS) {
	NetworkTermListItem item;
	const char * p = message.data();
	const char * p_end = p + message.size();
	decode_length(&p, p_end, item.termfreq);
	item.tname.assign(p, p_end);
	items.push_back(item);
    }
    if (type != REPLY_DONE) {
	throw Xapian::NetworkError("Bad message received", context);
    }

    tlist->current_position = tlist->items.begin();
    return tlist.release();
}

LeafPostList *
RemoteDatabase::open_post_list(const string &term) const
{
    return new NetworkPostList(Xapian::Internal::RefCntPtr<const RemoteDatabase>(this), term);
}

Xapian::doccount
RemoteDatabase::read_post_list(const string &term, NetworkPostList & pl) const
{
    send_message(MSG_POSTLIST, term);

    string message;
    char type;
    get_message(message, REPLY_POSTLISTSTART);

    const char * p = message.data();
    const char * p_end = p + message.size();
    Xapian::doccount termfreq;
    decode_length(&p, p_end, termfreq);

    while ((type = get_message(message)) == REPLY_POSTLISTITEM) {
	pl.append_posting(message);
    }
    if (type != REPLY_DONE) {
	throw Xapian::NetworkError("Bad message received", context);
    }

    return termfreq;
}

PositionList *
RemoteDatabase::open_position_list(Xapian::docid did, const string &term) const
{
    send_message(MSG_POSITIONLIST, encode_length(did) + term);

    vector<Xapian::termpos> positions;

    string message;
    char type;
    Xapian::termpos lastpos = static_cast<Xapian::termpos>(-1);
    while ((type = get_message(message)) == REPLY_POSITIONLIST) {
	const char * p = message.data();
	const char * p_end = p + message.size();
	Xapian::termpos inc;
	decode_length(&p, p_end, inc);
	lastpos += inc + 1;
	positions.push_back(lastpos);
    }
    if (type != REPLY_DONE) {
	throw Xapian::NetworkError("Bad message received", context);
    }

    return new InMemoryPositionList(positions);
}

bool
RemoteDatabase::has_positions() const
{
    if (!cached_stats_valid) update_stats();
    return has_positional_info;
}

void
RemoteDatabase::reopen()
{
    update_stats(MSG_REOPEN);
    mru_slot = Xapian::BAD_VALUENO;
}

void
RemoteDatabase::close()
{
    do_close();
}

// Currently lazy is used when fetching documents from the MSet, and in three
// cases in multimatch.cc.  One of the latter is when using a MatchDecider,
// which we don't support with the remote backend currently.  The others are
// for the sort key and collapse key which in the remote cases are fetched
// during the remote match and passed across with the MSet.  So we can safely
// ignore "lazy" here for now without any performance penalty during the match
// process.
Xapian::Document::Internal *
RemoteDatabase::open_document(Xapian::docid did, bool /*lazy*/) const
{
    Assert(did);

    send_message(MSG_DOCUMENT, encode_length(did));
    string doc_data;
    map<Xapian::valueno, string> values;
    get_message(doc_data, REPLY_DOCDATA);

    reply_type type;
    string message;
    while ((type = get_message(message)) == REPLY_VALUE) {
	const char * p = message.data();
	const char * p_end = p + message.size();
	Xapian::valueno slot;
	decode_length(&p, p_end, slot);
	values.insert(make_pair(slot, string(p, p_end)));
    }
    if (type != REPLY_DONE) {
	throw Xapian::NetworkError("Bad message received", context);
    }

    return new RemoteDocument(this, did, doc_data, values);
}

void
RemoteDatabase::update_stats(message_type msg_code) const
{
    send_message(msg_code, string());
    string message;
    get_message(message, REPLY_UPDATE);
    const char * p = message.c_str();
    const char * p_end = p + message.size();
    apply_stats_update(p, p_end);
}

void
RemoteDatabase::apply_stats_update(const char * p, const char * p_end) const
{
    decode_length(&p, p_end, doccount);
    decode_length(&p, p_end, lastdocid);
    decode_length(&p, p_end, doclen_lbound);
    decode_length(&p, p_end, doclen_ubound);
    if (p == p_end) {
	throw Xapian::NetworkError("Bad stats update message received", context);
    }
    has_positional_info = (*p++ == '1');
    decode_length(&p, p_end, total_length);
    uuid.assign(p, p_end);
    cached_stats_valid = true;
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

totlen_t
RemoteDatabase::get_total_length() const
{
    if (!cached_stats_valid) update_stats();
    return total_length;
}

Xapian::doclength
RemoteDatabase::get_avlength() const
{
    if (!cached_stats_valid) update_stats();
    if (rare(doccount == 0)) return 0;
    return Xapian::doclength(total_length) / doccount;
}

bool
RemoteDatabase::term_exists(const string & tname) const
{
    Assert(!tname.empty());
    send_message(MSG_TERMEXISTS, tname);
    string message;
    reply_type type = get_message(message);
    if (type != REPLY_TERMEXISTS && type != REPLY_TERMDOESNTEXIST) {
	throw Xapian::NetworkError("Bad message received", context);
    }
    return (type == REPLY_TERMEXISTS);
}

Xapian::doccount
RemoteDatabase::get_termfreq(const string & tname) const
{
    Assert(!tname.empty());
    send_message(MSG_TERMFREQ, tname);
    string message;
    get_message(message, REPLY_TERMFREQ);
    const char * p = message.data();
    const char * p_end = p + message.size();
    Xapian::doccount r;
    decode_length(&p, p_end, r);
    return r;
}

Xapian::termcount
RemoteDatabase::get_collection_freq(const string & tname) const
{
    Assert(!tname.empty());
    send_message(MSG_COLLFREQ, tname);
    string message;
    get_message(message, REPLY_COLLFREQ);
    const char * p = message.data();
    const char * p_end = p + message.size();
    Xapian::termcount r;
    decode_length(&p, p_end, r);
    return r;
}


void
RemoteDatabase::read_value_stats(Xapian::valueno slot) const
{
    if (mru_slot != slot) {
	send_message(MSG_VALUESTATS, encode_length(slot));
	string message;
	get_message(message, REPLY_VALUESTATS);
	const char * p = message.data();
	const char * p_end = p + message.size();
	mru_slot = slot;
	decode_length(&p, p_end, mru_valstats.freq);
	size_t len;
	decode_length_and_check(&p, p_end, len);
	mru_valstats.lower_bound.assign(p, len);
	p += len;
	decode_length_and_check(&p, p_end, len);
	mru_valstats.upper_bound.assign(p, len);
	p += len;
	if (p != p_end) {
	    throw Xapian::NetworkError("Bad REPLY_VALUESTATS message received", context);
	}
    }
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
    send_message(MSG_DOCLENGTH, encode_length(did));
    string message;
    get_message(message, REPLY_DOCLENGTH);
    const char * p = message.c_str();
    const char * p_end = p + message.size();
    Xapian::termcount doclen;
    decode_length(&p, p_end, doclen);
    if (p != p_end) {
	throw Xapian::NetworkError("Bad REPLY_DOCLENGTH message received", context);
    }
    return doclen;
}

reply_type
RemoteDatabase::get_message(string &result, reply_type required_type) const
{
    double end_time = RealTime::end_time(timeout);
    int type_int = link.get_message(result, end_time);
    if (type_int < 0)
	throw_connection_closed_unexpectedly();
    reply_type type = static_cast<reply_type>(type_int);
    if (type == REPLY_EXCEPTION) {
	unserialise_error(result, "REMOTE:", context);
    }
    if (required_type != REPLY_MAX && type != required_type) {
	string errmsg("Expecting reply type ");
	errmsg += str(int(required_type));
	errmsg += ", got ";
	errmsg += str(int(type));
	throw Xapian::NetworkError(errmsg);
    }

    return type;
}

void
RemoteDatabase::send_message(message_type type, const string &message) const
{
    double end_time = RealTime::end_time(timeout);
    link.send_message(static_cast<unsigned char>(type), message, end_time);
}

void
RemoteDatabase::do_close()
{
    // In the constructor, we set transaction_state to
    // TRANSACTION_UNIMPLEMENTED if we aren't writable so that we can check
    // it here.
    bool writable = (transaction_state != TRANSACTION_UNIMPLEMENTED);

    // Only call dtor_called() if we're writable.
    if (writable) dtor_called();

    // If we're writable, wait for a confirmation of the close, so we know that
    // changes have been written and flushed, and the database write lock
    // released.  For the non-writable case, there's no need to wait, so don't
    // slow down searching by waiting here.
    link.do_close(writable);
}

void
RemoteDatabase::set_query(const Xapian::Query::Internal *query,
			 Xapian::termcount qlen,
			 Xapian::doccount collapse_max,
			 Xapian::valueno collapse_key,
			 Xapian::Enquire::docid_order order,
			 Xapian::valueno sort_key,
			 Xapian::Enquire::Internal::sort_setting sort_by,
			 bool sort_value_forward,
			 int percent_cutoff, Xapian::weight weight_cutoff,
			 const Xapian::Weight *wtscheme,
			 const Xapian::RSet &omrset,
			 const vector<Xapian::MatchSpy *> & matchspies)
{
    string tmp = query->serialise();
    string message = encode_length(tmp.size());
    message += tmp;

    // Serialise assorted Enquire settings.
    message += encode_length(qlen);
    message += encode_length(collapse_max);
    if (collapse_max) message += encode_length(collapse_key);
    message += char('0' + order);
    message += encode_length(sort_key);
    message += char('0' + sort_by);
    message += char('0' + sort_value_forward);
    message += char(percent_cutoff);
    message += serialise_double(weight_cutoff);

    tmp = wtscheme->name();
    message += encode_length(tmp.size());
    message += tmp;

    tmp = wtscheme->serialise();
    message += encode_length(tmp.size());
    message += tmp;

    tmp = serialise_rset(omrset);
    message += encode_length(tmp.size());
    message += tmp;

    vector<Xapian::MatchSpy *>::const_iterator i;
    for (i = matchspies.begin(); i != matchspies.end(); ++i) {
	tmp = (*i)->name();
	if (tmp.empty()) {
	    throw Xapian::UnimplementedError("MatchSpy subclass not suitable for use with remote searches - name() method returned empty string");
	}
	message += encode_length(tmp.size());
	message += tmp;

	tmp = (*i)->serialise();
	message += encode_length(tmp.size());
	message += tmp;
    }

    send_message(MSG_QUERY_NEW, message);
}

bool
RemoteDatabase::get_remote_stats(bool nowait, Xapian::Weight::Internal &out)
{
    if (nowait && !link.ready_to_read()) return false;

    string message;
    get_message(message, REPLY_STATS);
    out = unserialise_stats(message);

    return true;
}

void
RemoteDatabase::send_global_stats(Xapian::doccount first,
				  Xapian::doccount maxitems,
				  Xapian::doccount check_at_least,
				  const Xapian::Weight::Internal &stats)
{
    string message = encode_length(first);
    message += encode_length(maxitems);
    message += encode_length(check_at_least);
    message += serialise_stats(stats);
    send_message(MSG_GETMSET, message);
}

void
RemoteDatabase::get_mset(Xapian::MSet &mset,
			 const vector<Xapian::MatchSpy *> & matchspies)
{
    string message;
    get_message(message, REPLY_RESULTS_NEW);
    const char * p = message.data();
    const char * p_end = p + message.size();

    vector<Xapian::MatchSpy *>::const_iterator i;
    for (i = matchspies.begin(); i != matchspies.end(); ++i) {
	if (p == p_end)
	    throw Xapian::NetworkError("Expected serialised matchspy");
	size_t len;
	decode_length_and_check(&p, p_end, len);
	string spyresults(p, len);
	p += len;
	(*i)->merge_results(spyresults);
    }
    mset = unserialise_mset_new(p, p_end);
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
}

Xapian::docid
RemoteDatabase::add_document(const Xapian::Document & doc)
{
    cached_stats_valid = false;
    mru_slot = Xapian::BAD_VALUENO;

    send_message(MSG_ADDDOCUMENT, serialise_document(doc));

    string message;
    get_message(message, REPLY_ADDDOCUMENT);

    const char * p = message.data();
    const char * p_end = p + message.size();
    Xapian::docid did;
    decode_length(&p, p_end, did);
    return did;
}

void
RemoteDatabase::delete_document(Xapian::docid did)
{
    cached_stats_valid = false;
    mru_slot = Xapian::BAD_VALUENO;

    send_message(MSG_DELETEDOCUMENT, encode_length(did));
    string dummy;
    get_message(dummy, REPLY_DONE);
}

void
RemoteDatabase::delete_document(const std::string & unique_term)
{
    cached_stats_valid = false;
    mru_slot = Xapian::BAD_VALUENO;

    send_message(MSG_DELETEDOCUMENTTERM, unique_term);
}

void
RemoteDatabase::replace_document(Xapian::docid did,
				 const Xapian::Document & doc)
{
    cached_stats_valid = false;
    mru_slot = Xapian::BAD_VALUENO;

    string message = encode_length(did);
    message += serialise_document(doc);

    send_message(MSG_REPLACEDOCUMENT, message);
}

Xapian::docid
RemoteDatabase::replace_document(const std::string & unique_term,
				 const Xapian::Document & doc)
{
    cached_stats_valid = false;
    mru_slot = Xapian::BAD_VALUENO;

    string message = encode_length(unique_term.size());
    message += unique_term;
    message += serialise_document(doc);

    send_message(MSG_REPLACEDOCUMENTTERM, message);

    get_message(message, REPLY_ADDDOCUMENT);

    const char * p = message.data();
    const char * p_end = p + message.size();
    Xapian::docid did;
    decode_length(&p, p_end, did);
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
    string data = encode_length(key.size());
    data += key;
    data += value;
    send_message(MSG_SETMETADATA, data);
}

void
RemoteDatabase::add_spelling(const string & word,
			     Xapian::termcount freqinc) const
{
    string data = encode_length(freqinc);
    data += word;
    send_message(MSG_ADDSPELLING, data);
}

void
RemoteDatabase::remove_spelling(const string & word,
				Xapian::termcount freqdec) const
{
    string data = encode_length(freqdec);
    data += word;
    send_message(MSG_REMOVESPELLING, data);
}
