/** @file flint_version.cc
 * @brief FlintVersion class
 */
/* Copyright (C) 2006,2007,2008,2009,2013 Olly Betts
 * Copyright 2010 Richard Boulton
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

#include "safeerrno.h"

#include <xapian/error.h>

#include "flint_version.h"
#include "io_utils.h"
#include "str.h"
#include "stringutils.h" // For STRINGIZE() and CONST_STRLEN().
#include "utils.h"

#ifdef __WIN32__
# include "msvc_posix_wrapper.h"
#endif

#include <cstdio> // For rename().
#include <cstring> // For memcmp().
#include <string>

#include "common/safeuuid.h"

using namespace std;

// YYYYMMDDX where X allows multiple format revisions in a day
#define FLINT_VERSION 200709120
// 200709120 1.0.3 Database::get_metadata(), WritableDatabase::set_metadata().
//                 Kill the unused "has_termfreqs" flag in the termlist table.
// 200706140 1.0.2 Optional value and position tables.
// 200704230 1.0.0 Use zlib compression of tags for record and termlist tables.
// 200611200  N/A  Fixed occasional, architecture-dependent surplus bits in
//		   interpolative coding; "flicklock" -> "flintlock".
// 200506110 0.9.2 Fixed interpolative coding to work(!)
// 200505310 0.9.1 Interpolative coding for position lists.
// 200505280  N/A  Total doclen and last docid entry moved to postlist table.
// 200505270  N/A  First dated version.

#define MAGIC_STRING "IAmFlint"

#define MAGIC_LEN CONST_STRLEN(MAGIC_STRING)
#define VERSIONFILE_SIZE (MAGIC_LEN + 4)

void FlintVersion::create()
{
    char buf[VERSIONFILE_SIZE] = MAGIC_STRING;
    unsigned char *v = reinterpret_cast<unsigned char *>(buf) + MAGIC_LEN;
    v[0] = static_cast<unsigned char>(FLINT_VERSION & 0xff);
    v[1] = static_cast<unsigned char>((FLINT_VERSION >> 8) & 0xff);
    v[2] = static_cast<unsigned char>((FLINT_VERSION >> 16) & 0xff);
    v[3] = static_cast<unsigned char>((FLINT_VERSION >> 24) & 0xff);

    int fd = ::open(filename.c_str(), O_WRONLY|O_CREAT|O_TRUNC|O_BINARY, 0666);

    if (fd < 0) {
	string msg("Failed to create flint version file: ");
	msg += filename;
	throw Xapian::DatabaseOpeningError(msg, errno);
    }

    try {
	io_write(fd, buf, VERSIONFILE_SIZE);
    } catch (...) {
	(void)close(fd);
	throw;
    }

    io_sync(fd);
    if (close(fd) != 0) {
	string msg("Failed to create flint version file: ");
	msg += filename;
	throw Xapian::DatabaseOpeningError(msg, errno);
    }

    uuid_clear(uuid);
    ensure_uuid();
}

void FlintVersion::read_and_check(bool readonly)
{
    int fd = ::open(filename.c_str(), O_RDONLY|O_BINARY);

    if (fd < 0) {
	string msg("Failed to open flint version file for reading: ");
	msg += filename;
	throw Xapian::DatabaseOpeningError(msg, errno);
    }

    // Try to read an extra byte so we know if the file is too long.
    char buf[VERSIONFILE_SIZE + 1];
    size_t size;
    try {
	size = io_read(fd, buf, VERSIONFILE_SIZE + 1, 0);
    } catch (...) {
	(void)close(fd);
	throw;
    }
    (void)close(fd);

    if (size != VERSIONFILE_SIZE) {
	string msg("Flint version file ");
	msg += filename;
	msg += " should be "STRINGIZE(VERSIONFILE_SIZE)" bytes, actually ";
	msg += str(size);
	throw Xapian::DatabaseCorruptError(msg);
    }

    if (memcmp(buf, MAGIC_STRING, MAGIC_LEN) != 0) {
	string msg("Flint version file doesn't contain the right magic string: ");
	msg += filename;
	throw Xapian::DatabaseCorruptError(msg);
    }

    const unsigned char *v;
    v = reinterpret_cast<const unsigned char *>(buf) + MAGIC_LEN;
    unsigned int version = v[0] | (v[1] << 8) | (v[2] << 16) | (v[3] << 24);
    if (version >= 200704230 && version < 200709120) {
	if (readonly) return;
	// Upgrade the database to the current version since any changes we
	// make won't be compatible with older versions of Xapian.
	string filename_save = filename;
	filename += ".tmp";
	create();
	int result;
#ifdef __WIN32__
	result = msvc_posix_rename(filename.c_str(), filename_save.c_str());
#else
	result = rename(filename.c_str(), filename_save.c_str());
#endif
	filename = filename_save;
	if (result == -1) {
	    string msg("Failed to update flint version file: ");
	    msg += filename;
	    throw Xapian::DatabaseOpeningError(msg);
	}
	return;
    }
    if (version != FLINT_VERSION) {
	string msg("Flint version file ");
	msg += filename;
	msg += " is version ";
	msg += str(version);
	msg += " but I only understand "STRINGIZE(FLINT_VERSION);
	throw Xapian::DatabaseVersionError(msg);
    }

    string f = filename;
    f.resize(f.size() - CONST_STRLEN("iamflint"));
    f += "uuid";
    fd = ::open(f.c_str(), O_RDONLY|O_BINARY);

    if (fd < 0) {
	uuid_clear(uuid);
	return;
    }

    try {
	(void)io_read(fd, reinterpret_cast<char*>(uuid), 16, 16);
    } catch (...) {
	uuid_clear(uuid);
	(void)close(fd);
	throw;
    }
    (void)close(fd);
}

void
FlintVersion::ensure_uuid() const
{
    if (uuid_is_null(uuid)) {
	string f = filename;
	f.resize(f.size() - CONST_STRLEN("iamflint"));
	f += "uuid";
	int fd = ::open(f.c_str(), O_WRONLY|O_CREAT|O_TRUNC|O_BINARY, 0666);

	// Might be read-only, so don't error, but instead fall back to
	// using the mtime of the version file.
	if (fd < 0) {
	    struct stat statbuf;
	    if (stat(filename, &statbuf) != 0) {
		throw Xapian::DatabaseError("Couldn't stat " + filename, errno);
	    }
	    unsigned long mtime = statbuf.st_mtime;
	    // This isn't a validly generated UUID, but it'll do for the
	    // purpose of identifying a database master for replication
	    // while we transition to "real" UUIDs.
	    unsigned char *v = reinterpret_cast<unsigned char *>(uuid);
	    v[0] = static_cast<unsigned char>(mtime & 0xff);
	    v[1] = static_cast<unsigned char>((mtime >> 8) & 0xff);
	    v[2] = static_cast<unsigned char>((mtime >> 16) & 0xff);
	    v[3] = static_cast<unsigned char>((mtime >> 24) & 0xff);
	    return;
	}

	uuid_generate(uuid);
	try {
	    io_write(fd, reinterpret_cast<const char*>(uuid), 16);
	} catch (...) {
	    (void)close(fd);
	    throw;
	}

	if (close(fd) != 0) {
	    string msg("Failed to create flint uuid file: ");
	    msg += f;
	    throw Xapian::DatabaseError(msg, errno);
	}
    }
}
