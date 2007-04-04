/* da_document.cc: C++ class for reading DA records
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2003,2004 Olly Betts
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
#include "da_database.h"
#include "da_document.h"
#include "daread.h"
#include "omdebug.h"

/** Create a DADocument: this is only called by DADatabase::open_document()
 */
DADocument::DADocument(const DADatabase * database_, Xapian::docid did_,
		       bool heavy_duty_, bool lazy)
        : Xapian::Document::Internal(database_, did_), database(database_),
	  rec(NULL), heavy_duty(heavy_duty_)
{
    if (!lazy) rec = database->get_record(did);
}

/** Clean up the document: frees the record structure.  The creating
 *  DADatabase must still exist at the time this is called.
 */
DADocument::~DADocument()
{
    if (rec != NULL) M_lose_record(rec);
}

/** Get the value for a DA document.
 *  If a value file is available, this will be used to provide extremely fast
 *  value lookup.
 */
string
DADocument::do_get_value(Xapian::valueno valueid) const
{
    if (valueid == 0) return database->get_value(did, valueid);

    DebugMsg("Looking in record for valueno " << valueid <<
	     " in document " << did);
    if (rec == 0) rec = database->get_record(did);

    string value;
    unsigned char *pos = reinterpret_cast<unsigned char *>(rec->p);
    unsigned int len = LENGTH_OF(pos, 0, heavy_duty);
    unsigned int valuepos = valueid;
    if (valuepos + 8 > len) {
	// Record not big enough.
	DEBUGLINE(DB, ": not found in record");
    } else {
	value = string(reinterpret_cast<char *>(pos) + LWIDTH(heavy_duty) + 3 + valuepos, 8);
	DEBUGLINE(DB, ": found in record - value is `" << value << "'");
    }
    return value;
}

/** Get all the values for a DA document.
 *
 *  Note: this only returns values from the valuefile.  If values are being
 *  read from the record, this will not return them.
 */
map<Xapian::valueno, string>
DADocument::do_get_all_values() const
{
    Xapian::valueno valueid = 0;
    map<Xapian::valueno, string> values;

    string value = database->get_value(did, valueid);
    if (!value.empty()) values[valueid] = value;

    return values;
}

/** Get the data for a DA Document.
 *  This can be expensive, and shouldn't normally be used
 *  during the match operation (such as in a match decider functor):
 *  use a value instead, if at all possible.
 */
string
DADocument::do_get_data() const
{
    if (rec == 0) rec = database->get_record(did);
    unsigned char *pos = reinterpret_cast<unsigned char *>(rec->p);
    unsigned int len = LENGTH_OF(pos, 0, heavy_duty);
    return string(reinterpret_cast<char *>(pos) + LWIDTH(heavy_duty) + 3,
		  len - LWIDTH(heavy_duty) - 3);
}
