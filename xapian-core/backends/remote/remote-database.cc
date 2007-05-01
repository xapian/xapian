/** @file remote-database.cc
 *  @brief Remote backend database class
 */
/* Copyright (C) 2006,2007 Olly Betts
 * Copyright (C) 2007 Lemur Consulting Ltd
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

#include "safeerrno.h"
#include <signal.h>

#include <xapian/error.h>

#include "autoptr.h"
#include "emptypostlist.h"
#include "inmemory_positionlist.h"
#include "net_postlist.h"
#include "net_termlist.h"
#include "net_document.h"
#include "omassert.h"
#include "serialise.h"
#include "serialise-double.h"
#include "stats.h"
#include "utils.h"

#include <string>
#include <vector>

using namespace std;

RemoteDatabase::RemoteDatabase(int fd, Xapian::timeout timeout_,
			       const string & context_)
	: link(fd, fd, context_),
	  context(context_),
	  cached_stats_valid(),
	  timeout(timeout_),
	  end_time()
{
#ifndef __WIN32__
    // It's simplest to just ignore SIGPIPE.  We'll still know if the
    // connection dies because we'll get EPIPE back from write().
    if (signal(SIGPIPE, SIG_IGN) == SIG_ERR) {
	throw Xapian::NetworkError("Couldn't set SIGPIPE to SIG_IGN", errno);
    }
#endif

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

    if (*p != XAPIAN_REMOTE_PROTOCOL_VERSION) {
	string errmsg("Unknown protocol version ");
	errmsg += om_tostring(int(*p));
	errmsg += " ("STRINGIZE(XAPIAN_REMOTE_PROTOCOL_VERSION)" supported)";
	throw Xapian::NetworkError(errmsg, context);
    }
    ++p;

    doccount = decode_length(&p, p_end, false);
    lastdocid = decode_length(&p, p_end, false);
    if (p == p_end) {
	throw Xapian::NetworkError("Bad greeting message received (bool)", context);
    }
    has_positional_info = (*p++ == '1');
    avlength = unserialise_double(&p, p_end);
    if (p != p_end || avlength < 0) {
	throw Xapian::NetworkError("Bad greeting message received (double)", context);
    }
}

RemoteDatabase *
RemoteDatabase::as_remotedatabase()
{
    return this;
}

void
RemoteDatabase::keep_alive()
{
    send_message(MSG_KEEPALIVE, "");
    string message;
    get_message(message, REPLY_DONE);
}

LeafTermList *
RemoteDatabase::open_term_list(Xapian::docid did) const
{
    if (did == 0) throw Xapian::InvalidArgumentError("Docid 0 invalid");

    // Ensure that avlength and doccount are up-to-date.
    if (!cached_stats_valid) update_stats();

    send_message(MSG_TERMLIST, encode_length(did));

    string message;
    get_message(message, REPLY_DOCLENGTH);
    const char * p = message.c_str();
    const char * p_end = p + message.size();
    Xapian::doclength doclen = unserialise_double(&p, p_end);
    if (p != p_end || doclen < 0) {
	throw Xapian::NetworkError("Bad REPLY_DOCLENGTH message received", context);
    }

    AutoPtr<NetworkTermList> tlist;
    tlist = new NetworkTermList(doclen, doccount,
				Xapian::Internal::RefCntPtr<const RemoteDatabase>(this),
				did);
    vector<NetworkTermListItem> & items = tlist->items;

    char type;
    while ((type = get_message(message)) == REPLY_TERMLIST) {
	NetworkTermListItem item;
	p = message.data();
	p_end = p + message.size();
	item.wdf = decode_length(&p, p_end, false);
	item.termfreq = decode_length(&p, p_end, false);
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
RemoteDatabase::open_allterms() const {
    // Ensure that avlength and doccount are up-to-date.
    if (!cached_stats_valid) update_stats();

    send_message(MSG_ALLTERMS, "");

    AutoPtr<NetworkTermList> tlist;
    tlist = new NetworkTermList(0.0, doccount,
				Xapian::Internal::RefCntPtr<const RemoteDatabase>(this),
				0);
    vector<NetworkTermListItem> & items = tlist->items;

    string message;
    char type;
    while ((type = get_message(message)) == REPLY_ALLTERMS) {
	NetworkTermListItem item;
	const char * p = message.data();
	const char * p_end = p + message.size();
	item.termfreq = decode_length(&p, p_end, false);
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

void
RemoteDatabase::read_post_list(const string &term, NetworkPostList & pl) const
{
    send_message(MSG_POSTLIST, term);

    string message;
    char type;
    get_message(message, REPLY_POSTLISTSTART);

    const char * p = message.data();
    const char * p_end = p + message.size();
    Xapian::doccount termfreq = decode_length(&p, p_end, false);
    Xapian::termcount collfreq = decode_length(&p, p_end, false);

    pl.initialise(termfreq, collfreq);

    while ((type = get_message(message)) == REPLY_POSTLISTITEM) {
	pl.append_posting(message);
    }
    if (type != REPLY_DONE) {
	throw Xapian::NetworkError("Bad message received", context);
    }
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
	lastpos += decode_length(&p, p_end, false) + 1;
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
}

// Currently lazy is only used in three cases, all in multimatch.cc.  One is
// when using a MatchDecider, which we don't support with the remote backend
// currently.  The others are for the sort key and collapse key which in the
// remote cases is fetched during the remote match and passed across with the
// MSet.  So we can safely ignore it here for now without any performance
// penalty.
Xapian::Document::Internal *
RemoteDatabase::open_document(Xapian::docid did, bool /*lazy*/) const
{
    if (did == 0) throw Xapian::InvalidArgumentError("Docid 0 invalid");

    send_message(MSG_DOCUMENT, encode_length(did));
    string doc_data;
    map<Xapian::valueno, string> values;
    get_message(doc_data, REPLY_DOCDATA);

    reply_type type;
    string message;
    while ((type = get_message(message)) == REPLY_VALUE) {
	const char * p = message.data();
	const char * p_end = p + message.size();
	Xapian::valueno valueno = decode_length(&p, p_end, false);
	values.insert(make_pair(valueno, string(p, p_end)));
    }
    if (type != REPLY_DONE) {
	throw Xapian::NetworkError("Bad message received", context);
    }

    return new NetworkDocument(this, did, doc_data, values);
}

