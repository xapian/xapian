/* quartzalltermslist.cc
 *
 * ----START-LICENCE----
 * Copyright 1999,2000 BrightStation PLC
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 * -----END-LICENCE-----
 */

#include "quartz_alltermslist.h"
#include "quartz_utils.h"
#include "quartz_postlist.h"

QuartzAllTermsList::QuartzAllTermsList(RefCntPtr<const Database> database_,
				       AutoPtr<QuartzCursor> pl_cursor_)
	: database(database_), pl_cursor(pl_cursor_), started(false)
{
    /* Seek to the first term */
    QuartzDbKey key;
    key.value = std::string("\0", 1);

    pl_cursor->find_entry(key);

    if (pl_cursor->current_key.value.size() == 0) {
	pl_cursor->next();
    }

    is_at_end = pl_cursor->after_end();
    have_stats = false;
}

QuartzAllTermsList::~QuartzAllTermsList()
{
}

om_termcount
QuartzAllTermsList::get_approx_size() const
{
    return 1000000000; // FIXME
}

om_termname
QuartzAllTermsList::get_termname() const
{
    Assert(started);
    if (!is_at_end) {
	const char *start = pl_cursor->current_key.value.data();
	const char *end = start + pl_cursor->current_key.value.length();
	std::string result;
	if (unpack_string_preserving_sort(&start, end, result)) {
	    return result;
	} else {
	    DEBUGLINE(DB, "QuartzAllTermsList[" << this
		      << "]: Failed to read from key: `"
		      << pl_cursor->current_key.value << "'");
	    throw OmDatabaseCorruptError("Failed to read the key field from a QuartzCursor's key");
	}
    } else {
	throw OmInvalidArgumentError("Attempt to get termname after end");
    }
}

void QuartzAllTermsList::get_stats() const
{
    const char *start = pl_cursor->current_tag.value.data();
    const char *end = start + pl_cursor->current_tag.value.length();
    QuartzPostList::read_number_of_entries(&start, end,
					   &termfreq, &collection_freq);

    have_stats = true;
}

om_doccount
QuartzAllTermsList::get_termfreq() const
{
    Assert(started);
    if (have_stats) {
	return termfreq;
    } else if (!is_at_end) {
	get_stats();
	return termfreq;
    } else {
	throw OmInvalidArgumentError("Attempt to get termfreq after end");
    }
}

om_termcount
QuartzAllTermsList::get_collection_freq() const
{
    Assert(started);
    if (have_stats) {
	return collection_freq;
    } else if (!is_at_end) {
	get_stats();
	return collection_freq;
    } else {
	throw OmInvalidArgumentError("Attempt to get collection_freq after end");
    }
}

TermList *
QuartzAllTermsList::skip_to(const om_termname &tname)
{
    DEBUGLINE(DB, "QuartzAllTermList::skip_to(" << tname << ")");
    started = true;
    QuartzDbKey key;
    key.value = pack_string_preserving_sort(tname);

    have_stats = false;

    bool result = pl_cursor->find_entry(key);

    if (!result) {
	if (pl_cursor->after_end()) {
	    is_at_end = true;
	} else {
	    next();
	}
    } else {
	DEBUGLINE(DB, "QuartzAllTermList[" << this << "]::skip_to(): key is " <<
		  pl_cursor->current_key.value);
    }
    return NULL;
}

TermList *
QuartzAllTermsList::next()
{
    if (!started) {
	started = true;
    } else {
	pl_cursor->next();

	is_at_end = pl_cursor->after_end();

	have_stats = false;
    }
    return NULL;
}

bool
QuartzAllTermsList::at_end() const
{
    return is_at_end;
}
