/* sleepy_list.h: class definition for sleepycat list access routines
 *
 * ----START-LICENCE----
 * Copyright 1999,2000 Dialog Corporation
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

#ifndef OM_HGUARD_SLEEPY_LIST_H
#define OM_HGUARD_SLEEPY_LIST_H

#include <om/omtypes.h>
#include <vector>
#include <db_cxx.h>

/** An item in a SleepyList.  This might represent either a term in a
 *  termlist or a document in a postlist.  In either situation, it
 *  represents a term / document combination.
 */
class SleepyListItem {
    public:
	/** Able to represent either a document or a term ID.
	 */
	typedef unsigned int id_type;

	/** The ID of this item.
	 *  For postlists, the id is a document ID; for termlists, the id
	 *  is a term ID.
	 */
	id_type id;

	/** The WDF of this item.
	 *
	 *  The WDF of a term in a given document is the number of
	 *  occurrences of that term in the document.
	 *
	 *  For postlists, the WDF is the number of occurences of the
	 *  postlist term in this document.
	 *
	 *  For termlists, the WDF is the number of occurences of this term
	 *  in the termlist document.
	 *
	 *  If WDF information is not present, this field will have a value
	 *  of zero.
	 */
	om_termcount wdf;

	/** Positional information.
	 *
	 *  This is a list of positions at which the term occurs in the
	 *  document.  The list is in strictly increasing order of term
	 *  position.
	 *
	 *  The positions start at 1.
	 *
	 *  Note that, even if positional information is present, the WDF
	 *  might not be equal to the length of the position list, since a
	 *  term might occur multiple times at a single position, but will
	 *  only have one entry in the position list for each position.
	 */
	vector<om_termpos> positions;

	/** Term frequency of this item.
	 *
	 *  The term frequency is the number of documents which contain the
	 *  term.  If this information is not present, the value is 0.
	 */
	om_doccount termfreq;

	/** The length of this document.
	 *
	 *  This is the sum of the wdfs of the terms in the document.  If
	 *  this information is not present, the value is 0.
	 */
	om_doclength doclength;

	/** Create a new SleepyListItem, based on given values.
	 *
	 *  @param id_        The ID for the entry.
	 *  @param wdf_       The WDF for the entry (number of occurrences
	 *                    in the given document).
	 *  @param positions_ A list of positions at which the term
	 *                    occurs.  This list must be strictly
	 *                    increasing (ie, no duplicates).
	 *  @param termfreq_  The term frequency (number of documents
	 *                    containing the term).
	 *  @param doclength_ The length of this document (which is the
	 *                    sum of the wdfs of the terms in the document).
	 */
	SleepyListItem(id_type id_,
		       om_termcount wdf_,
		       const vector<om_termpos> & positions_,
		       om_doccount termfreq_,
		       om_doclength doclength_);

	/** Create a new SleepyListItem, based on a packed version.
	 *
	 *  @param packed     The packed representation of the item.
	 *  @param store_termfreq  If true, term frequencies are assumed to
	 *                         be stored in the packed list.
	 */
	SleepyListItem(string packed,
		       bool store_termfreq = true);

	/** Return a packed representation of the item, for storing in
	 *  the file.
	 *
	 *  @param store_termfreq  If true, term frequencies will be stored
	 *                         in the packed list.
	 */
	string pack(bool store_termfreq) const;
};

/** A list of items which might comprise a termlist or a postlist,
 *  which are stored in a sleepycat database.
 */
class SleepyList {
    private:
	/** The database which the list is held in.
	 */
	Db *db;

	/** The key used to look for the data.
	 */
	Dbt key;

	/** Flag to say whether the list has been modified from that held
	 *  on disk.  Before the list is modified, the list must be locked,
	 *  and the list must be unlocked when the modifications are flushed.
	 */
	bool modified_and_locked;

	/** Flag to say whether an iteration through the list has begun.
	 */
	bool iteration_in_progress;

	/** Flag to say that iteration is at the initial (non-existent) item.
	 */
	bool iteration_at_start;

	/** Current position of iteration through the list.
	 */
	vector<SleepyListItem>::const_iterator iteration_position;

	/** The items stored in the list.
	 */
	vector<SleepyListItem> items;

	/** The sum of the wdfs of the items in the list.
	 *
	 *  This corresponds to the document length for termlists, and the
	 *  collection frequency for postlists.  If this information is not
	 *  present, the value is 0.
	 */
	om_termcount wdfsum;

	/** Whether to store term frequency information.
	 */
	bool store_termfreq;

	/** Whether to store wdfsum information.
	 */
	bool store_wdfsum;

	/** Perform a flush operation.
	 *
	 *  This is a private method, containing the implementation for the
	 *  public method flush(), so that other methods (such as the
	 *  destructor) don't have to call a public method.
	 *
	 *  @exception OmDatabaseError is thrown if an error occurs when
	 *  writing to the database.
	 */
	void do_flush();

