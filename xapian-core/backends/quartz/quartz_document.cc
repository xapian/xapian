/* quartz_document.cc: Implementation of document for Quartz database
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
#include "omdebug.h"
#include "quartz_database.h"
#include "quartz_document.h"
#include "quartz_attributes.h"
#include "quartz_record.h"

/** Create a QuartzDocument: this is only called by
 *  QuartzDatabase::open_document().
 */
QuartzDocument::QuartzDocument(RefCntPtr<const Database> database_,
			       QuartzTable *attribute_table_,
			       QuartzTable *record_table_,
			       om_docid did_)
	: Document(database_.get(), did_),
	  database(database_),
	  attribute_table(attribute_table_),
	  record_table(record_table_)
{
}

/** Destroy a QuartzDocument.
 */
QuartzDocument::~QuartzDocument()
{
}

/** Retrieve a key value from the database
 *
 *  @param keyid	The key number for this document.
 */
OmKey
QuartzDocument::do_get_key(om_keyno keyid) const
{
    OmKey retval;
    QuartzAttributesManager::get_attribute(
		*attribute_table,
		retval,
		did,
		keyid);

    return retval;
}

/** Retrieve all key values from the database
 */
std::map<om_keyno, OmKey>
QuartzDocument::do_get_all_keys() const
{
    std::map<om_keyno, OmKey> keys;
    QuartzAttributesManager::get_all_attributes(
		*attribute_table,
		keys,
		did);

    return keys;
}

/** Retrieve the document data from the database
 */
OmData
QuartzDocument::do_get_data() const
{
    return QuartzRecordManager::get_record(*record_table, did);
}
