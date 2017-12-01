/** @file glass_metadata.cc
 * @brief Access to metadata for a glass database.
 */
/* Copyright (C) 2004,2005,2006,2007,2008,2009,2010,2011 Olly Betts
 * Copyright (C) 2008 Lemur Consulting Ltd
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

#include "glass_metadata.h"

#include "glass_cursor.h"

#include "backends/database.h"
#include "debuglog.h"
#include "omassert.h"
#include "stringutils.h"

#include "xapian/error.h"

using namespace std;
using Xapian::Internal::intrusive_ptr;

GlassMetadataTermList::GlassMetadataTermList(
	intrusive_ptr<const Xapian::Database::Internal> database_,
	GlassCursor * cursor_,
	const string &prefix_)
	: database(database_), cursor(cursor_), prefix(string("\x00\xc0", 2) + prefix_)
{
    LOGCALL_CTOR(DB, "GlassMetadataTermList", database_ | cursor_ | prefix_);
    Assert(cursor);
    // Seek to the first key before the first metadata key.
    cursor->find_entry_lt(prefix);
}

GlassMetadataTermList::~GlassMetadataTermList()
{
    LOGCALL_DTOR(DB, "GlassMetadataTermList");
    delete cursor;
}

string
GlassMetadataTermList::get_termname() const
{
    LOGCALL(DB, string, "GlassMetadataTermList::get_termname", NO_ARGS);
    Assert(!at_end());
    Assert(!cursor->current_key.empty());
    Assert(startswith(cursor->current_key, prefix));
    RETURN(cursor->current_key.substr(2));
}

Xapian::doccount
GlassMetadataTermList::get_termfreq() const
{
    throw Xapian::InvalidOperationError("GlassMetadataTermList::get_termfreq() not meaningful");
}

Xapian::termcount
GlassMetadataTermList::get_collection_freq() const
{
    throw Xapian::InvalidOperationError("GlassMetadataTermList::get_collection_freq() not meaningful");
}

TermList *
GlassMetadataTermList::next()
{
    LOGCALL(DB, TermList *, "GlassMetadataTermList::next", NO_ARGS);
    Assert(!at_end());

    cursor->next();
    if (!cursor->after_end() && !startswith(cursor->current_key, prefix)) {
	// We've reached the end of the prefixed terms.
	cursor->to_end();
    }

    RETURN(NULL);
}

TermList *
GlassMetadataTermList::skip_to(const string &key)
{
    LOGCALL(DB, TermList *, "GlassMetadataTermList::skip_to", key);
    Assert(!at_end());

    if (!cursor->find_entry_ge(string("\x00\xc0", 2) + key)) {
	// The exact term we asked for isn't there, so check if the next
	// term after it also has the right prefix.
	if (!cursor->after_end() && !startswith(cursor->current_key, prefix)) {
	    // We've reached the end of the prefixed terms.
	    cursor->to_end();
	}
    }
    RETURN(NULL);
}

bool
GlassMetadataTermList::at_end() const
{
    LOGCALL(DB, bool, "GlassMetadataTermList::at_end", NO_ARGS);
    RETURN(cursor->after_end());
}
