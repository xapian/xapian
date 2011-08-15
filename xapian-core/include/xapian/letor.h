/** @file letor.h
 *  @brief weighting scheme based on Learning to Rank
 */
/* Copyright (C) 2011 Parth Gupta
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#ifndef XAPIAN_INCLUDED_LETOR_H
#define XAPIAN_INCLUDED_LETOR_H

#include <xapian.h>
#include <xapian/base.h>
#include <xapian/types.h>
#include <xapian/unicode.h>
#include <xapian/visibility.h>

#include <string>
#include <map>

namespace Xapian {

class XAPIAN_VISIBILITY_DEFAULT Letor {
  public:
    /// @private @internal Class representing the Letor internals.
    class Internal;
    /// @private @internal Reference counted internals.
    Xapian::Internal::RefCntPtr<Internal> internal;

    /// Copy constructor.
    Letor(const Letor & o);

    /// Assignment.
    Letor & operator=(const Letor & o);

    /// Default constructor.
    Letor();

    /// Destructor.
    ~Letor();

    /// Specify the database to use for retrieval. This database will be used directly by the methods of Xapian::Letor::Internal
    void set_database(const Xapian::Database & db);

    /// Specify the query. This will be used by the internal class.
    void set_query(const Xapian::Query & query);

    /** This method finds the frequency of the query terms in the specified documents. This method is a helping method and statistics gathered through 
     *  this method are used in feature value calculation. It return the frequency of the terms of query in std::map<string,long int> form.
     *
     *  @param  doc     This is the document in which we want the frequency of the terms in query. Generally in Letor, this document is taken 
     *          from the Xapain::MSet retieved from the first retrieval.
     *  @param  query   This is the query for which terms we want frequency in the document.
     */
    std::map<std::string,long int> termfreq( const Xapian::Document & doc , const Xapian::Query & query);

    /** This method calculated the inverse document frequency(idf) of query terms in the database. It returns the idf of each term in 
     *  std::map<string,double> form.
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
    std::map<std::string,double> inverse_doc_freq(const Xapian::Database & db, const Xapian::Query & query);

    /** This method calculated the length of the documents as number of 'terms'. It calculated the length for three different
     *  parts: title, body and whole document. This information is returned in the std::map<string,long int> format. 
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
    std::map<std::string,long int> doc_length(const Xapian::Database & db, const Xapian::Document & doc);

    /** This method calculates the length of the collenction in number of terms for different parts like 'title', 'body' and 'whole'. This is calculated 
     *  as a stored user metadata in omindex otherwise it is calculated out of scratch(this might take some time depending upon the size of the 
     *  database. Lenght information is stored in std::map<string,long int> format and can be accessed as below:
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
    std::map<std::string,long int> collection_length(const Xapian::Database & db);

    /** This method calculates the frequecny of query terms in the whole database. The information is stored in std::map<string, long int> format and 
     *  used during the feature calculation methods.
     *
     *  @param  db      Database to be used
     *  @param  query   Query being searched.
     */
    std::map<std::string,long int> collection_termfreq(const Xapian::Database & db, const Xapian::Query & query);

    double calculate_f1(const Xapian::Query&, std::map<std::string,long int> &,char);

    double calculate_f2(const Xapian::Query & query, std::map<std::string,long int> & tf, std::map<std::string,long int> & doc_length, char ch);

    double calculate_f3(const Xapian::Query & query, std::map<std::string,double> & idf, char ch);

    double calculate_f4(const Xapian::Query & query, std::map<std::string,long int> & tf, std::map<std::string,long int> & coll_len, char ch);

    double calculate_f5(const Xapian::Query & query, std::map<std::string,long int> & tf, std::map<std::string,double> & idf, std::map<std::string,long int> & doc_length,char ch);

    double calculate_f6(const Xapian::Query & query, std::map<std::string,long int> & tf, std::map<std::string,long int> & doc_length,std::map<std::string,long int> & coll_tf, std::map<std::string,long int> & coll_length, char ch);

    std::map<Xapian::docid,double> letor_score(const Xapian::MSet & mset);

    void letor_learn_model();

    void prepare_training_file(std::string query_file, std::string qrel_file);
};

}
#endif	/* XAPIAN_INCLUDED_LETOR_H */
