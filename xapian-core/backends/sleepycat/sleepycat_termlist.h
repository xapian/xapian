/* sleepy_termlist.h: C++ class definition for sleepycat access routines
 *
 * ----START-LICENCE----
 * Copyright 1999,2000 Dialog Corporation
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

#ifndef OM_HGUARD_SLEEPY_TERMLIST_H
#define OM_HGUARD_SLEEPY_TERMLIST_H

#include "utils.h"
#include "leafpostlist.h"
#include "termlist.h"
#include "database.h"
#include <stdlib.h>

#include "omassert.h"
#include "om/omtypes.h"
#include "sleepy_termcache.h"

/** A termlist in a sleepycat database.
 */
class SleepyTermList : public LeafTermList {
    friend class SleepyDatabase;
    private:
	om_termcount pos;
	om_termid *data;
	om_termcount terms;
	om_doccount dbsize;

	const SleepyDatabaseTermCache *termcache;

	SleepyTermList(const SleepyDatabaseTermCache *tc_new,
		       om_termid *data_new,
		       om_termcount terms_new,
		       om_doccount dbsize_new);
    public:
	~SleepyTermList();
	om_termcount get_approx_size() const;

	OmExpandBits get_weighting() const;  // Gets weight of current term
	const om_termname get_termname() const;  // Current term
	om_termcount get_wdf() const;  // Occurences of current term in doc
	om_doccount get_termfreq() const;  // Docs indexed by current term
	TermList * next();
	bool   at_end() const;
};

#endif /* OM_HGUARD_SLEEPY_TERMLIST_H */
