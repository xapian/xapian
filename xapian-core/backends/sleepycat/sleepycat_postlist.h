/* sleepy_database.h: C++ class definition for sleepycat access routines
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

#ifndef OM_HGUARD_SLEEPY_POSTLIST_H
#define OM_HGUARD_SLEEPY_POSTLIST_H

#include "utils.h"
#include "omassert.h"
#include "leafpostlist.h"
#include "termlist.h"
#include "database.h"
#include <stdlib.h>

// Postlist - a list of documents indexed by a given term
class SleepyPostList : public LeafPostList {
    friend class SleepyDatabase;
    private:
	om_doccount pos;
	om_docid *data;

	om_termname tname;
	om_doccount termfreq;

	SleepyPostList(const om_termname &tn, om_docid *data_new, om_doccount tf);
    public:
	~SleepyPostList();

	om_doccount   get_termfreq() const;// Number of docs indexed by this term

	om_docid      get_docid() const;   // Current docid
	om_weight     get_weight() const;  // Current weight
        PostList * next(om_weight w_min);  // Move to next docid
        PostList * skip_to(om_docid did, om_weight w_min);  // Skip to next docid >= docid
	bool       at_end() const;      // True if we're off the end of the list

	string intro_term_description() const;
};

#endif /* OM_HGUARD_SLEEPY_POSTLIST_H */
