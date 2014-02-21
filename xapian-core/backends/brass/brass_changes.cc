/** @file brass_changes.cc
 * @brief Brass changesets
 */
/* Copyright 2014 Olly Betts
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

#include "brass_changes.h"

#include "brass_replicate_internal.h"
#include "fd.h"
#include "io_utils.h"
#include "pack.h"
#include "posixy_wrapper.h"
#include "stringutils.h"
#include "unaligned.h"
#include "xapian/constants.h"
#include "xapian/error.h"

#include <cstdlib>
#include <string>
#include "safeerrno.h"

using namespace std;

BrassChanges::~BrassChanges()
{
    if (changes_fd >= 0) {
	::close(changes_fd);
	string changes_tmp = changes_stem;
	changes_tmp += "tmp";
	io_unlink(changes_tmp);
    }
}

BrassChanges *
BrassChanges::start(brass_revision_number_t old_rev,
		    brass_revision_number_t rev,
		    int flags)
{
    if (rev == 0) {
	// Don't generate a changeset for the first revision.
	return NULL;
    }

    // Always check max_changesets for modification since last revision.
    const char *p = getenv("XAPIAN_MAX_CHANGESETS");
    if (p) {
	max_changesets = atoi(p);
    } else {
	max_changesets = 0;
    }

    if (max_changesets == 0)
	return NULL;

    string changes_tmp = changes_stem;
    changes_tmp += "tmp";
    changes_fd = posixy_open(changes_tmp.c_str(),
			     O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, 0666);
    if (changes_fd < 0) {
	string message = "Couldn't open changeset ";
	message += changes_tmp;
	message += " to write";
	throw Xapian::DatabaseError(message, errno);
    }

    // Write header for changeset file.
    string header = CHANGES_MAGIC_STRING;
    header += char(CHANGES_VERSION);
    pack_uint(header, old_rev);
    pack_uint(header, rev);

    if (flags & Xapian::DB_DANGEROUS) {
	header += '\x01'; // Changes can't be applied to a live database.
    } else {
	header += '\x00'; // Changes can be applied to a live database.
    }

    io_write(changes_fd, header.data(), header.size());
    // FIXME: save the block stream as a single zlib stream...

    // bool compressed = CHANGES_VERSION != 1; FIXME: always true for brass, but make optional?
    return this;
}

void
BrassChanges::write_block(const char * p, size_t len)
{
    io_write(changes_fd, p, len);
}

void
BrassChanges::commit(brass_revision_number_t new_rev, int flags)
{
    if (changes_fd < 0)
	return;

    io_write(changes_fd, "\xff", 1);

    string changes_tmp = changes_stem;
    changes_tmp += "tmp";

    if (!(flags & Xapian::DB_NO_SYNC) && !io_sync(changes_fd)) {
	int saved_errno = errno;
	(void)::close(changes_fd);
	changes_fd = -1;
	(void)unlink(changes_tmp.c_str());
	string m = changes_tmp;
	m += ": Failed to sync";
	throw Xapian::DatabaseError(m, saved_errno);
    }

    (void)::close(changes_fd);
    changes_fd = -1;

    string changes_file = changes_stem;
    changes_file += str(new_rev - 1); // FIXME: ?

    if (posixy_rename(changes_tmp.c_str(), changes_file.c_str()) < 0) {
	// With NFS, rename() failing may just mean that the server crashed
	// after successfully renaming, but before reporting this, and then
	// the retried operation fails.  So we need to check if the source
	// file still exists, which we do by calling unlink(), since we want
	// to remove the temporary file anyway.
	int saved_errno = errno;
	if (unlink(changes_tmp.c_str()) == 0 || errno != ENOENT) {
	    string m = changes_tmp;
	    m += ": Failed to rename to ";
	    m += changes_file;
	    throw Xapian::DatabaseError(m, saved_errno);
	}
    }

    if (new_rev <= max_changesets) {
	// We can't yet have max_changesets old changesets.
	return;
    }

    // Only remove old changesets if we successfully wrote a new changeset.
    // Start at the oldest changeset we know about, and stop at max_changesets
    // before new_rev.  If max_changesets is unchanged from the previous
    // commit and nothing went wrong, exactly one changeset file should be
    // deleted.
    brass_revision_number_t stop_changeset = new_rev - max_changesets;
    while (oldest_changeset < stop_changeset) {
	changes_file.resize(changes_stem.size());
	changes_file += str(oldest_changeset);
	(void)io_unlink(changes_file);
	++oldest_changeset;
    }
}

void
BrassChanges::check(const string & changes_file)
{
    FD fd(posixy_open(changes_file.c_str(), O_RDONLY | O_CLOEXEC, 0666));
    if (fd < 0) {
	string message = "Couldn't open changeset ";
	message += changes_file;
	throw Xapian::DatabaseError(message, errno);
    }

    char buf[10240];

    size_t n = io_read(fd, buf, sizeof(buf), CONST_STRLEN(CHANGES_MAGIC_STRING) + 4);
    if (memcmp(buf, CHANGES_MAGIC_STRING,
	       CONST_STRLEN(CHANGES_MAGIC_STRING)) != 0) {
	throw Xapian::DatabaseError("Changes file has wrong magic");
    }

    const char * p = buf + CONST_STRLEN(CHANGES_MAGIC_STRING);
    if (*p++ != CHANGES_VERSION) {
	throw Xapian::DatabaseError("Changes file has unknown version");
    }
    const char * end = buf + n;

    brass_revision_number_t old_rev, rev;
    if (!unpack_uint(&p, end, &old_rev))
	throw Xapian::DatabaseError("Changes file has bad old_rev");
    if (!unpack_uint(&p, end, &rev))
	throw Xapian::DatabaseError("Changes file has bad rev");
    if (rev <= old_rev)
	throw Xapian::DatabaseError("Changes file has rev <= old_rev");
    if (p == end || (*p != 0 && *p != 1))
	throw Xapian::DatabaseError("Changes file has bad dangerous flag");
    ++p;

    while (true) {
	n -= (p - buf);
	memmove(buf, p, n);
	n += io_read(fd, buf + n, sizeof(buf) - n, 0);

	if (n == 0)
	    throw Xapian::DatabaseError("Changes file truncated");

	p = buf;
	end = buf + n;

	unsigned char v = *p++;
	if (v == 0xff) {
	    if (p != end)
		throw Xapian::DatabaseError("Changes file - junk at end");
	    break;
	}
	unsigned table = (v & 0x7);
	bool base = (v & 0x80);
	v = (v >> 3) & 0x0f;
	if (table > 5)
	    throw Xapian::DatabaseError("Changes file - bad table code");
	if (base) {
	    // Base file.
	    if (v > 1)
		throw Xapian::DatabaseError("Changes file - bad base letter");
	    size_t base_len;
	    if (!unpack_uint(&p, end, &base_len))
		throw Xapian::DatabaseError("Changes file - bad base length");
	    if (base_len <= size_t(end - p)) {
		p += base_len;
	    } else {
		if (lseek(fd, base_len - (end - p), SEEK_CUR) == off_t(-1))
		    throw Xapian::DatabaseError("Changes file - base file data truncated");
		p = end = buf;
		n = 0;
	    }
	} else {
	    // Changed block.
	    if (v > 5)
		throw Xapian::DatabaseError("Changes file - bad block size");
	    unsigned block_size = 2048 << v;
	    uint4 block_number;
	    if (!unpack_uint(&p, end, &block_number))
		throw Xapian::DatabaseError("Changes file - bad block number");
	    uint4 block_rev = getint4(reinterpret_cast<const unsigned char *>(p), 0);
	    (void)block_rev; // FIXME: Sanity check value.
	    unsigned level = (unsigned char)p[4];
	    (void)level; // FIXME: Sanity check value.
	    if (block_size <= unsigned(end - p)) {
		p += block_size;
	    } else {
		if (lseek(fd, block_size - (end - p), SEEK_CUR) == off_t(-1))
		    throw Xapian::DatabaseError("Changes file - block data truncated");
		p = end = buf;
		n = 0;
	    }
	}
    }
}
