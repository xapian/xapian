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
SleepyListItem::pack()
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
	  modified_and_locked(false)
{
    // Copy and store key data
    void *keydata = malloc(keylen_);
    if(keydata == NULL) throw std::bad_alloc();
    memcpy(keydata, keydata_, keylen_);
    key.set_data(keydata);
    key.set_size(keylen_);

    // Read database
    Dbt data;
    data.set_flags(DB_DBT_MALLOC);
    int found;

    try {
	// For now, just read entire list
	// FIXME - read list only as desired, for efficiency
	found = db->get(NULL, &key, &data, 0);

	// Unpack list
	if(found != DB_NOTFOUND) {
	    Assert(found == 0); // Any other errors should cause an exception.

	    string packed(reinterpret_cast<char *>(data.get_data()),
			  data.get_size());

	    string::size_type pos = 0;
	    entry_type number_of_items = readentry<entry_type>(packed, pos);

	    while(items.size() < number_of_items) {
		entry_type itemsize = readentry<entry_type>(packed, pos);

		items.push_back(SleepyListItem(packed.substr(pos, itemsize)));
		pos += itemsize;
	    }
	}
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

void
SleepyList::flush()
{
    do_flush();
}

void
SleepyList::add(const SleepyListItem & newitem)
{
    modified_and_locked = true;
    throw OmUnimplementedError("SleepyList::add() not yet implemented");
}

// /////////////////////
// // Private methods //
// /////////////////////


void
SleepyList::do_flush()
{
    if(modified_and_locked) {
	throw OmUnimplementedError("SleepyList::do_flush() writing to database not yet implemented");
	// Pack entry
	;

	try {
	    // Write list
	    //found = db->put(NULL, &key, &data, 0);
	} catch (DbException e) {
	    throw OmDatabaseError("Database error:" + string(e.what()));
	}
	modified_and_locked = false;
    }
}
