/** @file fileutils.h
 *  @brief File and path manipulation routines.
 */
/* Copyright (C) 2008 Lemur Consulting Ltd
 * Copyright (C) 2009 Olly Betts
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

#ifndef XAPIAN_INCLUDED_FILEUTILS_H
#define XAPIAN_INCLUDED_FILEUTILS_H

#include "xapian/visibility.h"

#include <string>

/** Return the path of the directory holding filename.
 *
 *  The returned path will always end with a directory separator.
 */
std::string calc_dirname(const std::string & filename);

/** Join two paths together.
 *
 *  Pays attention to whether path2 is absolute or relative.
 */
std::string join_paths(const std::string & path1, const std::string & path2);

#endif /* XAPIAN_INCLUDED_FILEUTILS_H */
