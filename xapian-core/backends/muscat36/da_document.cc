/* da_document.cc: C++ class for reading DA records
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

#include <om/omdocument.h>
#include "da_database.h"
#include "da_document.h"
#include "daread.h"
#include "omdebug.h"

/** Create a DADocument: this is only called by DADatabase::open_document()
 */
DADocument::DADocument(const DADatabase * database_,
		       om_docid did_,
		       int heavy_duty_)
        : database(database_),
	  did(did_),
	  rec(NULL),
	  heavy_duty(heavy_duty_)
{
}

/** Clean up the document: frees the record structure.  The creating
 *  DADatabase must still exist at the time this is called.
 */
DADocument::~DADocument()
{
    if(rec != NULL) M_lose_record(rec);
}

/** Get the key for a DA document.
 *  If a key file is available, this will be used to provide extremely fast
 *  key lookup.
 */
OmKey
DADocument::do_get_key(om_keyno keyid) const
{
    OmKey key = database->get_key(did, keyid);

    if (key.value.size() == 0 && keyid != 0) {
	DebugMsg("Looking in record for keyno " << keyid <<
		 " in document " << did);
	if (rec == 0) rec = database->get_record(did);

	unsigned char *pos = (unsigned char *)rec->p;
	unsigned int len = LENGTH_OF(pos, 0, heavy_duty);
	unsigned int keypos = keyid;
	if (keypos + 8 > len) {
	    // Record not big enough.
	    DEBUGLINE(DB, ": not found in record");
	} else {
	    key.value = std::string((char *)pos + LWIDTH(heavy_duty) + 3 + keypos, 8);
	    DEBUGLINE(DB, ": found in record - value is `" << key.value << "'");
	}
    }

    return key;
}

/** Get all the keys for a DA document.
 */
std::map<om_keyno, OmKey>
DADocument::do_get_all_keys() const
{

    throw OmUnimplementedError("DaDocument::do_get_all_keys() unimplemented");
}

/** Get the data for a DA Document.
 *  This can be expensive, and shouldn't normally be used
 *  during the match operation (such as in a match decider functor):
 *  use a key instead, if at all possible.
 */
OmData
DADocument::do_get_data() const
{
    if(rec == 0) rec = database->get_record(did);
    OmData data;
    unsigned char *pos = (unsigned char *)rec->p;
    unsigned int len = LENGTH_OF(pos, 0, heavy_duty);
    data.value = std::string((char *)pos + LWIDTH(heavy_duty) + 3,
			len - LWIDTH(heavy_duty) - 3);
    return data;
}
