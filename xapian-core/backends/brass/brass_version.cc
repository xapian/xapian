/** @file brass_version.cc
 * @brief BrassVersion class
 */
/* Copyright (C) 2006,2007,2008,2009,2010,2013,2014 Olly Betts
 * Copyright (C) 2011 Dan Colish
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

#include <config.h>

#include "brass_version.h"

#include "debuglog.h"
#include "fd.h"
#include "io_utils.h"
#include "omassert.h"
#include "pack.h"
#include "posixy_wrapper.h"
#include "stringutils.h" // For STRINGIZE() and CONST_STRLEN().

#include <cstring> // For memcmp().
#include <string>
#include "safeerrno.h"
#include <sys/types.h>
#include "safesysstat.h"
#include "safefcntl.h"
#include "stringutils.h"

#include "common/safeuuid.h"

#include "xapian/constants.h"
#include "xapian/error.h"

using namespace std;

/// Brass format version (date of change):
#define BRASS_FORMAT_VERSION DATE_TO_VERSION(2014,7,28)
// 2014,7,28  1.3.2 Order position table by term first; merge base files into revision file

/// Convert date <-> version number.  Dates up to 2190-03-02 fit in 2 bytes.
#define DATE_TO_VERSION(Y,M,D) (((((Y)-2014)*12+((M)-1))*31)+(D)-1)
#define VERSION_TO_YEAR(V) ((V) / 31 / 12 + 2014)
#define VERSION_TO_MONTH(V) ((V) / 31 % 12 + 1)
#define VERSION_TO_DAY(V) ((V) % 31 + 1)

#define BRASS_VERSION_MAGIC_LEN 14
#define BRASS_VERSION_MAGIC_AND_VERSION_LEN 16

static const char BRASS_VERSION_MAGIC[BRASS_VERSION_MAGIC_AND_VERSION_LEN] = {
    '\x0f', '\x0d', 'X', 'a', 'p', 'i', 'a', 'n', ' ', 'B', 'r', 'a', 's', 's',
    char((BRASS_FORMAT_VERSION >> 8) & 0xff), char(BRASS_FORMAT_VERSION & 0xff)
};

void
BrassVersion::read()
{
    LOGCALL_VOID(DB, "BrassVersion::read", NO_ARGS);
    string filename = db_dir;
    filename += "/iambrass";
    int fd_in = posixy_open(filename.c_str(), O_RDONLY|O_BINARY);
    if (rare(fd_in < 0)) {
	string msg = filename;
	msg += ": Failed to open brass revision file for reading";
	throw Xapian::DatabaseOpeningError(msg, errno);
    }

    FD close_fd(fd_in);

    char buf[256];

    const char * p = buf;
    const char * end = p + io_read(fd_in, buf, sizeof(buf), 33);

    if (memcmp(buf, BRASS_VERSION_MAGIC, BRASS_VERSION_MAGIC_LEN) != 0)
	throw Xapian::DatabaseCorruptError("Rev file magic incorrect");

    unsigned version;
    version = static_cast<unsigned char>(buf[BRASS_VERSION_MAGIC_LEN]);
    version <<= 8;
    version |= static_cast<unsigned char>(buf[BRASS_VERSION_MAGIC_LEN + 1]);
    if (version != BRASS_FORMAT_VERSION) {
	char datebuf[9];
	string msg = filename;
	msg += ": Database is format version ";
	sprintf(datebuf, "%04d%02d%02d",
		VERSION_TO_YEAR(version),
		VERSION_TO_MONTH(version),
		VERSION_TO_DAY(version));
	msg += datebuf;
	msg += " but I only understand ";
	sprintf(datebuf, "%04d%02d%02d",
		VERSION_TO_YEAR(BRASS_FORMAT_VERSION),
		VERSION_TO_MONTH(BRASS_FORMAT_VERSION),
		VERSION_TO_DAY(BRASS_FORMAT_VERSION));
	msg += datebuf;
	throw Xapian::DatabaseVersionError(msg);
    }

    p += BRASS_VERSION_MAGIC_AND_VERSION_LEN;
    memcpy((void*)uuid, p, 16);
    p += 16;

    if (!unpack_uint(&p, end, &rev))
	throw Xapian::DatabaseCorruptError("Rev file failed to decode revision");

    for (unsigned table_no = 0; table_no < Brass::MAX_; ++table_no) {
	if (!root[table_no].unserialise(&p, end)) {
	    throw Xapian::DatabaseCorruptError("Rev file root_info missing");
	}
	old_root[table_no] = root[table_no];
    }

    if (p != end)
	throw Xapian::DatabaseCorruptError("Rev file has junk at end");
}

void
BrassVersion::cancel()
{
    LOGCALL_VOID(DB, "BrassVersion::cancel", NO_ARGS);
    for (unsigned table_no = 0; table_no < Brass::MAX_; ++table_no) {
	root[table_no] = old_root[table_no];
    }
}

const string
BrassVersion::write(brass_revision_number_t new_rev, int flags)
{
    LOGCALL(DB, const string, "BrassVersion::write", new_rev|flags);

    string s(BRASS_VERSION_MAGIC, BRASS_VERSION_MAGIC_AND_VERSION_LEN);
    s.append((const char *)uuid, 16);

    pack_uint(s, new_rev);

    for (unsigned table_no = 0; table_no < Brass::MAX_; ++table_no) {
	root[table_no].serialise(s);
    }

    string tmpfile = db_dir;
    // In dangerous mode, just write the new version file in place.
    if (flags & Xapian::DB_DANGEROUS)
	tmpfile += "/iambrass";
    else
	tmpfile += "/v.tmp";

    fd = posixy_open(tmpfile.c_str(), O_CREAT|O_TRUNC|O_WRONLY|O_BINARY, 0666);
    if (rare(fd < 0))
	throw Xapian::DatabaseOpeningError("Couldn't write new rev file: " + tmpfile,
					   errno);

    try {
	io_write(fd, s.data(), s.size());
    } catch (...) {
	(void)close(fd);
	throw;
    }

    if (changes) {
	string changes_buf;
	changes_buf += '\xfe';
	pack_uint(changes_buf, new_rev);
	pack_uint(changes_buf, s.size());
	changes->write_block(changes_buf);
	changes->write_block(s);
    }

    if (flags & Xapian::DB_DANGEROUS)
	tmpfile = string();
    RETURN(tmpfile);
}

bool
BrassVersion::sync(const string & tmpfile,
		   brass_revision_number_t new_rev, int flags)
{
    Assert(new_rev > rev || rev == 0);

    if ((flags & Xapian::DB_NO_SYNC) == 0 &&
	((flags & Xapian::DB_FULL_SYNC) ?
	  !io_full_sync(fd) :
	  !io_sync(fd))) {
	int save_errno = errno;
	(void)close(fd);
	if (!tmpfile.empty())
	    (void)unlink(tmpfile.c_str());
	errno = save_errno;
	return false;
    }

    if (close(fd) != 0) {
	if (!tmpfile.empty()) {
	    int save_errno = errno;
	    (void)unlink(tmpfile.c_str());
	    errno = save_errno;
	}
	return false;
    }

    if (!tmpfile.empty()) {
	string filename = db_dir;
	filename += "/iambrass";

	if (posixy_rename(tmpfile.c_str(), filename.c_str()) < 0) {
	    // Over NFS, rename() can sometimes report failure when the
	    // operation succeeded, so in this case we try to unlink the source
	    // to check if the rename really failed.
	    int save_errno = errno;
	    if (unlink(tmpfile.c_str()) == 0 || errno != ENOENT) {
		errno = save_errno;
		return false;
	    }
	}
    }

    for (unsigned table_no = 0; table_no < Brass::MAX_; ++table_no) {
	old_root[table_no] = root[table_no];
    }

    rev = new_rev;
    return true;
}

void
BrassVersion::create(unsigned blocksize, int flags)
{
    AssertRel(blocksize,>=,2048);
    uuid_generate(uuid);
    for (unsigned table_no = 0; table_no < Brass::MAX_; ++table_no) {
	root[table_no].init(blocksize);
    }
    sync(write(rev, flags), rev, flags);
}

namespace Brass {

void
RootInfo::init(unsigned blocksize_)
{
    AssertRel(blocksize_,>=,2048);
    root = 0;
    level = 0;
    num_entries = 0;
    root_is_fake = true;
    sequential_mode = true;
    blocksize = blocksize_;
    fl_serialised.resize(0);
}

void
RootInfo::serialise(string &s) const
{
    pack_uint(s, root);
    unsigned val = level << 2;
    if (sequential_mode) val |= 0x02;
    if (root_is_fake) val |= 0x01;
    pack_uint(s, val);
    pack_uint(s, num_entries);
    pack_uint(s, blocksize >> 11);
    pack_string(s, fl_serialised);
}

bool
RootInfo::unserialise(const char ** p, const char * end)
{
    unsigned val;
    if (!unpack_uint(p, end, &root) ||
	!unpack_uint(p, end, &val) ||
	!unpack_uint(p, end, &num_entries) ||
	!unpack_uint(p, end, &blocksize) ||
	!unpack_string(p, end, fl_serialised)) return false;
    level = val >> 2;
    sequential_mode = val & 0x02;
    root_is_fake = val & 0x01;
    blocksize <<= 11;
    AssertRel(blocksize,>=,2048);
    return true;
}

}
