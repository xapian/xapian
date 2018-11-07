/** @file glass_dbcheck.h
 * @brief Check a glass table.
 */
/* Copyright (C) 2008,2009,2012,2013,2014,2016 Olly Betts
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

#ifndef XAPIAN_INCLUDED_GLASS_DBCHECK_H
#define XAPIAN_INCLUDED_GLASS_DBCHECK_H

#include "xapian/types.h"

#include <cstring> // For size_t.
#include <iosfwd>
#include <string>
#include <vector>

class GlassVersion;

size_t check_glass_table(const char * tablename, const std::string &db_dir,
			 int fd, off_t offset_,
			 const GlassVersion & version_file, int opts,
			 std::vector<Xapian::termcount> & doclens,
			 std::ostream * out);

inline size_t
check_glass_table(const char * tablename, const std::string &db_dir,
		  const GlassVersion & version_file, int opts,
		  std::vector<Xapian::termcount> & doclens,
		  std::ostream * out)
{
    return check_glass_table(tablename, db_dir, -1, 0, version_file, opts,
			     doclens, out);
}

inline size_t
check_glass_table(const char * tablename, int fd, off_t offset_,
		  const GlassVersion & version_file, int opts,
		  std::vector<Xapian::termcount> & doclens,
		  std::ostream * out)
{
    return check_glass_table(tablename, std::string(), fd, offset_,
			     version_file, opts,
			     doclens, out);
}

#endif // XAPIAN_INCLUDED_GLASS_DBCHECK_H
