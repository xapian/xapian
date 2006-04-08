/* da_alltermslist.h
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2003 Olly Betts
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

#ifndef OM_HGUARD_DA_ALLTERMSLIST_H
#define OM_HGUARD_DA_ALLTERMSLIST_H

#include "alltermslist.h"
#include "da_database.h"
#include "daread.h"

/** class for alltermslists over several databases */
class DAAllTermsList : public AllTermsList
{
    private:
	/// Copying is not allowed.
	DAAllTermsList(const DAAllTermsList &);

	/// Assignment is not allowed.
	void operator=(const DAAllTermsList &);

	/// Keep our database around
	Xapian::Internal::RefCntPtr<const Xapian::Database::Internal> database;

	DA_term_info term;
	DA_file *DA_t;

	std::string current_term;
	Xapian::termcount termfreq;

	void update_cache();

	bool started;
	bool is_at_end;
    public:
	/// Standard constructor for base class.
	DAAllTermsList(Xapian::Internal::RefCntPtr<const Xapian::Database::Internal> database_,
		       const DA_term_info &term_,
		       DA_file *DA_t);

	/// Standard destructor for base class.
	~DAAllTermsList();

        // Gets size of termlist
	Xapian::termcount get_approx_size() const;

	// Gets current termname
	string get_termname() const;

	// Get num of docs indexed by term
	Xapian::doccount get_termfreq() const;

	// Get num of docs indexed by term
	Xapian::termcount get_collection_freq() const;

	TermList * skip_to(const string &tname);

	/** next() causes the AllTermsList to move to the next term in the list.
	 */
	TermList * next();

	// True if we're off the end of the list
	bool at_end() const;
};

#endif /* OM_HGUARD_DA_ALLTERMSLIST_H */
