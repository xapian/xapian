/* multialltermslist.h
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

#ifndef OM_HGUARD_MULTIALLTERMSLIST_H
#define OM_HGUARD_MULTIALLTERMSLIST_H

#include "alltermslist.h"

/** class for alltermslists over several databases */
class MultiAllTermsList : public AllTermsList
{
    private:
	/// Copying is not allowed.
	MultiAllTermsList(const MultiAllTermsList &);

	/// Assignment is not allowed.
	void operator=(const MultiAllTermsList &);

	std::vector<RefCntPtr<AllTermsList> > lists;

	/// The current term being pointed at.
	om_termname current;

	/// Flag indicating the end of the lists
	bool is_at_end;

	void update_current();
    public:
	/// Standard constructor for base class.
	MultiAllTermsList(const std::vector<RefCntPtr<AllTermsList> > &lists_);

	/// Standard destructor for base class.
	~MultiAllTermsList();

	// Gets current termname
	om_termname get_termname() const;

	// Get num of docs indexed by term
	om_doccount get_termfreq() const;

	// Get num of docs indexed by term
	om_termcount get_collection_freq() const;

	bool skip_to(const om_termname &tname);

	/** next() causes the AllTermsList to move to the next term in the list.
	 */
	bool next();

	// True if we're off the end of the list
	bool at_end() const;
};

#endif /* OM_HGUARD_MULTIALLTERMSLIST_H */
