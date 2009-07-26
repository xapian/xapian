/** @file limitvalwtsource.cc
 * @brief A posting source which returns weights limited by a maxmium value.
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

#include "xapian/postingsource.h"
#include "xapian/error.h"
#include "serialise.h"
#include "serialise-double.h"
#include <algorithm>

using namespace Xapian;

LimitedValueWeightPostingSource::LimitedValueWeightPostingSource(
    Xapian::valueno slot_,
    Xapian::weight specified_max_weight_)
	: Xapian::ValueWeightPostingSource(slot_),
	  specified_max_weight(specified_max_weight_)
{
}

Xapian::weight
LimitedValueWeightPostingSource::get_weight() const {
    return std::min(Xapian::ValueWeightPostingSource::get_weight(),
		    specified_max_weight);
}

LimitedValueWeightPostingSource *
LimitedValueWeightPostingSource::clone() const {
    return new LimitedValueWeightPostingSource(slot, specified_max_weight);
}

std::string
LimitedValueWeightPostingSource::name() const {
    return "Xapian::LimitedValueWeightPostingSource";
}

std::string
LimitedValueWeightPostingSource::serialise() const {
    std::string result;
    result += encode_length(slot);
    result += serialise_double(specified_max_weight);
    return result;
}

LimitedValueWeightPostingSource *
LimitedValueWeightPostingSource::unserialise(const std::string &s) const {
    const char * pos = s.data();
    const char * end = pos + s.size();
    Xapian::valueno new_slot = decode_length(&pos, end, false);
    Xapian::weight new_specified_max_weight = unserialise_double(&pos, end);
    if (pos != end)
	throw Xapian::NetworkError("Junk at end of serialised "
				   "LimitedValueWeightPostingSource");
    return new LimitedValueWeightPostingSource(new_slot,
					       new_specified_max_weight);
}

void
LimitedValueWeightPostingSource::init(const Xapian::Database & db_) {
    Xapian::ValueWeightPostingSource::init(db_);
    set_maxweight(std::min(get_maxweight(), specified_max_weight));
}

std::string
LimitedValueWeightPostingSource::get_description() const {
    return "LimitedValueWeightPostingSource()";
}
