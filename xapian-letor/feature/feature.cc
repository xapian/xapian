/** @file feature.cc
 * @brief Description of Feature class
 */
/* Copyright (C) 2016 Ayush Tomar
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

#include "xapian-letor/feature.h"
#include "feature_internal.h"

namespace Xapian {

Feature::Feature(const Feature & o) : internal(o.internal) { }

void
Feature::operator=(const Feature & o)
{
    internal = o.internal;
}

Feature::Feature() : internal(new Feature::Internal) { }

Feature::~Feature() { }


void
Feature::set_database(const Xapian::Database & db) {
    internal->feature_db = db;
}

void
Feature::set_query(const Xapian::Query & query) {
    internal->feature_query = query;
}

void
Feature::set_doc(const Xapian::Document & doc) {
    internal->feature_doc = doc;
}

}
