/** @file replicationprotocol.h
 *  @brief Replication protocol version and message numbers
 */
/* Copyright (C) 2008 Lemur Consulting Ltd
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

#ifndef XAPIAN_INCLUDED_REPLICATIONPROTOCOL_H
#define XAPIAN_INCLUDED_REPLICATIONPROTOCOL_H

// Versions:
// 1: Initial support
#define XAPIAN_REPLICATION_PROTOCOL_MAJOR_VERSION 1
#define XAPIAN_REPLICATION_PROTOCOL_MINOR_VERSION 0

// Reply types (master -> slave)
enum replicate_reply_type {
    REPL_REPLY_END_OF_CHANGES,	// No more changes to transfer.
    REPL_REPLY_FAIL,		// Couldn't generate full set of changes.
    REPL_REPLY_DB_HEADER,	// The start of a whole DB copy.
    REPL_REPLY_DB_FILENAME,	// The name of a file in a DB copy.
    REPL_REPLY_DB_FILEDATA,	// Contents of a file in a DB copy.
    REPL_REPLY_DB_FOOTER,	// End of a whole DB copy.
    REPL_REPLY_CHANGESET	// A changeset file is being sent.
};

// The maximum number of copies of a database to send in a single conversation.
// If more copies than this are required, a REPL_REPLY_FAIL message will be
// sent.
#define MAX_DB_COPIES_PER_CONVERSATION 5

#endif // XAPIAN_INCLUDED_REPLICATIONPROTOCOL_H
