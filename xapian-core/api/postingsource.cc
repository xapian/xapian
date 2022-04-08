/** @file
 * @brief External sources of posting information
 */
/* Copyright (C) 2008-2022 Olly Betts
 * Copyright (C) 2008,2009 Lemur Consulting Ltd
 * Copyright (C) 2010 Richard Boulton
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

// We need to be able to set deprecated members of ValuePostingSource.
#define XAPIAN_DEPRECATED(X) X
#include "xapian/postingsource.h"

#include "autoptr.h"

#include "backends/database.h"
#include "backends/document.h"
#include "matcher/multimatch.h"

#include "xapian/document.h"
#include "xapian/error.h"
#include "xapian/queryparser.h" // For sortable_unserialise().

#include "omassert.h"
#include "net/length.h"
#include "serialise-double.h"
#include "str.h"

#include <cfloat>

using namespace std;

namespace Xapian {

PostingSource::~PostingSource() { }

void
PostingSource::set_maxweight(double max_weight)
{
    if (usual(matcher_)) {
	MultiMatch * multimatch = static_cast<MultiMatch*>(matcher_);
	multimatch->recalc_maxweight();
    }
    max_weight_ = max_weight;
}

double
PostingSource::get_weight() const
{
    return 0;
}

void
PostingSource::skip_to(Xapian::docid did, double min_wt)
{
    while (!at_end() && get_docid() < did) {
	next(min_wt);
    }
}

bool
PostingSource::check(Xapian::docid did, double min_wt)
{
    skip_to(did, min_wt);
    return true;
}

PostingSource *
PostingSource::clone() const
{
    return NULL;
}

string
PostingSource::name() const
{
    return string();
}

string
PostingSource::serialise() const
{
    throw Xapian::UnimplementedError("serialise() not supported for this PostingSource");
}

PostingSource *
PostingSource::unserialise(const string &) const
{
    throw Xapian::UnimplementedError("unserialise() not supported for this PostingSource");
}

PostingSource *
PostingSource::unserialise_with_registry(const std::string &s,
					 const Registry &) const
{
    return unserialise(s);
}

string
PostingSource::get_description() const
{
    return "Xapian::PostingSource subclass";
}


ValuePostingSource::ValuePostingSource(Xapian::valueno slot_)
	: real_slot(slot_),
	  db(real_db),
	  slot(real_slot),
	  value_it(real_value_it),
	  started(real_started),
	  termfreq_min(real_termfreq_min),
	  termfreq_est(real_termfreq_est),
	  termfreq_max(real_termfreq_max)
{
}

Xapian::doccount
ValuePostingSource::get_termfreq_min() const
{
    return real_termfreq_min;
}

Xapian::doccount
ValuePostingSource::get_termfreq_est() const
{
    return real_termfreq_est;
}

Xapian::doccount
ValuePostingSource::get_termfreq_max() const
{
    return real_termfreq_max;
}

void
ValuePostingSource::next(double min_wt)
{
    if (!real_started) {
	real_started = true;
	real_value_it = real_db.valuestream_begin(real_slot);
    } else {
	++real_value_it;
    }

    if (real_value_it == real_db.valuestream_end(real_slot)) return;

    if (min_wt > get_maxweight()) {
	real_value_it = real_db.valuestream_end(real_slot);
	return;
    }
}

void
ValuePostingSource::skip_to(Xapian::docid min_docid, double min_wt)
{
    if (!real_started) {
	real_started = true;
	real_value_it = real_db.valuestream_begin(real_slot);

	if (real_value_it == real_db.valuestream_end(real_slot)) return;
    }

    if (min_wt > get_maxweight()) {
	real_value_it = real_db.valuestream_end(real_slot);
	return;
    }
    real_value_it.skip_to(min_docid);
}

bool
ValuePostingSource::check(Xapian::docid min_docid, double min_wt)
{
    if (!real_started) {
	real_started = true;
	real_value_it = real_db.valuestream_begin(real_slot);

	if (real_value_it == real_db.valuestream_end(real_slot)) return true;
    }

    if (min_wt > get_maxweight()) {
	real_value_it = real_db.valuestream_end(real_slot);
	return true;
    }
    return real_value_it.check(min_docid);
}

bool
ValuePostingSource::at_end() const
{
    return real_started && real_value_it == real_db.valuestream_end(real_slot);
}

Xapian::docid
ValuePostingSource::get_docid() const
{
    return real_value_it.get_docid();
}

void
ValuePostingSource::init(const Database & db_)
{
    real_db = db_;
    real_started = false;
    set_maxweight(DBL_MAX);
    real_termfreq_max = real_db.get_value_freq(real_slot);
    real_termfreq_est = real_termfreq_max;
    real_termfreq_min = real_termfreq_max;
}


ValueWeightPostingSource::ValueWeightPostingSource(Xapian::valueno slot_)
	: ValuePostingSource(slot_)
{
}

double
ValueWeightPostingSource::get_weight() const
{
    Assert(!at_end());
    Assert(get_started());
    return sortable_unserialise(get_value());
}

ValueWeightPostingSource *
ValueWeightPostingSource::clone() const
{
    return new ValueWeightPostingSource(get_slot());
}

string
ValueWeightPostingSource::name() const
{
    return string("Xapian::ValueWeightPostingSource");
}

string
ValueWeightPostingSource::serialise() const
{
    return encode_length(get_slot());
}

ValueWeightPostingSource *
ValueWeightPostingSource::unserialise(const string &s) const
{
    const char * p = s.data();
    const char * end = p + s.size();

    Xapian::valueno new_slot;
    decode_length(&p, end, new_slot);
    if (p != end) {
	throw Xapian::NetworkError("Bad serialised ValueWeightPostingSource - junk at end");
    }

    return new ValueWeightPostingSource(new_slot);
}

void
ValueWeightPostingSource::init(const Database & db_)
{
    ValuePostingSource::init(db_);

    string upper_bound = get_database().get_value_upper_bound(get_slot());
    if (upper_bound.empty()) {
	// This should only happen if there are no entries, in which case the
	// maxweight is 0.
	set_maxweight(0.0);
    } else {
	set_maxweight(sortable_unserialise(upper_bound));
    }
}

string
ValueWeightPostingSource::get_description() const
{
    string desc("Xapian::ValueWeightPostingSource(slot=");
    desc += str(get_slot());
    desc += ")";
    return desc;
}


ValueMapPostingSource::ValueMapPostingSource(Xapian::valueno slot_)
	: ValuePostingSource(slot_),
	  default_weight(0.0),
	  max_weight_in_map(0.0)
{
}

void
ValueMapPostingSource::add_mapping(const string & key, double wt)
{
    weight_map[key] = wt;
    max_weight_in_map = max(wt, max_weight_in_map);
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

double
ValueMapPostingSource::get_weight() const
{
    map<string, double>::const_iterator wit = weight_map.find(get_value());
    if (wit == weight_map.end()) {
	return default_weight;
    }
    return wit->second;
}

ValueMapPostingSource *
ValueMapPostingSource::clone() const
{
    AutoPtr<ValueMapPostingSource> res(new ValueMapPostingSource(get_slot()));
    map<string, double>::const_iterator i;
    for (i = weight_map.begin(); i != weight_map.end(); ++i) {
	res->add_mapping(i->first, i->second);
    }
    res->set_default_weight(default_weight);
    return res.release();
}

string
ValueMapPostingSource::name() const
{
    return string("Xapian::ValueMapPostingSource");
}

string
ValueMapPostingSource::serialise() const
{
    string result = encode_length(get_slot());
    result += serialise_double(default_weight);

    map<string, double>::const_iterator i;
    for (i = weight_map.begin(); i != weight_map.end(); ++i) {
	result.append(encode_length(i->first.size()));
	result.append(i->first);
	result.append(serialise_double(i->second));
    }

    return result;
}

ValueMapPostingSource *
ValueMapPostingSource::unserialise(const string &s) const
{
    const char * p = s.data();
    const char * end = p + s.size();

    Xapian::valueno new_slot;
    decode_length(&p, end, new_slot);
    AutoPtr<ValueMapPostingSource> res(new ValueMapPostingSource(new_slot));
    res->set_default_weight(unserialise_double(&p, end));
    while (p != end) {
	size_t keylen;
	decode_length_and_check(&p, end, keylen);
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
    set_maxweight(max(max_weight_in_map, default_weight));
}

string
ValueMapPostingSource::get_description() const
{
    string desc("Xapian::ValueMapPostingSource(slot=");
    desc += str(get_slot());
    desc += ")";
    return desc;
}

FixedWeightPostingSource::FixedWeightPostingSource(double wt)
    : started(false)
{
    // The weight is fixed at wt, so that's the maxweight too.  So just store wt
    // as the maxweight and we can read it from there when we need it.
    set_maxweight(wt);
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

double
FixedWeightPostingSource::get_weight() const
{
    return get_maxweight();
}

void
FixedWeightPostingSource::next(double min_wt)
{
    if (!started) {
	started = true;
	it = db.postlist_begin(string());
    } else {
	++it;
    }

    if (it == db.postlist_end(string())) return;

    if (check_docid) {
	it.skip_to(check_docid + 1);
	check_docid = 0;
    }

    if (min_wt > get_maxweight()) {
	it = db.postlist_end(string());
    }
}

void
FixedWeightPostingSource::skip_to(Xapian::docid min_docid, double min_wt)
{
    if (!started) {
	started = true;
	it = db.postlist_begin(string());

	if (it == db.postlist_end(string())) return;
    }

    if (check_docid) {
	if (min_docid < check_docid)
	    min_docid = check_docid + 1;
	check_docid = 0;
    }

    if (min_wt > get_maxweight()) {
	it = db.postlist_end(string());
	return;
    }
    it.skip_to(min_docid);
}

bool
FixedWeightPostingSource::check(Xapian::docid min_docid, double)
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
    return started && it == db.postlist_end(string());
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
    return new FixedWeightPostingSource(get_maxweight());
}

string
FixedWeightPostingSource::name() const
{
    return string("Xapian::FixedWeightPostingSource");
}

string
FixedWeightPostingSource::serialise() const
{
    return serialise_double(get_maxweight());
}

FixedWeightPostingSource *
FixedWeightPostingSource::unserialise(const string &s) const
{
    const char * p = s.data();
    const char * s_end = p + s.size();
    double new_wt = unserialise_double(&p, s_end);
    if (p != s_end) {
	throw Xapian::NetworkError("Bad serialised FixedWeightPostingSource - junk at end");
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

string
FixedWeightPostingSource::get_description() const
{
    string desc("Xapian::FixedWeightPostingSource(wt=");
    desc += str(get_maxweight());
    desc += ")";
    return desc;
}

}
