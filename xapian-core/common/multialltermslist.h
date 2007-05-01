/* multialltermslist.h
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2003,2007 Olly Betts
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

#ifndef OM_HGUARD_MULTIALLTERMSLIST_H
#define OM_HGUARD_MULTIALLTERMSLIST_H

#include "alltermslist.h"

#include <vector>

/** class for alltermslists over several databases */
class MultiAllTermsList : public AllTermsList
{
    private:
	/// Copying is not allowed.
	MultiAllTermsList(const MultiAllTermsList &);

	/// Assignment is not allowed.
	void operator=(const MultiAllTermsList &);

	std::vector<TermList *> lists;

	/// The current term being pointed at.
	string current;

	/// Flag indicating the end of the lists
	bool is_at_end;

	/// Flag saying whether we've started iterating yet
	bool started;

	void update_current();
    public:
	/// Standard constructor for base class.
	MultiAllTermsList(const std::vector<TermList *> &lists_);

	/// Standard destructor for base class.
	~MultiAllTermsList();

        // Gets size of termlist
	Xapian::termcount get_approx_size() const;

	// Gets current termname
	string get_termname() const;

	// Get num of docs indexed by term
	Xapian::doccount get_termfreq() const;

	// Get num of docs indexed by term
	Xapian::termcount get_collection_freq() const;

	TermList *skip_to(const string &tname);

	/** next() causes the AllTermsList to move to the next term in the list.
	 */
	TermList *next();

	// True if we're off the end of the list
	bool at_end() const;
};

#endif /* OM_HGUARD_MULTIALLTERMSLIST_H */
