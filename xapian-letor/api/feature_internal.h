/** @file feature_internal.h
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
    /// get database
    Database get_database() const;

    /// get query
    Query get_query() const;

    /// get document
    Document get_doc() const;

    /// Returns true if the term is found and false otherwise.
    bool get_termfreq(const std::string & term,
		      double & termfreq_) const;

    /// Returns true if the term is found and false otherwise.
    bool get_inverse_doc_freq(const std::string & term,
			      double & inverse_doc_freq_) const;

    /// Returns true if the term is found and false otherwise.
    bool get_doc_length(const std::string & term,
			double & doc_length_) const;

    /// Returns true if the term is found and false otherwise.
    bool get_collection_length(const std::string & term,
			       double & collection_length_);

    /// Returns true if the term is found and false otherwise.
    bool get_collection_termfreq(const std::string & term,
				 double & collection_termfreq_) const;

    /** Specify the database to use for feature building.
     *
     */
    void set_database(const Xapian::Database & db);

    /** Specify the query to use for feature building.
     *
     *
     *  @param query  Xapian::Query which has to be queried
     */
    void set_query(const Xapian::Query & query);

    /** Specify the document to use for feature building.
     *
     */
    void set_doc(const Xapian::Document & doc);

    /** Sets the termfrequency that is going to be used for
     *  Feature building.
     *
     */
    void set_termfreq(const std::map<std::string, Xapian::termcount> &tf);

    /** Sets the inverse_doc_freq that is going to be used for
     *  Feature building.
     *
     */
    void set_inverse_doc_freq(const std::map<std::string, double> & idf);

    /** Sets the doc_length that is going to be used for Feature building.
     *
     *  This is used by Feature::Internal while populating Statistics.
     */
    void set_doc_length(const std::map<std::string,
			Xapian::termcount> & doc_len);

    /** Sets the collection_length that is going to be used for
     *  Feature building.
     *
     */
    void set_collection_length(const std::map<std::string,
			       Xapian::termcount> & collection_len);

    /** Sets the collection_termfreq that is going to be used
     *  for Feature building.
     *
     */
    void set_collection_termfreq(const std::map<std::string,
				 Xapian::termcount> & collection_tf);
};

}

#endif // XAPIAN_INCLUDED_FEATURE_INTERNAL_H
