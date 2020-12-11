/** @file
 * @brief load a file into a std::string.
 */
/* Copyright (C) 2006,2010,2012,2018 Olly Betts
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

#ifndef OMEGA_INCLUDED_LOADFILE_H
#define OMEGA_INCLUDED_LOADFILE_H

#include <string>

enum { NOCACHE = 0x1, NOATIME = 0x2 };

bool load_file_from_fd(int fd, std::string& output);

static inline bool
load_file_from_fd(int fd, size_t size_hint, std::string& output)
{
    output.resize(0);
    output.reserve(size_hint);
    return load_file_from_fd(fd, output);
}

bool load_file(const std::string& file_name, size_t max_to_read, int flags,
	       std::string& output, bool* truncated);

inline bool
load_file(const std::string& file_name, size_t max_to_read, int flags,
	  std::string& output, bool& truncated)
{
    return load_file(file_name, max_to_read, flags, output, &truncated);
}

inline bool
load_file(const std::string& file_name, std::string& output, int flags = 0)
{
    return load_file(file_name, 0, flags, output, nullptr);
}

#endif // OMEGA_INCLUDED_LOADFILE_H