void
RemoteDatabase::update_stats(message_type msg_code) const
{
    send_message(msg_code, "");
    string message;
    get_message(message, REPLY_UPDATE);
    const char * p = message.c_str();
    const char * p_end = p + message.size();
    doccount = decode_length(&p, p_end, false);
    lastdocid = decode_length(&p, p_end, false);
    if (p == p_end) {
	throw Xapian::NetworkError("Bad REPLY_UPDATE message received", context);
    }
    has_positional_info = (*p++ == '1');
    avlength = unserialise_double(&p, p_end);
    if (p != p_end || avlength < 0) {
	throw Xapian::NetworkError("Bad REPLY_UPDATE message received", context);
    }
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

Xapian::doclength
RemoteDatabase::get_avlength() const
{
    if (!cached_stats_valid) update_stats();
    return avlength;
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
    return decode_length(&p, p_end, false);
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
    return decode_length(&p, p_end, false);
}

Xapian::doclength
RemoteDatabase::get_doclength(Xapian::docid did) const
{
    Assert(did != 0);
    send_message(MSG_DOCLENGTH, encode_length(did));
    string message;
    get_message(message, REPLY_DOCLENGTH);
    const char * p = message.c_str();
    const char * p_end = p + message.size();
    Xapian::doclength doclen = unserialise_double(&p, p_end);
    if (p != p_end || doclen < 0) {
	throw Xapian::NetworkError("Bad REPLY_DOCLENGTH message received", context);
    }
    return doclen;
}

reply_type
RemoteDatabase::get_message(string &result, reply_type required_type) const
{
    reply_type type = static_cast<reply_type>(link.get_message(result, end_time));
    if (type == REPLY_EXCEPTION) {
	unserialise_error(result, "REMOTE:", context);
    }
    if (required_type != REPLY_MAX && type != required_type) {
	string errmsg("Expecting reply type ");
	errmsg += om_tostring(required_type);
	errmsg += ", got ";
	errmsg += om_tostring(type);
	throw Xapian::NetworkError(errmsg);
    }

    return type;
}

void
RemoteDatabase::send_message(message_type type, const string &message) const
{
    link.send_message(static_cast<unsigned char>(type), message, end_time);
}

void
RemoteDatabase::do_close()
{
    dtor_called(); // FIXME: we should only call this if we're writable...
    link.do_close();
}

void
RemoteDatabase::set_query(const Xapian::Query::Internal *query,
			 Xapian::termcount qlen,
			 Xapian::valueno collapse_key,
			 Xapian::Enquire::docid_order order,
			 Xapian::valueno sort_key,
			 Xapian::Enquire::Internal::sort_setting sort_by,
			 bool sort_value_forward,
			 int percent_cutoff, Xapian::weight weight_cutoff,
			 const Xapian::Weight *wtscheme,
			 const Xapian::RSet &omrset)
{
    string tmp = query->serialise();
    string message = encode_length(tmp.size());
    message += tmp;

    // Serialise assorted Enquire settings.
    message += encode_length(qlen);
    message += encode_length(collapse_key);
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

    message += serialise_rset(omrset);

    send_message(MSG_QUERY, message);
}

bool
RemoteDatabase::get_remote_stats(bool nowait, Stats &out)
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
				const Stats &stats)
{
    string message = encode_length(first);
    message += encode_length(maxitems);
    message += serialise_stats(stats);
    send_message(MSG_GETMSET, message);
}

