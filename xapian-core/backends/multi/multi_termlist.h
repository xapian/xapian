/* multi_termlist.h: C++ class declaration for multiple database access
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002,2003,2004,2005 Olly Betts
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

#include "termlist.h"
#include "database.h"

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

#endif /* OM_HGUARD_MULTI_TERMLIST_H */
