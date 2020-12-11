/** @file
 * @brief Description of Feature class
 */
/* Copyright (C) 2016 Ayush Tomar
 * Copyright (C) 2019 Vaibhav Kansagara
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

#include <config.h>

#include "xapian-letor/feature.h"
#include "api/feature_internal.h"
#include "debuglog.h"

namespace Xapian {

Feature::Feature() : stats_needed(), internal(new Feature::Internal())
{
    LOGCALL_CTOR(API, "Feature", NO_ARGS);
}

Feature::~Feature() {
    LOGCALL_DTOR(API, "Feature");
}

Xapian::termcount
Feature::get_termfreq(const std::string& term) const
{
    LOGCALL(API, Xapian::termcount, "Feature::get_termfreq", term);
    return internal->get_termfreq(term);
}

double
Feature::get_inverse_doc_freq(const std::string& term) const
{
    LOGCALL(API, double, "Feature::get_inverse_doc_freq", term);
    return internal->get_inverse_doc_freq(term);
}

Xapian::termcount
Feature::get_doc_length(const std::string& term) const
{
    LOGCALL(API, Xapian::termcount, "Feature::get_doc_length", term);
    return internal->get_doc_length(term);
}

Xapian::termcount
Feature::get_collection_length(const std::string& term) const
{
    LOGCALL(API, Xapian::termcount, "Feature::get_collection_length", term);
    return internal->get_collection_length(term);
}

Xapian::termcount
Feature::get_collection_termfreq(const std::string& term) const
{
    LOGCALL(API, Xapian::termcount, "Feature::get_collection_termfreq", term);
    return internal->get_collection_termfreq(term);
}

}
