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
#include "quartz_table_manager.h"

/** Create a QuartzDocument: this is only called by
 *  QuartzDatabase::open_document().
 */
QuartzDocument::QuartzDocument(RefCntPtr<const Database> database_,
			       QuartzTableManager *tables_,
			       om_docid did_)
	: Document(database_.get(), did_),
	  database(database_),
	  tables(tables_)
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
    int tries_left = 5;
    while (1) {
	try {
	    QuartzAttributesManager::get_attribute(
			*tables->get_attribute_table(),
			retval,
			did,
			keyid);

	    return retval;
	} catch (OmDatabaseModifiedError &e) {
	    if (--tries_left == 0) {
		throw;
	    } else {
		tables->reopen_tables_because_overwritten();
	    }
	}
    }
}

/** Retrieve all key values from the database
 */
std::map<om_keyno, OmKey>
QuartzDocument::do_get_all_keys() const
{
    std::map<om_keyno, OmKey> keys;
    int tries_left = 5;
    while (tries_left > 0) {
	try {
	    QuartzAttributesManager::get_all_attributes(
			*tables->get_attribute_table(),
			keys,
			did);
	} catch (OmDatabaseModifiedError &e) {
	    if (--tries_left == 0) {
		throw;
	    } else {
		tables->reopen_tables_because_overwritten();
	    }
	}
    }

    return keys;
}

/** Retrieve the document data from the database
 */
OmData
QuartzDocument::do_get_data() const
{
    int tries_left = 5;
    while (1) {
	try {
	    OmData result = QuartzRecordManager::get_record(*tables->get_record_table(), did);
	    return result;
	} catch (OmDatabaseModifiedError &e) {
	    if (--tries_left == 0) {
		throw;
	    } else {
		tables->reopen_tables_because_overwritten();
	    }
	}
    }
}
