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
#include <xapian/error.h>
using std::string;
using std::make_pair;

#include "omdebug.h"

void
ChertValueTable::make_key(string & key, Xapian::docid did, Xapian::valueno valueno)
{
    DEBUGCALL_STATIC(DB, void, "ChertValueTable::make_key",
		     key << ", " << did << ", " << valueno);
    (void)valueno; // no warning
    key = chert_docid_to_key(did);
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
	else throw Xapian::RangeError("Value number in value table is too large");
    }

    if (!unpack_string(pos, end, this_value)) {
	if (*pos == 0) throw Xapian::DatabaseCorruptError("Incomplete item in value table");
	else throw Xapian::RangeError("Item in value table is too large");
    }

    DEBUGLINE(DB, "ChertValueTable::unpack_entry(): value no " <<
	      this_value_no << " is `" << this_value << "'");
}

void
ChertValueTable::add_value(const string & value,
			      Xapian::docid did,
			      Xapian::valueno valueno)
{
    DEBUGCALL(DB, void, "ChertValueTable::add_value", value << ", " << did << ", " << valueno);
    string key;
    make_key(key, did, valueno);
    string tag;
    (void)get_exact_entry(key, tag);
    string newvalue;

    const char * pos = tag.data();
    const char * end = pos + tag.size();

    bool have_added = false;
    
    while (pos && pos != end) {
	Xapian::valueno this_value_no;
	string this_value;

	unpack_entry(&pos, end, &this_value_no, this_value);

	if (this_value_no > valueno && !have_added) {
	    DEBUGLINE(DB, "Adding value (number, value) = (" <<
		      valueno << ", " << value << ")");
	    have_added = true;
	    newvalue += pack_uint(valueno);
	    newvalue += pack_string(value);
	}

	newvalue += pack_uint(this_value_no);
	newvalue += pack_string(this_value);
    }
    if (!have_added) {
	DEBUGLINE(DB, "Adding value (number, value) = (" <<
		  valueno << ", " << value << ")");
	have_added = true;
	newvalue += pack_uint(valueno);
	newvalue += pack_string(value);
    }

    add(key, newvalue);
}

void
ChertValueTable::get_value(string & value,
			      Xapian::docid did,
			      Xapian::valueno valueno) const
{
    DEBUGCALL(DB, void, "ChertValueTable::get_value", value << ", " << did << ", " << valueno);
    string key;
    make_key(key, did, valueno);
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
    make_key(key, did, 0);
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
ChertValueTable::delete_all_values(Xapian::docid did)
{
    DEBUGCALL(DB, void, "ChertValueTable::delete_all_values", did);
    string key;
    make_key(key, did, 0);
    del(key);
}
