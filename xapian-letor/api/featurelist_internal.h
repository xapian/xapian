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

#ifndef XAPIAN_LETOR_INCLUDED_FEATURE_INTERNAL_H
#define XAPIAN_LETOR_INCLUDED_FEATURE_INTERNAL_H

#include "xapian-letor/featurelist.h"

#include <map>

namespace Xapian {

/** Class defining internals of Feature class. */
class FeatureList::Internal : public Xapian::Internal::intrusive_base {
    friend class FeatureList;

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
    } stat_flags;

    /// Xapian::Database using which features will be calculated.
    Database featurelist_db;

    /// Xapian::Query using which features will be calculated.
    Query featurelist_query;

    /// Xapian::Document using which features will be calculated.
    Document featurelist_doc;

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

    /** This method finds the frequency of the query terms in the
     *  specified documents.
     *
     *  This method is a helper method and statistics gathered through
     *  this method are used in feature value calculation. This information
     *  is stored in termfreq.
     */
    void compute_termfreq();

    /** This method calculates the inverse document frequency(idf) of query
     *  terms in the database.
     *
     *  This method is a helper method and statistics gathered through
     *  this method are used in feature value calculation. This information
     *  is stored in inverse_doc_freq.
     *
     *  Note: idf of a term 't' is calculated as below:
     *
     *  idf(t) = log(N/df(t))
     *  Where,
     *  N = Total number of documents in database and
     *  df(t) = number of documents containing term 't'
     */
    void compute_inverse_doc_freq();

    /** This method calculates the length of the documents as number of 'terms'.
     *  It calculates the length for three different parts:
     *  title, body and whole document.
     *
     *  This method is a helper method and statistics gathered through
     *  this method are used in feature value calculation. This information
     *  is stored in doc_length in the following format:
     *
     *  @code
     *  map<string, long int> len;
     *  len["title"];
     *  len["body"];
     *  len["whole"];
     *  @endcode
     */
    void compute_doc_length();

    /** This method calculates the length of the collection in number of terms
     *  for different parts like 'title', 'body' and 'whole'.
     *
     *  This is calculated as a stored user metadata in omindex otherwise
     *  it is calculated out of scratch
     *  (this might take some time depending upon the size of the database).
     *
     *  This method is a helper method and statistics gathered through
     *  this method are used in feature value calculation. This information
     *  is stored in collection_length in the following format.
     *
     *  @code
     *  map<string, long int> len;
     *  len["title"];
     *  len["body"];
     *  len["whole"];
     *  @endcode
     *
     */
    void compute_collection_length();

    /** This method calculates the frequency of query terms in
     *  the whole database.
     *
     *  This method is a helper method and statistics gathered through
     *  this method are used in feature value calculation. This information
     *  is stored in collection_termfreq.
     */
    void compute_collection_termfreq();

    /** Specify the database to use for feature building.
     *
     *  This will be used by the Internal class.
     */
    void set_database(const Xapian::Database & db);

    /** Specify the query to use for feature building.
     *
     *  This will be used by the Internal class.
     */
    void set_query(const Xapian::Query & query);

    /** Specify the document to use for feature building.
     *
     *  This will be used by the Internal class.
     */
    void set_doc(const Xapian::Document & doc);

  public:

    /** Vector containing Feature pointer objects.
     *  Each will be used to return feature value.
     */
    std::vector<Feature *> feature;

    /// This method sets all the data members required for computing stats.
    void set_data(const Xapian::Query & query,
			    const Xapian::Database & db,
			    const Xapian::Document & doc);

    /// Computes and Populates the stats needed by a Feature.
    void populate_feature(Feature *feature_);

    /// Clears all the stats.
    void clear_stats();
};

}

#endif // XAPIAN_LETOR_INCLUDED_FEATURE_INTERNAL_H
