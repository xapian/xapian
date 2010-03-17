/** @file replicate_utils.cc
 * @brief Utility functions for replication implementations
 */
/* Copyright (C) 2010 Richard Boulton
 * Copyright (C) 2010 Olly Betts
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

#include "replicate_utils.h"

#include "xapian/error.h"

#include "io_utils.h"

#ifdef __WIN32__
# include "msvc_posix_wrapper.h"
#endif
#include "safeerrno.h"
#include "safefcntl.h"
#include "safesysstat.h"
#include "safeunistd.h"

#include <sys/types.h>

#include <string>

using namespace std;

int
create_changeset_file(const string & changeset_dir,
		      const string & filename,
		      string & changes_name)
{
    changes_name = changeset_dir;
    changes_name += '/';
    changes_name += filename;
#ifdef __WIN32__
    int changes_fd = msvc_posix_open(changes_name.c_str(),
				     O_WRONLY | O_CREAT | O_TRUNC | O_BINARY);
#else
    int changes_fd = open(changes_name.c_str(),
			  O_WRONLY | O_CREAT | O_TRUNC | O_BINARY, 0666);
#endif
    if (changes_fd < 0) {
	string message("Couldn't open changeset to write: ");
	message += changes_name;
	throw Xapian::DatabaseError(message, errno);
    }
    return changes_fd;
}

void
write_and_clear_changes(int changes_fd, string & buf, size_t bytes)
{
    if (changes_fd != -1) {
	io_write(changes_fd, buf.data(), bytes);
    }
    buf.erase(0, bytes);
}
