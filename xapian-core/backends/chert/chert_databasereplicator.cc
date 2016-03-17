/** @file chert_databasereplicator.cc
 * @brief Support for chert database replication
 */
/* Copyright 2008 Lemur Consulting Ltd
 * Copyright 2009,2010,2011,2012,2014,2015,2016 Olly Betts
 * Copyright 2010 Richard Boulton
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301
 * USA
 */

#include <config.h>

#include "chert_databasereplicator.h"

#include "xapian/error.h"

#include "../flint_lock.h"
#include "chert_record.h"
#include "chert_replicate_internal.h"
#include "chert_types.h"
#include "chert_version.h"
#include "debuglog.h"
#include "fd.h"
#include "filetests.h"
#include "io_utils.h"
#include "noreturn.h"
#include "pack.h"
#include "posixy_wrapper.h"
#include "net/remoteconnection.h"
#include "replicate_utils.h"
#include "replicationprotocol.h"
#include "safeerrno.h"
#include "str.h"
#include "stringutils.h"

#include <cstdlib>

XAPIAN_NORETURN(static void throw_connection_closed_unexpectedly());
static void
throw_connection_closed_unexpectedly()
{
    throw Xapian::NetworkError("Connection closed unexpectedly");
}

using namespace std;
using namespace Xapian;

ChertDatabaseReplicator::ChertDatabaseReplicator(const string & db_dir_)
	: db_dir(db_dir_),
	  max_changesets(0)
{
    const char *p = getenv("XAPIAN_MAX_CHANGESETS");
    if (p)
	max_changesets = atoi(p);
}

bool
ChertDatabaseReplicator::check_revision_at_least(const string & rev,
						 const string & target) const
{
    LOGCALL(DB, bool, "ChertDatabaseReplicator::check_revision_at_least", rev | target);

    chert_revision_number_t rev_val;
    chert_revision_number_t target_val;

    const char * ptr = rev.data();
    const char * end = ptr + rev.size();
    if (!unpack_uint(&ptr, end, &rev_val)) {
	throw NetworkError("Invalid revision string supplied to check_revision_at_least");
    }

    ptr = target.data();
    end = ptr + target.size();
    if (!unpack_uint(&ptr, end, &target_val)) {
	throw NetworkError("Invalid revision string supplied to check_revision_at_least");
    }

    RETURN(rev_val >= target_val);
}

void
ChertDatabaseReplicator::process_changeset_chunk_base(const string & tablename,
						      string & buf,
						      RemoteConnection & conn,
						      double end_time,
						      int changes_fd) const
{
    const char *ptr = buf.data();
    const char *end = ptr + buf.size();

    // Get the letter
    char letter = ptr[0];
    if (letter != 'A' && letter != 'B')
	throw NetworkError("Invalid base file letter in changeset");
    ++ptr;


    // Get the base size
    if (ptr == end)
	throw NetworkError("Unexpected end of changeset (5)");
    string::size_type base_size;
    if (!unpack_uint(&ptr, end, &base_size))
	throw NetworkError("Invalid base file size in changeset");

    // Get the new base file into buf.
    write_and_clear_changes(changes_fd, buf, ptr - buf.data());
    int res = conn.get_message_chunk(buf, base_size, end_time);
    if (res <= 0) {
	if (res < 0)
	    throw_connection_closed_unexpectedly();
	throw NetworkError("Unexpected end of changeset (6)");
    }

    // Write base_size bytes from start of buf to base file for tablename
    string tmp_path = db_dir + "/" + tablename + "tmp";
    string base_path = db_dir + "/" + tablename + ".base" + letter;
    int fd = posixy_open(tmp_path.c_str(), O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, 0666);
    if (fd == -1) {
	string msg = "Failed to open ";
	msg += tmp_path;
	throw DatabaseError(msg, errno);
    }
    {
	FD closer(fd);

	io_write(fd, buf.data(), base_size);
	io_sync(fd);
    }

    // Finish writing the changeset before moving the base file into place.
    write_and_clear_changes(changes_fd, buf, base_size);

    if (!io_tmp_rename(tmp_path, base_path)) {
	string msg("Couldn't update base file ");
	msg += tablename;
	msg += ".base";
	msg += letter;
	throw DatabaseError(msg, errno);
    }
}

void
ChertDatabaseReplicator::process_changeset_chunk_blocks(const string & tablename,
							string & buf,
							RemoteConnection & conn,
							double end_time,
							int changes_fd) const
{
    const char *ptr = buf.data();
    const char *end = ptr + buf.size();

    unsigned int changeset_blocksize;
    if (!unpack_uint(&ptr, end, &changeset_blocksize))
	throw NetworkError("Invalid blocksize in changeset");
    write_and_clear_changes(changes_fd, buf, ptr - buf.data());

    string db_path = db_dir + "/" + tablename + ".DB";
    int fd = posixy_open(db_path.c_str(), O_WRONLY | O_CREAT | O_CLOEXEC, 0666);
    if (fd == -1) {
	string msg = "Failed to open ";
	msg += db_path;
	throw DatabaseError(msg, errno);
    }
    {
	FD closer(fd);

	while (true) {
	    if (conn.get_message_chunk(buf, REASONABLE_CHANGESET_SIZE, end_time) < 0)
		throw_connection_closed_unexpectedly();

	    ptr = buf.data();
	    end = ptr + buf.size();

	    uint4 block_number;
	    if (!unpack_uint(&ptr, end, &block_number))
		throw NetworkError("Invalid block number in changeset");
	    write_and_clear_changes(changes_fd, buf, ptr - buf.data());
	    if (block_number == 0)
		break;
	    --block_number;

	    int res = conn.get_message_chunk(buf, changeset_blocksize, end_time);
	    if (res <= 0) {
		if (res < 0)
		    throw_connection_closed_unexpectedly();
		throw NetworkError("Incomplete block in changeset");
	    }

	    io_write_block(fd, buf.data(), changeset_blocksize, block_number);

	    write_and_clear_changes(changes_fd, buf, changeset_blocksize);
	}
	io_sync(fd);
    }
}

