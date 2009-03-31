/** @file limitvalwtsource.h
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

#ifndef XAPIAN_INCLUDED_LIMITVALWTSOURCE_H
#define XAPIAN_INCLUDED_LIMITVALWTSOURCE_H

#include <xapian/types.h>
#include <xapian/postingsource.h>
#include <xapian/visibility.h>

namespace Xapian {

/// Read weights from a value, limiting them to a fixed maximum value.
class XAPIAN_VISIBILITY_DEFAULT LimitedValueWeightPostingSource
	: public Xapian::ValueWeightPostingSource {
    Xapian::weight specified_max_weight;
  public:
    LimitedValueWeightPostingSource(Xapian::valueno slot_,
				    Xapian::weight specified_max_weight_);

    Xapian::weight get_weight() const;
    LimitedValueWeightPostingSource * clone() const;
    std::string name() const;
    std::string serialise() const;
    LimitedValueWeightPostingSource * unserialise(const std::string &s) const;
    void init(const Xapian::Database & db_);

    std::string get_description() const;
};

}

#endif // XAPIAN_INCLUDED_LIMITVALWTSOURCE_H
