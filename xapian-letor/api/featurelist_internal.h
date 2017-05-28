/** @file feature_internal.h
 * @brief Internals of Feature class
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

#ifndef FEATURE_INTERNAL_H
#define FEATURE_INTERNAL_H

#include "xapian-letor/featurelist.h"

#include <map>

namespace Xapian {

/** Class defining internals of Feature class. */
class FeatureList::Internal : public Xapian::Internal::intrusive_base {
    friend class FeatureList;

  public:

    /// Stats which FeatureList can use to determine the stats needed by a Feature.
    typedef enum {
	/// Number of documents in the collection.
	TERM_FREQUENCY = 1,
	/// Number of documents in the RSet.
	INVERSE_DOCUMENT_FREQUENCY = 2,
	/// Average length of documents in the collection.
	DOCUMENT_LENGTH = 4,
	/// How many documents the current term is in.
	COLLECTION_LENGTH = 8,
	/// How many documents in the RSet the current term is in.
	COLLECTION_TERM_FREQ = 16,
    }stat_flags;

    /// Xapian::Database using which features will be calculated.
    Database featurelist_db;

    /// Xapian::Query using which features will be calculated.
    Query featurelist_query;

    /// Xapian::Document using which features will be calculated.
    Document featurelist_doc;


    /// Frequency of the Query Terms in the specified documents.
    std::map<std::string, long int> termfreq;

    /// Inverse Document Frequency of Query terms in the database.
    std::map<std::string, double> inverse_doc_freq;

    /// Length of the document as number of "terms"
    std::map<std::string, long int> doc_length;

    /// Length of the collection in number of terms for different parts like 'title', 'body' and 'whole'
    std::map<std::string, long int> collection_length;

    /// Frequency of the Query Terms in the whole database
    std::map<std::string, long int> collection_termfreq;

    /** This method finds the frequency of the query terms in the specified documents. This method is a helping method and statistics gathered through
     *  this method are used in feature value calculation. It return the frequency of the terms of query in std::map<string, long int> form.
     */
    std::map<std::string,long int> compute_termfreq() const;

    /** This method calculates the inverse document frequency(idf) of query terms in the database. It returns the idf of each term in
     *  std::map<string, double> form.
     *
     *  Note: idf of a term 't' is calculated as below:
     *
     *  idf(t) = log(N/df(t))
     *                                  Where,
     *                                  N = Total number of documents in database and
     *                                  df(t) = number of documents containing term 't'
     */
    std::map<std::string,double> compute_inverse_doc_freq() const;

    /** This method calculates the length of the documents as number of 'terms'. It calculates the length for three different
     *  parts: title, body and whole document. This information is returned in the std::map<string, long int> format.
     *  It can be accessed as below:
     *
     *  @code
     *  map<string, long int> len;
     *  len["title"];
     *  len["body"];
     *  len["whole"];
     *  @endcode
     */
    std::map<std::string,long int> compute_doc_length() const;

    /** This method calculates the length of the collection in number of terms for different parts like 'title', 'body' and 'whole'. This is calculated
     *  as a stored user metadata in omindex otherwise it is calculated out of scratch (this might take some time depending upon the size of the
     *  database. Length information is stored in std::map<string, long int> format and can be accessed as below:
     *
     *  @code
     *  map<string, long int> len;
     *  len["title"];
     *  len["body"];
     *  len["whole"];
     *  @endcode
     *
     */
    std::map<std::string,long int> compute_collection_length() const;

    /** This method calculates the frequency of query terms in the whole database. The information is stored in std::map<string, long int> format and
     *  used during the feature calculation methods.
     */
    std::map<std::string,long int> compute_collection_termfreq() const;

    /** This Method computes all the statistics and stores them in their corresponding variables
     */
    void compute_statistics(const Xapian::Query & query, const Xapian::Database & db, const Xapian::Document & doc);

    /// Specify the database to use for feature building. This will be used by the Internal class.
    void set_database(const Xapian::Database & db);

    /** Specify the query to use for feature building. This will be used by the Internal class.
     * @param query  Xapian::Query which has to be queried
     * @exception Xapian::InvalidArgumentError will be thrown if an empty
     *  query is supplied
     */
    void set_query(const Xapian::Query & query);

    /// Specify the document to use for feature building. This will be used by the Internal class.
    void set_doc(const Xapian::Document & doc);

    /// Populates the statistics needed by a Feature
    void populate_feature(Feature *feature);

};

}

#endif // FEATURE_INTERNAL_H
