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
#include "multi_database.h"
#include "omlocks.h"
#include <vector>
#include <memory>
#include <algorithm>
#include <math.h>

/////////////////////////////////
// Internals of enquire system //
/////////////////////////////////

/** Internals of enquire system.
 *  This allows the implementation of OmEnquire to be hidden, allows
 *  cleaner pthread locking by separating the API calls from the internals,
 *  and allows the implementation to be shared with
 *  OmBatchEnquire::Internal.
 */
class OmEnquireInternal {
    private:
	/** The databases which this enquire object uses.
	 *  These are specified in the constructor, and are opened lazily,
	 *  by calling OmEnquireInternal::open_database();
	 */
	mutable MultiDatabase * database;

	/** A copy of the OmDatabaseGroup object used to specify the database
	 *  or databases to be used.
	 *
	 *  This is set by the constructor.
	 */
	OmDatabaseGroup dbdesc;
	
	/** The user's query.
	 *  This may need to be mutable in future so that it can be
	 *  replaced by an optimised version.
	 */
	OmQuery * query;

	/// pthread mutexes, used if available.
	OmLock mutex;

	/** Read a document from the database.
	 *  This method does the work for get_doc().
	 */
	const OmDocument read_doc(om_docid did) const;

	/** Calculate the matching terms.
	 *  This method does the work for get_matching_terms().
	 */
	om_termname_list calc_matching_terms(om_docid did) const;

	/** Open the database, if it is not already open.
	 *
	 */
	void open_database() const;
    public:
	OmEnquireInternal(const OmDatabaseGroup &databases);
	~OmEnquireInternal();

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
	const OmDocument get_doc(om_docid did) const;
	const OmDocument get_doc(const OmMSetItem &mitem) const;
	om_termname_list get_matching_terms(om_docid did) const;
	om_termname_list get_matching_terms(const OmMSetItem &mitem) const;
};
