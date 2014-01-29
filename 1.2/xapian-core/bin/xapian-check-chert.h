/** @file xapian-check-chert.h
 * @brief Check a chert table.
 */
/* Copyright (C) 2008,2009,2013 Olly Betts
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

#ifndef XAPIAN_INCLUDED_XAPIAN_CHECK_CHERT_H
#define XAPIAN_INCLUDED_XAPIAN_CHECK_CHERT_H

#include "xapian/types.h"

#include "chert_types.h"

#include <cstring> // For size_t.
#include <string>
#include <vector>

size_t check_chert_table(const char * tablename, std::string table,
			 chert_revision_number_t * rev_ptr, int opts,
			 std::vector<Xapian::termcount> & doclens,
			 Xapian::docid db_last_docid);

#endif // XAPIAN_INCLUDED_XAPIAN_CHECK_CHERT_H
