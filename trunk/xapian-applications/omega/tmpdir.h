/** @file tmpdir.h
 * @brief create a temporary directory securely
 *
 * Copyright (C) 2011 Olly Betts
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

#ifndef XAPIAN_OMEGA_INCLUDED_TMPDIR_H
#define XAPIAN_OMEGA_INCLUDED_TMPDIR_H

#include <string>

/** Return path to temporary directory.
 *
 *  If successfully called before, this just returns the same path.
 *
 *  @return If successful, path to directory; otherwise empty string.
 */
const std::string & get_tmpdir();

/** Attempt to remove the directory if we created one.
 *
 *  If get_tmpdir() hasn't been called or hasn't succeeded, does nothing.
 *  If the directory isn't empty or rmdir() otherwise fails, the directory
 *  won't get removed.
 */
void remove_tmpdir();

#endif // XAPIAN_OMEGA_INCLUDED_TMPDIR_H
