/** @file remote-database.h
 *  @brief RemoteDatabase is the baseclass for remote database implementations.
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

#ifndef XAPIAN_INCLUDED_REMOTE_DATABASE_H
#define XAPIAN_INCLUDED_REMOTE_DATABASE_H

#include "database.h"
#include "omenquireinternal.h"
#include "omqueryinternal.h"
#include "omtime.h"
#include "remoteconnection.h"

namespace Xapian {
    class RSet;
    class Weight;
}

class Stats;
class NetworkPostList;

/** RemoteDatabase is the baseclass for remote database implementations.
 *
 *  A subclass of this class is required which opens a tcp connection or
 *  pipe to the remote database server.  This subclass works in combination
 *  with the RemoteSubMatch class during the match process.
 */
class RemoteDatabase : public Xapian::Database::Internal {
    /// Don't allow assignment.
    void operator=(const RemoteDatabase &);

    /// Don't allow copying.
    RemoteDatabase(const RemoteDatabase &);

    /// The object which does the I/O.
    mutable RemoteConnection link;

    /// The remote document count, given at open.
    mutable Xapian::doccount doccount;

    /// The remote last docid, given at open.
    mutable Xapian::docid lastdocid;

    /// The remote document avlength, given at open.
    mutable Xapian::doclength avlength;

    /// Has positional information?
    mutable bool has_positional_info;

    /// The context to return with any error messages
    string context;

    mutable bool cached_stats_valid;

    void update_stats(message_type msg_code = MSG_UPDATE) const;

  protected:
    /** Constructor.  The constructor is protected so that raw instances
     *  can't be created - a derived class must be instantiated which
     *  has code in the constructor to open the socket.
     *
     *  @param fd	The file descriptor for the connection to the server.
     *  @param timeout_ The timeout used with the network operations.
     *                       Generally a Xapian::NetworkTimeout exception will
     *                       be thrown if the remote end doesn't respond
     *                       for this length of time (in milliseconds).
     *  @param context_     The context to return with any error messages.
     */
    RemoteDatabase(int fd, Xapian::timeout timeout_, const string & context_);

    /// Receive a message from the server.
    reply_type get_message(string & message, reply_type required_type = REPLY_MAX) const;

    /// Send a message to the server.
    void send_message(message_type type, const string & data) const;

    /// Close the socket
    void do_close();

    bool get_posting(Xapian::docid &did, Xapian::weight &w, string &value);

    /// The timeout value used in network communications, in milliseconds
    Xapian::timeout timeout;

    /** The time at which the current operation will time out.
     *
     *  If !end_time.is_set(), then no timeout is currently set.
     */
    mutable OmTime end_time;

  public:
    /// Return this pointer as a RemoteDatabase*.
    RemoteDatabase * as_remotedatabase();

    /// Send a keep-alive message.
    void keep_alive();

    /** Set the query
     *
     * @param query			The query.
     * @param qlen			The query length.
     * @param collapse_key		The value number to collapse matches on.
     * @param order			Sort order for docids.
     * @param sort_key			The value number to sort on.
     * @param sort_by			Which order to apply sorts in.
     * @param sort_value_forward	Sort order for values.
     * @param percent_cutoff		Percentage cutoff.
     * @param weight_cutoff		Weight cutoff.
     * @param wtscheme			Weighting scheme.
     * @param omrset			The rset.
     */
    void set_query(const Xapian::Query::Internal *query,
		   Xapian::termcount qlen,
		   Xapian::valueno collapse_key,
		   Xapian::Enquire::docid_order order,
		   Xapian::valueno sort_key,
		   Xapian::Enquire::Internal::sort_setting sort_by,
		   bool sort_value_forward,
		   int percent_cutoff, Xapian::weight weight_cutoff,
		   const Xapian::Weight *wtscheme,
		   const Xapian::RSet &omrset);

    /** Get the Stats from the remote server.
     *
     *  @return	true if we got the remote stats; false if we should try again.
     */
    bool get_remote_stats(bool nowait, Stats &out);

    /// Send the global Stats to the remote server.
    void send_global_stats(Xapian::doccount first,
			   Xapian::doccount maxitems,
			   const Stats &stats);

    /// Get the MSet from the remote server.
    void get_mset(Xapian::MSet &mset);

    /// Get remote termlist.
    TermList * open_term_list(Xapian::docid did) const;

    /// Iterate all terms.
    TermList * open_allterms() const;

    bool has_positions() const;

    void reopen();

    LeafPostList * open_post_list(const string & tname) const;

    void read_post_list(const string &term, NetworkPostList & pl) const;

    PositionList * open_position_list(Xapian::docid did,
				      const string & tname) const;

    /// Get a remote document.
    Xapian::Document::Internal * open_document(Xapian::docid did, bool lazy) const;

    /// Get the document count.
    Xapian::doccount get_doccount() const;

    /// Get the last used docid.
    Xapian::docid get_lastdocid() const;

    /// Find out the remote average document length.
    Xapian::doclength get_avlength() const;

    Xapian::doclength get_doclength(Xapian::docid did) const;

    /// Check if term exists.
    bool term_exists(const string & tname) const;

    /// Find frequency of term.
    Xapian::doccount get_termfreq(const string & tname) const;

    Xapian::termcount get_collection_freq(const string & tname) const;

    void flush();

    void cancel();

    Xapian::docid add_document(const Xapian::Document & doc);

    void delete_document(Xapian::docid did);
    void delete_document(const std::string & unique_term);

    void replace_document(Xapian::docid did, const Xapian::Document & doc);
    Xapian::docid replace_document(const std::string & unique_term,
				   const Xapian::Document & document);
};

#endif // XAPIAN_INCLUDED_REMOTE_DATABASE_H
