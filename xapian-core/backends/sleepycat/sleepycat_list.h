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

#include <db_cxx.h>

#include <om/omtypes.h>

typedef unsigned int entry_type;   // Able to represent any entry

/** A list of items which might comprise a termlist or a postlist,
 *  which are stored in a sleepycat database.
 */
class SleepyList {
    private:
	Db *db;
	Dbt key;

	bool opened;
	bool modified;

	entry_type length;  // How many ids
	entry_type *ids;    // List of ids
	entry_type *wdfs;   // List of wdfs, 1 for each id
	entry_type *freqs;  // How many things are indexed by this item
	om_termpos *positions;

	entry_type readentry(char ** pos, char * end); // Read entry
    public:
	SleepyList(Db * db_new);
	~SleepyList();

	void open(void * keydata_new, size_t keylen_new);
	void add(entry_type id, entry_type wdf);
	void close();
};

#endif /* OM_HGUARD_SLEEPY_LIST_H */