void
RemoteDatabase::get_mset(Xapian::MSet &mset)
{
    string message;
    get_message(message, REPLY_RESULTS);
    mset = unserialise_mset(message);
}

void
RemoteDatabase::flush()
{
    send_message(MSG_FLUSH, "");

    // We need to wait for a response to ensure documents have been committed.
    string message;
    get_message(message, REPLY_DONE);
}

void
RemoteDatabase::cancel()
{
    cached_stats_valid = false;

    send_message(MSG_CANCEL, "");
}

Xapian::docid
RemoteDatabase::add_document(const Xapian::Document & doc)
{
    cached_stats_valid = false;

    send_message(MSG_ADDDOCUMENT, serialise_document(doc));

    string message;
    get_message(message, REPLY_ADDDOCUMENT);

    const char * p = message.data();
    const char * p_end = p + message.size();
    return decode_length(&p, p_end, false);
}

void
RemoteDatabase::delete_document(Xapian::docid did)
{
    cached_stats_valid = false;

    send_message(MSG_DELETEDOCUMENT, encode_length(did));
}

void
RemoteDatabase::delete_document(const std::string & unique_term)
{
    cached_stats_valid = false;

    send_message(MSG_DELETEDOCUMENTTERM, unique_term);
}

void
RemoteDatabase::replace_document(Xapian::docid did,
				 const Xapian::Document & doc)
{
    cached_stats_valid = false;

    string message = encode_length(did);
    message += serialise_document(doc);

    send_message(MSG_REPLACEDOCUMENT, message);
}

Xapian::docid
RemoteDatabase::replace_document(const std::string & unique_term,
				 const Xapian::Document & doc)
{
    cached_stats_valid = false;

    string message = encode_length(unique_term.size());
    message += unique_term;
    message += serialise_document(doc);

    send_message(MSG_REPLACEDOCUMENTTERM, message);

    get_message(message, REPLY_ADDDOCUMENT);

    const char * p = message.data();
    const char * p_end = p + message.size();
    return decode_length(&p, p_end, false);
}
