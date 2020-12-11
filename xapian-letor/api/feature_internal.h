/** @file
 * @brief Internals of Feature class
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

#ifndef XAPIAN_INCLUDED_FEATURE_INTERNAL_H
#define XAPIAN_INCLUDED_FEATURE_INTERNAL_H

#include "xapian-letor/feature.h"
#include "xapian-letor/featurelist.h"

#include <map>

namespace Xapian {

/** Class defining internals of Feature class. */
class Feature::Internal : public Xapian::Internal::intrusive_base {
    friend class Feature;

    /// Xapian::Database using which features will be calculated.
    Database feature_db;

    /// Xapian::Query using which features will be calculated.
    Query feature_query;

    /// Xapian::Document using which features will be calculated.
    Document feature_doc;

    /// Frequency of the Query Terms in the specified documents.
    std::map<std::string, Xapian::termcount> termfreq;

    /// Inverse Document Frequency of Query terms in the database.
    std::map<std::string, double> inverse_doc_freq;

    /** Length of the document as number of terms for different parts like
     *  'title', 'body' and 'whole'.
     */
    std::map<std::string, Xapian::termcount> doc_length;

    /** Length of the collection in number of terms for different parts like
     * 'title', 'body' and 'whole'.
     */
    std::map<std::string, Xapian::termcount> collection_length;

    /// Frequency of the Query Terms in the whole database
    std::map<std::string, Xapian::termcount> collection_termfreq;

  public:
    /// Default constructor
    Internal() {}

    /// Constructor creating an object instantiated with db,query and doc
    Internal(const Xapian::Database& db, const Xapian::Query& query,
	     const Xapian::Document& doc)
	 : feature_db(db), feature_query(query), feature_doc(doc) {}

    /// get database
    Database get_database() const {
	return feature_db;
    }

    /// get query
    Query get_query() const {
	return feature_query;
    }

    /// get document
    Document get_document() const {
	return feature_doc;
    }

    /// Get termfreq
    Xapian::termcount get_termfreq(const std::string& term) const;

    /// Get inverse_doc_freq
    double get_inverse_doc_freq(const std::string& term) const;

    /// Get doc_length
    Xapian::termcount get_doc_length(const std::string& term) const;

    /// Get collection_length
    Xapian::termcount get_collection_length(const std::string& term) const;

    /// Get collection_termfreq
    Xapian::termcount get_collection_termfreq(const std::string& term) const;

    /// Set the term frequency to use for Feature building.
    void set_termfreq(std::map<std::string, Xapian::termcount>&& tf) {
	termfreq = tf;
    }

    /// Set the inverse_doc_freq to use for Feature building.
    void set_inverse_doc_freq(std::map<std::string, double>&& idf) {
	inverse_doc_freq = idf;
    }

    /** Set the doc_length to use for Feature building.
     *
     *  This is used by Feature::Internal while populating Statistics.
     */
    void set_doc_length(std::map<std::string, Xapian::termcount>&& doc_len) {
	doc_length = doc_len;
    }

    /// Set the collection_length to use for Feature building.
    void set_collection_length(std::map<std::string,
					Xapian::termcount>&& collection_len) {
	collection_length = collection_len;
    }

    /// Set the collection_termfreq to use for Feature building.
    void set_collection_termfreq(std::map<std::string,
					  Xapian::termcount>&& collection_tf) {
	collection_termfreq = collection_tf;
    }
};

}

#endif // XAPIAN_INCLUDED_FEATURE_INTERNAL_H
