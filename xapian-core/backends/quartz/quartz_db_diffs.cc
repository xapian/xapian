/* quartz_db_diffs.cc: Diffs made to a given quartz database
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

#include "config.h"

#include "quartz_db_diffs.h"

QuartzDbTag *
QuartzDbDiffs::get_tag(const QuartzDbKey &key)
{
    if (changed_entries.have_entry(key)) {
	return changed_entries.get_tag(key);
    } else {
	auto_ptr<QuartzDbTag> tag(new QuartzDbTag);
	QuartzDbTag * tagptr = tag.get();

	bool found = table->get_exact_entry(key, *tagptr);

	if (found) {
	    changed_entries.set_tag(key, tag);
	} else {
	    tagptr = 0;
	}

	Assert(changed_entries.get_tag(key) == tagptr);
	return tagptr;
    }
}

bool
QuartzDbDiffs::apply()
{
    bool result;
    try {
	result = table->set_entries(changed_entries.get_all_entries());
    } catch (...) {
	changed_entries.clear();
	throw;
    }
    changed_entries.clear();
    return result;
}

void
QuartzPostListDbDiffs::add_posting(om_termname tname,
				   om_docid did,
				   om_termcount wdf)
{
}


void
QuartzPositionListDbDiffs::add_positionlist(om_docid did,
	om_termname tname,
	OmDocumentTerm::term_positions positions)
{
}

