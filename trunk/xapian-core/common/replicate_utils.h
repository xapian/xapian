/** @file replicate_utils.h
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

#ifndef XAPIAN_INCLUDED_REPLICATE_UTILS_H
#define XAPIAN_INCLUDED_REPLICATE_UTILS_H

#include <string>

/** Create a new changeset file, and return an open fd for writing to it.
 *
 *  Creates the changeset directory, if required.
 *
 *  If there is already a changeset file of the given name, it is truncated by
 *  this.
 *
 *  @param changeset_dir The directory for the changesets.
 *  @param filename The name of the changeset file.
 *  @param changes_name A string which will be set to the path of the changeset
 *  file.
 *
 *  @return The open file descriptor.
 *
 *  @exception Xapian::DatabaseError if the changeset couldn't be opened.
 */
int
create_changeset_file(const std::string & changeset_dir,
		      const std::string & filename,
		      std::string & changes_name);

/** Write some changes from a buffer, and then drop them from the buffer.
 *
 *  @param changes_fd The filedescriptor to write to (-1 to skip writing).
 *  @param buf The buffer holding the changes.
 *  @param bytes The number of bytes to write and drop.
 */
void
write_and_clear_changes(int changes_fd, std::string & buf, size_t bytes);

#endif // XAPIAN_INCLUDED_REPLICATE_UTILS_H
