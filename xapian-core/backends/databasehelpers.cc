/** @file databasehelpers.cc
 * @brief Helper functions for database handling
 */
/* Copyright 2002-2020 Olly Betts
 * Copyright 2008 Lemur Consulting Ltd
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

#include "databasehelpers.h"

#include "backends.h"

#include "glass/glass_defs.h"
#include "honey/honey_defs.h"

#include "io_utils.h"
#include "omassert.h"
#include "posixy_wrapper.h"

#include <algorithm>
#include <cstring>
#include <string>
#include <sys/types.h>
#include "safesysstat.h"
#include "safeunistd.h"

using namespace std;

static int
test_if_single_file_db_(int fd, off_t pos)
{
#if defined XAPIAN_HAS_GLASS_BACKEND || \
    defined XAPIAN_HAS_HONEY_BACKEND
    char magic_buf[14];
    // FIXME: Don't duplicate magic check here...
    if (io_read(fd, magic_buf, 14) == 14 &&
	lseek(fd, pos, SEEK_SET) == pos &&
	memcmp(magic_buf, "\x0f\x0dXapian ", 9) == 0) {
	switch (magic_buf[9]) {
#ifdef XAPIAN_HAS_GLASS_BACKEND
	    case 'G':
		if (memcmp(magic_buf + 10, "lass", 4) == 0) {
		    return BACKEND_GLASS;
		}
		break;
#endif
#ifdef XAPIAN_HAS_HONEY_BACKEND
	    case 'H':
		if (memcmp(magic_buf + 10, "oney", 4) == 0) {
		    return BACKEND_HONEY;
		}
		break;
#endif
	}
	return BACKEND_UNKNOWN;
    }
#else
    (void)fd;
#endif
    return BACKEND_UNKNOWN;
}

int
test_if_single_file_db(int fd)
{
    off_t pos = lseek(fd, 0, SEEK_CUR);
    if (pos < 0) {
	return BACKEND_UNKNOWN;
    }
    return test_if_single_file_db_(fd, pos);
}

int
test_if_single_file_db(const struct stat& sb,
		       const string& path,
		       int* fd_ptr)
{
    Assert(fd_ptr);
#if defined XAPIAN_HAS_GLASS_BACKEND || \
    defined XAPIAN_HAS_HONEY_BACKEND
    if (!S_ISREG(sb.st_mode)) return BACKEND_UNKNOWN;
    // Look at the size as a clue - if it's less than both GLASS_MIN_BLOCKSIZE
    // and HONEY_MIN_DB_SIZE then it's not a single-file glass or honey
    // database.  For a larger file, we peek at the start of the file to
    // determine what it is.
    if (sb.st_size < min(GLASS_MIN_BLOCKSIZE, HONEY_MIN_DB_SIZE))
	return BACKEND_UNKNOWN;
    int fd = posixy_open(path.c_str(), O_RDONLY|O_BINARY);
    if (fd != -1) {
	int result = test_if_single_file_db_(fd, off_t{0});
	if (result != BACKEND_UNKNOWN) {
	    *fd_ptr = fd;
	} else {
	    ::close(fd);
	}
	return result;
    }
#else
    (void)sb;
    (void)path;
    (void)fd_ptr;
#endif
    return BACKEND_UNKNOWN;
}
