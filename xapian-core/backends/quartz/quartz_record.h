/* quartz_record.h: Records in quartz databases
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2002,2003,2004 Olly Betts
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
#include "quartz_table.h"

using namespace std;

/** A record in a quartz database.
 */
class QuartzRecordTable : public QuartzTable {
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
	 *  @param blocksize_     - Size of blocks to use.  This parameter is
	 *                          only used when creating the table.
	 */
	QuartzRecordTable(string path_, bool readonly_, unsigned int blocksize_)
	    : QuartzTable(path_ + "/record_", readonly_, blocksize_) { }

	/** Retrieve a document from the table.
	 */
	string get_record(Xapian::docid did) const;

	/** Get the number of records in the table.
	 */
	Xapian::doccount get_doccount() const;

	/** Return the average length of records in the table.
	 */
	Xapian::doclength get_avlength() const;

	/** Get the next document ID to use.
	 */
	Xapian::docid get_newdocid();

	/** Get the last document id used.
	 */
	Xapian::docid get_lastdocid() const;

	/** Add a new record to the table.
	 *
	 */
	Xapian::docid add_record(const string & data);

	/* Replace an existing record in the table
	 *
	 * @param did	The document ID to use.  If not specified, then
	 * 		a new docid is generated.  Otherwise, this record
	 * 		will be created (or replace) document did.
	 */
	void replace_record(const string & data, Xapian::docid did);

	/** Delete a record from the table.
	 */
	void delete_record(Xapian::docid did);

	/** Modify the stored total length of the records, by supplying an
	 *  old length for a document, and the new length of the document
	 *  replacing it.
	 *
	 *  @param old_doclen  The old length of the document.
	 *  @param new_doclen  The new length of the document.
	 */
	void modify_total_length(quartz_doclen_t old_doclen,
				 quartz_doclen_t new_doclen);
};

#endif /* OM_HGUARD_QUARTZ_RECORD_H */
