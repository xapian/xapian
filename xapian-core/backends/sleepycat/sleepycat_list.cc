/* sleepy_list.cc: lists of data from a sleepycat database
 *
 * ----START-LICENCE----
 * Copyright 1999,2000 Dialog Corporation
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

#include <algorithm>

#include "omassert.h"
#include "sleepy_list.h"

// Sleepycat database stuff
#include <db_cxx.h>
#include <stdlib.h>
#include <string.h>

/* For the moment, the packed format is:
 *    Number of items.
 *    foreach item {
 *      Length of item.
 *      Item in packed format.
 *    }
 *
 * This requires that the entire list be read in and unpacked for it to be
 * accessed: FIXME - remedy this.
 */

/// A type which can hold any entry.  FIXME: specialise this.
typedef SleepyListItem::id_type entry_type;

/** Read an id_type from the specified position in the string,
 *  and update the position.
 *
 *  @exception OmDatabaseError thrown if string is not long enough.
 *
 *  @param packed  The string to read the id_type from.
 *  @param pos     The offset to start reading the id_type at.
 */
template<class X>
static const X readentry(const string &packed, string::size_type & pos)
{
    string::size_type endpos = pos + sizeof(X) / sizeof(char);

    if(endpos > packed.length()) {
	throw(OmDatabaseError("Database corrupt - unexpected end of item."));
    }

    X entry = * reinterpret_cast<const X *>(packed.data() + pos * sizeof(char));
    pos = endpos;

    return entry;
}


SleepyListItem::SleepyListItem(id_type id_,
			       om_doccount termfreq_,
			       om_termcount wdf_,
			       const vector<om_termpos> & positions_)
	: id(id_),
	  termfreq(termfreq_),
	  wdf(wdf_),
	  positions(positions_)
{
}

SleepyListItem::SleepyListItem(string packed)
{
    string::size_type pos = 0;

    id = readentry<id_type>(packed, pos);
    termfreq = readentry<entry_type>(packed, pos);
    wdf = readentry<entry_type>(packed, pos);

    vector<om_termpos>::size_type positions_size;
    positions_size = readentry<entry_type>(packed, pos);

    while(positions.size() < positions_size) {
	positions.push_back(readentry<entry_type>(packed, pos));
    }
}


string
SleepyListItem::pack() const
{
    string packed;
    id_type idtemp;
    entry_type entrytemp;

    idtemp = id;
    packed.append(reinterpret_cast<char *>(&idtemp), sizeof(id_type));

    entrytemp = termfreq;
    packed.append(reinterpret_cast<char *>(&entrytemp), sizeof(entry_type));

    entrytemp = wdf;
    packed.append(reinterpret_cast<char *>(&entrytemp), sizeof(entry_type));

    entrytemp = positions.size();
    packed.append(reinterpret_cast<char *>(&entrytemp), sizeof(entry_type));

    vector<om_termpos>::const_iterator i;
    for(i = positions.begin(); i != positions.end(); i++) {
	entrytemp = *i;
	packed.append(reinterpret_cast<char *>(&entrytemp), sizeof(entry_type));
    }

    return packed;
}



SleepyList::SleepyList(Db * db_, void * keydata_, size_t keylen_,
		       bool store_termfreq,
		       bool store_wdf,
		       bool store_positional)
	: db(db_),
	  modified_and_locked(false),
	  iteration_in_progress(false)
{
    // Copy and store key data
    void *keydata = malloc(keylen_);
    if(keydata == NULL) throw std::bad_alloc();
    memcpy(keydata, keydata_, keylen_);
    key.set_data(keydata);
    key.set_size(keylen_);

    // Read database
    Dbt data;

    // FIXME - this flag results in extra copying - more inefficiency.
    // We should use DB_DBT_USERMEM and DB_DBT_PARTIAL
    data.set_flags(DB_DBT_MALLOC);
    int found;

    try {
	// For now, just read entire list
	// FIXME - read list only as desired, for efficiency
	found = db->get(NULL, &key, &data, 0);

	if(found == DB_NOTFOUND) {
	    throw OmDatabaseError("Database error: item not found");
	}
	Assert(found == 0); // Any other errors should cause an exception.

	// Unpack list
	string packed(reinterpret_cast<char *>(data.get_data()),
		      data.get_size());
	unpack(packed);

	free(data.get_data());
    } catch (DbException e) {
	throw OmDatabaseError("PostlistDb error:" + string(e.what()));
    }
}

