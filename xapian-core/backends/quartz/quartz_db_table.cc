/* quartz_db_manager.cc: Database management for quartz
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

#include "quartz_db_table.h"
#include "om/omerror.h"
#include "utils.h"

string
QuartzRevisionNumber::get_description() const
{
    return om_tostring(value);
}


QuartzDbTable::QuartzDbTable(bool readonly_)
	: revision(0),
          readonly(readonly_)
{
}

QuartzDbTable::~QuartzDbTable()
{
}

QuartzRevisionNumber
QuartzDbTable::get_revision_number() const
{
    return QuartzRevisionNumber(revision);
}

bool
QuartzDbTable::read_entry(QuartzDbKey &key, QuartzDbTag & tag) const
{
    if (key.value.empty()) throw OmInvalidArgumentError("Keys may not be null, in QuartzDbTable::read_entry_exact()");

    /// FIXME: replace with calls to martin's code
    std::map<QuartzDbKey, QuartzDbTag>::const_iterator j;
    j = data.lower_bound(key);

    if (j != data.end() && j->first.value == key.value) {
	// Exact match
	tag.value = j->second.value;
	return true;
    }

    if (j == data.begin()) {
	// Nothing before this match
	key.value = "";
	tag.value = "";
	return false;
    }
    
    // Make j point to match _before_ that searched for.
    j--;

    key.value = (j->first).value;
    tag.value = (j->second).value;
    return false;
}

bool
QuartzDbTable::read_entry_exact(const QuartzDbKey &key, QuartzDbTag & tag) const
{
    if (key.value.empty()) throw OmInvalidArgumentError("Keys may not be null, in QuartzDbTable::read_entry_exact()");

    /// FIXME: replace with calls to martin's code
    std::map<QuartzDbKey, QuartzDbTag>::const_iterator j = data.find(key);
    if (j == data.end()) {
	return false;
    }
    tag.value = (j->second).value;
    return true;
}

bool
QuartzDbTable::set_entries(std::map<QuartzDbKey, QuartzDbTag *> & entries)
{
    if(readonly) throw OmInvalidOperationError("Attempt to set entries in a readonly table.");

    /// FIXME: replace with calls to martin's code
    bool modified = false;
    std::map<QuartzDbKey, QuartzDbTag *>::const_iterator i;
    for (i = entries.begin(); i != entries.end(); i++) {
	if ((i->first).value.empty()) throw OmInvalidArgumentError("Keys may not be null, in QuartzDbTable::read_entry_exact()");
	std::map<QuartzDbKey, QuartzDbTag>::iterator j = data.find(i->first);
	if (i->second == 0) {
	    // delete j
	    if (j != data.end()) {
		data.erase(j);
		modified = true;
	    }
	} else {
	    if (j == data.end()) {
		data.insert(make_pair(i->first, *(i->second)));
		modified = true;
	    } else {
		if ((j->second).value != (*(i->second)).value) {
		    j->second = *(i->second);
		    modified = true;
		}
	    }
	}
    }

    if (modified) revision++;

    return true;
}

