/** @file remoteprotocol.h
 *  @brief Remote protocol version and message numbers
 */
/* Copyright (C) 2006,2007,2008,2009,2010 Olly Betts
 * Copyright (C) 2007,2010 Lemur Consulting Ltd
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

#ifndef XAPIAN_INCLUDED_REMOTEPROTOCOL_H
#define XAPIAN_INCLUDED_REMOTEPROTOCOL_H

// Versions:
// 21: Overhauled remote backend supporting WritableDatabase
// 22: Lossless double serialisation
// 23: Support get_lastdocid() on remote databases
// 24: Support for OP_VALUE_RANGE in query serialisation
// 25: Support for delete_document and replace_document with unique term
// 26: Tweak delete_document with unique term; delta encode rset and termpos
// 27: Support for postlists (always passes the whole list across)
// 28: Pass document length in reply to MSG_TERMLIST
// 29: Serialisation of Xapian::Error includes error_string
// 30: Add minor protocol version numbers, to reduce need for client upgrades
// 30.1: Pass the prefix parameter for MSG_ALLTERMS, and use it.
// 30.2: New REPLY_DELETEDOCUMENT returns MSG_DONE to allow exceptions.
// 30.3: New MSG_GETMSET which passes check_at_least parameter.
// 30.4: New query operator OP_SCALE_WEIGHT.
// 30.5: New MSG_GETMSET which expects MSet's percent_factor to be returned.
// 30.6: Support for OP_VALUE_GE and OP_VALUE_LE in query serialisation
// 31: 1.1.0 Clean up for Xapian 1.1.0
// 32: 1.1.1 Serialise termfreq and reltermfreqs together in serialise_stats.
// 33: 1.1.3 Support for passing matchspies over the remote connection.
// 34: 1.1.4 Support for metadata over with remote databases.
// 35: 1.1.5 Support for add_spelling() and remove_spelling().
// 35.1: 1.2.4 Support for metadata_keys_begin().
#define XAPIAN_REMOTE_PROTOCOL_MAJOR_VERSION 35
#define XAPIAN_REMOTE_PROTOCOL_MINOR_VERSION 1

/** Message types (client -> server).
 *
 *  When modifying this list, you probably need to update the array of function
 *  pointers in net/remoteserver.cc too.
 */
enum message_type {
    MSG_ALLTERMS,		// All Terms
    MSG_COLLFREQ,		// Get Collection Frequency
    MSG_DOCUMENT,		// Get Document
    MSG_TERMEXISTS,		// Term Exists?
    MSG_TERMFREQ,		// Get Term Frequency
    MSG_VALUESTATS,		// Get value statistics
    MSG_KEEPALIVE,		// Keep-alive
    MSG_DOCLENGTH,		// Get Doc Length
    MSG_QUERY,			// Run Query
    MSG_TERMLIST,		// Get TermList
    MSG_POSITIONLIST,		// Get PositionList
    MSG_POSTLIST,		// Get PostList
    MSG_REOPEN,			// Reopen
    MSG_UPDATE,			// Get Updated DocCount and AvLength
    MSG_ADDDOCUMENT,		// Add Document
    MSG_CANCEL,			// Cancel
    MSG_DELETEDOCUMENTTERM,	// Delete Document by term
    MSG_COMMIT,			// Commit
    MSG_REPLACEDOCUMENT,	// Replace Document
    MSG_REPLACEDOCUMENTTERM,	// Replace Document by term
    MSG_DELETEDOCUMENT,		// Delete Document
    MSG_WRITEACCESS,		// Upgrade to WritableDatabase
    MSG_GETMETADATA,		// Get metadata
    MSG_SETMETADATA,		// Set metadata
    MSG_ADDSPELLING,		// Add a spelling
    MSG_REMOVESPELLING,		// Remove a spelling
    MSG_GETMSET,		// Get MSet
    MSG_SHUTDOWN,		// Shutdown
    MSG_METADATAKEYLIST,	// Iterator for metadata keys
    MSG_MAX
};

/// Reply types (server -> client).
enum reply_type {
    REPLY_GREETING,		// Greeting
    REPLY_EXCEPTION,		// Exception
    REPLY_DONE,			// Done sending list
    REPLY_ALLTERMS,		// All Terms
    REPLY_COLLFREQ,		// Get Collection Frequency
    REPLY_DOCDATA,		// Get Document
    REPLY_TERMDOESNTEXIST,	// Term Doesn't Exist
    REPLY_TERMEXISTS,		// Term Exists
    REPLY_TERMFREQ,		// Get Term Frequency
    REPLY_VALUESTATS,		// Value statistics
    REPLY_DOCLENGTH,		// Get Doc Length
    REPLY_STATS,		// Stats
    REPLY_TERMLIST,		// Get Termlist
    REPLY_POSITIONLIST,		// Get PositionList
    REPLY_POSTLISTSTART,	// Start of a postlist
    REPLY_POSTLISTITEM,		// Item in body of a postlist
    REPLY_UPDATE,		// Get Updated DocCount and AvLength
    REPLY_VALUE,		// Document Value
    REPLY_ADDDOCUMENT,		// Add Document
    REPLY_RESULTS,		// Results (MSet)
    REPLY_METADATA,		// Metadata
    REPLY_METADATAKEYLIST,	// Iterator for metadata keys
    REPLY_MAX
};

#endif // XAPIAN_INCLUDED_REMOTEPROTOCOL_H
