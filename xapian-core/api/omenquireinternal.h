/* omenquireinternal.cc: Internals of OmEnquire
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
#include "config.h"

#include <om/omenquire.h>
#include "database.h"
#include "omlocks.h"
#include <vector>
#include <memory>
#include <algorithm>
#include <math.h>

/////////////////////////////////
// Internals of enquire system //
/////////////////////////////////

class OmEnquireInternal {
	mutable IRDatabase * database;
	OmDatabase dbdesc;
	
	/* This may need to be mutable in future so that it can be
	 * replaced by an optimised version.
	 */
	OmQuery * query;

    public:
	// pthread mutexes, if available.
	OmLock mutex;

	OmEnquireInternal(const OmDatabase &db);
	~OmEnquireInternal();

	void open_database() const;
	void set_query(const OmQuery & query_);
	OmMSet get_mset(om_doccount first,
			om_doccount maxitems,
			const OmRSet *omrset,
			const OmMatchOptions *moptions,
			const OmMatchDecider *mdecider) const;
	OmESet get_eset(om_termcount maxitems,
			const OmRSet & omrset,
			const OmExpandOptions * eoptions,
			const OmExpandDecider * edecider) const;
	const OmDocument *get_doc(const OmMSetItem &mitem) const;
	const OmDocument *get_doc(om_docid did) const;
	om_termname_list get_matching_terms(const OmMSetItem &mitem) const;
	om_termname_list get_matching_terms(om_docid did) const;
};
