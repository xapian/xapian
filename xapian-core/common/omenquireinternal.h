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
#include "omlocks.h"
#include "refcnt.h"
#include <algorithm>
#include <math.h>
#include <map>

class OmErrorHandler;

/** An item in the ESet.
 *  This item contains the termname, and the weight calculated for
 *  the document.
 */
class OmESetItem {
    public:
	OmESetItem(om_weight wt_, om_termname tname_)
		: wt(wt_), tname(tname_) {}
	/// Weight calculated.
	om_weight wt;
	/// Term suggested.
	om_termname tname;
	
	/** Returns a string representing the eset item.
	 *  Introspection method.
	 */
	std::string get_description() const;
};

class OmESetIterator::Internal {
    private:
	friend class OmESetIterator; // allow access to it
        friend bool operator==(const OmESetIterator &a, const OmESetIterator &b);

	std::vector<OmESetItem>::const_iterator it;
	std::vector<OmESetItem>::const_iterator end;
    
    public:
        Internal(std::vector<OmESetItem>::const_iterator it_,
		 std::vector<OmESetItem>::const_iterator end_)
	    : it(it_), end(end_)
	{ }

        Internal(const Internal &other) : it(other.it), end(other.end)
	{ }
};

/** Internals of enquire system.
 *  This allows the implementation of OmEnquire to be hidden, allows
 *  cleaner pthread locking by separating the API calls from the internals,
 *  and allows the implementation to be shared with
 *  OmBatchEnquire::Internal.
 */
class OmEnquire::Internal {
    private:
	/// The database which this enquire object uses.
	const OmDatabase db;

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
	/** The error handler, if set.  (0 if not set).
	 */
	OmErrorHandler * errorhandler;

	Internal(const OmDatabase &databases, OmErrorHandler * errorhandler_);
	~Internal();

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

	const std::vector<OmDocument> get_docs(
		std::vector<OmMSetItem>::const_iterator begin,
		std::vector<OmMSetItem>::const_iterator end) const;

	om_termname_list get_matching_terms(om_docid did) const;
	om_termname_list get_matching_terms(const OmMSetItem &mitem) const;
	std::string get_description() const;
};

class OmExpand;

class OmESet::Internal {
    friend class OmESet;
    friend class OmExpand;
    private:
	/// A list of items comprising the (selected part of the) eset.
	std::vector<OmESetItem> items;

	/** A lower bound on the number of terms which are in the full
	 *  set of results of the expand.  This will be greater than or
	 *  equal to items.size()
	 */
	om_termcount ebound;

    public:
	Internal() : ebound(0) {}

	/** Returns a string representing the eset.
	 *  Introspection method.
	 */
	std::string get_description() const;
};

#endif // OM_HGUARD_OMENQUIREINTERNAL_H
