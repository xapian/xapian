/* sleepycat_list.cc: lists of data from a sleepycat database
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

#include <algorithm>

#include "omdebug.h"
#include "sleepycat_list.h"

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
typedef SleepycatListItem::id_type entry_type;
/// A type which can hold a document length.  FIXME: make this portable.
typedef om_doclength len_type;

/** Read an type X from the specified position in the string,
 *  and update the position.
 *
 *  @exception OmDatabaseError thrown if string is not long enough.
 *
 *  @param packed  The string to read the type X from.
 *  @param pos     The offset to start reading the type X at.
 */
template<class X>
static const X readentry(const std::string &packed,
			 std::string::size_type & pos)
{
    std::string::size_type endpos = pos + sizeof(X) / sizeof(char);

    if(endpos > packed.length()) {
	throw(OmDatabaseError("Database corrupt - unexpected end of item."));
    }

    X entry = * reinterpret_cast<const X *>(packed.data() + pos * sizeof(char));
    pos = endpos;

    return entry;
}


SleepycatListItem::SleepycatListItem(id_type id_,
			       om_termcount wdf_,
			       const OmDocumentTerm::term_positions & positions_,
			       om_doccount termfreq_,
			       om_doclength doclength_)
	: id(id_),
	  wdf(wdf_),
	  positions(positions_),
	  termfreq(termfreq_),
	  doclength(doclength_)
{
}

SleepycatListItem::SleepycatListItem(std::string packed,
			       bool store_termfreq)
	: id(0),
	  wdf(0),
	  termfreq(0),
	  doclength(0)
{
    std::string::size_type pos = 0;

    id = readentry<id_type>(packed, pos);
    wdf = readentry<entry_type>(packed, pos);
    if(store_termfreq) {
	termfreq = readentry<entry_type>(packed, pos);
    }
    doclength = readentry<len_type>(packed, pos);

    std::vector<om_termpos>::size_type positions_size;
    positions_size = readentry<entry_type>(packed, pos);

    while(positions.size() < positions_size) {
	positions.push_back(readentry<entry_type>(packed, pos));
    }
}


std::string
SleepycatListItem::pack(bool store_termfreq) const
{
    std::string packed;
    id_type idtemp;
    entry_type entrytemp;
    len_type lentemp;

    idtemp = id;
    packed.append(reinterpret_cast<char *>(&idtemp), sizeof(idtemp));

    entrytemp = wdf;
    packed.append(reinterpret_cast<char *>(&entrytemp), sizeof(entrytemp));

    if(store_termfreq) {
	entrytemp = termfreq;
	packed.append(reinterpret_cast<char *>(&entrytemp), sizeof(entrytemp));
    }

    lentemp = doclength;
    packed.append(reinterpret_cast<char *>(&lentemp), sizeof(lentemp));

    entrytemp = positions.size();
    packed.append(reinterpret_cast<char *>(&entrytemp), sizeof(entry_type));

    OmDocumentTerm::term_positions::const_iterator i;
    for(i = positions.begin(); i != positions.end(); i++) {
	entrytemp = *i;
	packed.append(reinterpret_cast<char *>(&entrytemp), sizeof(entry_type));
    }

    return packed;
}



SleepycatList::SleepycatList(Db * db_, void * keydata_, size_t keylen_,
		       bool throw_if_not_found,
		       bool store_termfreq_,
		       bool store_wdf_,
		       bool store_positional_,
		       bool store_wdfsum_)
	: db(db_),
	  modified_and_locked(false),
	  iteration_in_progress(false),
	  wdfsum(0),
	  store_termfreq(store_termfreq_),
	  store_wdfsum(store_wdfsum_)
{
    // Copy and store key data
    void *keydata = malloc(keylen_);
    if(keydata == NULL) throw std::bad_alloc();
    memcpy(keydata, keydata_, keylen_);
    key.set_data(keydata);
    key.set_size(keylen_);

    try {
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
		// Item not found: we have an empty list.
		if (throw_if_not_found)
		    // FIXME: wrong error if this is a posting list...
		    throw OmDocNotFoundError("Can't open termlist");
	    } else {
		Assert(found == 0); // Any other errors should cause an exception.

		// Unpack list
		std::string packed(reinterpret_cast<char *>(data.get_data()),
				   data.get_size());
		unpack(packed);

		free(data.get_data());
	    }
	} catch (DbException e) {
	    throw OmDatabaseError("PostlistDb error:" + std::string(e.what()));
	}
    } catch (...) {
	free(key.get_data());
	key.set_data(NULL);
	throw;
    }
}

SleepycatList::~SleepycatList()
{
    do_flush();

    // Close the list
    free(key.get_data());
    key.set_data(NULL);
}

SleepycatList::itemcount_type
SleepycatList::get_item_count() const
{
    return items.size();
}

