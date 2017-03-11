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

#include <config.h>

#include "xapian-letor/feature.h"
#include "debuglog.h"

namespace Xapian {

Feature::Feature(const Feature & o) : feature_db(o.feature_db),feature_query(o.feature_query),
        feature_doc(feature_doc),termfreq(o.termfreq),inverse_doc_freq(o.inverse_doc_freq),
        doc_length(o.doc_length),collection_length(o.collection_length),collection_termfreq(o.collection_termfreq)
{
    LOGCALL_CTOR(API, "Feature", o);
}

void
Feature::operator=(const Feature & o)
{
    LOGCALL_VOID(API, "Feature::operator=", o);

    feature_db = o.feature_db;

    feature_query = o.feature_query;

    feature_doc = o.feature_doc;

    termfreq = o.termfreq;

    inverse_doc_freq = o.inverse_doc_freq;

    doc_length = o.doc_length;

    collection_length = o.collection_length;

    collection_termfreq = o.collection_termfreq;
}

Feature::Feature()
{
    LOGCALL_CTOR(API, "Feature", NO_ARGS);
}

Feature::~Feature() {
    LOGCALL_DTOR(API, "Feature");
}


void
Feature::set_database(const Xapian::Database & db)
{
    LOGCALL_VOID(API, "Feature::set_database", db);
    feature_db = db;
}

void
Feature::set_query(const Xapian::Query & query)
{
    LOGCALL_VOID(API, "Feature::set_query", query);
    if (query.empty()) {
        throw Xapian::InvalidArgumentError("Can't initialise with an empty query string");
    }
    feature_query = query;
}

void
Feature::set_doc(const Xapian::Document & doc)
{
    LOGCALL_VOID(API, "Feature::set_doc", doc);
    feature_doc = doc;
}

}
