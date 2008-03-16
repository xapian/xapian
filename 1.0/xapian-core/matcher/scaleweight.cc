/** @file scaleweight.cc
 * @brief Xapian::Weight subclass for implementing OP_SCALE_WEIGHT.
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

#include "scaleweight.h"

#include "serialise-double.h"

ScaleWeight *
ScaleWeight::clone() const {
    return new ScaleWeight(real_wt->clone(), factor);
}

ScaleWeight::~ScaleWeight() { delete real_wt; }

std::string
ScaleWeight::name() const {
    return "Scale";
}

std::string
ScaleWeight::serialise() const {
    return serialise_double(factor) + real_wt->serialise();
}

ScaleWeight *
ScaleWeight::unserialise(const std::string & s) const
{
    const char *start = s.data();
    const char *p = start;
    const char *p_end = p + s.size();
    double factor_ = unserialise_double(&p, p_end);
    return new ScaleWeight(real_wt->unserialise(s.substr(p - start)), factor_);
}

Xapian::weight
ScaleWeight::get_sumpart(Xapian::termcount t, Xapian::doclength l) const
{
    return factor * real_wt->get_sumpart(t, l);
}

Xapian::weight
ScaleWeight::get_maxpart() const {
    return factor * real_wt->get_maxpart();
}

Xapian::weight ScaleWeight::get_sumextra(Xapian::doclength l) const {
    return factor * real_wt->get_sumextra(l);
}

Xapian::weight ScaleWeight::get_maxextra() const {
    return factor * real_wt->get_maxextra();
}

bool ScaleWeight::get_sumpart_needs_doclength() const {
    return real_wt->get_sumpart_needs_doclength();
}
