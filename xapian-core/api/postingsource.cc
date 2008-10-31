/** @file postingsource.cc
 * @brief External sources of posting information
 */
/* Copyright (C) 2008 Olly Betts
 * Copyright (C) 2008 Lemur Consulting Ltd
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#include <config.h>

#include "xapian/postingsource.h"

#include "autoptr.h"

#include "database.h"
#include "document.h"
#include "xapian/document.h"
#include "xapian/error.h"
#include "xapian/queryparser.h" // For sortable_unserialise().

#include "omassert.h"
#include "utils.h"

#include <cfloat>

namespace Xapian {

PostingSource::~PostingSource() { }

Xapian::weight
PostingSource::get_maxweight() const
{
    return 0;
}

Xapian::weight
PostingSource::get_weight() const
{
    return 0;
}

void
PostingSource::skip_to(Xapian::docid did, Xapian::weight min_wt)
{
    while (!at_end() && get_docid() < did) {
	next(min_wt);
    }
}

bool
PostingSource::check(Xapian::docid did, Xapian::weight min_wt)
{
    skip_to(did, min_wt);
    return true;
}

std::string
PostingSource::get_description() const
{
    return "Xapian::PostingSource subclass";
}


ValueWeightPostingSource::ValueWeightPostingSource(Xapian::Database db_,
						   Xapian::valueno slot_)
	: db(db_), slot(slot_),
	  it(db.valuestream_begin(slot)), end(db.valuestream_end(slot)),
	  started(false)
{
    try {
	termfreq_max = db.get_value_freq(slot);
	termfreq_est = termfreq_max;
	termfreq_min = termfreq_max;
	max_weight = sortable_unserialise(db.get_value_upper_bound(slot));
    } catch (const Xapian::UnimplementedError &) {
	termfreq_max = db.get_doccount();
	termfreq_est = termfreq_max / 2;
	termfreq_min = 0;
	max_weight = DBL_MAX;
    }
}

ValueWeightPostingSource::ValueWeightPostingSource(Xapian::Database db_,
						   Xapian::valueno slot_,
						   double max_weight_)
	: db(db_), slot(slot_),
	  it(db.valuestream_begin(slot)), end(db.valuestream_end(slot)),
	  started(false),
	  max_weight(max_weight_)
{
    try {
	termfreq_max = db.get_value_freq(slot);
	termfreq_est = termfreq_max;
	termfreq_min = termfreq_max;
	double ubound = sortable_unserialise(db.get_value_upper_bound(slot));
	max_weight = std::min(max_weight, ubound);
    } catch (const Xapian::UnimplementedError &) {
	termfreq_max = db.get_doccount();
	termfreq_est = termfreq_max / 2;
	termfreq_min = 0;
    }
}

Xapian::doccount
ValueWeightPostingSource::get_termfreq_min() const
{
    return termfreq_min;
}

Xapian::doccount
ValueWeightPostingSource::get_termfreq_est() const
{
    return termfreq_est;
}

Xapian::doccount
ValueWeightPostingSource::get_termfreq_max() const
{
    return termfreq_max;
}

Xapian::weight
ValueWeightPostingSource::get_maxweight() const
{
    return max_weight;
}

Xapian::weight
ValueWeightPostingSource::get_weight() const
{
    Assert(!at_end());
    Assert(started);
    return sortable_unserialise(*it);
}

void
ValueWeightPostingSource::next(Xapian::weight min_wt)
{
    if (!started) {
	started = true;
	it = db.valuestream_begin(slot);
	end = db.valuestream_end(slot);
    } else {
	++it;
    }

    if (it == end) return;

    if (min_wt > max_weight) {
	it = end;
	return;
    }
}

void
ValueWeightPostingSource::skip_to(Xapian::docid min_docid,
				  Xapian::weight min_wt)
{
    if (!started) {
	started = true;
	it = db.valuestream_begin(slot);
	end = db.valuestream_end(slot);
    }

    if (min_wt > max_weight) {
	it = end;
	return;
    }
    it.skip_to(min_docid);
}

bool
ValueWeightPostingSource::check(Xapian::docid min_docid,
				Xapian::weight min_wt)
{
    if (!started) {
	started = true;
	it = db.valuestream_begin(slot);
	end = db.valuestream_end(slot);
    }

    if (min_wt > max_weight) {
	it = end;
	return true;
    }
    return it.check(min_docid);
}

bool
ValueWeightPostingSource::at_end() const
{
    return it == end;
}

Xapian::docid
ValueWeightPostingSource::get_docid() const
{
    return it.get_docid();
}

void
ValueWeightPostingSource::reset()
{
    started = false;
}

std::string
ValueWeightPostingSource::get_description() const
{
    return "Xapian::ValueWeightPostingSource(slot=" + om_tostring(slot) + ")";
}


FixedWeightPostingSource::FixedWeightPostingSource(Xapian::Database db_,
						   Xapian::weight wt_)
	: db(db_), termfreq(db_.get_doccount()),
	  it(db.postlist_begin(std::string())),
	  end(db.postlist_end(std::string())),
	  wt(wt_),
	  started(false)
{
}

Xapian::doccount
FixedWeightPostingSource::get_termfreq_min() const
{
    return termfreq;
}

Xapian::doccount
FixedWeightPostingSource::get_termfreq_est() const
{
    return termfreq;
}

Xapian::doccount
FixedWeightPostingSource::get_termfreq_max() const
{
    return termfreq;
}

Xapian::weight
FixedWeightPostingSource::get_maxweight() const
{
    return wt;
}

Xapian::weight
FixedWeightPostingSource::get_weight() const
{
    return wt;
}

void
FixedWeightPostingSource::next(Xapian::weight min_wt)
{
    if (!started) {
	started = true;
	it = db.postlist_begin(std::string());
	end = db.postlist_end(std::string());
    } else {
	++it;
    }

    if (it == end) return;

    if (check_docid) {
	it.skip_to(check_docid + 1);
	check_docid = 0;
    }

    if (min_wt > wt) {
	it = end;
	return;
    }
}

void
FixedWeightPostingSource::skip_to(Xapian::docid min_docid,
				  Xapian::weight min_wt)
{
    if (!started) {
	started = true;
	it = db.postlist_begin(std::string());
	end = db.postlist_end(std::string());
    }

    if (check_docid) {
	if (min_docid < check_docid)
	    min_docid = check_docid + 1;
	check_docid = 0;
    }

    if (min_wt > wt) {
	it = end;
	return;
    }
    it.skip_to(min_docid);
}

bool
FixedWeightPostingSource::check(Xapian::docid min_docid,
				Xapian::weight)
{
    // We're guaranteed not to be called if the document doesn't
    // exist, so just remember the docid passed, and return true.
    check_docid = min_docid;
    return true;
}

bool
FixedWeightPostingSource::at_end() const
{
    return it == end;
}

Xapian::docid
FixedWeightPostingSource::get_docid() const
{
    if (check_docid != 0) return check_docid;
    return *it;
}

void
FixedWeightPostingSource::reset()
{
    started = false;
    check_docid = 0;
}

std::string
FixedWeightPostingSource::get_description() const
{
    return "Xapian::FixedWeightPostingSource(wt=" + om_tostring(wt) + ")";
}


}
