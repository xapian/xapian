/* quartz_termlist.h: Termlists in quartz databases
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

#ifndef OM_HGUARD_QUARTZ_TERMLIST_H
#define OM_HGUARD_QUARTZ_TERMLIST_H

#include "config.h"
#include "quartz_table.h"
#include "om/omtypes.h"
#include "termlist.h"
#include "quartz_database.h"
#include "om/autoptr.h"

class QuartzTermListIterator;

/** A termlist in a quartz database.
 */
class QuartzTermList : public LeafTermList {
    private:
	/** The database we are searching.  This pointer is held so that the
	 *  database doesn't get deleted before us, and also so that we can
	 *  look up term frequencies in the lexicon if they are not stored
	 *  in termlists.
	 */
	RefCntPtr<const QuartzDatabase> this_db;

	/** The table holding the termlist.
	 */
	const QuartzTable * table;


	/** The data for the (part of the) termlist currently being read.
	 *
	 *  FIXME: currently, we read the whole termlist as one chunk.
	 */
	QuartzDbTag termlist_part;
	
	/** Position within tag that we're reading from.
	 */
	std::string::const_iterator pos;

	/** End of data within tag;
	 */
	std::string::const_iterator end;

	/** Whether we have moved past the final item yet.
	 */
	bool have_finished;


	/** The size of the termlist.
	 */
	om_termcount termlist_size;
	
	/** Current termname.
	 */
	om_termname current_tname;

	/** Current wdf.
	 */
	om_termcount current_wdf;

	/** Whether the termlist stores term frequencies (this cannot be
	 *  done in an updatable database, but improves expand efficiency
	 *  considerably).
	 */
	bool has_termfreqs;

	/** Current term frequency.
	 *
	 *  This will have the value 0 if the term frequency is not
	 *  available (in which case it will be looked up in the database
	 *  if requested).
	 */
	mutable om_doccount current_termfreq;


	/** Read the size from the termlist.
	 *
	 *  The value read is stored in the termlist_size member variable.
	 *
	 *  @exception OmDatabaseCorruptError is thrown if there is an error
	 *  (eg, data runs out).
	 */
	void read_size();

	/** Write the size into the string supplied.
	 *
	 *  @param data A string to append the representation of the size to.
	 *  @param size The size (number of entries) of the termlist.
	 */
	static void write_size(std::string & data, 
			       om_termcount size);

	/** Read whether the termlist stores term frequencies.
	 *
	 *  @exception OmDatabaseCorruptError is thrown if there is an error
	 *  (eg, data runs out).
	 */
	void read_has_termfreqs();

	/** Write whether termfreqs are being stored into the string supplied.
	 *
	 *  @param data A string to append the data to.
	 *  @param store_termfreqs Whether termfreqs are being stored.
	 */
	static void write_has_termfreqs(std::string & data, 
					bool store_termfreqs);

	/** Read an item from the termlist.
	 *
	 *  The values read are stored in the current_tname, current_wdf,
	 *  etc, member variables.
	 *
	 *  @exception OmDatabaseCorruptError is thrown if there is an error
	 *  (eg, data runs out).
	 */
	void read_item();

	/** Write an item into the string supplied.
	 *
	 *  @param data           A string to append the data to.
	 *  @param tname          The termname of the item.
	 *  @param wdf            The wdf of the item.
	 *  @param store_termfreq Whether to store the term frequency.
	 *  @param termfreq       The term frequency of the item.  This
	 *         will be ignored if this quantity is not being stored.
	 */
	static void write_item(std::string & data, 
			       om_termname tname,
			       om_termcount wdf,
			       bool store_termfreq,
			       om_doccount termfreq);

    public:
	/** Set the entries in the termlist.
	 *
	 *  If the termlist already exists, its contents are replaced.
	 *
	 *  @param store_termfreqs  If true, term frequencies are stored
	 *         in the termlist.  This should only be done with static
	 *         databases (which are probably generated from dumps of
	 *         dynamic databases) - updating cannot be done efficiently
	 *         while storing term frequencies.
	 */
	static void set_entries(QuartzBufferedTable * table,
			om_docid did,
			const OmDocumentContents::document_terms & terms,
			bool store_termfreqs);

	/** Clear the termlist.  After this call, the termlist for the
	 *  specified document ID will not exist.
	 */
	static void delete_termlist(QuartzBufferedTable * table,
				    om_docid did);


	/** Open the termlist for the specified document, for reading.
	 */
	QuartzTermList(RefCntPtr<const QuartzDatabase> this_db_,
		       const QuartzTable * table_,
		       om_docid did);

	/** Return number of items in termlist.
	 *  (This is actually exact - it may be approximate for combined
	 *  termlists.)
	 */
	om_termcount get_approx_size() const;


	/** Move to next entry.  Must be called before any of the other
	 *  methods, including at_end(), to move onto the first entry.
	 *
	 *  @return This method always returns 0.  Other values would
	 *          represent a termlist which should be used to replace
	 *          this termlist - this never happens.
	 */
	TermList * next();

	/** Check whether the termlist reader has reached the end.
	 */
	bool   at_end() const;


	/** Get the current term in the termlist.
	 */
	const om_termname get_termname() const;

	/** Get the within document frequency of the current term.
	 */
	om_termcount get_wdf() const;

	/** Get the term frequency of the current term - the number of
	 *  documents containing this term.  For an updateable database, this
	 *  cannot be stored in the termlist, so involves a relatively expensive
	 *  lookup in the lexicon.
	 */
	om_doccount get_termfreq() const;

	/** Get weighting information for the current term in termlist.
	 */
	OmExpandBits get_weighting() const;
};

#endif /* OM_HGUARD_QUARTZ_TERMLIST_H */
