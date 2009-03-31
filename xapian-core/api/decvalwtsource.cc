/** @file decvalwtsource.cc
 * @brief A posting source which returns decreasing weights from a value.
 */
/* Copyright (C) 2009 Lemur Consulting Ltd
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

#include "xapian/decvalwtsource.h"
#include "xapian/error.h"
#include "serialise.h"

using namespace Xapian;

DecreasingValueWeightPostingSource::DecreasingValueWeightPostingSource(
    Xapian::valueno slot_,
    Xapian::docid range_start_,
    Xapian::docid range_end_)
	: Xapian::ValueWeightPostingSource(slot_),
	  range_start(range_start_),
	  range_end(range_end_)
{
}

Xapian::weight
DecreasingValueWeightPostingSource::get_weight() const {
    return curr_weight;
}

Xapian::DecreasingValueWeightPostingSource *
DecreasingValueWeightPostingSource::clone() const {
    return new DecreasingValueWeightPostingSource(slot, range_start,
						  range_end);
}

std::string
DecreasingValueWeightPostingSource::name() const {
    return "Xapian::DecreasingValueWeightPostingSource()";
}

std::string
DecreasingValueWeightPostingSource::serialise() const {
    std::string result;
    result += encode_length(slot);
    result += encode_length(range_start);
    result += encode_length(range_end);
    return result;
}

Xapian::DecreasingValueWeightPostingSource *
DecreasingValueWeightPostingSource::unserialise(const std::string &s) const {
    const char * pos = s.data();
    const char * end = pos + s.size();
    Xapian::valueno new_slot = decode_length(&pos, end, false);
    Xapian::docid new_range_start = decode_length(&pos, end, false);
    Xapian::docid new_range_end = decode_length(&pos, end, false);
    if (pos != end)
	throw Xapian::NetworkError("Junk at end of serialised "
				   "DecreasingValueWeightPostingSource");
    return new DecreasingValueWeightPostingSource(new_slot, new_range_start,
						  new_range_end);
}

void
DecreasingValueWeightPostingSource::init(const Xapian::Database & db_) {
    Xapian::ValueWeightPostingSource::init(db_);
}

void
DecreasingValueWeightPostingSource::skip_if_in_range(Xapian::weight min_wt)
{
    if (value_it == value_end) return;
    curr_weight = Xapian::ValueWeightPostingSource::get_weight();
    Xapian::docid docid = Xapian::ValueWeightPostingSource::get_docid();
    if (docid >= range_start && (range_end == 0 || docid <= range_end)) {
	if (curr_weight < min_wt) {
	    // skip to end of range.
	    if (range_end == 0) {
		value_it = value_end;
	    } else {
		value_it.skip_to(range_end + 1);
		if (value_it != value_end)
		    curr_weight = Xapian::ValueWeightPostingSource::get_weight();
	    }
	}
    }
}

void
DecreasingValueWeightPostingSource::next(Xapian::weight min_wt) {
    Xapian::ValueWeightPostingSource::next(min_wt);
    skip_if_in_range(min_wt);
}

void
DecreasingValueWeightPostingSource::skip_to(Xapian::docid min_docid,
					    Xapian::weight min_wt) {
    Xapian::ValueWeightPostingSource::skip_to(min_docid, min_wt);
    skip_if_in_range(min_wt);
}

bool
DecreasingValueWeightPostingSource::check(Xapian::docid min_docid,
					  Xapian::weight min_wt) {
    bool valid = Xapian::ValueWeightPostingSource::check(min_docid, min_wt);
    if (valid)
	skip_if_in_range(min_wt);
    return valid;
}

std::string
DecreasingValueWeightPostingSource::get_description() const {
    return "DecreasingValueWeightPostingSource()";
}
