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

	/** Term frequency of this item.
	 *
	 *  The term frequency is the number of documents which contain the
	 *  term.
	 */
	om_doccount termfreq;

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
	 *  If WDF information is not empty, this field will have a value of
	 *  zero.
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

	/** Create a new SleepyListItem, based on given values.
	 *
	 *  @param id_        The ID for the entry.
	 *  @param termfreq_  The term frequency (number of documents
	 *                    containing the term).
	 *  @param wdf_       The WDF for the entry (number of occurrences
	 *                    in the given document).
	 *  @param positions_ A list of positions at which the term
	 *                    occurs.  This list must be strictly
	 *                    increasing (ie, no duplicates).
	 */
	SleepyListItem(id_type id_,
		       om_doccount termfreq_,
		       om_termcount wdf_,
		       const vector<om_termpos> & positions_);

	/** Create a new SleepyListItem, based on a packed version.
	 *
	 *  @param packed     The packed representation of the item.
	 */
	SleepyListItem(string packed);

	/** Return a packed representation of the item, for storing in
	 *  the file.
	 */
	string pack() const;
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

	/** The items stored in the list.
	 */
	vector<SleepyListItem> items;

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

    public:
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
	 *  @param store_termfreq    If true, term frequencies will be stored
	 *                           in the list.
	 *  @param store_wdf         If true, wdf information will be stored
	 *                           in the list.
	 *  @param store_positional  If true, positional information will be
	 *                           stored in the list.
	 */
	SleepyList(Db * db_, void * keydata_, size_t keylen_,
		   bool store_termfreq = true,
		   bool store_wdf = true,
		   bool store_positional = true);

	/** Close the list.
	 *
	 *  If the list has been modified, this will flush the list first.
	 *
	 *  @exception OmDatabaseError may be thrown if flush is called.
	 */
	~SleepyList();

	/** Add an entry to the list.
	 *
	 *  If no changes have yet been made to the list, this method will
	 *  obtain a lock on the list before proceeding.
	 *
	 *  @param item  The item to add to the list.
	 */
	void add(const SleepyListItem & newitem);

	/** Flush the list.  This writes any changes to the list to disk,
	 *  and unlocks the list.
	 *
	 *  @exception OmDatabaseError is thrown if an error occurs when
	 *  writing to the database.
	 */
	void flush();
};

#endif /* OM_HGUARD_SLEEPY_LIST_H */
