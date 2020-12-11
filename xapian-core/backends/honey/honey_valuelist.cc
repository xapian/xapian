/** @file
 * @brief Honey class for value streams.
 */
/* Copyright (C) 2007,2008,2009 Olly Betts
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

#include <config.h>

#include "honey_valuelist.h"

#include "honey_cursor.h"
#include "honey_database.h"
#include "omassert.h"
#include "str.h"

using namespace Honey;
using namespace std;

bool
HoneyValueList::update_reader()
{
    Xapian::docid last_did = docid_from_key(slot, cursor->current_key);
    if (!last_did) return false;

    cursor->read_tag();
    const string& tag = cursor->current_tag;
    reader.assign(tag.data(), tag.size(), last_did);
    return true;
}

HoneyValueList::~HoneyValueList()
{
    delete cursor;
}

Xapian::docid
HoneyValueList::get_docid() const
{
    Assert(!at_end());
    return reader.get_docid();
}

Xapian::valueno
HoneyValueList::get_valueno() const
{
    return slot;
}

std::string
HoneyValueList::get_value() const
{
    Assert(!at_end());
    return reader.get_value();
}

bool
HoneyValueList::at_end() const
{
    return cursor == NULL;
}

void
HoneyValueList::next()
{
    if (!cursor) {
	cursor = db->get_postlist_cursor();
	if (!cursor) return;
	cursor->find_entry_ge(make_valuechunk_key(slot, 1));
    } else if (!reader.at_end()) {
	reader.next();
	if (!reader.at_end()) return;
	cursor->next();
    }

    if (!cursor->after_end()) {
	if (update_reader()) {
	    if (!reader.at_end()) return;
	}
    }

    // We've reached the end.
    delete cursor;
    cursor = NULL;
}

void
HoneyValueList::skip_to(Xapian::docid did)
{
    if (!cursor) {
	cursor = db->get_postlist_cursor();
	if (!cursor) return;
    } else if (!reader.at_end()) {
	reader.skip_to(did);
	if (!reader.at_end()) return;
    }

    if (cursor->find_entry_ge(make_valuechunk_key(slot, did))) {
	// Exact match.
	if (rare(!update_reader())) {
	    // Shouldn't be possible.
	    Assert(false);
	}
	reader.skip_to(did);
	if (!at_end()) return;
	// The chunk's last docid is did, so skip_to() should always succeed.
	Assert(false);
    } else if (!cursor->after_end()) {
	if (update_reader()) {
	    reader.skip_to(did);
	    if (!reader.at_end()) return;
	    // The chunk's last docid is >= did, so skip_to() shouldn't reach
	    // the end.
	    Assert(false);
	}
    }

    // We've reached the end.
    delete cursor;
    cursor = NULL;
}

string
HoneyValueList::get_description() const
{
    string desc("HoneyValueList(slot=");
    desc += str(slot);
    desc += ')';
    return desc;
}
