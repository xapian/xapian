/* alltermslist.h
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

#ifndef OM_HGUARD_EMTPYALLTERMSLIST_H
#define OM_HGUARD_EMTPYALLTERMSLIST_H

#include "alltermslist.h"

/** Abstract base class for alltermslists. */
class EmptyAllTermsList : public AllTermsList
{
    private:
	/// Copying is not allowed.
	EmptyAllTermsList(const EmptyAllTermsList &);

	/// Assignment is not allowed.
	void operator=(const EmptyAllTermsList &);
    public:
	/// Standard constructor for base class.
	EmptyAllTermsList() {}

	// Gets current termname
	const om_termname get_termname() const
	{
	    Assert(false);
	    return "";
	}

	// Get num of docs indexed by term
	om_doccount get_termfreq() const
	{
	    Assert(false);
	    return 0;
	}

	// Get num of docs indexed by term
	om_termcount get_collection_freq() const
	{
	    Assert(false);
	    return 0;
	}

	bool skip_to(const om_termname &tname)
	{
	    return false;
	}

	/** next() causes the AllTermsList to move to the next term in the list.
	 */
	bool next()
	{
	    Assert(false);
	    return false;
	}

	// True if we're off the end of the list
	bool at_end() const
	{
	    return true;
	}
};

#endif /* OM_HGUARD_EMTPYALLTERMSLIST_H */
