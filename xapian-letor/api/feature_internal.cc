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

Database
Feature::Internal::get_database() const
{
    LOGCALL(API, Database, "Feature::Internal::get_database", NO_ARGS);
    return feature_db;
}

Query
Feature::Internal::get_query() const
{
    LOGCALL(API, Query, "Feature::Internal::get_query", NO_ARGS);
    return feature_query;
}

Document
Feature::Internal::get_doc() const
{
    LOGCALL(API, Document, "Feature::Internal::get_doc", NO_ARGS);
    return feature_doc;
}

bool
Feature::Internal::get_termfreq(const std::string & term,
			double & termfreq_) const
{
    LOGCALL(API, bool, "Feature::Internal::get_termfreq", term | termfreq_);
    auto termfreq_iterator = termfreq.find(term);
    if (termfreq_iterator != termfreq.end()) {
	termfreq_ = (double)termfreq_iterator->second;
	return true;
    }
    return false;
}

bool
Feature::Internal::get_inverse_doc_freq(const std::string & term,
			double & inverse_doc_freq_) const
{
    LOGCALL(API, bool, "Feature::Internal::get_inverse_doc_freq", term | inverse_doc_freq_);
    auto inverse_doc_freq_iterator = inverse_doc_freq.find(term);
    if (inverse_doc_freq_iterator != inverse_doc_freq.end()) {
	inverse_doc_freq_ = (double)inverse_doc_freq_iterator->second;
	return true;
    }
    return false;
}

bool
Feature::Internal::get_doc_length(const std::string & term,
			double & doc_length_) const
{
    LOGCALL(API, bool, "Feature::Internal::get_doc_length", term | doc_length_);
    auto doc_length_iterator = doc_length.find(term);
    if (doc_length_iterator != doc_length.end()) {
	doc_length_ = (double)doc_length_iterator->second;
	return true;
    }
    return false;
}

bool
Feature::Internal::get_collection_length(const std::string & term,
			double & collection_length_)
{
    LOGCALL(API, bool, "Feature::Internal::get_collection_length", term | collection_length_);
    auto collection_length_iterator = collection_length.find(term);
    if (collection_length_iterator != collection_length.end()) {
	collection_length_ = (double)collection_length_iterator->second;
	return true;
    }
    return false;
}

bool
Feature::Internal::get_collection_termfreq(const std::string & term,
			double & collection_termfreq_) const
{
    LOGCALL(API, bool, "Feature::Internal::get_collection_termfreq", term | collection_termfreq);
    auto collection_termfreq_iterator = collection_termfreq.find(term);
    if (collection_termfreq_iterator != collection_termfreq.end()) {
	collection_termfreq_ = (double)collection_termfreq_iterator->second;
	return true;
    }
    return false;
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
