/* multi_postlist.h: C++ class definition for multiple database access
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2003,2005,2007 Olly Betts
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

#ifndef OM_HGUARD_MULTI_POSTLIST_H
#define OM_HGUARD_MULTI_POSTLIST_H

#include "leafpostlist.h"
#include <vector>

class MultiPostList : public LeafPostList {
    friend class Xapian::Database;
    private:
	std::vector<LeafPostList *> postlists;

	const Xapian::Database &this_db;

	bool   finished;
	Xapian::docid  currdoc;

	string tname;
	mutable bool freq_initialised;
	mutable Xapian::doccount termfreq;

	mutable bool collfreq_initialised;
	mutable Xapian::termcount collfreq;

	Xapian::weight termweight;

	Xapian::doccount multiplier;

	MultiPostList(std::vector<LeafPostList *> & pls,
		      const Xapian::Database &this_db_);
    public:
	~MultiPostList();

	Xapian::doccount get_termfreq() const;

	Xapian::docid  get_docid() const;     // Gets current docid
	Xapian::doclength get_doclength() const; // Get length of current document
        Xapian::termcount get_wdf() const;	    // Within Document Frequency
	PositionList *read_position_list();
	PositionList * open_position_list() const;
	PostList *next(Xapian::weight w_min);          // Moves to next docid
	PostList *skip_to(Xapian::docid did, Xapian::weight w_min);// Moves to next docid >= specified docid
	bool   at_end() const;        // True if we're off the end of the list

	std::string get_description() const;
};

#endif /* OM_HGUARD_MULTI_POSTLIST_H */
