/* omenquireinternal.h: Internals
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

#ifndef OM_HGUARD_OMENQUIREINTERNAL_H
#define OM_HGUARD_OMENQUIREINTERNAL_H

#include "config.h"

#include <om/omenquire.h>
#include "database.h"
#include "multi_database.h"
#include "omlocks.h"
#include "omrefcnt.h"
#include <algorithm>
#include <math.h>
#include <map>

/** Internals of enquire system.
 *  This allows the implementation of OmEnquire to be hidden, allows
 *  cleaner pthread locking by separating the API calls from the internals,
 *  and allows the implementation to be shared with
 *  OmBatchEnquire::Internal.
 */
class OmEnquireInternal {
    private:
	/** The multidatabase which this enquire object uses.
	 *  This is obtained from the OmDatabase set in the constructor.
	 */
	OmRefCntPtr<MultiDatabase> database;

	/** The user's query.
	 *  This may need to be mutable in future so that it can be
	 *  replaced by an optimised version.
	 */
	OmQuery * query;

	/// pthread mutex, used if available.
	OmLock mutex;

	/** Read a document from the database.
	 *  This method does the work for get_doc().
	 */
	const OmDocument read_doc(om_docid did) const;

	/** Calculate the matching terms.
	 *  This method does the work for get_matching_terms().
	 */
	om_termname_list calc_matching_terms(om_docid did) const;
    public:
	OmEnquireInternal(const OmDatabase &databases);
	~OmEnquireInternal();

	void set_query(const OmQuery & query_);
	const OmQuery & get_query();
	OmMSet get_mset(om_doccount first,
			om_doccount maxitems,
			const OmRSet *omrset,
			const OmSettings *moptions,
			const OmMatchDecider *mdecider) const;
	OmESet get_eset(om_termcount maxitems,
			const OmRSet & omrset,
			const OmSettings *eoptions,
			const OmExpandDecider *edecider) const;
	const OmDocument get_doc(om_docid did) const;
	const OmDocument get_doc(const OmMSetItem &mitem) const;
	om_termname_list get_matching_terms(om_docid did) const;
	om_termname_list get_matching_terms(const OmMSetItem &mitem) const;
	std::string get_description() const;
};


class OmMSet::InternalInterface {
    public:
	/** Get a reference to the termfreqandwts member of an OmMSet.
	 */
	static std::map<om_termname, TermFreqAndWeight> &
		get_termfreqandwts(OmMSet & mset) {
		    return mset.termfreqandwts;
		}
};

#endif // OM_HGUARD_OMENQUIREINTERNAL_H
