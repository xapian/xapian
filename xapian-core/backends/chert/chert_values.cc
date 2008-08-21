/* chert_values.cc: Values in chert databases
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2002,2003,2004,2005,2008 Olly Betts
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301
 * USA
 */

#include <config.h>
#include "chert_values.h"
#include "chert_utils.h"
#include "utils.h"
#include "valuestats.h"
#include <xapian/error.h>
using std::string;
using std::make_pair;

#include "omdebug.h"

/** Generate key for document @a docid's values. */
inline void
make_key(string & key, Xapian::docid did)
{
    DEBUGCALL_STATIC(DB, void, "make_key", key << ", " << did);
    key = chert_docid_to_key(did);
}

/** Generate a key for a value statistics item. */
inline void
make_valuestats_key(string & key, Xapian::valueno valueno)
{
    DEBUGCALL_STATIC(DB, void, "make_valuestats_key", key << ", " << valueno);
    key = "\xff" + chert_docid_to_key(valueno);
}

void
ChertValueTable::unpack_entry(const char ** pos,
				 const char * end,
				 Xapian::valueno * this_value_no,
				 string & this_value)
{
    DEBUGCALL_STATIC(DB, void, "ChertValueTable::unpack_entry",
		     "[pos], [end], " << this_value_no << ", " << this_value);
    if (!unpack_uint(pos, end, this_value_no)) {
	if (*pos == 0) throw Xapian::DatabaseCorruptError("Incomplete item in value table");
	throw Xapian::RangeError("Value number in value table is too large");
    }

    if (!unpack_string(pos, end, this_value)) {
	if (*pos == 0) throw Xapian::DatabaseCorruptError("Incomplete item in value table");
	throw Xapian::RangeError("Item in value table is too large");
    }

    LOGLINE(DB, "ChertValueTable::unpack_entry(): value no " <<
	    this_value_no << " is `" << this_value << "'");
}

void
ChertValueTable::encode_values(string & s,
			       Xapian::ValueIterator it,
			       const Xapian::ValueIterator & end,
			       map<Xapian::valueno, ValueStats> & stats)
{
    DEBUGCALL(DB, void, "ChertValueTable::encode_values", "[&s], " << it << ", " << end << ", [stats]");
    while (it != end) {
	Xapian::valueno valueno = it.get_valueno();
	string value = *it;

	s += pack_uint(valueno);
	s += pack_string(value);

	// Update the statistics.
	std::pair<map<Xapian::valueno, ValueStats>::iterator, bool> i;
	i = stats.insert(make_pair(valueno, ValueStats()));
	if (i.second) {
	    // There were no statistics stored already, so read them.
	    get_value_stats(i.first->second, valueno);
	}

	// Now, modify the stored statistics.
	if ((i.first->second.freq)++ == 0) {
	    // If the value count was previously zero, set the upper and lower
	    // bounds to the newly added value.
	    i.first->second.lower_bound = value;
	    i.first->second.upper_bound = value;
	} else {
	    // Otherwise, simply make sure they reflect the new value.
	    if (value < i.first->second.lower_bound) {
		i.first->second.lower_bound = value;
	    }
	    if (value > i.first->second.upper_bound) {
		i.first->second.upper_bound = value;
	    }
	}

	++it;
    }
}

void
ChertValueTable::set_encoded_values(Xapian::docid did, const string & enc)
{
    DEBUGCALL(DB, void, "ChertValueTable::set_encoded_values", did << ", " << enc);
    string key;
    make_key(key, did);
    add(key, enc);
}

void
ChertValueTable::get_value(string & value,
			      Xapian::docid did,
			      Xapian::valueno valueno) const
{
    DEBUGCALL(DB, void, "ChertValueTable::get_value", value << ", " << did << ", " << valueno);
    string key;
    make_key(key, did);
    string tag;
    bool found = get_exact_entry(key, tag);

    if (found) {
	const char * pos = tag.data();
	const char * end = pos + tag.size();

	while (pos && pos != end) {
	    Xapian::valueno this_value_no;
	    string this_value;

	    unpack_entry(&pos, end, &this_value_no, this_value);

	    if (this_value_no == valueno) {
		value = this_value;
		return;
	    }

	    // Values are stored in sorted order.
	    if (this_value_no > valueno) break;
	}
    }
    value = "";
}

