/* quartz_record.h: Records in quartz databases
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

#ifndef OM_HGUARD_QUARTZ_RECORD_H
#define OM_HGUARD_QUARTZ_RECORD_H

#include "config.h"
#include "quartz_table.h"
#include "om/omtypes.h"
#include "om/omdocument.h"

/** A record in a quartz database.
 */
class QuartzRecordManager {
    private:
	QuartzRecordManager();
	~QuartzRecordManager();
    public:
	/** Initialise table for searching.
	 */
	static void initialise(QuartzDiskTable & table,                                                        QuartzRevisionNumber new_revision);

	/** Retrieve a document from the table.
	 */
	static OmData get_record(QuartzTable & table,
				 om_docid did);

	/** Get the number of records in the table.
	 */
	static om_doccount get_doccount(QuartzTable & table);

	/** Read the total length of the records in the table.
	 */
	static om_totlength get_total_length(QuartzTable & table);

	/** Get the next document ID to use.
	 */
	static om_docid get_newdocid(QuartzBufferedTable & table);

	/** Add a new record to the table.
	 */
	static om_docid add_record(QuartzBufferedTable & table,
				   const OmData & data,
				   om_doclength doclen);

	/** Delete a record from the table.
	 */
	static void delete_record(QuartzBufferedTable & table,
				  om_docid did);

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
