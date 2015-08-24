/** @file brass_version.cc
 * @brief BrassVersion class
 */
/* Copyright (C) 2006,2007,2008,2009,2010,2013 Olly Betts
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

#include "safeerrno.h"

#include <xapian/error.h>

#include "brass_version.h"
#include "io_utils.h"
#include "omassert.h"
#include "stringutils.h" // For STRINGIZE() and CONST_STRLEN().
#include "str.h"

#ifdef __WIN32__
# include "msvc_posix_wrapper.h"
#endif

#include <cstring> // For memcmp() and memcpy().
#include <string>

#include "common/safeuuid.h"

using namespace std;

// YYYYMMDDX where X allows multiple format revisions in a day
#define BRASS_VERSION 201103110
// 201103110 1.2.5 Bump for new max changesets dbstats
// 200912150 1.1.4 Brass debuts.

#define MAGIC_STRING "IAmBrass"

#define MAGIC_LEN CONST_STRLEN(MAGIC_STRING)
// 4 for the version number; 16 for the UUID.
#define VERSIONFILE_SIZE (MAGIC_LEN + 4 + 16)

// Literal version of VERSIONFILE_SIZE, used for error message.  This needs
// to be updated by hand should VERSIONFILE_SIZE change, but that rarely
// happens so this isn't an onerous requirement.
#define VERSIONFILE_SIZE_LITERAL 28

void
BrassVersion::create()
{
    char buf[VERSIONFILE_SIZE] = MAGIC_STRING;
    unsigned char *v = reinterpret_cast<unsigned char *>(buf) + MAGIC_LEN;
    v[0] = static_cast<unsigned char>(BRASS_VERSION & 0xff);
    v[1] = static_cast<unsigned char>((BRASS_VERSION >> 8) & 0xff);
    v[2] = static_cast<unsigned char>((BRASS_VERSION >> 16) & 0xff);
    v[3] = static_cast<unsigned char>((BRASS_VERSION >> 24) & 0xff);

    uuid_generate(uuid);
    memcpy(buf + MAGIC_LEN + 4, (void*)uuid, 16);

    int fd = ::open(filename.c_str(), O_WRONLY|O_CREAT|O_TRUNC|O_BINARY, 0666);

    if (fd < 0) {
	string msg("Failed to create brass version file: ");
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
	string msg("Failed to create brass version file: ");
	msg += filename;
	throw Xapian::DatabaseOpeningError(msg, errno);
    }
}

void
BrassVersion::read_and_check()
{
    int fd = ::open(filename.c_str(), O_RDONLY|O_BINARY);

    if (fd < 0) {
	string msg = filename;
	msg += ": Failed to open brass version file for reading";
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
	CompileTimeAssert(VERSIONFILE_SIZE == VERSIONFILE_SIZE_LITERAL);
	string msg = filename;
	msg += ": Brass version file should be "
	       STRINGIZE(VERSIONFILE_SIZE_LITERAL)" bytes, actually ";
	msg += str(size);
	throw Xapian::DatabaseCorruptError(msg);
    }

    if (memcmp(buf, MAGIC_STRING, MAGIC_LEN) != 0) {
	string msg = filename;
	msg += ": Brass version file doesn't contain the right magic string";
	throw Xapian::DatabaseCorruptError(msg);
    }

    const unsigned char *v;
    v = reinterpret_cast<const unsigned char *>(buf) + MAGIC_LEN;
    unsigned int version = v[0] | (v[1] << 8) | (v[2] << 16) | (v[3] << 24);
    if (version != BRASS_VERSION) {
	string msg = filename;
	msg += ": Brass version file is version ";
	msg += str(version);
	msg += " but I only understand " STRINGIZE(BRASS_VERSION);
	throw Xapian::DatabaseVersionError(msg);
    }

    memcpy((void*)uuid, buf + MAGIC_LEN + 4, 16);
}