void
ChertValueTable::get_all_values(map<Xapian::valueno, string> & values,
				   Xapian::docid did) const
{
    DEBUGCALL(DB, void, "ChertValueTable::get_all_values", "[values], " << did);
    string key;
    make_key(key, did);
    string tag;
    bool found = get_exact_entry(key, tag);

    values.clear();
    if (!found) return;

    const char * pos = tag.data();
    const char * end = pos + tag.size();

    while (pos && pos != end) {
	Xapian::valueno this_value_no;
	string this_value;

	unpack_entry(&pos, end, &this_value_no, this_value);
	values.insert(make_pair(this_value_no, this_value));
    }
}


void
ChertValueTable::get_value_stats(ValueStats & stats,
				 Xapian::valueno valueno) const
{
    DEBUGCALL(DB, void, "ChertValueTable::get_value_stats", "stats, " << valueno);
    stats.clear();
    string key;
    make_valuestats_key(key, valueno);
    string tag;
    if (get_exact_entry(key, tag)) {
	const char * pos = tag.data();
	const char * end = pos + tag.size();

	if (!unpack_uint(&pos, end, &(stats.freq))) {
	    if (*pos == 0) throw Xapian::DatabaseCorruptError("Incomplete stats item in value table");
	    throw Xapian::RangeError("Frequency statistic in value table is too large");
	}
	if (!unpack_string(&pos, end, stats.lower_bound)) {
	    if (*pos == 0) throw Xapian::DatabaseCorruptError("Incomplete stats item in value table");
	    throw Xapian::RangeError("Lower bound in value table is too large");
	}
	if (!unpack_string(&pos, end, stats.upper_bound)) {
	    if (*pos == 0) throw Xapian::DatabaseCorruptError("Incomplete stats item in value table");
	    throw Xapian::RangeError("Upper bound in value table is too large");
	}
	if (pos != end) {
	    throw Xapian::DatabaseCorruptError("Junk found at end of value stats item");
	}
    }
}

void
ChertValueTable::set_value_stats(const ValueStats & stats,
				 Xapian::valueno valueno)
{
    DEBUGCALL(DB, void, "ChertValueTable::set_value_stats", "stats, " << valueno);
    string key;
    make_valuestats_key(key, valueno);

    if (stats.freq != 0) {
	string new_value;
	new_value += pack_uint(stats.freq);
	new_value += pack_string(stats.lower_bound);
	new_value += pack_string(stats.upper_bound);
	add(key, new_value);
    } else {
	del(key);
    }
}

void
ChertValueTable::delete_all_values(Xapian::docid did,
				   map<Xapian::valueno, ValueStats> & stats)
{
    DEBUGCALL(DB, void, "ChertValueTable::delete_all_values", did);
    string key;
    make_key(key, did);
    string tag;
    bool found = get_exact_entry(key, tag);
    if (!found) return;

    // Read the tag to get the old value entries.
    const char * pos = tag.data();
    const char * end = pos + tag.size();
    while (pos && pos != end) {
	Xapian::valueno this_value_no;
	string this_value;

	unpack_entry(&pos, end, &this_value_no, this_value);
	std::pair<map<Xapian::valueno, ValueStats>::iterator, bool> i;
	i = stats.insert(make_pair(this_value_no, ValueStats()));
	if (i.second) {
	    // There were no statistics stored already, so read them.
	    get_value_stats(i.first->second, this_value_no);
	}

	// Now, modify the stored statistics.
	AssertRelParanoid(i.first->second.freq, >, 0);
	if (--(i.first->second.freq) == 0) {
	    i.first->second.lower_bound.resize(0);
	    i.first->second.upper_bound.resize(0);
	}
    }

    del(key);
}
