/* omalltermsiteratorinternal.h
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

#ifndef OM_HGUARD_OMALLTERMSITERATORINTERNAL_H
#define OM_HGUARD_OMALLTERMSITERATORINTERNAL_H

#include "om/omalltermsiterator.h"
#include "alltermslist.h"

class OmAllTermsIterator::Internal {
    private:
	friend class OmAllTermsIterator; // allow access to termlist
        friend bool operator==(const OmAllTermsIterator &a, const OmAllTermsIterator &b);

	RefCntPtr<AllTermsList> alltermslist;

	/// The database to read position lists from.
	OmDatabase database;

	std::string current;

    public:
        Internal(RefCntPtr<AllTermsList> alltermslist_,
		 const OmDatabase &database_)
		: alltermslist(alltermslist_),
		  database(database_),
		  current()
	{
	}

        Internal(const Internal &other)
		: alltermslist(alltermslist),
		  database(other.database),
		  current(other.current)
	{ }

	void next()
	{
	    alltermslist->next();
	}

	void skip_to(const om_termname & tname)
	{
	    alltermslist->skip_to(tname);

	    Assert(alltermslist->at_end() == false);
	    Assert(alltermslist->get_termname() == tname);
	}

	bool at_end()
	{
	    return alltermslist->at_end();
	}

	bool operator== (const OmAllTermsIterator::Internal &other)
	{
	    return (alltermslist.get() == other.alltermslist.get())
		    && (current == other.current);
	}
};

#endif /* OM_HGUARD_OMALLTERMSITERATORINTERNAL_H */
