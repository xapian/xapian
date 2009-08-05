/* chert_record.h: Records in chert databases
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2002,2003,2004,2005,2007,2008 Olly Betts
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

#ifndef OM_HGUARD_CHERT_RECORD_H
#define OM_HGUARD_CHERT_RECORD_H

#include <string>

#include <xapian/types.h>
#include "chert_types.h"
#include "chert_table.h"

using namespace std;

/** A record in a chert database.
 */
class ChertRecordTable : public ChertTable {
    public:
	/** Create a new table object.
	 *
	 *  This does not create the table on disk - the create() method must
	 *  be called before the table is created on disk
	 *
	 *  This also does not open the table - the open() method must be
	 *  called before use is made of the table.
	 *
	 *  @param path_          - Path at which the table is stored.
	 *  @param readonly_      - whether to open the table for read only
	 *                          access.
	 */
	ChertRecordTable(const string & path_, bool readonly_)
	    : ChertTable("record", path_ + "/record.", readonly_, Z_DEFAULT_STRATEGY) { }

	/** Retrieve a document from the table.
	 */
	string get_record(Xapian::docid did) const;

	/** Get the number of records in the table.
	 */
	Xapian::doccount get_doccount() const;

	/* Add a new record to the table, or replace an existing record.
	 *
	 * @param did	The document ID to use.
	 */
	void replace_record(const string & data, Xapian::docid did);

	/** Delete a record from the table.
	 */
	void delete_record(Xapian::docid did);
};

#endif /* OM_HGUARD_CHERT_RECORD_H */
