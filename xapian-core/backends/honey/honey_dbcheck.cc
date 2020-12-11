/** @file
 * @brief Check a honey table.
 */
/* Copyright (C) 2019 Olly Betts
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

#include "honey_dbcheck.h"

using namespace std;

size_t
check_honey_table(const char* tablename,
		  const string& db_dir,
		  int fd,
		  off_t offset_,
		  const HoneyVersion& version_file,
		  int opts,
		  vector<Xapian::termcount>& doclens,
		  ostream* out)
{
    (void)tablename;
    (void)db_dir;
    (void)fd;
    (void)offset_;
    (void)version_file;
    (void)opts;
    (void)doclens;
    (void)out;
    // Dummy implementation for now.
    return 0;
}
