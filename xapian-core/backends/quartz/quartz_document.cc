/* quartz_document.cc: Implementation of document for Quartz database
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

#include "om/omdocument.h"
#include "omdebug.h"
#include "quartz_database.h"
#include "quartz_document.h"
#include "quartz_values.h"
#include "quartz_record.h"

/** Create a QuartzDocument: this is only called by
 *  QuartzDatabase::open_document().
 */
QuartzDocument::QuartzDocument(RefCntPtr<const Database> database_,
			       QuartzTable *value_table_,
			       QuartzTable *record_table_,
			       om_docid did_, bool lazy)
	: Document(database_.get(), did_),
	  database(database_),
	  value_table(value_table_),
	  record_table(record_table_)
{
    DEBUGCALL(DB, void, "QuartzDocument", "[database_], " << value_table_ << ", " << record_table_ << ", " << did_ << ", " << lazy);
    // FIXME: this should work but isn't great - in fact I wonder if
    // we should cache the results anyway...
    if (!lazy) (void)QuartzRecordManager::get_record(*record_table, did);
}

/** Destroy a QuartzDocument.
 */
QuartzDocument::~QuartzDocument()
{
    DEBUGCALL(DB, void, "~QuartzDocument", "");
}

/** Retrieve a value from the database
 *
 *  @param valueid	The value number to retrieve.
 */
OmValue
QuartzDocument::do_get_value(om_valueno valueid) const
{
    DEBUGCALL(DB, OmValue, "QuartzDocument::do_get_value", valueid);
    OmValue retval;
    QuartzValueManager::get_value(*value_table, retval, did, valueid);
    RETURN(retval);
}

/** Retrieve all value values from the database
 */
map<om_valueno, OmValue>
QuartzDocument::do_get_all_values() const
{
    DEBUGCALL(DB, void, "QuartzDocument::do_get_all_values", "");
    map<om_valueno, OmValue> values;
    QuartzValueManager::get_all_values(*value_table, values, did);
    return values;
}

/** Retrieve the document data from the database
 */
string
QuartzDocument::do_get_data() const
{
    DEBUGCALL(DB, string, "QuartzDocument::do_get_data", "");
    RETURN(QuartzRecordManager::get_record(*record_table, did));
}
