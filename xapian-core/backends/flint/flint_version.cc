/** @file flint_version.cc
 * @brief FlintVersion class
 */
/* Copyright (C) 2006 Olly Betts
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

#include "flint_io.h"
#include "flint_version.h"
#include "omassert.h" // For STRINGIZE()
#include "utils.h"

#include <string>

#include <string.h> // for memcmp

using std::string;

// YYYYMMDDX where X allows multiple format revisions in a day
static const unsigned int FLINT_VERSION = 200506110;
// 200506110 Fixed interpolative coding to work(!)
// 200505310 Interpolative coding for position lists
// 200505280 Total doclen and last docid entry moved to postlist table
// 200505270 First dated version

#define MAGIC_STRING "IAmFlint"

#define CONST_STRLEN(S) (sizeof(S"")-1)
#define MAGIC_LEN CONST_STRLEN(MAGIC_STRING)
#define VERSIONFILE_SIZE (MAGIC_LEN + 4)

void FlintVersion::create()
{
    char buf[VERSIONFILE_SIZE] = MAGIC_STRING;
    unsigned char *v = reinterpret_cast<unsigned char *>(buf) + MAGIC_LEN;
    v[0] = char(FLINT_VERSION);
    v[1] = char(FLINT_VERSION >> 8);
    v[2] = char(FLINT_VERSION >> 16);
    v[3] = char(FLINT_VERSION >> 24);

    int fd = ::open(filename.c_str(), O_WRONLY|O_CREAT|O_TRUNC|O_BINARY, 0666);

    if (fd < 0) {
	string msg("Failed to create flint version file: ");
	msg += filename;
	throw Xapian::DatabaseOpeningError(msg, errno);
    }

    try {
	flint_io_write(fd, buf, VERSIONFILE_SIZE);
    } catch (...) {
	(void)close(fd);
	throw;
    }

    if (close(fd) != 0) {
	string msg("Failed to create flint version file: ");
	msg += filename;
	throw Xapian::DatabaseOpeningError(msg, errno);
    }
}

void FlintVersion::read_and_check()
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
	size = flint_io_read(fd, buf, VERSIONFILE_SIZE + 1, 0);
    } catch (...) {
	(void)close(fd);
	throw;
    }
    (void)close(fd);

    if (size != VERSIONFILE_SIZE) {
	string msg("Flint version file ");
	msg += filename;
	msg += " should be "STRINGIZE(VERSIONFILE_SIZE)" bytes, actually ";
	msg += om_tostring(size);
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
    if (version != FLINT_VERSION) {
	string msg("Flint version file ");
	msg += filename;
	msg += " is version ";
	msg += om_tostring(version);
	msg += " but I only understand "STRINGIZE(FLINT_VERSION);
	throw Xapian::DatabaseOpeningError(msg);
    }
}
