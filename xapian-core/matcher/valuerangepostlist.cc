/** @file valuerangepostlist.cc
 * @brief Return document ids matching a range test on a specified doc value.
 */
/* Copyright 2007,2008,2009,2010,2011,2013,2016 Olly Betts
 * Copyright 2009 Lemur Consulting Ltd
 * Copyright 2010 Richard Boulton
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#include <config.h>

#include "valuerangepostlist.h"

#include "debuglog.h"
#include "omassert.h"
#include "str.h"
#include "unicode/description_append.h"

using namespace std;

ValueRangePostList::~ValueRangePostList()
{
    delete valuelist;
}

Xapian::doccount
ValueRangePostList::get_termfreq_min() const
{
    return 0;
}

static double
string_frac(const string &s, size_t prefix)
{
    double r = 0;
    double f = 1.0;
    for (size_t i = prefix; i != s.size(); ++i) {
	f /= 256.0;
	r += static_cast<unsigned char>(s[i]) * f;
    }

    return r;
}

Xapian::doccount
ValueRangePostList::get_termfreq_est() const
{
    // Assume the values are evenly spread out between the min and max.
    // FIXME: Perhaps we should store some sort of binned distribution?
    const string& lo = db->get_value_lower_bound(slot);
    const string& hi = db->get_value_upper_bound(slot);

    size_t common_prefix_len = 0;
    while (common_prefix_len != lo.size() &&
	   common_prefix_len != hi.size() &&
	   lo[common_prefix_len] == hi[common_prefix_len]) {
	++common_prefix_len;
    }

    double l = string_frac(lo, common_prefix_len);
    double h = string_frac(hi, common_prefix_len);
    double b = l;
    if (begin > lo) {
	b = string_frac(begin, common_prefix_len);
    }
    double e = h;
    if (!end.empty() && end < hi) {
	// end is empty for a ValueGePostList
	e = string_frac(end, common_prefix_len);
    }

    double est = (e - b) / (h - l) * db->get_value_freq(slot);
    return Xapian::doccount(est + 0.5);
}

TermFreqs
ValueRangePostList::get_termfreq_est_using_stats(
	const Xapian::Weight::Internal & stats) const
{
    LOGCALL(MATCH, TermFreqs, "ValueRangePostList::get_termfreq_est_using_stats", stats);
    // FIXME: It's hard to estimate well - perhaps consider the values of
    // begin and end like above?
    RETURN(TermFreqs(stats.collection_size / 2,
		     stats.rset_size / 2,
		     stats.total_term_count / 2));
}

Xapian::doccount
ValueRangePostList::get_termfreq_max() const
{
    return db->get_value_freq(slot);
}

double
ValueRangePostList::get_maxweight() const
{
    return 0;
}

Xapian::docid
ValueRangePostList::get_docid() const
{
    Assert(valuelist);
    Assert(db);
    return valuelist->get_docid();
}

double
ValueRangePostList::get_weight() const
{
    Assert(db);
    return 0;
}

Xapian::termcount
ValueRangePostList::get_doclength() const
{
    Assert(db);
    return 0;
}

Xapian::termcount
ValueRangePostList::get_unique_terms() const
{
    Assert(db);
    return 0;
}

double
ValueRangePostList::recalc_maxweight()
{
    Assert(db);
    return 0;
}

PositionList *
ValueRangePostList::read_position_list()
{
    Assert(db);
    return NULL;
}

PostList *
ValueRangePostList::next(double)
{
    Assert(db);
    if (!valuelist) valuelist = db->open_value_list(slot);
    valuelist->next();
    while (!valuelist->at_end()) {
	const string & v = valuelist->get_value();
	if (v >= begin && v <= end) {
	    return NULL;
	}
	valuelist->next();
    }
    db = NULL;
    return NULL;
}

PostList *
ValueRangePostList::skip_to(Xapian::docid did, double)
{
    Assert(db);
    if (!valuelist) valuelist = db->open_value_list(slot);
    valuelist->skip_to(did);
    while (!valuelist->at_end()) {
	const string & v = valuelist->get_value();
	if (v >= begin && v <= end) {
	    return NULL;
	}
	valuelist->next();
    }
    db = NULL;
    return NULL;
}

PostList *
ValueRangePostList::check(Xapian::docid did, double, bool &valid)
{
    Assert(db);
    AssertRelParanoid(did, <=, db->get_lastdocid());
    if (!valuelist) valuelist = db->open_value_list(slot);
    valid = valuelist->check(did);
    if (!valid) {
	return NULL;
    }
    const string & v = valuelist->get_value();
    valid = (v >= begin && v <= end);
    return NULL;
}

bool
ValueRangePostList::at_end() const
{
    return (db == NULL);
}

Xapian::termcount
ValueRangePostList::count_matching_subqs() const
{
    return 1;
}

string
ValueRangePostList::get_description() const
{
    string desc = "ValueRangePostList(";
    desc += str(slot);
    desc += ", ";
    description_append(desc, begin);
    desc += ", ";
    description_append(desc, end);
    desc += ")";
    return desc;
}
