/* quartz_values.cc: Values in quartz databases
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
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

#include <config.h>
#include "quartz_values.h"
#include "quartz_utils.h"
#include "utils.h"
#include "om/omerror.h"
using std::string;
using std::make_pair;

#include "omdebug.h"

void
QuartzValueManager::make_key(QuartzDbKey & key,
				om_docid did,
				om_valueno valueno)
{
    DEBUGCALL_STATIC(DB, void, "QuartzValueManager::make_key", "QuartzDbKey(" << key.value << "), " << did << ", " << valueno);
    (void)valueno; // no warning
    key.value = pack_uint(did);
}

void
QuartzValueManager::unpack_entry(const char ** pos,
				 const char * end,
				 om_valueno * this_attrib_no,
				 string & this_value)
{
    DEBUGCALL_STATIC(DB, void, "QuartzValueManager::unpack_entry", pos << ", " << end << ", " << this_attrib_no << ", " << this_value);
    if (!unpack_uint(pos, end, this_attrib_no)) {
	if (*pos == 0) throw OmDatabaseCorruptError("Incomplete item in value table");
	else throw OmRangeError("Value number in value table is too large");
    }

    if (!unpack_string(pos, end, this_value)) {
	if (*pos == 0) throw OmDatabaseCorruptError("Incomplete item in value table");
	else throw OmRangeError("Item in value table is too large");
    }

    DEBUGLINE(DB, "QuartzValueManager::unpack_entry(): attrib no " <<
	      this_attrib_no << " is `" << this_value << "'");
}

void
QuartzValueManager::add_value(QuartzBufferedTable & table,
			       const string & value,
			       om_docid did,
			       om_valueno valueno)
{
    DEBUGCALL_STATIC(DB, void, "QuartzValueManager::add_value", "[table], " << value << ", " << did << ", " << valueno);
    QuartzDbKey key;
    make_key(key, did, valueno);
    QuartzDbTag * tag = table.get_or_make_tag(key);
    string newvalue;

    const char * pos = tag->value.data();
    const char * end = pos + tag->value.size();

    bool have_added = false;
    
    while (pos && pos != end) {
	om_valueno this_attrib_no;
	string this_value;

	unpack_entry(&pos, end, &this_attrib_no, this_value);

	if (this_attrib_no > valueno && !have_added) {
	    DEBUGLINE(DB, "Adding value (number, value) = (" <<
		      valueno << ", " << value << ")");
	    have_added = true;
	    newvalue += pack_uint(valueno);
	    newvalue += pack_string(value);
	}

	newvalue += pack_uint(this_attrib_no);
	newvalue += pack_string(this_value);
    }
    if (!have_added) {
	DEBUGLINE(DB, "Adding value (number, value) = (" <<
		  valueno << ", " << value << ")");
	have_added = true;
	newvalue += pack_uint(valueno);
	newvalue += pack_string(value);
    }
    tag->value = newvalue;
}

void
QuartzValueManager::get_value(const QuartzTable & table,
				       string & value,
				       om_docid did,
				       om_valueno valueno)
{
    DEBUGCALL_STATIC(DB, void, "QuartzValueManager::get_value", "[table], " << value << ", " << did << ", " << valueno);
    QuartzDbKey key;
    make_key(key, did, valueno);
    QuartzDbTag tag;
    bool found = table.get_exact_entry(key, tag);

    if (found) {
	const char * pos = tag.value.data();
	const char * end = pos + tag.value.size();

	while (pos && pos != end) {
	    om_valueno this_value_no;
	    string this_value;

	    unpack_entry(&pos, end, &this_value_no, this_value);

	    if (this_value_no == valueno) {
		value = this_value;
		return;
	    }
	}
    }
    value = "";
}

void
QuartzValueManager::get_all_values(const QuartzTable & table,
				map<om_valueno, string> & values,
				om_docid did)
{
    DEBUGCALL_STATIC(DB, void, "QuartzValueManager::get_all_values", "[table], [values], " << did);
    QuartzDbKey key;
    make_key(key, did, 0);
    QuartzDbTag tag;
    bool found = table.get_exact_entry(key, tag);

    values.clear();
    if (!found) return;

    const char * pos = tag.value.data();
    const char * end = pos + tag.value.size();

    while (pos && pos != end) {
	om_valueno this_value_no;
	string this_value;

	unpack_entry(&pos, end, &this_value_no, this_value);
	values.insert(make_pair(this_value_no, this_value));
    }
}

void
QuartzValueManager::delete_all_values(QuartzBufferedTable & table, om_docid did)
{
    DEBUGCALL_STATIC(DB, void, "QuartzValueManager::delete_all_values", "[table], " << did);
    QuartzDbKey key;
    make_key(key, did, 0);
    QuartzDbTag tag;
    bool found = table.get_exact_entry(key, tag);

    if (found) {
	table.delete_tag(key);
    }
}
