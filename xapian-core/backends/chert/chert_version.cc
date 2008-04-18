/** @file chert_version.cc
 * @brief ChertVersion class
 */
/* Copyright (C) 2006,2007,2008 Olly Betts
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

#include "chert_io.h"
#include "chert_version.h"
#include "stringutils.h" // For STRINGIZE().
#include "utils.h"

#ifdef __WIN32__
# include "msvc_posix_wrapper.h"
#endif

#include <string>

#include <string.h> // for memcmp

using std::string;

// YYYYMMDDX where X allows multiple format revisions in a day
#define CHERT_VERSION 200804180
// 200804180 1.1.0 Chert debuts.

#define MAGIC_STRING "IAmChert"

#define CONST_STRLEN(S) (sizeof(S"")-1)
#define MAGIC_LEN CONST_STRLEN(MAGIC_STRING)
#define VERSIONFILE_SIZE (MAGIC_LEN + 4)

void ChertVersion::create()
{
    char buf[VERSIONFILE_SIZE] = MAGIC_STRING;
    unsigned char *v = reinterpret_cast<unsigned char *>(buf) + MAGIC_LEN;
    v[0] = static_cast<unsigned char>(CHERT_VERSION & 0xff);
    v[1] = static_cast<unsigned char>((CHERT_VERSION >> 8) & 0xff);
    v[2] = static_cast<unsigned char>((CHERT_VERSION >> 16) & 0xff);
    v[3] = static_cast<unsigned char>((CHERT_VERSION >> 24) & 0xff);

    int fd = ::open(filename.c_str(), O_WRONLY|O_CREAT|O_TRUNC|O_BINARY, 0666);

    if (fd < 0) {
	string msg("Failed to create chert version file: ");
	msg += filename;
	throw Xapian::DatabaseOpeningError(msg, errno);
    }

    try {
	chert_io_write(fd, buf, VERSIONFILE_SIZE);
    } catch (...) {
	(void)close(fd);
	throw;
    }

    if (close(fd) != 0) {
	string msg("Failed to create chert version file: ");
	msg += filename;
	throw Xapian::DatabaseOpeningError(msg, errno);
    }
}

void ChertVersion::read_and_check(bool readonly)
{
    int fd = ::open(filename.c_str(), O_RDONLY|O_BINARY);

    if (fd < 0) {
	string msg("Failed to open chert version file for reading: ");
	msg += filename;
	throw Xapian::DatabaseOpeningError(msg, errno);
    }

    // Try to read an extra byte so we know if the file is too long.
    char buf[VERSIONFILE_SIZE + 1];
    size_t size;
    try {
	size = chert_io_read(fd, buf, VERSIONFILE_SIZE + 1, 0);
    } catch (...) {
	(void)close(fd);
	throw;
    }
    (void)close(fd);

    if (size != VERSIONFILE_SIZE) {
	string msg("Chert version file ");
	msg += filename;
	msg += " should be "STRINGIZE(VERSIONFILE_SIZE)" bytes, actually ";
	msg += om_tostring(size);
	throw Xapian::DatabaseCorruptError(msg);
    }

    if (memcmp(buf, MAGIC_STRING, MAGIC_LEN) != 0) {
	string msg("Chert version file doesn't contain the right magic string: ");
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
	    string msg("Failed to update chert version file: ");
	    msg += filename;
	    throw Xapian::DatabaseOpeningError(msg);
	}
	return;
    }
    if (version != CHERT_VERSION) {
	string msg("Chert version file ");
	msg += filename;
	msg += " is version ";
	msg += om_tostring(version);
	msg += " but I only understand "STRINGIZE(CHERT_VERSION);
	throw Xapian::DatabaseVersionError(msg);
    }
}
