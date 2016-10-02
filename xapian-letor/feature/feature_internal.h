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

#include "xapian-letor/feature.h"

#include <map>

namespace Xapian {

/** Class defining internals of Feature class. */
class Feature::Internal : public Xapian::Internal::intrusive_base {
    friend class Feature;

  public:
    /// Xapian::Database using which features will be calculated.
    Database feature_db;

    /// Xapian::Query using which features will be calculated.
    Query feature_query;

    /// Xapian::Document using which features will be calculated.
    Document feature_doc;

    /** This method finds the frequency of the query terms in the specified documents. This method is a helping method and statistics gathered through
     *  this method are used in feature value calculation. It return the frequency of the terms of query in std::map<string, long int> form.
     */
    std::map<std::string,long int> termfreq() const;

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
    std::map<std::string,double> inverse_doc_freq() const;

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
    std::map<std::string,long int> doc_length() const;

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
    std::map<std::string,long int> collection_length() const;

    /** This method calculates the frequency of query terms in the whole database. The information is stored in std::map<string, long int> format and
     *  used during the feature calculation methods.
     */
    std::map<std::string,long int> collection_termfreq() const;

};

}

#endif // FEATURE_INTERNAL_H
