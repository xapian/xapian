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
#include "omdebug.h"

#include "quartz_db_table.h"
#include "om/omerror.h"
#include "utils.h"
#include <string.h>
#include <errno.h>

string
QuartzRevisionNumber::get_description() const
{
    return om_tostring(value);
}


static void writerevnos(FILE * fp, quartz_revision_number_t * rev1, quartz_revision_number_t * rev2)
{
    clearerr(fp);
    Assert(!fseek(fp, 0, SEEK_SET));

    size_t bytes;
    bytes = fwrite((void *) rev1, sizeof(quartz_revision_number_t), 1, fp);
    if (bytes != 0) 
	bytes = fwrite((void *) rev2, sizeof(quartz_revision_number_t), 1, fp);
    fflush(fp);
    if (bytes == 0) {
	throw OmDatabaseError("Table not writeable");
    }
}

static void readrevnos(bool readonly, FILE * fp, quartz_revision_number_t * rev1, quartz_revision_number_t * rev2)
{
    clearerr(fp);
    Assert(!fseek(fp, 0, SEEK_SET));

    size_t bytes;
    bytes = fread((void *) rev1, sizeof(quartz_revision_number_t), 1, fp);
    if (bytes != 0) 
	bytes = fread((void *) rev2, sizeof(quartz_revision_number_t), 1, fp);
    if (bytes == 0) {
	if (readonly) throw OmOpeningError("Table not present");
	*rev1 = 0;
	*rev2 = 0;
	writerevnos(fp, rev1, rev2);
    }
}

QuartzDbTable::QuartzDbTable(string path_,
			     bool readonly_)
	: path(path_),
          readonly(readonly_),
	  revision(0)
{
}

bool
QuartzDbTable::open()
{
    // FIXME implement
}

bool
QuartzDbTable::open(QuartzRevisionNumber revision_)
{
    // FIXME implement

    string filename = path + "fakefoo";
    if (readonly) {
	fp = fopen(filename.c_str(), "r");
    } else {
	fp = fopen(filename.c_str(), "a+");
    }
    if (fp == 0) {
	throw OmOpeningError("Can't open Quartz table (" + filename + ")" +
			     strerror(errno));
    }

    quartz_revision_number_t rev1 = 0;
    quartz_revision_number_t rev2 = 0;
    readrevnos(readonly, fp, &rev1, &rev2);

    if (rev1 != revision.value && rev2 != revision.value) {
	throw OmOpeningError("Can't open table at revision " +
			     revision.get_description() + ".");
    }
}

QuartzDbTable::~QuartzDbTable()
{
    fclose(fp);
}

QuartzRevisionNumber
QuartzDbTable::get_open_revision_number() const
{
    return revision;
}

QuartzRevisionNumber
QuartzDbTable::get_latest_revision_number() const
{
    // FIXME: replace with a call to martin's code
    quartz_revision_number_t rev1 = 0;
    quartz_revision_number_t rev2 = 0;
    readrevnos(readonly, fp, &rev1, &rev2);

    if (rev1 > rev2) return QuartzRevisionNumber(rev1);
    return QuartzRevisionNumber(rev2);
}

quartz_tablesize_t
QuartzDbTable::get_entry_count() const
{
    return data.size();
}

bool
QuartzDbTable::get_nearest_entry(QuartzDbKey &key, QuartzDbTag & tag) const
{
    Assert(!(key.value.empty()));

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
QuartzDbTable::get_exact_entry(const QuartzDbKey &key, QuartzDbTag & tag) const
{
    Assert(!(key.value.empty()));

    /// FIXME: replace with calls to martin's code
    std::map<QuartzDbKey, QuartzDbTag>::const_iterator j = data.find(key);
    if (j == data.end()) {
	return false;
    }
    tag.value = (j->second).value;
    return true;
}

bool
QuartzDbTable::set_entries(std::map<QuartzDbKey, QuartzDbTag *> & entries,
			   QuartzRevisionNumber new_revision)
{
    if(readonly) throw OmInvalidOperationError("Attempt to set entries in a readonly table.");

    // FIXME: replace with calls to martin's code
    {
	std::map<QuartzDbKey, QuartzDbTag *>::const_iterator i;
	for (i = entries.begin(); i != entries.end(); i++) {
	    Assert(!((i->first).value.empty()));
	    std::map<QuartzDbKey, QuartzDbTag>::iterator j;
	    j = data.find(i->first);
	    if (i->second == 0) {
		// delete j
		if (j != data.end()) {
		    data.erase(j);
		}
	    } else {
		if (j == data.end()) {
		    data.insert(make_pair(i->first, *(i->second)));
		} else {
		    if ((j->second).value != (*(i->second)).value) {
			j->second = *(i->second);
		    }
		}
	    }
	}

	quartz_revision_number_t rev1 = 0;
	quartz_revision_number_t rev2 = 0;
	readrevnos(readonly, fp, &rev1, &rev2);
	    cout << revision.value << " " << rev1 << " " << rev2 << endl;

	if (rev1 == revision.value) {
	    revision = new_revision;
	    rev2 = revision.value;
	} else if (rev2 == revision.value) {
	    revision = new_revision;
	    rev1 = revision.value;
	} else {
	    Assert(0);
	}
	writerevnos(fp, &rev1, &rev2);
    }

    return true;
}

