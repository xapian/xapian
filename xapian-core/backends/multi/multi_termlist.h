/* multi_termlist.h: C++ class definition for multiple database access
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

#ifndef OM_HGUARD_MULTI_TERMLIST_H
#define OM_HGUARD_MULTI_TERMLIST_H

#include "omdebug.h"
#include "termlist.h"
#include "database.h"
#include <stdlib.h>
#include <set>
#include <vector>
#include <list>

class MultiTermList : public LeafTermList {
    friend class Xapian::Database;
    private:
	LeafTermList *tl;
	Xapian::Internal::RefCntPtr<const Xapian::Database::Internal> termdb;
	const Xapian::Database &rootdb;
	double termfreq_factor;

	MultiTermList(LeafTermList * tl_,
		      const Xapian::Internal::RefCntPtr<const Xapian::Database::Internal> & termdb_,
		      const Xapian::Database &rootdb_);
    public:
	void set_weighting(const OmExpandWeight *wt_new);

	Xapian::termcount get_approx_size() const;

	OmExpandBits get_weighting() const; // Gets weight info of current term
	string get_termname() const;
	Xapian::termcount get_wdf() const; // Number of occurrences of term in current doc
	Xapian::doccount get_termfreq() const;  // Number of docs indexed by term
	TermList * next();
	bool   at_end() const;

	~MultiTermList();
};

inline MultiTermList::MultiTermList(LeafTermList * tl_,
				    const Xapian::Internal::RefCntPtr<const Xapian::Database::Internal> & termdb_,
				    const Xapian::Database &rootdb_)
	: tl(tl_), termdb(termdb_), rootdb(rootdb_)
{
    termfreq_factor = double(rootdb.get_doccount()) / termdb->get_doccount();
    DEBUGLINE(DB, "Approximation factor for termfreq: " << termfreq_factor);
}

inline MultiTermList::~MultiTermList()
{
    delete tl;
}

inline Xapian::termcount
MultiTermList::get_approx_size() const
{
    return tl->get_approx_size();
}

inline void
MultiTermList::set_weighting(const OmExpandWeight * wt_new)
{
    // Note: wt in the MultiTermList base class isn't ever set or used
    tl->set_weighting(wt_new);
}

inline OmExpandBits
MultiTermList::get_weighting() const {
    return tl->get_weighting();
}

inline string
MultiTermList::get_termname() const
{
    return tl->get_termname();
}

inline Xapian::termcount MultiTermList::get_wdf() const
{
    return tl->get_wdf();
}

inline Xapian::doccount MultiTermList::get_termfreq() const
{
    // Approximate term frequency
    return Xapian::doccount(tl->get_termfreq() * termfreq_factor);
}

inline TermList * MultiTermList::next()
{
    return tl->next();
}

inline bool MultiTermList::at_end() const
{
    return tl->at_end();
}

#endif /* OM_HGUARD_MULTI_TERMLIST_H */
