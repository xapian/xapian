/* inmemory_alltermslist.h
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

#ifndef OM_HGUARD_INMEMORY_ALLTERMSLIST_H
#define OM_HGUARD_INMEMORY_ALLTERMSLIST_H

#include "alltermslist.h"
#include "inmemory_database.h"

/** class for alltermslists over several databases */
class InMemoryAllTermsList : public AllTermsList
{
    private:
	/// Copying is not allowed.
	InMemoryAllTermsList(const InMemoryAllTermsList &);

	/// Assignment is not allowed.
	void operator=(const InMemoryAllTermsList &);

	std::map<om_termname, InMemoryTerm>::const_iterator it;
	std::map<om_termname, InMemoryTerm>::const_iterator end;

	const std::map<om_termname, InMemoryTerm> *tmap;
	RefCntPtr<const InMemoryDatabase> database;
    public:
	/// Standard constructor for base class.
	InMemoryAllTermsList(std::map<om_termname, InMemoryTerm>::const_iterator begin,
			     std::map<om_termname, InMemoryTerm>::const_iterator end_,
			     const std::map<om_termname, InMemoryTerm> *tmap_,
			     RefCntPtr<const InMemoryDatabase> database_);

	/// Standard destructor for base class.
	~InMemoryAllTermsList();

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

#endif /* OM_HGUARD_INMEMORY_ALLTERMSLIST_H */
