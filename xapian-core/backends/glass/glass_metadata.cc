/** @file
 * @brief Access to metadata for a glass database.
 */
/* Copyright (C) 2004,2005,2006,2007,2008,2009,2010,2011,2017 Olly Betts
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

#include "backends/databaseinternal.h"
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

Xapian::termcount
GlassMetadataTermList::get_approx_size() const
{
    // Very approximate!  This is only used to build a balanced or tree, so
    // at least we'll get an even tree by returning a constant answer.
    return 1;
}

Xapian::doccount
GlassMetadataTermList::get_termfreq() const
{
    throw Xapian::InvalidOperationError("GlassMetadataTermList::get_termfreq() not meaningful");
}

TermList *
GlassMetadataTermList::next()
{
    LOGCALL(DB, TermList *, "GlassMetadataTermList::next", NO_ARGS);
    Assert(!cursor->after_end());

    if (!cursor->next() || !startswith(cursor->current_key, prefix)) {
	// We've reached the end of the prefixed terms.
	RETURN(this);
    }
    current_term.assign(cursor->current_key, 2);
    RETURN(NULL);
}

TermList *
GlassMetadataTermList::skip_to(const string &key)
{
    LOGCALL(DB, TermList *, "GlassMetadataTermList::skip_to", key);
    Assert(!cursor->after_end());

    if (cursor->find_entry_ge(string("\x00\xc0", 2) + key)) {
	// Exact match.
	current_term = key;
    } else {
	// The exact term we asked for isn't there, so check if the next
	// term after it also has the right prefix.
	if (cursor->after_end() || !startswith(cursor->current_key, prefix)) {
	    // We've reached the end of the prefixed terms.
	    RETURN(this);
	}
	current_term.assign(cursor->current_key, 2);
    }
    RETURN(NULL);
}
