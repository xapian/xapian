/** @file
 *  @brief Xapian remote backend server base class
 */
/* Copyright (C) 2006,2007,2008,2009,2010,2014,2017 Olly Betts
 * Copyright (C) 2007,2009,2010 Lemur Consulting Ltd
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

#ifndef XAPIAN_INCLUDED_REMOTESERVER_H
#define XAPIAN_INCLUDED_REMOTESERVER_H

#include "xapian/database.h"
#include "xapian/postingsource.h"
#include "xapian/registry.h"
#include "xapian/visibility.h"
#include "xapian/weight.h"

#include "remoteconnection.h"

#include <string>

/** Remote backend server base class. */
class XAPIAN_VISIBILITY_DEFAULT RemoteServer : private RemoteConnection {
    /// Don't allow assignment.
    void operator=(const RemoteServer &);

    /// Don't allow copying.
    RemoteServer(const RemoteServer &);

    /** The database we're using.
     *
     *  If we're writable, this is the same as wdb.
     */
    Xapian::Database * db;

    /// The WritableDatabase we're using, or NULL if we're read-only.
    Xapian::WritableDatabase * wdb;

    /// Do we support writing?
    bool writable;

    /** Timeout for actions during a conversation.
     *
     *  The timeout is specified in seconds.  If the timeout is exceeded then a
     *  Xapian::NetworkTimeoutError is thrown.
     */
    double active_timeout;

    /** Timeout while waiting for a new action from the client.
     *
     *  The timeout is specified in seconds.  If the timeout is exceeded then a
     *  Xapian::NetworkTimeoutError is thrown.
     */
    double idle_timeout;

    /// The registry, which allows unserialisation of user subclasses.
    Xapian::Registry reg;

    /// Accept a message from the client.
    XAPIAN_VISIBILITY_INTERNAL
    message_type get_message(double timeout, std::string & result,
			     message_type required_type = MSG_MAX);

    /// Send a message to the client.
    XAPIAN_VISIBILITY_INTERNAL
    void send_message(reply_type type, const std::string &message);

    /// Send a message to the client, with specific end_time.
    XAPIAN_VISIBILITY_INTERNAL
    void send_message(reply_type type, const std::string &message,
		      double end_time) {
	unsigned char type_as_char = static_cast<unsigned char>(type);
	RemoteConnection::send_message(type_as_char, message, end_time);
    }

    // all terms
    XAPIAN_VISIBILITY_INTERNAL
    void msg_allterms(const std::string & message);

    // get document
    XAPIAN_VISIBILITY_INTERNAL
    void msg_document(const std::string & message);

    // term exists?
    XAPIAN_VISIBILITY_INTERNAL
    void msg_termexists(const std::string & message);

    // get collection freq
    XAPIAN_VISIBILITY_INTERNAL
    void msg_collfreq(const std::string & message);

    // get termfreq
    XAPIAN_VISIBILITY_INTERNAL
    void msg_termfreq(const std::string & message);

    // get termfreq and collection freq
    XAPIAN_VISIBILITY_INTERNAL
    void msg_freqs(const std::string & message);

    // get value statistics
    XAPIAN_VISIBILITY_INTERNAL
    void msg_valuestats(const std::string & message);

    // keep alive
    XAPIAN_VISIBILITY_INTERNAL
    void msg_keepalive(const std::string & message);

    // get doclength
    XAPIAN_VISIBILITY_INTERNAL
    void msg_doclength(const std::string & message);

    // set the query; return the mset
    XAPIAN_VISIBILITY_INTERNAL
    void msg_query(const std::string & message);

    // get termlist
    XAPIAN_VISIBILITY_INTERNAL
    void msg_termlist(const std::string & message);

    // get postlist
    XAPIAN_VISIBILITY_INTERNAL
    void msg_postlist(const std::string & message);

    // get positionlist
    XAPIAN_VISIBILITY_INTERNAL
    void msg_positionlist(const std::string &message);

    // get positionlist count
    XAPIAN_VISIBILITY_INTERNAL
    void msg_positionlistcount(const std::string &message);

