/* quartz_diffs.cc: Diffs made to a given quartz database
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

#include "quartz_diffs.h"
#include "autoptr.h"

QuartzDbTag *
QuartzDiffs::get_tag(const QuartzDbKey &key)
{
    if (changed_entries.have_entry(key)) {
	return changed_entries.get_tag(key);
    } else {
	AutoPtr<QuartzDbTag> tag(new QuartzDbTag);
	QuartzDbTag * tagptr = tag.get();

	bool found = table->get_exact_entry(key, *tagptr);

	if (found) {
	    changed_entries.set_tag(key, tag);
	    Assert(changed_entries.get_tag(key) == tagptr);
	} else {
	    tagptr = 0;
	}

	return tagptr;
    }
}

QuartzDbTag *
QuartzDiffs::get_or_make_tag(const QuartzDbKey &key)
{
    if (changed_entries.have_entry(key)) {
	return changed_entries.get_tag(key);
    } else {
	AutoPtr<QuartzDbTag> tag(new QuartzDbTag);
	QuartzDbTag * tagptr = tag.get();

	table->get_exact_entry(key, *tag);
	changed_entries.set_tag(key, tag);
	Assert(changed_entries.have_entry(key));
	Assert(changed_entries.get_tag(key) == tagptr);
	Assert(tag.get() == 0);

	return tagptr;
    }
}

bool
QuartzDiffs::apply(QuartzRevisionNumber new_revision)
{
    bool result;
    try {
	result = table->set_entries(changed_entries.get_all_entries(),
				    new_revision);
    } catch (...) {
	changed_entries.clear();
	throw;
    }
    changed_entries.clear();
    return result;
}

void
QuartzPostListDiffs::add_posting(om_termname tname,
				 om_docid did,
				 om_termcount wdf)
{
}


void
QuartzPositionListDiffs::add_positionlist(om_docid did,
	om_termname tname,
	OmDocumentTerm::term_positions positions)
{
}

void
QuartzRecordDiffs::add_record(om_docid did, const OmData & data)
{
    QuartzDbKey key;
    key.value = did;

    QuartzDbTag * tag = get_or_make_tag(key);

    tag->value = data.value;
}