	/** Insert an entry into the list of items.
	 *
	 *  This searches the list for an item with the same ID as the new
	 *  item.  If such an item is found, it is replaced with the new
	 *  item.  If none is found, a new entry is added into the list of
	 *  items in the correct place (to keep the list in sorted order of
	 *  ID).
	 *
	 *  @param newitem The item to be added.
	 */
	void make_entry(const SleepyListItem & newitem);

	/** Unpack a string representation of this item into the item.
	 */
	void unpack(string packed);

	/** Pack the contents of this item into a string.
	 */
	string pack() const;

	/// Copying is not allowed.
	SleepyList(const SleepyList &);

	/// Assignment is not allowed.
	void operator=(const SleepyList &);
	
    public:
	/** A type big enough to represent the number of items which a
	 *  list could hold.
	 */
	typedef unsigned int itemcount_type;

	/** Make the list referring to that stored in the specified database
	 *  referenced by the given key.
	 *
	 *  If the key is not currently in the database, an empty list will
	 *  be returned; if entries are then be added to this list, it will
	 *  be added to the database.
	 *
	 *  @exception OmDatabaseError thrown if the list can't be read.
	 *
	 *  @param db_      The database to find the list in.
	 *  @param keydata_ The key to use for looking up list.
	 *  @param keylen_  The size of keydata_.
	 *  @param store_termfreq_   If true, term frequencies will be stored
	 *                           in the list.
	 *  @param store_wdf_        If true, wdf information will be stored
	 *                           in the list.
	 *  @param store_positional_ If true, positional information will be
	 *                           stored in the list.
	 *  @param store_wdfsum_     If true, the sum of the wdfs will be
	 *                           stored in the list.  This corresponds to
	 *                           the document length for termlists, and
	 *                           the collection frequency for postlists.
	 */
	SleepyList(Db * db_, void * keydata_, size_t keylen_,
		   bool store_termfreq_ = true,
		   bool store_wdf_ = true,
		   bool store_positional_ = true,
		   bool store_wdfsum_ = true);

	/** Close the list.
	 *
	 *  If the list has been modified, this will flush the list first.
	 *
	 *  @exception OmDatabaseError may be thrown if flush is called.
	 */
	~SleepyList();

	/** Get the number of items in the list.
	 */
	itemcount_type get_item_count() const;

	/** Get the wdfsum.  If this information is not present, the value
	 *  returned is 0.
	 */
	om_termcount get_wdfsum() const;

	/** Move to the start of the list.
	 *
	 *  Note that there is not an item at the start of the list: after
	 *  this method move_to_next_item() must be called before an item
	 *  may be accessed (eg, by get_current_item())
	 *
	 *  This convention makes it easier to have a setup step before
	 *  accessing the list, makes it easier to deal with empty lists,
	 *  and matches that used by PostList and TermList.
	 */
	void move_to_start();

	/** Move to the next item in the list.
	 *
	 *  This must not be called if there are no more items in the list:
	 *  you should always check this condition, by calling
	 *  are_more_items(), before calling this method.
	 */
	void move_to_next_item();

	/** Skip forward through the list until an item with id greater
	 *  than or equal to that specified is found.
	 *
	 *  This method may not move forward in the list, if the current
	 *  item satisfies the condition.  However, if the iteration
	 *  through the list has not yet started, this method guarantees
	 *  to move forward.
	 *
	 *  Like move_to_next_item(), this method must not be called if
	 *  there are no more items in the list.
	 */
	void skip_to_item(SleepyListItem::id_type id);

	/** Determine whether the iteration has reached the end of the list.
	 *
	 *  There is no item at the end of the list; it is not valid to
	 *  call get_current_item() if this method returns true.
	 *
	 *  An iteration must be in progress when this method is called:
	 *  ie. move_to_start() must have been called, and add_item() must
	 *  not have been called since that.
	 *  
	 *  @return true  if we are at the end of the list,
	 *          false if there is an item at the current position, or
	 *                we havn't yet moved from the start of the list.
	 */
	bool at_end() const;

	/** Get the current item pointed to in the list.
	 *
	 *  There is initially no current item: this method is not valid
	 *  until move_to_start() has been called to start an iteration
	 *  through the list, and move_to_next_item() has then been called
	 *  to move to the first item in the list.
	 */
	const SleepyListItem & get_current_item() const;

	/** Add an entry to the list.
	 *
	 *  If no changes have yet been made to the list, this method will
	 *  obtain a lock on the list before proceeding.
	 *
	 *  This method also causes there to be no "current" item in the
	 *  list - if an iteration through the list is in progress, it must
	 *  be restarted with move_to_start(), before get_current_item()
	 *  may be called.
	 *
	 *  @param item  The item to add to the list.
	 */
	void add_item(const SleepyListItem & newitem);

	/** Flush the list.  This writes any changes to the list to disk,
	 *  and unlocks the list.
	 *
	 *  @exception OmDatabaseError is thrown if an error occurs when
	 *  writing to the database.
	 */
	void flush();
};

#endif /* OM_HGUARD_SLEEPY_LIST_H */
