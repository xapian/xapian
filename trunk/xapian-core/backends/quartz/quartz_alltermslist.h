/* quartz_alltermslist.h
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002,2003,2004 Olly Betts
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

#ifndef OM_HGUARD_QUARTZ_ALLTERMSLIST_H
#define OM_HGUARD_QUARTZ_ALLTERMSLIST_H

#include "alltermslist.h"
#include "quartz_database.h"

class Bcursor;

/** class for alltermslists over several databases */
class QuartzAllTermsList : public AllTermsList
{
    private:
	/// Copying is not allowed.
	QuartzAllTermsList(const QuartzAllTermsList &);

	/// Assignment is not allowed.
	void operator=(const QuartzAllTermsList &);

	/// Keep our database around
	Xapian::Internal::RefCntPtr<const Xapian::Database::Internal> database;

	/// A cursor pointing at the current term's postlist entry
	AutoPtr<Bcursor> pl_cursor;

	/// Cached "at-end" value
	bool is_at_end;

	quartz_tablesize_t size;

	bool started;

	/// Cached termname
	string current_term;

	/// Cached statistics
	mutable bool have_stats;
	mutable Xapian::termcount termfreq;
	mutable Xapian::termcount collection_freq;

	void get_stats() const;
    public:
	/// Standard constructor for base class.
	QuartzAllTermsList(Xapian::Internal::RefCntPtr<const Xapian::Database::Internal> database_,
			   AutoPtr<Bcursor> pl_cursor_,
		       	   quartz_tablesize_t size_);

	/// Standard destructor for base class.
	~QuartzAllTermsList();

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

#endif /* OM_HGUARD_QUARTZ_ALLTERMSLIST_H */
