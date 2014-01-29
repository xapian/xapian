/* flint_values.cc: Values in flint databases
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
#include "flint_values.h"
#include "flint_utils.h"
#include "utils.h"
#include <xapian/error.h>
using std::string;
using std::make_pair;

#include "debuglog.h"

/** Generate key for document @a docid's values. */
inline void
make_key(string & key, Xapian::docid did)
{
    LOGCALL_STATIC_VOID(DB, "make_key", key | did);
    key = flint_docid_to_key(did);
}

void
FlintValueTable::unpack_entry(const char ** pos,
				 const char * end,
				 Xapian::valueno * this_value_no,
				 string & this_value)
{
    LOGCALL_STATIC_VOID(DB, "FlintValueTable::unpack_entry", pos | (const void*)end | this_value_no | this_value);
    if (!F_unpack_uint(pos, end, this_value_no)) {
	if (*pos == 0) throw Xapian::DatabaseCorruptError("Incomplete item in value table");
	else throw Xapian::RangeError("Value number in value table is too large");
    }

    if (!F_unpack_string(pos, end, this_value)) {
	if (*pos == 0) throw Xapian::DatabaseCorruptError("Incomplete item in value table");
	else throw Xapian::RangeError("Item in value table is too large");
    }

    LOGLINE(DB, "FlintValueTable::unpack_entry(): value no " <<
	    this_value_no << " is `" << this_value << "'");
}

void
FlintValueTable::encode_values(string & s,
			       Xapian::ValueIterator it,
			       const Xapian::ValueIterator & end)
{
    LOGCALL_VOID(DB, "FlintValueTable::encode_values", s | it | end);
    while (it != end) {
	s += F_pack_uint(it.get_valueno());
	s += F_pack_string(*it);
	++it;
    }
}
 
void
FlintValueTable::set_encoded_values(Xapian::docid did, const string & enc)
{
    LOGCALL_VOID(DB, "FlintValueTable::set_encoded_values", did | enc);
    string key;
    make_key(key, did);
    add(key, enc);
}

void
FlintValueTable::get_value(string & value,
			      Xapian::docid did,
			      Xapian::valueno slot) const
{
    LOGCALL_VOID(DB, "FlintValueTable::get_value", value | did | slot);
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

	    if (this_value_no == slot) {
		value = this_value;
		return;
	    }

	    // Values are stored in sorted order.
	    if (this_value_no > slot) break;
	}
    }
    value = string();
}

void
FlintValueTable::get_all_values(map<Xapian::valueno, string> & values,
				   Xapian::docid did) const
{
    LOGCALL_VOID(DB, "FlintValueTable::get_all_values", values | did);
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
FlintValueTable::delete_all_values(Xapian::docid did)
{
    LOGCALL_VOID(DB, "FlintValueTable::delete_all_values", did);
    string key;
    make_key(key, did);
    del(key);
}
