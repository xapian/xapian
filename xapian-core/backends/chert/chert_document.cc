/* chert_document.cc: Implementation of document for Chert database
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2003,2004,2008 Olly Betts
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
#include "omdebug.h"
#include "chert_database.h"
#include "chert_document.h"
#include "chert_values.h"
#include "chert_record.h"

/** Create a ChertDocument: this is only called by
 *  ChertDatabase::open_document().
 */
ChertDocument::ChertDocument(Xapian::Internal::RefCntPtr<const ChertDatabase> database_,
			       const ChertValueManager *value_manager_,
			       const ChertRecordTable *record_table_,
			       Xapian::docid did_, bool lazy)
	: Xapian::Document::Internal(database_.get(), did_),
	  database(database_),
	  value_manager(value_manager_),
	  record_table(record_table_)
{
    DEBUGCALL(DB, void, "ChertDocument", "[database_], " << value_manager_ << ", " << record_table_ << ", " << did_ << ", " << lazy);
    if (!lazy) (void)database->get_doclength(did);
}

/** Retrieve a value from the database
 *
 *  @param valueid	The value number to retrieve.
 */
string
ChertDocument::do_get_value(Xapian::valueno valueid) const
{
    DEBUGCALL(DB, string, "ChertDocument::do_get_value", valueid);
    RETURN(value_manager->get_value(did, valueid));
}

/** Retrieve all value values from the database
 */
void
ChertDocument::do_get_all_values(map<Xapian::valueno, string> & values_) const
{
    DEBUGCALL(DB, void, "ChertDocument::do_get_all_values", "[values_]");
    value_manager->get_all_values(values_, did);
}

/** Retrieve the document data from the database
 */
string
ChertDocument::do_get_data() const
{
    DEBUGCALL(DB, string, "ChertDocument::do_get_data", "");
    RETURN(record_table->get_record(did));
}
