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


// FIXME: just temporary
#include <stdio.h>

static string
readline(FILE *fp)
{
    string res;

    while(1) {
	int ch = fgetc(fp);
	if (ch == EOF) break;
	if (ch == '\n') break;
	if (ch == '\r') break;
	res += string(&((char)ch), 1);
    }

    return res;
}

static void
writefile(string filename,
	  std::map<QuartzDbKey, QuartzDbTag> & data,
	  quartz_revision_number_t rev)
{
    FILE * fp = fopen(filename.c_str(), "w+");

    if (fp == 0) {
	throw OmDatabaseCorruptError(string("Can't access database: ") +
				     strerror(errno));
    }

    size_t items;
    items = fwrite((const void *) &rev,
		   sizeof(quartz_revision_number_t),
		   1,
		   fp);
    if (items != 1) {
	fclose(fp);
	throw OmDatabaseCorruptError("Can't write to Quartz table (" + filename + ")" + strerror(errno));
    }

    std::map<QuartzDbKey, QuartzDbTag>::const_iterator i;
    for (i = data.begin(); i != data.end(); i++) {
	fprintf(fp, "%s\n%s\n",
		i->first.value.c_str(),
		i->second.value.c_str());
    }

    fclose(fp);
}

static void
readfile(string filename,
	 std::map<QuartzDbKey, QuartzDbTag> & data,
	 quartz_revision_number_t * rev,
	 bool readonly)
{
    FILE * fp = fopen(filename.c_str(), "r");

    if (fp == 0) {
	if(readonly)
	    throw OmOpeningError("Table `" + filename + "' does not exist.");
	*rev = 0;
	data.clear();
	writefile(filename, data, *rev);
	return;
    }

    size_t items;
    items = fread((void *) rev, sizeof(quartz_revision_number_t), 1, fp);
    if (items != 1) {
	fclose(fp);
	throw OmDatabaseCorruptError("Can't open Quartz table (" + filename + ")" + strerror(errno));
    }

    while(!feof(fp)) {
	QuartzDbKey key;
	QuartzDbTag tag ;
	key.value = readline(fp);
	if (feof(fp) && key.value != "") {
	    fclose(fp);
	    throw OmDatabaseCorruptError("Can't open Quartz table (" + filename + ") - no tag for key `" + key.value + "': " + strerror(errno));
	}

	tag.value = readline(fp);
	if (!feof(fp)) {
	    data[key] = tag;
	}
    }
    fclose(fp);
}

QuartzDbTable::QuartzDbTable(string path_,
			     bool readonly_)
	: path(path_),
          readonly(readonly_),
	  revision(0)
{
}

void
QuartzDbTable::open()
{
    // FIXME implement
    std::map<QuartzDbKey, QuartzDbTag> data1;
    readfile(path + "data_1", data1, &revision1, readonly);

    std::map<QuartzDbKey, QuartzDbTag> data2;
    readfile(path + "data_2", data2, &revision2, readonly);

    if(revision1 > revision2) {
	data = data1;
	revision.value = revision1;
    } else {
	data = data2;
	revision.value = revision2;
    }
}

bool
QuartzDbTable::open(QuartzRevisionNumber revision_)
{
    // FIXME implement
    std::map<QuartzDbKey, QuartzDbTag> data1;
    readfile(path + "data_1", data1, &revision1, readonly);

    std::map<QuartzDbKey, QuartzDbTag> data2;
    readfile(path + "data_2", data2, &revision2, readonly);

    if (revision1 ==revision_.value) {
	data = data1;
	revision.value = revision1;
    } else if (revision2 == revision_.value) {
	data = data2;
	revision.value = revision2;
    } else {
	return false;
    }
    return true;
}

QuartzDbTable::~QuartzDbTable()
{
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
    std::map<QuartzDbKey, QuartzDbTag> data1;
    quartz_revision_number_t rev1;
    readfile(path + "data_1", data1, &rev1, readonly);

    std::map<QuartzDbKey, QuartzDbTag> data2;
    quartz_revision_number_t rev2;
    readfile(path + "data_2", data2, &rev2, readonly);

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

    // Find out which table is not opened
    std::map<QuartzDbKey, QuartzDbTag> data1;
    quartz_revision_number_t rev1;
    readfile(path + "data_1", data1, &rev1, readonly);

    std::map<QuartzDbKey, QuartzDbTag> data2;
    quartz_revision_number_t rev2;
    readfile(path + "data_2", data2, &rev2, readonly);

    data1.clear();
    data2.clear();


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
    }


    // Write data
    if(revision.value == rev1) {
	revision.value = new_revision.value;
	writefile(path + "data_2", data, revision.value);
    } else {
	Assert(revision.value == rev2);
	revision.value = new_revision.value;
	writefile(path + "data_1", data, revision.value);
    }

    return true;
}