    // get write access
    XAPIAN_VISIBILITY_INTERNAL
    void msg_writeaccess(const std::string & message);

    // reopen
    XAPIAN_VISIBILITY_INTERNAL
    void msg_reopen(const std::string & message);

    // get updated doccount and avlength
    XAPIAN_VISIBILITY_INTERNAL
    void msg_update(const std::string &message);

    // commit
    XAPIAN_VISIBILITY_INTERNAL
    void msg_commit(const std::string & message);

    // cancel
    XAPIAN_VISIBILITY_INTERNAL
    void msg_cancel(const std::string &message);

    // add document
    XAPIAN_VISIBILITY_INTERNAL
    void msg_adddocument(const std::string & message);

    // delete document
    XAPIAN_VISIBILITY_INTERNAL
    void msg_deletedocument(const std::string & message);

    // delete document with unique term
    XAPIAN_VISIBILITY_INTERNAL
    void msg_deletedocumentterm(const std::string & message);

    // replace document
    XAPIAN_VISIBILITY_INTERNAL
    void msg_replacedocument(const std::string & message);

    // replace document with unique term
    XAPIAN_VISIBILITY_INTERNAL
    void msg_replacedocumentterm(const std::string & message);

    // get metadata
    XAPIAN_VISIBILITY_INTERNAL
    void msg_getmetadata(const std::string & message);

    // read metadata key list
    XAPIAN_VISIBILITY_INTERNAL
    void msg_metadatakeylist(const std::string & message);

    // set metadata
    XAPIAN_VISIBILITY_INTERNAL
    void msg_setmetadata(const std::string & message);

    // add a spelling
    XAPIAN_VISIBILITY_INTERNAL
    void msg_addspelling(const std::string & message);

    // remove a spelling
    XAPIAN_VISIBILITY_INTERNAL
    void msg_removespelling(const std::string & message);

    // get number of unique terms
    XAPIAN_VISIBILITY_INTERNAL
    void msg_uniqueterms(const std::string & message);

    // get max_wdf
    XAPIAN_VISIBILITY_INTERNAL
    void msg_wdfdocmax(const std::string& message);

    // reconstruct document text
    XAPIAN_VISIBILITY_INTERNAL
    void msg_reconstructtext(const std::string& message);

    // get synonyms for a term
    XAPIAN_VISIBILITY_INTERNAL
    void msg_synonymtermlist(const std::string& message);

    // get terms with an entry in synonym table, starting with a prefix
    XAPIAN_VISIBILITY_INTERNAL
    void msg_synonymkeylist(const std::string& message);

    // add a synonym
    XAPIAN_VISIBILITY_INTERNAL
    void msg_addsynonym(const std::string& message);

    // remove a synonym
    XAPIAN_VISIBILITY_INTERNAL
    void msg_removesynonym(const std::string& message);

    // clear synonyms for a term
    XAPIAN_VISIBILITY_INTERNAL
    void msg_clearsynonyms(const std::string& message);

  public:
    /** Construct a RemoteServer.
     *
     *  @param dbpaths	The paths to the Xapian databases to use.
     *  @param fdin	The file descriptor to read from.
     *  @param fdout	The file descriptor to write to (fdin and fdout may be
     *			the same).
     *  @param active_timeout_	Timeout for actions during a conversation
     *			(specified in seconds).
     *  @param idle_timeout_	Timeout while waiting for a new action from
     *			the client (specified in seconds).
     *  @param writable Should the database be opened for writing?
     */
    RemoteServer(const std::vector<std::string> &dbpaths,
		 int fdin, int fdout,
		 double active_timeout_,
		 double idle_timeout_,
		 bool writable = false);

    /// Destructor.
    ~RemoteServer();

    /** Repeatedly accept messages from the client and process them.
     *
     *  The loop continues until either the connection is closed, or a
     *  non-Xapian exception is thrown.
     */
    void run();

    /// Set the registry used for (un)serialisation.
    void set_registry(const Xapian::Registry & reg_) { reg = reg_; }
};

#endif // XAPIAN_INCLUDED_REMOTESERVER_H
