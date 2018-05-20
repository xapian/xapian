/** @file honey_table.cc
 * @brief HoneyTable class
 */
/* Copyright (C) 2017,2018 Olly Betts
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

#include "honey_table.h"

#include "honey_cursor.h"
#include "stringutils.h"

#include "unicode/description_append.h"

using Honey::RootInfo;

using namespace std;

void
HoneyTable::create_and_open(int flags_, const RootInfo& root_info)
{
    Assert(!single_file());
    flags = flags_;
    compress_min = root_info.get_compress_min();
    if (read_only) {
	num_entries = root_info.get_num_entries();
	root = root_info.get_root();
	// FIXME: levels
    }
    if (!store.open(path, read_only))
	throw Xapian::DatabaseOpeningError("Failed to open HoneyTable", errno);
}

void
HoneyTable::open(int flags_, const RootInfo& root_info, honey_revision_number_t)
{
    flags = flags_;
    compress_min = root_info.get_compress_min();
    num_entries = root_info.get_num_entries();
    offset = root_info.get_offset();
    root = root_info.get_root();
    if (!single_file() && !store.open(path, read_only)) {
	if (!lazy)
	    throw Xapian::DatabaseOpeningError("Failed to open HoneyTable",
					       errno);
    }
    store.set_pos(offset);
}

HoneyCursor*
HoneyTable::cursor_get() const
{
    return new HoneyCursor(store, root, offset);
}