SleepyList::~SleepyList()
{
    do_flush();

    // Close the list
    free(key.get_data());
    key.set_data(NULL);
}

SleepyList::itemcount_type
SleepyList::get_item_count() const
{
    return items.size();
}

void
SleepyList::move_to_start()
{
    iteration_position = items.begin();
    iteration_in_progress = true;
    iteration_at_start = true;
}

void
SleepyList::move_to_next_item()
{
    Assert(iteration_in_progress);
    if(iteration_at_start) {
	iteration_at_start = false;
    } else {
	Assert(!at_end());
	iteration_position++;
    }
}

void
SleepyList::skip_to_item(SleepyListItem::id_type id)
{
    Assert(iteration_in_progress);
    iteration_at_start = false;
    while(!at_end() && iteration_position->id < id) {
	iteration_position++;
    }
}

bool
SleepyList::at_end() const
{
    Assert(iteration_in_progress);
    return(iteration_position == items.end());
}

const SleepyListItem &
SleepyList::get_current_item() const
{
    Assert(iteration_in_progress);
    Assert(!iteration_at_start);
    Assert(!at_end());
    return *iteration_position;
}

void
SleepyList::add_item(const SleepyListItem & newitem)
{
    iteration_in_progress = false;
    modified_and_locked = true;
    // FIXME - actually get a lock

    make_entry(newitem);
}

void
SleepyList::flush()
{
    do_flush();
}

// /////////////////////
// // Private methods //
// /////////////////////

/// Compare two SleepyListItems by their ID field
class SleepyListItemLess {
    public:
	int operator() (const SleepyListItem &p1, const SleepyListItem &p2) {
	    return p1.id < p2.id;
	}
};

void
SleepyList::make_entry(const SleepyListItem & newitem) {
    vector<SleepyListItem>::iterator p;
    p = lower_bound(items.begin(), items.end(),
		    newitem,
		    SleepyListItemLess());
    if(p == items.end() || SleepyListItemLess()(newitem, *p)) {
        // An item with the specified ID is not in list yet - insert new one
	items.insert(p, newitem);
    } else {
        // An item with the specified ID is already in list - overwrite it
	*p = newitem;
    }
}

void
SleepyList::unpack(string packed)
{
    string::size_type pos = 0;
    entry_type number_of_items = readentry<entry_type>(packed, pos);

    while(items.size() < number_of_items) {
	entry_type itemsize = readentry<entry_type>(packed, pos);

	items.push_back(SleepyListItem(packed.substr(pos, itemsize)));
	pos += itemsize;
    }
}

string
SleepyList::pack() const
{
    string packed;
    entry_type temp;

    temp = items.size();
    packed.append(reinterpret_cast<char *>(&temp), sizeof(entry_type));

    vector<SleepyListItem>::const_iterator i;
    for(i = items.begin(); i != items.end(); i++) {
	packed.append(i->pack());
    }

    return packed;
}

void
SleepyList::do_flush()
{
    if(modified_and_locked) {
	// Pack list

	string packed = pack();
	Dbt data;

	try {
	    // Write list
	    int err_num = db->put(NULL, &key, &data, 0);
	    if(err_num)
		throw OmDatabaseError(string("Database error:") +
				      strerror(err_num));
	} catch (DbException e) {
	    throw OmDatabaseError("Database error:" + string(e.what()));
	}
	modified_and_locked = false;
    }
}
