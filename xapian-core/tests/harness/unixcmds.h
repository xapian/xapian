/** @file
 *  @brief C++ function versions of useful Unix commands.
 */
/* Copyright (C) 2007 Olly Betts
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#ifndef XAPIAN_INCLUDED_UNIXCMDS_H
#define XAPIAN_INCLUDED_UNIXCMDS_H

#include <string>

/// Recursively copy a directory, just like the Unix "cp -R" command.
void cp_R(const std::string &src, const std::string &dest);

/// Remove a directory and contents, just like the Unix "rm -rf" command.
void rm_rf(const std::string &filename);

/// Touch a file, just like the Unix "touch" command.
void touch(const std::string &filename);

#endif // XAPIAN_INCLUDED_UNIXCMDS_H
