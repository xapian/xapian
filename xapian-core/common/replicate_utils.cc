/** @file replicate_utils.cc
 * @brief Utility functions for replication implementations
 */
/* Copyright (C) 2010 Richard Boulton
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
#include "utils.h"

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

static void
ensure_directory(const std::string dirname)
{
    struct stat statbuf;
    if (stat(dirname, &statbuf) == 0) {
	if (S_ISDIR(statbuf.st_mode))
	    return;
	throw Xapian::DatabaseCreateError("Cannot create directory `" +
					  dirname + "': it already exists "
					  "but is not a directory");
    }
    if (errno != ENOENT) {
	throw Xapian::DatabaseCreateError("Cannot stat directory `" +
					  dirname + "'", errno);
    }

    if (mkdir(dirname, 0755) == -1) {
	throw Xapian::DatabaseCreateError("Cannot create directory `" +
					  dirname + "'", errno);
    }
}

int
create_changeset_file(const std::string & changeset_dir,
		      const std::string & filename,
		      std::string & changes_name)
{
    // Create the changeset directory if it doesn't already exist.
    ensure_directory(changeset_dir);

    changes_name = changeset_dir + "/" + filename;
#ifdef __WIN32__
    int changes_fd = msvc_posix_open(changes_name.c_str(),
				     O_WRONLY | O_CREAT | O_TRUNC | O_BINARY);
#else
    int changes_fd = open(changes_name.c_str(),
			  O_WRONLY | O_CREAT | O_TRUNC | O_BINARY, 0666);
#endif
    if (changes_fd < 0) {
	string message = string("Couldn't open changeset ")
		+ changes_name + " to write";
	throw Xapian::DatabaseError(message, errno);
    }
    return changes_fd;
}

void
write_and_clear_changes(int changes_fd, std::string & buf, size_t bytes)
{
    if (changes_fd != -1) {
	io_write(changes_fd, buf.data(), bytes);
    }
    buf.erase(0, bytes);
}
