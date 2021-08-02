/** @file
 * @brief Xapian::Weight base class
 */
/* Copyright (C) 2007,2008,2009,2014,2017,2019 Olly Betts
 * Copyright (C) 2009 Lemur Consulting Ltd
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include <config.h>

#include "xapian/weight.h"

#include "api/leafpostlist.h"
#include "weightinternal.h"

#include "omassert.h"
#include "debuglog.h"

#include "xapian/error.h"

using namespace std;

namespace Xapian {

void
Weight::init_(const Internal & stats, Xapian::termcount query_length)
{
    LOGCALL_VOID(MATCH, "Weight::init_", stats | query_length);
    collection_size_ = stats.collection_size;
    rset_size_ = stats.rset_size;
    if (stats_needed & AVERAGE_LENGTH)
	average_length_ = stats.get_average_length();
    if (stats_needed & DOC_LENGTH_MAX)
	doclength_upper_bound_ = stats.db.get_doclength_upper_bound();
    if (stats_needed & DOC_LENGTH_MIN)
	doclength_lower_bound_ = stats.db.get_doclength_lower_bound();
    collectionfreq_ = 0;
    wdf_upper_bound_ = 0;
    termfreq_ = 0;
    reltermfreq_ = 0;
    query_length_ = query_length;
    wqf_ = 1;
    init(0.0);
}

void
Weight::init_(const Internal & stats, Xapian::termcount query_length,
	      const string & term, Xapian::termcount wqf, double factor,
	      void* postlist_void)
{
    LOGCALL_VOID(MATCH, "Weight::init_", stats | query_length | term | wqf | factor | postlist_void);
    collection_size_ = stats.collection_size;
    rset_size_ = stats.rset_size;
    if (stats_needed & AVERAGE_LENGTH)
	average_length_ = stats.get_average_length();
    if (stats_needed & DOC_LENGTH_MAX)
	doclength_upper_bound_ = stats.db.get_doclength_upper_bound();
    if (stats_needed & DOC_LENGTH_MIN)
	doclength_lower_bound_ = stats.db.get_doclength_lower_bound();
    if (stats_needed & WDF_MAX) {
	if (usual(postlist_void != nullptr)) {
	    auto postlist = static_cast<LeafPostList*>(postlist_void);
	    wdf_upper_bound_ = postlist->get_wdf_upper_bound();
	} else {
	    wdf_upper_bound_ = stats.db.get_wdf_upper_bound(term);
	}
    }
    if (stats_needed & (TERMFREQ | RELTERMFREQ | COLLECTION_FREQ)) {
	bool ok = stats.get_stats(term,
				  termfreq_, reltermfreq_, collectionfreq_);
	(void)ok;
	Assert(ok);
    }
    query_length_ = query_length;
    wqf_ = wqf;
    init(factor);
}

void
Weight::init_(const Internal & stats, Xapian::termcount query_length,
	      const string & term, Xapian::termcount wqf, double factor)
{
    init_(stats, query_length, term, wqf, factor, nullptr);
}

void
Weight::init_(const Internal & stats, Xapian::termcount query_length,
	      double factor, Xapian::doccount termfreq,
	      Xapian::doccount reltermfreq, Xapian::termcount collection_freq)
{
    LOGCALL_VOID(MATCH, "Weight::init_", stats | query_length | factor | termfreq | reltermfreq | collection_freq);
    // Synonym case.
    collection_size_ = stats.collection_size;
    rset_size_ = stats.rset_size;
    if (stats_needed & AVERAGE_LENGTH)
	average_length_ = stats.get_average_length();
    if (stats_needed & (DOC_LENGTH_MAX | WDF_MAX)) {
	doclength_upper_bound_ = stats.db.get_doclength_upper_bound();
	// The doclength is an upper bound on the wdf.  This is obviously true
	// for normal terms, but SynonymPostList ensures that it is also true
	// for synonym terms by clamping the wdf values returned to the
	// doclength.
	//
	// (This clamping is only actually necessary in cases where a constituent
	// term of the synonym is repeated.)
	wdf_upper_bound_ = doclength_upper_bound_;
    }
    if (stats_needed & DOC_LENGTH_MIN)
	doclength_lower_bound_ = stats.db.get_doclength_lower_bound();

    termfreq_ = termfreq;
    reltermfreq_ = reltermfreq;
    query_length_ = query_length;
    collectionfreq_ = collection_freq;
    wqf_ = 1;
    init(factor);
}

Weight::~Weight() { }

string
Weight::name() const
{
    return string();
}

string
Weight::serialise() const
{
    throw Xapian::UnimplementedError("serialise() not supported for this Xapian::Weight subclass");
}

Weight *
Weight::unserialise(const string &) const
{
    throw Xapian::UnimplementedError("unserialise() not supported for this Xapian::Weight subclass");
}

}
