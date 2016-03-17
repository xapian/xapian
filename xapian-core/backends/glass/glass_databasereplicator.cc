/** @file glass_databasereplicator.cc
 * @brief Support for glass database replication
 */
/* Copyright 2008 Lemur Consulting Ltd
 * Copyright 2009,2010,2011,2012,2013,2014,2015,2016 Olly Betts
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

#include "glass_databasereplicator.h"

#include "xapian/error.h"

#include "../flint_lock.h"
#include "glass_defs.h"
#include "glass_replicate_internal.h"
#include "glass_version.h"
#include "compression_stream.h"
#include "debuglog.h"
#include "fd.h"
#include "internaltypes.h"
#include "io_utils.h"
#include "noreturn.h"
#include "pack.h"
#include "posixy_wrapper.h"
#include "net/remoteconnection.h"
#include "replicationprotocol.h"
#include "safeerrno.h"
#include "str.h"
#include "stringutils.h"

#include <algorithm>

XAPIAN_NORETURN(static void throw_connection_closed_unexpectedly());
static void
throw_connection_closed_unexpectedly()
{
    throw Xapian::NetworkError("Connection closed unexpectedly");
}

using namespace std;
using namespace Xapian;

static const char * dbnames =
	"/postlist." GLASS_TABLE_EXTENSION "\0"
	"/docdata." GLASS_TABLE_EXTENSION "\0\0"
	"/termlist." GLASS_TABLE_EXTENSION "\0"
	"/position." GLASS_TABLE_EXTENSION "\0"
	"/spelling." GLASS_TABLE_EXTENSION "\0"
	"/synonym." GLASS_TABLE_EXTENSION;

GlassDatabaseReplicator::GlassDatabaseReplicator(const string & db_dir_)
    : db_dir(db_dir_)
{
    std::fill_n(fds, sizeof(fds) / sizeof(fds[0]), -1);
}

void
GlassDatabaseReplicator::commit() const
{
    for (size_t i = 0; i != Glass::MAX_; ++i) {
	int fd = fds[i];
	if (fd >= 0) {
	    io_sync(fd);
#if 0 // FIXME: close or keep open?
	    close(fd);
	    fds[i] = -1;
#endif
	}
    }
}

GlassDatabaseReplicator::~GlassDatabaseReplicator()
{
    for (size_t i = 0; i != Glass::MAX_; ++i) {
	int fd = fds[i];
	if (fd >= 0) {
	    close(fd);
	}
    }
}

bool
GlassDatabaseReplicator::check_revision_at_least(const string & rev,
						 const string & target) const
{
    LOGCALL(DB, bool, "GlassDatabaseReplicator::check_revision_at_least", rev | target);

    glass_revision_number_t rev_val;
    glass_revision_number_t target_val;

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
GlassDatabaseReplicator::process_changeset_chunk_version(string & buf,
							 RemoteConnection & conn,
							 double end_time) const
{
    const char *ptr = buf.data();
    const char *end = ptr + buf.size();

    glass_revision_number_t rev;
    if (!unpack_uint(&ptr, end, &rev))
	throw NetworkError("Invalid revision in changeset");

    string::size_type size;
    if (!unpack_uint(&ptr, end, &size))
	throw NetworkError("Invalid version file size in changeset");

    // Get the new version file into buf.
    buf.erase(0, ptr - buf.data());
    int res = conn.get_message_chunk(buf, size, end_time);
    if (res <= 0) {
	if (res < 0)
	    throw_connection_closed_unexpectedly();
	throw NetworkError("Unexpected end of changeset (6)");
    }

    // Write size bytes from start of buf to new version file.
    string tmpfile = db_dir;
    tmpfile += "/v.rtmp";
    int fd = posixy_open(tmpfile.c_str(), O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, 0666);
    if (fd == -1) {
	string msg = "Failed to open ";
	msg += tmpfile;
	throw DatabaseError(msg, errno);
    }
    {
	FD closer(fd);
	io_write(fd, buf.data(), size);
	io_sync(fd);
    }
    string version_file = db_dir;
    version_file += "/iamglass";
    if (!io_tmp_rename(tmpfile, version_file)) {
	string msg("Couldn't create new version file ");
	msg += version_file;
	throw DatabaseError(msg, errno);
    }

    buf.erase(0, size);
}

void
GlassDatabaseReplicator::process_changeset_chunk_blocks(Glass::table_type table,
							unsigned v,
							string & buf,
							RemoteConnection & conn,
							double end_time) const
{
    const char *ptr = buf.data();
    const char *end = ptr + buf.size();

    unsigned int changeset_blocksize = 2048 << v;
    if (changeset_blocksize > 65536 ||
	(changeset_blocksize & (changeset_blocksize - 1))) {
	throw NetworkError("Invalid blocksize in changeset");
    }
    uint4 block_number;
    if (!unpack_uint(&ptr, end, &block_number))
	throw NetworkError("Invalid block number in changeset");

    buf.erase(0, ptr - buf.data());

    int fd = fds[table];
    if (fd == -1) {
	string db_path = db_dir;
	db_path += dbnames + table * (11 + CONST_STRLEN(GLASS_TABLE_EXTENSION));
	fd = posixy_open(db_path.c_str(), O_WRONLY | O_CREAT | O_CLOEXEC, 0666);
	if (fd == -1) {
	    string msg = "Failed to open ";
	    msg += db_path;
	    throw DatabaseError(msg, errno);
	}
	fds[table] = fd;
    }

    int res = conn.get_message_chunk(buf, changeset_blocksize, end_time);
    if (res <= 0) {
	if (res < 0)
	    throw_connection_closed_unexpectedly();
	throw NetworkError("Unexpected end of changeset (4)");
    }

    io_write_block(fd, buf.data(), changeset_blocksize, block_number);
    buf.erase(0, changeset_blocksize);
}

string
GlassDatabaseReplicator::apply_changeset_from_conn(RemoteConnection & conn,
						   double end_time,
						   bool valid) const
{
    LOGCALL(DB, string, "GlassDatabaseReplicator::apply_changeset_from_conn", conn | end_time | valid);

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
    const char *ptr = buf.data();
    const char *end = ptr + buf.size();
    // Check the magic string.
    if (!startswith(buf, CHANGES_MAGIC_STRING)) {
	throw NetworkError("Invalid ChangeSet magic string");
    }
    ptr += CONST_STRLEN(CHANGES_MAGIC_STRING);
    if (ptr == end)
	throw NetworkError("Couldn't read a valid version number from changeset");
    unsigned int changes_version = *ptr++;
    if (changes_version != CHANGES_VERSION)
	throw NetworkError("Unsupported changeset version");

    glass_revision_number_t startrev;
    glass_revision_number_t endrev;

    if (!unpack_uint(&ptr, end, &startrev))
	throw NetworkError("Couldn't read a valid start revision from changeset");
    if (!unpack_uint(&ptr, end, &endrev))
	throw NetworkError("Couldn't read a valid end revision from changeset");

    if (endrev <= startrev)
	throw NetworkError("End revision in changeset is not later than start revision");

    if (ptr == end)
	throw NetworkError("Unexpected end of changeset (1)");

    if (valid) {
	// Check the revision number.
	// If the database was not known to be valid, we cannot
	// reliably determine its revision number, so must skip this
	// check.
	GlassVersion version_file(db_dir);
	version_file.read();
	if (startrev != version_file.get_revision())
	    throw NetworkError("Changeset supplied is for wrong revision number");
    }

    unsigned char changes_type = *ptr++;
    if (changes_type != 0) {
	throw NetworkError("Unsupported changeset type: " + str(changes_type));
	// FIXME - support changes of type 1, produced when DANGEROUS mode is
	// on.
    }

    // Clear the bits of the buffer which have been read.
    buf.erase(0, ptr - buf.data());

    // Read the items from the changeset.
    while (true) {
	if (conn.get_message_chunk(buf, REASONABLE_CHANGESET_SIZE, end_time) < 0)
	    throw_connection_closed_unexpectedly();
	ptr = buf.data();
	end = ptr + buf.size();
	if (ptr == end)
	    throw NetworkError("Unexpected end of changeset (3)");

	// Read the type of the next chunk of data
	// chunk type can be (in binary):
	//
	// 11111111 - last chunk
	// 11111110 - version file
	// 00BBBTTT - table block:
	//   Block size = (2048<<BBB) BBB=0..5; Table TTT=0..(Glass::MAX_-1)
	unsigned char chunk_type = *ptr++;
	if (chunk_type == 0xff)
	    break;
	if (chunk_type == 0xfe) {
	    // Version file.
	    buf.erase(0, ptr - buf.data());
	    process_changeset_chunk_version(buf, conn, end_time);
	    continue;
	}
	size_t table_code = (chunk_type & 0x07);
	if (table_code >= Glass::MAX_)
	    throw NetworkError("Bad table code in changeset file");
	Glass::table_type table = static_cast<Glass::table_type>(table_code);
	unsigned char v = (chunk_type >> 3) & 0x0f;

	// Process the chunk
	buf.erase(0, ptr - buf.data());
	process_changeset_chunk_blocks(table, v, buf, conn, end_time);
    }

    if (ptr != end)
	throw NetworkError("Junk found at end of changeset");

    buf.resize(0);
    pack_uint(buf, endrev);

    commit();

    RETURN(buf);
}

string
GlassDatabaseReplicator::get_uuid() const
{
    LOGCALL(DB, string, "GlassDatabaseReplicator::get_uuid", NO_ARGS);
    GlassVersion version_file(db_dir);
    try {
	version_file.read();
    } catch (const Xapian::DatabaseError &) {
	RETURN(string());
    }
    RETURN(version_file.get_uuid_string());
}
