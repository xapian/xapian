/** @file scaleweight.h
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

#ifndef XAPIAN_INCLUDED_SCALEWEIGHT_H
#define XAPIAN_INCLUDED_SCALEWEIGHT_H

#include <xapian/enquire.h>

/// Xapian::Weight subclass for implementing OP_SCALE_WEIGHT.
class ScaleWeight : public Xapian::Weight {
    Xapian::Weight * real_wt;

    double factor;

  public:
    ScaleWeight * clone() const;
    ScaleWeight(Xapian::Weight * real_wt_, double factor_)
	: real_wt(real_wt_), factor(factor_) { }
    ~ScaleWeight();
    std::string name() const;
    std::string serialise() const;
    ScaleWeight * unserialise(const std::string & s) const;
    Xapian::weight get_sumpart(Xapian::termcount wdf, Xapian::doclength len) const;
    Xapian::weight get_maxpart() const;

    Xapian::weight get_sumextra(Xapian::doclength len) const;
    Xapian::weight get_maxextra() const;

    bool get_sumpart_needs_doclength() const;
};

#endif // XAPIAN_INCLUDED_SCALEWEIGHT_H
