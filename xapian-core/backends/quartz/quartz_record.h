/* quartz_record.h: Records in quartz databases
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2002,2003 Olly Betts
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

#ifndef OM_HGUARD_QUARTZ_RECORD_H
#define OM_HGUARD_QUARTZ_RECORD_H

#include <string>

#include <xapian/types.h>
#include "quartz_types.h"

class QuartzBufferedTable;
class QuartzTable;

using namespace std;

/** A record in a quartz database.
 */
class QuartzRecordManager {
    private:
	QuartzRecordManager();
	~QuartzRecordManager();
    public:
	/** Retrieve a document from the table.
	 */
	static string get_record(QuartzTable & table,
				 Xapian::docid did);

	/** Get the number of records in the table.
	 */
	static Xapian::doccount get_doccount(QuartzTable & table);

	/** Return the average length of records in the table.
	 */
	static Xapian::doclength get_avlength(QuartzTable & table);

	/** Get the next document ID to use.
	 */
	static Xapian::docid get_newdocid(QuartzBufferedTable & table);

	/** Add a new record to the table.
	 *
	 */
	static Xapian::docid add_record(QuartzBufferedTable & table,
				   const string & data);
	/* Replace an existing record in the table
	 *
	 * @param did	The document ID to use.  If not specified, then
	 * 		a new docid is generated.  Otherwise, this record
	 * 		will be created (or replace) document did.
	 */
	static void replace_record(QuartzBufferedTable &table,
				   const string & data,
				   Xapian::docid did);

	/** Delete a record from the table.
	 */
	static void delete_record(QuartzBufferedTable & table,
				  Xapian::docid did);

	/** Modify the stored total length of the records, by supplying an
	 *  old length for a document, and the new length of the document
	 *  replacing it.
	 *
	 *  @param old_doclen  The old length of the document.
	 *  @param new_doclen  The new length of the document.
	 */
	static void modify_total_length(QuartzBufferedTable & table,
					quartz_doclen_t old_doclen,
					quartz_doclen_t new_doclen);
};

#endif /* OM_HGUARD_QUARTZ_RECORD_H */
