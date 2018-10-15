/** @file letor_features.h
 *  @brief The feature manager file for Learning to Rank
 */
 /* Copyright (C) 2012 Parth Gupta
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

#ifndef LETOR_FEATURES_H
#define LETOR_FEATURES_H

#include "letor.h"

#include <list>
#include <map>

using namespace std;


namespace Xapian {

class XAPIAN_VISIBILITY_DEFAULT Features {
 public:

    /** This method finds the frequency of the query terms in the specified documents. This method is a helping method and statistics gathered through
     *  this method are used in feature value calculation. It return the frequency of the terms of query in std::map<string, long int> form.
     *
     *  @param  doc     This is the document in which we want the frequency of the terms in query. Generally in Letor, this document is taken
     *          from the Xapian::MSet retrieved from the first retrieval.
     *  @param  query   This is the query for which terms we want frequency in the document.
     */
    map<string,long int> termfreq(const Xapian::Document & doc,const Xapian::Query & query);

    /** This method calculates the inverse document frequency(idf) of query terms in the database. It returns the idf of each term in
     *  std::map<string, double> form.
     *
     *  Note: idf of a term 't' is calculated as below:
     *
     *  idf(t) = log(N/df(t))
     *                                  Where,
     *                                  N = Total number of documents in database and
     *                                  df(t) = number of documents containing term 't'
     *
     *  @param  db      specify the database being used for search to calculate idf values.
     *  @param  query   query being used in the retrieval
     */
    map<string,double> inverse_doc_freq(const Xapian::Database & db, const Xapian::Query & query);

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
     *
     *  @param  db      Database containing that document.
     *  @param  doc     The document whose length is to be found.
     */
    map<string,long int> doc_length(const Xapian::Database & db, const Xapian::Document & doc);

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
     *  @param  db      Database used for retrieval.
     *
     */
    map<string,long int> collection_length(const Xapian::Database & db);

    /** This method calculates the frequency of query terms in the whole database. The information is stored in std::map<string, long int> format and
     *  used during the feature calculation methods.
     *
     *  @param  db      Database to be used
     *  @param  query   Query being searched.
     */
    map<string,long int> collection_termfreq(const Xapian::Database & db, const Xapian::Query & query);


    /** It calculated the feature value for the query-document pair. These feature calculation methods uses the data generated using above defined
     *  methods like termfreq, inverse_doc_freq, doc_length, coll_freq and collection_length.
     *
     *  These features are depicted in overview documentation of Letor.
     *
     *  @param  query   Query being used.
     *  @param  tf      Map generated using Letor::termfreq() method.
     *  @param  ch      Specifies the part of document for which the value needs to be calculated. Values:
     *          't'     Title only
     *          'b'     Body only
     *          'w'     Whole document
     */

    double calculate_f1(const Xapian::Query & query, map<string,long int> & tf,char ch);

    double calculate_f2(const Xapian::Query & query, map<string,long int> & tf, map<string,long int> & doc_length, char ch);

    double calculate_f3(const Xapian::Query & query, map<string,double> & idf, char ch);

    double calculate_f4(const Xapian::Query & query, map<string,long int> & tf, map<string,long int> & coll_len, char ch);

    double calculate_f5(const Xapian::Query & query, map<string,long int> & tf, map<string,double> & idf, map<string,long int> & doc_length,char ch);

    double calculate_f6(const Xapian::Query & query, map<string,long int> & tf, map<string,long int> & doc_length,map<string,long int> & coll_tf, map<string,long int> & coll_length, char ch);



};

}
#endif /* FEATURES_H */
