/** @file feature_internal.cc
 * @brief Definition of Feature::Internal class.
 */
/* Copyright (C) 2019 Vaibhav Kansagara
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
#include "feature_internal.h"

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "debuglog.h"

using namespace std;
using namespace Xapian;

Xapian::termcount
Feature::Internal::get_termfreq(const std::string& term) const
{
    LOGCALL(API, Xapian::termcount, "Feature::Internal::get_termfreq", term);
    auto termfreq_iterator = termfreq.find(term);
    if (termfreq_iterator != termfreq.end()) {
	return termfreq_iterator->second;
    }
    return 0;
}

double
Feature::Internal::get_inverse_doc_freq(const std::string& term) const
{
    LOGCALL(API, double, "Feature::Internal::get_inverse_doc_freq", term);
    auto inverse_doc_freq_iterator = inverse_doc_freq.find(term);
    if (inverse_doc_freq_iterator != inverse_doc_freq.end()) {
	return inverse_doc_freq_iterator->second;
    }
    return 0;
}

Xapian::termcount
Feature::Internal::get_doc_length(const std::string& term) const
{
    LOGCALL(API, Xapian::termcount, "Feature::Internal::get_doc_length", term);
    auto doc_length_iterator = doc_length.find(term);
    if (doc_length_iterator != doc_length.end()) {
	return doc_length_iterator->second;
    }
    return 0;
}

Xapian::termcount
Feature::Internal::get_collection_length(const std::string& term) const
{
    LOGCALL(API, Xapian::termcount, "Feature::Internal::get_collection_length", term);
    auto collection_length_iterator = collection_length.find(term);
    if (collection_length_iterator != collection_length.end()) {
	return collection_length_iterator->second;
    }
    return 0;
}

Xapian::termcount
Feature::Internal::get_collection_termfreq(const std::string& term) const
{
    LOGCALL(API, Xapian::termcount, "Feature::Internal::get_collection_termfreq", term);
    auto collection_termfreq_iterator = collection_termfreq.find(term);
    if (collection_termfreq_iterator != collection_termfreq.end()) {
	return collection_termfreq_iterator->second;
    }
    return 0;
}

void
Feature::Internal::set_database(const Xapian::Database & db)
{
    LOGCALL_VOID(API, "Feature::Internal::set_database", db);
    feature_db = db;
}

void
Feature::Internal::set_query(const Xapian::Query & query)
{
    LOGCALL_VOID(API, "Feature::Internal::set_query", query);
    feature_query = query;
}

void
Feature::Internal::set_doc(const Xapian::Document & doc)
{
    LOGCALL_VOID(API, "Feature::Internal::set_doc", doc);
    feature_doc = doc;
}

void
Feature::Internal::set_termfreq(const std::map<std::string,
				Xapian::termcount> & tf)
{
    LOGCALL_VOID(API, "Feature::Internal::set_termfreq", tf);
    termfreq = tf;
}

void
Feature::Internal::set_inverse_doc_freq(const std::map<std::string,
					double> & idf)
{
    LOGCALL_VOID(API, "Feature::Internal::set_inverse_doc_freq", idf);
    inverse_doc_freq = idf;
}

void
Feature::Internal::set_doc_length(const std::map<std::string,
			Xapian::termcount> & doc_len)
{
    LOGCALL_VOID(API, "Feature::Internal::set_doc_length", doc_len);
    doc_length = doc_len;
}

void
Feature::Internal::set_collection_length(const std::map<std::string,
			       Xapian::termcount> & collection_len)
{
    LOGCALL_VOID(API, "Feature::Internal::set_collection_length", collection_len);
    collection_length = collection_len;
}

void
Feature::Internal::set_collection_termfreq(const std::map<std::string,
				 Xapian::termcount> &collection_tf)
{
    LOGCALL_VOID(API, "Feature::Internal::set_collection_termfreq", collection_tf);
    collection_termfreq = collection_tf;
}
