/* alltermslist.h
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2003,2005 Olly Betts
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

#ifndef OM_HGUARD_ALLTERMSLIST_H
#define OM_HGUARD_ALLTERMSLIST_H

#include <string>

#include <xapian/types.h>
#include <xapian/error.h>
#include "termlist.h"

using namespace std;

// Abstract base class for alltermslists.
class AllTermsList : public TermList
{
    private:
	/// Copying is not allowed.
	AllTermsList(const AllTermsList &);

	/// Assignment is not allowed.
	void operator=(const AllTermsList &);
    public:
	/// Standard constructor for base class.
	AllTermsList() { }

	/// Standard destructor for base class.
	virtual ~AllTermsList() { }

        // Gets size of termlist
	virtual Xapian::termcount get_approx_size() const = 0;
	
        // Gets weighting info for current term
	virtual OmExpandBits get_weighting() const {
	    Assert(false); // should never get called
	    abort();
#if defined __SUNPRO_CC || defined __sgi
	    // For compilers which worry abort() might return...
	    return OmExpandBits(0, 0, 0);
#endif
	}

	// Gets current termname
	virtual string get_termname() const = 0;

	// Get wdf of current term
	virtual Xapian::termcount get_wdf() const {
	    Assert(false);
	    return 0;
	}

	// Get num of docs indexed by term
	virtual Xapian::doccount get_termfreq() const = 0;

	// Get num of docs indexed by term
	virtual Xapian::termcount get_collection_freq() const = 0;

	/** next() causes the AllTermsList to move to the next term in the
	 *  list.
	 */
	virtual TermList *next() = 0;

	/** Skip to the given term.  If the term wasn't
	 *  found it will be positioned on the term just
	 *  after tname in the database.  This could be after the end!
	 */
	virtual TermList *skip_to(const string &tname) = 0;

	// True if we're off the end of the list
	virtual bool at_end() const = 0;
};

#endif /* OM_HGUARD_ALLTERMSLIST_H */
