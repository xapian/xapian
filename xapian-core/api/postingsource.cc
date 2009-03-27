/** @file postingsource.cc
 * @brief External sources of posting information
 */
/* Copyright (C) 2008,2009 Olly Betts
 * Copyright (C) 2008,2009 Lemur Consulting Ltd
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
#include "serialise.h"
#include "serialise-double.h"
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

PostingSource *
PostingSource::clone() const
{
    return NULL;
}

std::string
PostingSource::name() const
{
    return std::string();
}

std::string
PostingSource::serialise() const
{
    throw Xapian::InvalidOperationError("serialise() not supported for this PostingSource");
}

PostingSource *
PostingSource::unserialise(const std::string &) const
{
    throw Xapian::InvalidOperationError("unserialise() not supported for this PostingSource");
}

std::string
PostingSource::get_description() const
{
    return "Xapian::PostingSource subclass";
}


ValuePostingSource::ValuePostingSource(Xapian::valueno slot_)
	: slot(slot_)
{
}

Xapian::doccount
ValuePostingSource::get_termfreq_min() const
{
    return termfreq_min;
}

Xapian::doccount
ValuePostingSource::get_termfreq_est() const
{
    return termfreq_est;
}

Xapian::doccount
ValuePostingSource::get_termfreq_max() const
{
    return termfreq_max;
}

Xapian::weight
ValuePostingSource::get_maxweight() const
{
    return max_weight;
}

void
ValuePostingSource::next(Xapian::weight min_wt)
{
    if (!started) {
	started = true;
	value_it = db.valuestream_begin(slot);
	value_end = db.valuestream_end(slot);
    } else {
	++value_it;
    }

    if (value_it == value_end) return;

    if (min_wt > max_weight) {
	value_it = value_end;
	return;
    }
}

void
ValuePostingSource::skip_to(Xapian::docid min_docid,
				  Xapian::weight min_wt)
{
    if (!started) {
	started = true;
	value_it = db.valuestream_begin(slot);
	value_end = db.valuestream_end(slot);

	if (value_it == value_end) return;
    }

    if (min_wt > max_weight) {
	value_it = value_end;
	return;
    }
    value_it.skip_to(min_docid);
}

bool
ValuePostingSource::check(Xapian::docid min_docid,
				Xapian::weight min_wt)
{
    if (!started) {
	started = true;
	value_it = db.valuestream_begin(slot);
	value_end = db.valuestream_end(slot);

	if (value_it == value_end) return true;
    }

    if (min_wt > max_weight) {
	value_it = value_end;
	return true;
    }
    return value_it.check(min_docid);
}

bool
ValuePostingSource::at_end() const
{
    return started && value_it == value_end;
}

Xapian::docid
ValuePostingSource::get_docid() const
{
    return value_it.get_docid();
}

void
ValuePostingSource::init(const Database & db_)
{
    db = db_;
    started = false;
    max_weight = DBL_MAX;
    try {
	termfreq_max = db.get_value_freq(slot);
	termfreq_est = termfreq_max;
	termfreq_min = termfreq_max;
    } catch (const Xapian::UnimplementedError &) {
	termfreq_max = db.get_doccount();
	termfreq_est = termfreq_max / 2;
	termfreq_min = 0;
    }
}


ValueWeightPostingSource::ValueWeightPostingSource(Xapian::valueno slot_)
	: ValuePostingSource(slot_)
{
}

Xapian::weight
ValueWeightPostingSource::get_weight() const
{
    Assert(!at_end());
    Assert(started);
    return sortable_unserialise(*value_it);
}

ValueWeightPostingSource *
ValueWeightPostingSource::clone() const
{
    return new ValueWeightPostingSource(slot);
}

std::string
ValueWeightPostingSource::name() const
{
    return std::string("Xapian::ValueWeightPostingSource");
}

std::string
ValueWeightPostingSource::serialise() const
{
    return encode_length(slot);
}

PostingSource *
ValueWeightPostingSource::unserialise(const std::string &s) const
{
    const char * p = s.data();
    const char * end = p + s.size();

    Xapian::valueno new_valno = decode_length(&p, end, false);
    if (p != end) {
	throw Xapian::NetworkError("Bad serialised ValueWeightPostingSource - junk at end");
    }

    return new ValueWeightPostingSource(new_valno);
}

void
ValueWeightPostingSource::init(const Database & db_)
{
    ValuePostingSource::init(db_);

    try {
    	max_weight = sortable_unserialise(db.get_value_upper_bound(slot));
    } catch (const Xapian::UnimplementedError &) {
	max_weight = DBL_MAX;
    }
}

std::string
ValueWeightPostingSource::get_description() const
{
    return "Xapian::ValueWeightPostingSource(slot=" + om_tostring(slot) + ")";
}


ValueMapPostingSource::ValueMapPostingSource(Xapian::valueno slot_)
	: ValuePostingSource(slot_),
	  default_weight(0.0),
	  max_weight_in_map(0.0)
{
}

void
ValueMapPostingSource::add_mapping(const std::string & key, double weight)
{
    weight_map[key] = weight;
    max_weight_in_map = std::max(weight, max_weight_in_map);
}

void
ValueMapPostingSource::clear_mappings()
{
    weight_map.clear();
    max_weight_in_map = 0.0;
}

void
ValueMapPostingSource::set_default_weight(double wt)
{
    default_weight = wt;
}

Xapian::weight
ValueMapPostingSource::get_weight() const
{
    std::map<std::string, double>::const_iterator wit = weight_map.find(*value_it);
    if (wit == weight_map.end()) {
	return default_weight;
    }
    return wit->second;
}

ValueMapPostingSource *
ValueMapPostingSource::clone() const
{
    AutoPtr<ValueMapPostingSource> res(new ValueMapPostingSource(slot));
    std::map<std::string, double>::const_iterator i;
    for (i = weight_map.begin(); i != weight_map.end(); ++i) {
	res->add_mapping(i->first, i->second);
    }
    res->set_default_weight(default_weight);
    return res.release();
}

std::string
ValueMapPostingSource::name() const
{
    return std::string("Xapian::ValueMapPostingSource");
}

std::string
ValueMapPostingSource::serialise() const
{
    std::string result;
    result = encode_length(slot) + serialise_double(default_weight);

    std::map<std::string, double>::const_iterator i;
    for (i = weight_map.begin(); i != weight_map.end(); ++i) {
	result.append(encode_length(i->first.size()));
	result.append(i->first);
	result.append(serialise_double(i->second));
    }

    return result;
}

PostingSource *
ValueMapPostingSource::unserialise(const std::string &s) const
{
    const char * p = s.data();
    const char * end = p + s.size();

    Xapian::valueno new_slot = decode_length(&p, end, false);
    AutoPtr<ValueMapPostingSource> res(new ValueMapPostingSource(new_slot));
    res->set_default_weight(unserialise_double(&p, end));
    while (p != end) {
	size_t keylen = decode_length(&p, end, true);
	string key(p, keylen);
	p += keylen;
	res->add_mapping(key, unserialise_double(&p, end));
    }
    return res.release();
}

void
ValueMapPostingSource::init(const Database & db_)
{
    ValuePostingSource::init(db_);
    max_weight = std::max(max_weight_in_map, default_weight);
}

std::string
ValueMapPostingSource::get_description() const
{
    return "Xapian::ValueMapPostingSource(slot=" + om_tostring(slot) + ")";
}


FixedWeightPostingSource::FixedWeightPostingSource(Xapian::weight wt_)
	: wt(wt_),
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

	if (it == end) return;
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
    if (check_docid != 0) return false;
    return started && it == end;
}

Xapian::docid
FixedWeightPostingSource::get_docid() const
{
    if (check_docid != 0) return check_docid;
    return *it;
}

FixedWeightPostingSource *
FixedWeightPostingSource::clone() const
{
    return new FixedWeightPostingSource(wt);
}

std::string
FixedWeightPostingSource::name() const
{
    return std::string("Xapian::FixedWeightPostingSource");
}

std::string
FixedWeightPostingSource::serialise() const
{
    return serialise_double(wt);
}

PostingSource *
FixedWeightPostingSource::unserialise(const std::string &s) const
{
    const char * p = s.data();
    const char * s_end = p + s.size();
    double new_wt = unserialise_double(&p, s_end);
    if (p != s_end) {
	throw Xapian::NetworkError("Bad serialised ValueWeightPostingSource - junk at end");
    }
    return new FixedWeightPostingSource(new_wt);
}

void
FixedWeightPostingSource::init(const Xapian::Database & db_)
{
    db = db_;
    termfreq = db_.get_doccount();
    started = false;
    check_docid = 0;
}

std::string
FixedWeightPostingSource::get_description() const
{
    return "Xapian::FixedWeightPostingSource(wt=" + om_tostring(wt) + ")";
}


}
