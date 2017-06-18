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
    feature_query = query;
}

void
Feature::set_doc(const Xapian::Document & doc)
{
    LOGCALL_VOID(API, "Feature::set_doc", doc);
    feature_doc = doc;
}

void
Feature::set_termfreq(const std::map<std::string, Xapian::termcount> & tf)
{
    LOGCALL_VOID(API, "Feature::set_termfreq", tf);
    termfreq = tf;
}

void
Feature::set_inverse_doc_freq(const std::map<std::string, double> & idf)
{
    LOGCALL_VOID(API, "Feature::set_inverse_doc_freq", idf);
    inverse_doc_freq = idf;
}

void
Feature::set_doc_length(const std::map<std::string,
			Xapian::termcount> & doc_len)
{
    LOGCALL_VOID(API, "Feature::set_doc_length", doc_len);
    doc_length = doc_len;
}

void
Feature::set_collection_length(const std::map<std::string,
			       Xapian::termcount> & collection_len)
{
    LOGCALL_VOID(API, "Feature::set_collection_length", collection_len);
    collection_length = collection_len;
}

void
Feature::set_collection_termfreq(const std::map<std::string,
				 Xapian::termcount> &collection_tf)
{
    LOGCALL_VOID(API, "Feature::set_collection_termfreq", collection_tf);
    collection_termfreq = collection_tf;
}

}