string
ChertDatabaseReplicator::apply_changeset_from_conn(RemoteConnection & conn,
						   double end_time,
						   bool valid) const
{
    LOGCALL(DB, string, "ChertDatabaseReplicator::apply_changeset_from_conn", conn | end_time | valid);

    // Lock the database to perform modifications.
    FlintLock lock(db_dir);
    string explanation;
    FlintLock::reason why = lock.lock(true, false, explanation);
    if (why != FlintLock::SUCCESS) {
	lock.throw_databaselockerror(why, db_dir, explanation);
    }

    int type = conn.get_message_chunked(end_time);
    if (type < 0)
	throw_connection_closed_unexpectedly();
    AssertEq(type, REPL_REPLY_CHANGESET);

    string buf;
    // Read enough to be certain that we've got the header part of the
    // changeset.

    if (conn.get_message_chunk(buf, REASONABLE_CHANGESET_SIZE, end_time) < 0)
	throw_connection_closed_unexpectedly();
    // Check the magic string.
    if (!startswith(buf, CHANGES_MAGIC_STRING)) {
	throw NetworkError("Invalid ChangeSet magic string");
    }
    const char *ptr = buf.data();
    const char *end = ptr + buf.size();
    ptr += CONST_STRLEN(CHANGES_MAGIC_STRING);

    unsigned int changes_version;
    if (!unpack_uint(&ptr, end, &changes_version))
	throw NetworkError("Couldn't read a valid version number from changeset");
    if (changes_version != CHANGES_VERSION)
	throw NetworkError("Unsupported changeset version");

    chert_revision_number_t startrev;
    chert_revision_number_t endrev;

    if (!unpack_uint(&ptr, end, &startrev))
	throw NetworkError("Couldn't read a valid start revision from changeset");
    if (!unpack_uint(&ptr, end, &endrev))
	throw NetworkError("Couldn't read a valid end revision from changeset");

    if (endrev <= startrev)
	throw NetworkError("End revision in changeset is not later than start revision");

    if (ptr == end)
	throw NetworkError("Unexpected end of changeset (1)");

    FD changes_fd;
    string changes_name;
    if (max_changesets > 0) {
	changes_fd = create_changeset_file(db_dir, "changes" + str(startrev),
					   changes_name);
    }

    if (valid) {
	// Check the revision number.
	// If the database was not known to be valid, we cannot
	// reliably determine its revision number, so must skip this
	// check.
	ChertRecordTable record_table(db_dir, true);
	record_table.open();
	if (startrev != record_table.get_open_revision_number())
	    throw NetworkError("Changeset supplied is for wrong revision number");
    }

    unsigned char changes_type = ptr[0];
    if (changes_type != 0) {
	throw NetworkError("Unsupported changeset type: " + str(changes_type));
	// FIXME - support changes of type 1, produced when DANGEROUS mode is
	// on.
    }

    // Write and clear the bits of the buffer which have been read.
    write_and_clear_changes(changes_fd, buf, ptr + 1 - buf.data());

    // Read the items from the changeset.
    while (true) {
	if (conn.get_message_chunk(buf, REASONABLE_CHANGESET_SIZE, end_time) < 0)
	    throw_connection_closed_unexpectedly();
	ptr = buf.data();
	end = ptr + buf.size();

	// Read the type of the next chunk of data
	if (ptr == end)
	    throw NetworkError("Unexpected end of changeset (2)");
	unsigned char chunk_type = ptr[0];
	++ptr;
	if (chunk_type == 0)
	    break;

	// Get the tablename.
	string tablename;
	if (!unpack_string(&ptr, end, tablename))
	    throw NetworkError("Unexpected end of changeset (3)");
	if (tablename.empty())
	    throw NetworkError("Missing tablename in changeset");
	if (tablename.find_first_not_of("abcdefghijklmnopqrstuvwxyz") !=
	    tablename.npos)
	    throw NetworkError("Invalid character in tablename in changeset");

	// Process the chunk
	if (ptr == end)
	    throw NetworkError("Unexpected end of changeset (4)");
	write_and_clear_changes(changes_fd, buf, ptr - buf.data());

	switch (chunk_type) {
	    case 1:
		process_changeset_chunk_base(tablename, buf, conn, end_time,
					     changes_fd);
		break;
	    case 2:
		process_changeset_chunk_blocks(tablename, buf, conn, end_time,
					       changes_fd);
		break;
	    default:
		throw NetworkError("Unrecognised item type in changeset");
	}
    }
    chert_revision_number_t reqrev;
    if (!unpack_uint(&ptr, end, &reqrev))
	throw NetworkError("Couldn't read a valid required revision from changeset");
    if (reqrev < endrev)
	throw NetworkError("Required revision in changeset is earlier than end revision");
    if (ptr != end)
	throw NetworkError("Junk found at end of changeset");

    write_and_clear_changes(changes_fd, buf, buf.size());
    pack_uint(buf, reqrev);
    RETURN(buf);
}

string
ChertDatabaseReplicator::get_uuid() const
{
    LOGCALL(DB, string, "ChertDatabaseReplicator::get_uuid", NO_ARGS);
    ChertVersion version_file(db_dir);
    try {
	version_file.read_and_check();
    } catch (const Xapian::DatabaseError &) {
	RETURN(string());
    }
    RETURN(version_file.get_uuid_string());
}
