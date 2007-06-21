/** @file remoteprotocol.h
 *  @brief Remote protocol version and message numbers
 */
/* Copyright (C) 2006,2007 Olly Betts
 * Copyright (C) 2007 Lemur Consulting Ltd
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
#define XAPIAN_REMOTE_PROTOCOL_MAJOR_VERSION 30
#define XAPIAN_REMOTE_PROTOCOL_MINOR_VERSION 2

/* When we move to version 31:
 * + Remove MSG_DELETEDOCUMENT_PRE_30_2
 */

/// Message types (client -> server).
enum message_type {
    MSG_ALLTERMS,		// All Terms
    MSG_COLLFREQ,		// Get Collection Frequency
    MSG_DOCUMENT,		// Get Document
    MSG_TERMEXISTS,		// Term Exists?
    MSG_TERMFREQ,		// Get Term Frequency
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
    MSG_DELETEDOCUMENT_PRE_30_2,// Delete Document for < 30.2.
    MSG_DELETEDOCUMENTTERM,	// Delete Document by term
    MSG_FLUSH,			// Flush
    MSG_REPLACEDOCUMENT,	// Replace Document
    MSG_REPLACEDOCUMENTTERM,	// Replace Document by term
    MSG_GETMSET,		// Get MSet
    MSG_SHUTDOWN,		// Shutdown
    MSG_DELETEDOCUMENT,		// Delete Document
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
    REPLY_DOCLENGTH,		// Get Doc Length
    REPLY_RESULTS,		// Results (MSet)
    REPLY_STATS,		// Stats
    REPLY_TERMLIST,		// Get Termlist
    REPLY_POSITIONLIST,		// Get PositionList
    REPLY_POSTLISTSTART,	// Start of a postlist
    REPLY_POSTLISTITEM,		// Item in body of a postlist
    REPLY_UPDATE,		// Get Updated DocCount and AvLength
    REPLY_VALUE,		// Document Value
    REPLY_ADDDOCUMENT,		// Add Document
    REPLY_MAX
};

#endif // XAPIAN_INCLUDED_REMOTEPROTOCOL_H
