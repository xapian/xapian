/** @file brass_version.cc
 * @brief BrassVersion class
 */
/* Copyright (C) 2006,2007,2008,2009,2010 Olly Betts
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

#include "brass_io.h"

#include "safeerrno.h"

#include "debuglog.h"
#include "omassert.h"
#include "pack.h"
#include "stringutils.h" // For STRINGIZE() and CONST_STRLEN().
#include "str.h"

#ifdef __WIN32__
# include "msvc_posix_wrapper.h"
#endif

#include <cstdio> // For rename().
#include <cstring> // For memcmp() and memcpy().
#include <string>
#include "safedirent.h"
#include "safeerrno.h"
#include <sys/types.h>
#include "safesysstat.h"
#include "safefcntl.h"
#include "stringutils.h"
#include "utils.h"

#include "xapian/error.h"

#include "common/safeuuid.h"

using namespace std;

/// Brass format version (date of change):
#define BRASS_FORMAT_VERSION DATE_TO_VERSION(2010,3,7)

/// Convert date <-> version number.  Dates up to 2186-03-02 fit in 2 bytes.
#define DATE_TO_VERSION(Y,M,D) (((((Y)-2010)*12+((M)-1))*31)+(D)-1)
#define VERSION_TO_YEAR(V) ((V) / 31 / 12 + 2010)
#define VERSION_TO_MONTH(V) ((V) / 31 % 12 + 1)
#define VERSION_TO_DAY(V) ((V) % 31 + 1)

#define BRASS_VERSION_MAGIC_LEN 14
#define BRASS_VERSION_MAGIC_AND_VERSION_LEN 16

static const char BRASS_VERSION_MAGIC[BRASS_VERSION_MAGIC_AND_VERSION_LEN] = {
    '\x0f', '\x0d', 'X', 'a', 'p', 'i', 'a', 'n', ' ', 'B', 'r', 'a', 's', 's',
    (BRASS_FORMAT_VERSION >> 8) & 0xff, BRASS_FORMAT_VERSION & 0xff
};

void
BrassVersion::open_most_recent(const string & db_dir)
{
    LOGCALL_VOID(DB, "BrassVersion::open_most_recent", db_dir);
    DIR * dir = opendir(db_dir.c_str());
    if (rare(!dir))
	throw Xapian::DatabaseOpeningError("Couldn't open directory: " +
					   db_dir, errno);

    string newest;

    while (true) {
	errno = 0;
	struct dirent * entry = readdir(dir);
	if (!entry) {
	    closedir(dir);
	    if (errno == 0)
		break;
	    throw Xapian::DatabaseError("Couldn't read from directory: " +
					db_dir, errno);
	}

	const char * name = entry->d_name;
	if (name[0] != 'v')
	    continue;

	const char * p = name + 1;
	while (*p && p != name + 9 && C_islcxdigit(*p))
	    ++p;
	if (*p || p != name + 9)
	    continue;

	if (newest.empty() || memcmp(newest.data(), name + 1, 8) < 0)
	    newest.assign(name + 1, 8);
    }

    if (rare(newest.empty())) {
	// Empty database.
	blocksize = BRASS_DEFAULT_BLOCKSIZE;
	rev = 0;
	for (unsigned table_no = 0; table_no < Brass::MAX_; ++table_no) {
	    root[table_no] = static_cast<brass_block_t>(-1);
	}
	return;
    }

    unsigned long v = strtoul(newest.c_str(), NULL, 16);
    rev = (brass_revision_number_t)v;
    AssertEq((unsigned long)rev, v);
    string filename = db_dir;
    filename += "/v";
    filename += newest;
    read(filename);
}

void
BrassVersion::read(const string & filename)
{
    LOGCALL_VOID(DB, "BrassVersion::read", filename);
#ifndef __WIN32__
    int fd_in = open(filename.c_str(), O_RDONLY|O_BINARY);
#else
    int fd_in = msvc_posix_open(filename.c_str(), O_RDONLY|O_BINARY);
#endif
    if (rare(fd_in < 0)) {
	string msg = filename;
	msg += ": Failed to open brass revision file for reading";
	throw Xapian::DatabaseOpeningError(msg, errno);
    }

    fdcloser close_fd(fd_in);

    char buf[256];

    const char * p = buf;
    const char * end = p + brass_io_read(fd_in, buf, sizeof(buf), 33);

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

    blocksize = byte(*p++) << 11;

    for (unsigned table_no = 0; table_no < Brass::MAX_; ++table_no) {
	if (p != end) {
	    if (!unpack_uint(&p, end, &root[table_no])) {
		throw Xapian::DatabaseCorruptError("Rev file roots");
	    }
	} else {
	    root[table_no] = static_cast<brass_block_t>(-1);
	}
    }

    if (p != end)
	throw Xapian::DatabaseCorruptError("Rev file has junk at end");
}

const string
BrassVersion::write(const string & db_dir)
{
    LOGCALL(DB, const string, "BrassVersion::write", db_dir);

    string s(BRASS_VERSION_MAGIC, BRASS_VERSION_MAGIC_AND_VERSION_LEN);
    s.append((const char *)uuid, 16);
    s += char(blocksize >> 11);

    unsigned table_last = Brass::MAX_;
    while (table_last > 0 && new_root[table_last - 1] == brass_block_t(-1))
	--table_last;

    for (unsigned table_no = 0; table_no < table_last; ++table_no) {
	pack_uint(s, new_root[table_no]);
    }

    string tmpfile = db_dir;
    tmpfile += "/v.tmp";

#ifndef __WIN32__
    fd = open(tmpfile.c_str(), O_CREAT|O_TRUNC|O_WRONLY|O_BINARY, 0666);
#else
    fd = msvc_posix_open(tmpfile.c_str(), O_CREAT|O_TRUNC|O_WRONLY|O_BINARY, 0666);
#endif
    if (rare(fd < 0))
	throw Xapian::DatabaseOpeningError("Couldn't write new rev file: " + tmpfile,
					   errno);

    try {
	brass_io_write(fd, s.data(), s.size());
    } catch (...) {
	(void)close(fd);
	throw;
    }

    RETURN(tmpfile);
}

void
BrassVersion::sync(const string & db_dir, const string & tmpfile,
		   brass_revision_number_t new_rev)
{
    if (new_rev < rev)
	throw Xapian::DatabaseError("New revision " + str(new_rev) + " < old revision " + str(rev));

    string filename = db_dir;
    filename += "/v";
    char buf[9];
    sprintf(buf, "%08x", new_rev);
    filename.append(buf, 8);

    if (!brass_io_sync(fd)) {
	int save_errno = errno;
	(void)close(fd);
	throw Xapian::DatabaseOpeningError("Failed to sync new rev file: " + tmpfile,
					   save_errno);
    }

    if (close(fd) == -1) {
	int save_errno = errno;
	throw Xapian::DatabaseOpeningError("Failed to close new rev file: " + tmpfile,
					   save_errno);
    }

    if (rename(tmpfile.c_str(), filename.c_str()) < 0) {
	int save_errno = errno;
	throw Xapian::DatabaseOpeningError("Failed to rename new rev file: " + tmpfile,
					   save_errno);
    }

    rev = new_rev;
    memcpy(root, new_root, sizeof(root));
}

void
BrassVersion::create(unsigned blocksize_, const string & db_dir)
{
    blocksize = blocksize_;
    uuid_generate(uuid);
    for (unsigned table_no = 0; table_no < Brass::MAX_; ++table_no) {
	new_root[table_no] = static_cast<brass_block_t>(-1);
    }
    sync(db_dir, write(db_dir), rev);
}
