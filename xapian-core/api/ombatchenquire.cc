/* ombatchenquire.cc: External interface for running query batches
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
#include "omenquireinternal.h"
#include <vector>
#include <memory>
#include <algorithm>
#include <math.h>

/////////////////////////////////
// Internals of OmBatchEnquire //
/////////////////////////////////

class OmBatchEnquire::Internal {
    private:
        mutable OmEnquireInternal enquire;

	query_batch queries;
    public:
	// pthread mutexes, if available.
	OmLock &mutex;

	Internal(const OmDatabase &db)
		: enquire(db), mutex(enquire.mutex) {};
	~Internal() {};

	void set_queries(const query_batch &queries_);
        mset_batch get_msets() const;

	const OmDocument *get_doc(const OmMSetItem &mitem) const;
	const OmDocument *get_doc(om_docid did) const;
	om_termname_list get_matching_terms(const OmMSetItem &mitem) const;
	om_termname_list get_matching_terms(om_docid did) const;
};

////////////////////////////
// OmBatchEnquire methods //
////////////////////////////

OmBatchEnquire::OmBatchEnquire(const OmDatabase &db)
	: internal(0)
{
    internal = new Internal(db);
}

OmBatchEnquire::~OmBatchEnquire()
{
    delete internal;
}

void
OmBatchEnquire::set_queries(const query_batch &queries_)
{
    OmLockSentry locksentry(internal->mutex);
    internal->set_queries(queries_);
}

OmBatchEnquire::mset_batch
OmBatchEnquire::get_msets() const
{
    OmLockSentry locksentry(internal->mutex);
    return internal->get_msets();
}

const OmDocument *
OmBatchEnquire::get_doc(om_docid did) const
{
    OmLockSentry locksentry(internal->mutex);
    return internal->get_doc(did);
}

const OmDocument *
OmBatchEnquire::get_doc(const OmMSetItem &mitem) const
{
    OmLockSentry locksentry(internal->mutex);
    return internal->get_doc(mitem);
}

om_termname_list
OmBatchEnquire::get_matching_terms(om_docid did) const
{
    OmLockSentry locksentry(internal->mutex);
    return internal->get_matching_terms(did);
}

om_termname_list
OmBatchEnquire::get_matching_terms(const OmMSetItem &mitem) const
{
    OmLockSentry locksentry(internal->mutex);
    return internal->get_matching_terms(mitem);
}

/////////////////////////////////////////
// Methods of OmBatchEnquire::Internal //
/////////////////////////////////////////

void
OmBatchEnquire::Internal::set_queries(const query_batch &queries_)
{
    queries = queries_;
}

OmBatchEnquire::mset_batch
OmBatchEnquire::Internal::get_msets() const
{
    mset_batch result;

    query_batch::const_iterator q = queries.begin();
    while (q != queries.end()) {
	try {
	    enquire.set_query(q->query);
	    OmMSet mset = enquire.get_mset(q->first,
					   q->maxitems,
					   q->omrset,
					   q->moptions,
					   q->mdecider);
	    result.push_back(batch_result(mset, true));
	} catch (OmInvalidArgumentError &err) {
	    /* if it's a query-specific error (ie bad arguments),
	     * then put an invalid placeholder into the result
	     * and keep processing.
	     */
            result.push_back(batch_result(OmMSet(), false));
	}
    }

    Assert(result.size() == queries.size());

    return result;
}

const OmDocument *
OmBatchEnquire::Internal::get_doc(om_docid did) const
{
    return enquire.get_doc(did);
}

const OmDocument *
OmBatchEnquire::Internal::get_doc(const OmMSetItem &mitem) const
{
    return enquire.get_doc(mitem);
}

om_termname_list
OmBatchEnquire::Internal::get_matching_terms(om_docid did) const
{
    return enquire.get_matching_terms(did);
}

om_termname_list
OmBatchEnquire::Internal::get_matching_terms(const OmMSetItem &mitem) const
{
    return enquire.get_matching_terms(mitem);
}

////////////////////////////////////////
// OmBatchQuery::batch_result methods //
////////////////////////////////////////

OmBatchEnquire::batch_result::batch_result(const OmMSet &mset,
					   bool isvalid_)
	: isvalid(isvalid_), result(mset)
{}

OmMSet
OmBatchEnquire::batch_result::value() const
{
    if (isvalid) {
	return result;
    } else {
	throw OmInvalidResultError("Query was not successfully run.");
    }
}
