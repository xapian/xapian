/** @file
 * @brief Communication with worker processes
 */
/* Copyright (C) 2011 Olly Betts
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

#include <cstdio>
#include <string>

enum assistant_msg {
    MSG_OK,
    MSG_NON_FATAL_ERROR,
    MSG_FATAL_ERROR
};

/// Read a string from the file descriptor @a f and storage it in @a s
bool read_string(std::FILE* f, std::string& s);

/// Write the string @a s into the file descriptor @a f
bool write_string(std::FILE* f, const std::string& s);
