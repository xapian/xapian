/* quartz_attributes.cc: Attributes in quartz databases
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

#include "quartz_attributes.h"
#include "quartz_utils.h"
#include "utils.h"
#include "om/omerror.h"

#include "omdebug.h"

void
QuartzAttributesManager::make_key(QuartzDbKey & key,
				  om_docid did,
				  om_keyno keyno)
{
    key.value = pack_uint(did);
}

void
QuartzAttributesManager::unpack_entry(const char ** pos,
				      const char * end,
				      om_keyno * this_attrib_no,
				      std::string & this_attribute)
{
    if (!unpack_uint(pos, end, this_attrib_no)) {
	if (*pos == 0) throw OmDatabaseCorruptError("Incomplete item in attribute table");
	else throw OmRangeError("Key number in attribute table is too large");
    }

    if (!unpack_string(pos, end, this_attribute)) {
	if (*pos == 0) throw OmDatabaseCorruptError("Incomplete item in attribute table");
	else throw OmRangeError("Item in attribute table is too large");
    }

    DEBUGLINE(DB, "QuartzAttributesManager::unpack_entry(): attrib no " <<
	      this_attrib_no << " is `" << this_attribute << "'");
}

void
QuartzAttributesManager::add_attribute(QuartzBufferedTable & table,
				       const OmKey & attribute,
				       om_docid did,
				       om_keyno keyno)
{
    QuartzDbKey key;
    make_key(key, did, keyno);
    QuartzDbTag * tag = table.get_or_make_tag(key);
    std::string newvalue;

    const char * pos = tag->value.data();
    const char * end = pos + tag->value.size();

    bool have_added = false;
    
    while (pos && pos != end) {
	om_keyno this_attrib_no;
	std::string this_attribute;

	DEBUGLINE(DB, "Pos, end " << (void *)pos << ", " << (void *)end);
	unpack_entry(&pos, end, &this_attrib_no, this_attribute);
	DEBUGLINE(DB, "EndPos, end " << (void *)pos << ", " << (void *)end);

	if (this_attrib_no > keyno && !have_added) {
	    DEBUGLINE(DB, "Adding attribute (number, value) = (" <<
		      keyno << ", " << attribute.value << ")");
	    have_added = true;
	    newvalue += pack_uint(keyno);
	    newvalue += pack_string(attribute.value);
	}

	newvalue += pack_uint(this_attrib_no);
	newvalue += pack_string(this_attribute);
    }
    if (!have_added) {
	DEBUGLINE(DB, "Adding attribute (number, value) = (" <<
		  keyno << ", " << attribute.value << ")");
	have_added = true;
	newvalue += pack_uint(keyno);
	newvalue += pack_string(attribute.value);
    }
    tag->value = newvalue;
    DEBUGLINE(DB, "Pos, end " << (void *)tag->value.data() <<
	      ", " << (void *)(tag->value.data() + tag->value.size()));
}

void
QuartzAttributesManager::get_attribute(const QuartzTable & table,
				       OmKey & attribute,
				       om_docid did,
				       om_keyno keyno)
{
    QuartzDbKey key;
    make_key(key, did, keyno);
    QuartzDbTag tag;
    bool found = table.get_exact_entry(key, tag);

    if (found) {
	const char * pos = tag.value.data();
	const char * end = pos + tag.value.size();

	while (pos && pos != end) {
	    om_keyno this_attrib_no;
	    std::string this_attribute;

	    unpack_entry(&pos, end, &this_attrib_no, attribute.value);

	    if (this_attrib_no == keyno) {
		return;
	    }
	}
    }
    attribute.value = "";
}

void
QuartzAttributesManager::get_all_attributes(const QuartzTable & table,
					    std::map<om_keyno, OmKey> & attributes,
					    om_docid did)
{
    QuartzDbKey key;
    make_key(key, did, 0);
    QuartzDbTag tag;
    bool found = table.get_exact_entry(key, tag);

    attributes.clear();
    if (!found) return;

    const char * pos = tag.value.data();
    const char * end = pos + tag.value.size();

    while (pos && pos != end) {
	om_keyno this_attrib_no;
	std::string this_attribute;

	DEBUGLINE(DB, "Pos, end " << (void *)pos << ", " << (void *)end);
	unpack_entry(&pos, end, &this_attrib_no, this_attribute);
	DEBUGLINE(DB, "NewPos, end " << (void *)pos << ", " << (void *)end);
	attributes.insert(make_pair(this_attrib_no, OmKey(this_attribute)));
    }
}

