/** @file
 * @brief Xapian::Weight base class
 */
/* Copyright (C) 2007,2008,2009,2014,2017,2019,2024 Olly Betts
 * Copyright (C) 2009 Lemur Consulting Ltd
 * Copyright (C) 2017 Vivek Pal
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

#include "backends/leafpostlist.h"
#include "weightinternal.h"

#include "omassert.h"
#include "debuglog.h"

#include "xapian/error.h"

using namespace std;

namespace Xapian {

void
Weight::init_(const Internal & stats, Xapian::termcount query_length,
	      const Xapian::Database::Internal* shard)
{
    LOGCALL_VOID(MATCH, "Weight::init_", stats | query_length | shard);
    collection_size_ = stats.collection_size;
    rset_size_ = stats.rset_size;
    if (stats_needed & AVERAGE_LENGTH)
	average_length_ = stats.get_average_length();
    if (stats_needed & DOC_LENGTH_MAX)
	doclength_upper_bound_ = shard->get_doclength_upper_bound();
    if (stats_needed & DOC_LENGTH_MIN)
	doclength_lower_bound_ = shard->get_doclength_lower_bound();
    if (stats_needed & UNIQUE_TERMS_MAX)
	unique_terms_upper_bound_ = shard->get_unique_terms_upper_bound();
    if (stats_needed & UNIQUE_TERMS_MIN)
	unique_terms_lower_bound_ = shard->get_unique_terms_lower_bound();
    if (stats_needed & TOTAL_LENGTH)
	total_length_ = stats.total_length;
    if (stats_needed & DB_DOC_LENGTH_MAX)
	db_doclength_upper_bound_ = stats.db_doclength_upper_bound;
    if (stats_needed & DB_DOC_LENGTH_MIN)
	db_doclength_lower_bound_ = stats.db_doclength_lower_bound;
    if (stats_needed & DB_UNIQUE_TERMS_MAX)
	db_unique_terms_upper_bound_ = stats.db_unique_terms_upper_bound;
    if (stats_needed & DB_UNIQUE_TERMS_MIN)
	db_unique_terms_lower_bound_ = stats.db_unique_terms_lower_bound;
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
	      const Xapian::Database::Internal* shard,
	      void* postlist_void)
{
    LOGCALL_VOID(MATCH, "Weight::init_", stats | query_length | term | wqf | factor | shard | postlist_void);
    collection_size_ = stats.collection_size;
    rset_size_ = stats.rset_size;
    if (stats_needed & AVERAGE_LENGTH)
	average_length_ = stats.get_average_length();
    if (stats_needed & DOC_LENGTH_MAX)
	doclength_upper_bound_ = shard->get_doclength_upper_bound();
    if (stats_needed & DOC_LENGTH_MIN)
	doclength_lower_bound_ = shard->get_doclength_lower_bound();
    if (stats_needed & UNIQUE_TERMS_MAX)
	unique_terms_upper_bound_ = shard->get_unique_terms_upper_bound();
    if (stats_needed & UNIQUE_TERMS_MIN)
	unique_terms_lower_bound_ = shard->get_unique_terms_lower_bound();
    if (stats_needed & TOTAL_LENGTH)
	total_length_ = stats.total_length;
    if (stats_needed & WDF_MAX) {
	auto postlist = static_cast<LeafPostList*>(postlist_void);
	wdf_upper_bound_ = postlist->get_wdf_upper_bound();
    }
    if (stats_needed & DB_DOC_LENGTH_MAX)
	db_doclength_upper_bound_ = stats.db_doclength_upper_bound;
    if (stats_needed & DB_DOC_LENGTH_MIN)
	db_doclength_lower_bound_ = stats.db_doclength_lower_bound;
    if (stats_needed & DB_UNIQUE_TERMS_MAX)
	db_unique_terms_upper_bound_ = stats.db_unique_terms_upper_bound;
    if (stats_needed & DB_UNIQUE_TERMS_MIN)
	db_unique_terms_lower_bound_ = stats.db_unique_terms_lower_bound;
    if (stats_needed & DB_WDF_MAX) {
	// FIXME: Nothing uses this stat, so for now return a correct but
	// likely fairly loose upper bound.  Once we have something that
	// wants to use this we can implement tracking a per-term wdf_max
	// across the whole database.
	db_wdf_upper_bound_ = stats.db_doclength_upper_bound;
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
	      double factor, Xapian::doccount termfreq,
	      Xapian::doccount reltermfreq, Xapian::termcount collection_freq,
	      const Xapian::Database::Internal* shard)
{
    LOGCALL_VOID(MATCH, "Weight::init_", stats | query_length | factor | termfreq | reltermfreq | collection_freq | shard);
    // Synonym case.
    collection_size_ = stats.collection_size;
    rset_size_ = stats.rset_size;
    if (stats_needed & AVERAGE_LENGTH)
	average_length_ = stats.get_average_length();
    if (stats_needed & (DOC_LENGTH_MAX | WDF_MAX)) {
	doclength_upper_bound_ = shard->get_doclength_upper_bound();
	// The doclength is an upper bound on the wdf.  This is obviously true
	// for normal terms, but SynonymPostList ensures that it is also true
	// for synonym terms by clamping the wdf values returned to the
	// doclength.
	//
	// (This clamping is only actually necessary in cases where a
	// constituent term of the synonym is repeated.)
	wdf_upper_bound_ = doclength_upper_bound_;
    }
    if (stats_needed & DOC_LENGTH_MIN)
	doclength_lower_bound_ = shard->get_doclength_lower_bound();
    if (stats_needed & UNIQUE_TERMS_MAX)
	unique_terms_upper_bound_ = shard->get_unique_terms_upper_bound();
    if (stats_needed & UNIQUE_TERMS_MIN)
	unique_terms_lower_bound_ = shard->get_unique_terms_lower_bound();
    if (stats_needed & TOTAL_LENGTH)
	total_length_ = stats.total_length;
    if (stats_needed & (DB_DOC_LENGTH_MAX | DB_WDF_MAX)) {
	db_doclength_upper_bound_ = stats.db_doclength_upper_bound;
	// The doclength is an upper bound on the wdf.  This is obviously true
	// for normal terms, but SynonymPostList ensures that it is also true
	// for synonym terms by clamping the wdf values returned to the
	// doclength.
	//
	// (This clamping is only actually necessary in cases where a
	// constituent term of the synonym is repeated.)
	db_wdf_upper_bound_ = db_doclength_upper_bound_;
    }
    if (stats_needed & DB_DOC_LENGTH_MIN)
	db_doclength_lower_bound_ = stats.db_doclength_lower_bound;
    if (stats_needed & DB_UNIQUE_TERMS_MAX)
	db_unique_terms_upper_bound_ = stats.db_unique_terms_upper_bound;
    if (stats_needed & DB_UNIQUE_TERMS_MIN)
	db_unique_terms_lower_bound_ = stats.db_unique_terms_lower_bound;

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

double
Weight::get_sumextra(Xapian::termcount,
		     Xapian::termcount,
		     Xapian::termcount) const
{
    return 0.0;
}

double
Weight::get_maxextra() const
{
    return 0.0;
}

[[noreturn]]
static inline void
parameter_error(const char* message, const string& scheme, const char* params)
{
    Xapian::Weight::Internal::parameter_error(message, scheme, params);
}

const Weight *
Weight::create(const string & s, const Registry & reg)
{
    const char *p = s.c_str();
    std::string scheme;

    while (*p != ' ') {
	if (*p == '\0') break;
	scheme += *p;
	p++;
    }

    if (*p == ' ') p++;
    auto weight = reg.get_weighting_scheme(scheme);
    if (!weight) {
	// Allow "trad" and "trad <k>" to work despite TradWeight now just
	// being a thin subclass of BM25Weight.
	if (scheme == "trad") {
	    const char* params = p;
	    double k = 1.0;
	    if (*p != '\0') {
		if (!Xapian::Weight::Internal::double_param(&p, &k))
		    parameter_error("Parameter is invalid", scheme, params);
		if (*p)
		    parameter_error("Extra data after parameter",
				    scheme, params);
	    }
	    return new BM25Weight(k, 0.0, 0.0, 1.0, 0.0);
	}
	throw InvalidArgumentError("Unknown weighting scheme: " + scheme);
    }
    return weight->create_from_parameters(p);
}

Weight *
Weight::create_from_parameters(const char *) const
{
    throw Xapian::UnimplementedError("create_from_parameters() not supported for this Xapian::Weight subclass");
}

}
