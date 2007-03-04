/** @file weight.cc
 * @brief Xapian::Weight base class
 */
/* Copyright (C) 2007 Olly Betts
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

#include <xapian/enquire.h>

namespace Xapian {

/* Xapian::Weight */

Weight::~Weight() { }

Weight *
Weight::create(const Internal * internal_, Xapian::doclength querysize_,
	       Xapian::termcount wqf_, const std::string & tname_) const
{
    Weight * new_weight = clone();
    new_weight->internal = internal_;
    new_weight->querysize = querysize_;
    new_weight->wqf = wqf_;
    new_weight->tname = tname_;
    return new_weight;
}

bool Weight::get_sumpart_needs_doclength() const { return true; }

/* Xapian::BoolWeight */

BoolWeight * BoolWeight::clone() const { return new BoolWeight; }

BoolWeight::~BoolWeight() { }

std::string BoolWeight::name() const { return "Bool"; }

std::string BoolWeight::serialise() const { return ""; }

BoolWeight *
BoolWeight::unserialise(const std::string &) const
{
    return new BoolWeight;
}

Xapian::weight
BoolWeight::get_sumpart(Xapian::termcount, Xapian::doclength) const
{
    return 0;
}

Xapian::weight BoolWeight::get_maxpart() const { return 0; }

Xapian::weight BoolWeight::get_sumextra(Xapian::doclength) const { return 0; }

Xapian::weight BoolWeight::get_maxextra() const { return 0; }

bool BoolWeight::get_sumpart_needs_doclength() const { return false; }

}