om_termcount
SleepycatList::get_wdfsum() const
{
    return wdfsum;
}

void
SleepycatList::move_to_start()
{
    iteration_position = items.begin();
    iteration_in_progress = true;
    iteration_at_start = true;
}

void
SleepycatList::move_to_next_item()
{
    Assert(iteration_in_progress);
    if(iteration_at_start) {
	DEBUGLINE(DB, "SleepycatList[" << this << "]::move_to_next_item() - " <<
		  "moved to first item, ID=" << iteration_position->id);
	iteration_at_start = false;
    } else {
	Assert(!at_end());
	iteration_position++;
	DEBUGLINE(DB, "SleepycatList[" << this << "]::move_to_next_item() - " <<
		  "moved to " <<
		  (at_end() ? "end" :
		   "item ID=" + om_tostring(iteration_position->id)));
    }
}

void
SleepycatList::skip_to_item(SleepycatListItem::id_type id)
{
    Assert(iteration_in_progress);
    iteration_at_start = false;
    while(!at_end() && iteration_position->id < id) {
	iteration_position++;
	DEBUGLINE(DB, "SleepycatList[" << this << "]::skip_to_item() - advanced");
    }
    DEBUGLINE(DB, "SleepycatList[" << this << "]::skip_to_item() - " <<
	      "skipped to " <<
		  (at_end() ? "end" :
		   "item ID=" + om_tostring(iteration_position->id)));
}

bool
SleepycatList::at_end() const
{
    Assert(iteration_in_progress);
    return(iteration_position == items.end());
}

const SleepycatListItem &
SleepycatList::get_current_item() const
{
    Assert(iteration_in_progress);
    Assert(!iteration_at_start);
    Assert(!at_end());
    return *iteration_position;
}

void
SleepycatList::add_item(const SleepycatListItem & newitem)
{
    iteration_in_progress = false;
    modified_and_locked = true;
    // FIXME - actually get a lock

    make_entry(newitem);
    DEBUGLINE(DB, "wdfsum(" << wdfsum << ") += newitem.wdf(" << newitem.wdf << ")");
    wdfsum += newitem.wdf;
}

void
SleepycatList::flush()
{
    do_flush();
}

// /////////////////////
// // Private methods //
// /////////////////////

/// Compare two SleepycatListItems by their ID field
class SleepycatListItemLess {
    public:
	int operator() (const SleepycatListItem &p1, const SleepycatListItem &p2) {
	    return p1.id < p2.id;
	}
};

void
SleepycatList::make_entry(const SleepycatListItem & newitem)
{
    std::vector<SleepycatListItem>::iterator p;
    p = std::lower_bound(items.begin(), items.end(),
			 newitem,
			 SleepycatListItemLess());
    if(p == items.end() || SleepycatListItemLess()(newitem, *p)) {
        // An item with the specified ID is not in list yet - insert new one
	items.insert(p, newitem);
    } else {
        // An item with the specified ID is already in list - overwrite it
	*p = newitem;
    }
}

void
SleepycatList::unpack(std::string packed)
{
    std::string::size_type pos = 0;
    if(store_wdfsum) {
	wdfsum = readentry<entry_type>(packed, pos);
    }

    entry_type number_of_items = readentry<entry_type>(packed, pos);
    while(items.size() < number_of_items) {
	entry_type itemsize = readentry<entry_type>(packed, pos);

	items.push_back(SleepycatListItem(packed.substr(pos, itemsize),
				       store_termfreq));
	pos += itemsize;
    }
}

std::string
SleepycatList::pack() const
{
    std::string packed;
    entry_type temp;

    if(store_wdfsum) {
	temp = wdfsum;
	packed.append(reinterpret_cast<char *>(&temp), sizeof(entry_type));
    }

    temp = items.size();
    packed.append(reinterpret_cast<char *>(&temp), sizeof(entry_type));

    std::vector<SleepycatListItem>::const_iterator i;
    for(i = items.begin(); i != items.end(); i++) {
	std::string packeditem = i->pack(store_termfreq);
	temp = packeditem.size();
	packed.append(reinterpret_cast<char *>(&temp), sizeof(entry_type));
	packed.append(packeditem);
    }

    return packed;
}

void
SleepycatList::do_flush()
{
    if(modified_and_locked) {
	// Pack list
	std::string packed = pack();
	Dbt data(const_cast<char *>(packed.data()), packed.size());

	try {
	    // Write list
	    int err_num = db->put(NULL, &key, &data, 0);
	    if(err_num) throw OmDatabaseError(std::string("Database error:") +
					      strerror(err_num));
	} catch (DbException e) {
	    throw OmDatabaseError("Database error:" + std::string(e.what()));
	}
	modified_and_locked = false;
    }
}
