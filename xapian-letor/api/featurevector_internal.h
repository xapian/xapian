/** @file
 * @brief Internals of FeatureVector class
 */
/* Copyright (C) 2012 Parth Gupta
 * Copyright (C) 2016 Ayush Tomar
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301
 * USA
 */

#ifndef XAPIAN_INCLUDED_FEATUREVECTOR_INTERNAL_H
#define XAPIAN_INCLUDED_FEATUREVECTOR_INTERNAL_H

#include "xapian-letor/featurevector.h"

#include <vector>

namespace Xapian {

class FeatureVector::Internal : public Xapian::Internal::intrusive_base
{
    friend class FeatureVector;
    double label_;
    double score_;
    std::vector<double> fvals_;
    Xapian::docid did_;
};

}

#endif // XAPIAN_INCLUDED_FEATUREVECTOR_INTERNAL_H
