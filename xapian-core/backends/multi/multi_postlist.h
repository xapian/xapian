/** @file
 * @brief Class for merging PostList objects from subdatabases.
 */
/* Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2003,2005,2007,2009,2011 Olly Betts
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

#include "api/leafpostlist.h"
#include <vector>

class MultiPostList : public PostList {
    friend class Xapian::Database;
    private:
	std::vector<LeafPostList *> postlists;

	Xapian::Database this_db;

	bool finished;
	Xapian::docid currdoc;

	Xapian::doccount multiplier;

	MultiPostList(std::vector<LeafPostList *> & pls,
		      const Xapian::Database &this_db_);
    public:
	~MultiPostList();

	Xapian::doccount get_termfreq_min() const;
	Xapian::doccount get_termfreq_max() const;
	Xapian::doccount get_termfreq_est() const;

	double get_maxweight() const;
	double get_weight() const;
	double recalc_maxweight();

	Xapian::docid get_docid() const;     // Gets current docid
	Xapian::termcount get_doclength() const; // Get length of current document
	Xapian::termcount get_unique_terms() const; // Get number of unique term in current document
	Xapian::termcount get_wdf() const;	    // Within Document Frequency
	PositionList * open_position_list() const;
	PostList *next(double w_min);          // Moves to next docid
	PostList *skip_to(Xapian::docid did, double w_min);// Moves to next docid >= specified docid
	bool at_end() const;        // True if we're off the end of the list

	std::string get_description() const;
};

#endif /* OM_HGUARD_MULTI_POSTLIST_H */
