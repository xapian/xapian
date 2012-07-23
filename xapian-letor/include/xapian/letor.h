/** @file letor.h
 *  @brief weighting scheme based on Learning to Rank. Note: letor.h is not a part of official stable Xapian API.
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
#include <xapian/intrusive_ptr.h>
#include <xapian/types.h>
#include <xapian/visibility.h>

#include <string>
#include <map>

namespace Xapian {

class XAPIAN_VISIBILITY_DEFAULT Letor {
  public:
    /// @private @internal Class representing the Letor internals.
    class Internal;
    /// @private @internal Reference counted internals.
    Xapian::Internal::intrusive_ptr<Internal> internal;

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
     *  this method are used in feature value calculation. It return the frequency of the terms of query in std::map<string, long int> form.
     *
     *  @param  doc     This is the document in which we want the frequency of the terms in query. Generally in Letor, this document is taken
     *          from the Xapian::MSet retrieved from the first retrieval.
     *  @param  query   This is the query for which terms we want frequency in the document.
     */
//    std::map<std::string, long int> termfreq(const Xapian::Document & doc, const Xapian::Query & query);

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
//    std::map<std::string, double> inverse_doc_freq(const Xapian::Database & db, const Xapian::Query & query);

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
//    std::map<std::string, long int> doc_length(const Xapian::Database & db, const Xapian::Document & doc);

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
//    std::map<std::string, long int> collection_length(const Xapian::Database & db);

    /** This method calculates the frequency of query terms in the whole database. The information is stored in std::map<string, long int> format and
     *  used during the feature calculation methods.
     *
     *  @param  db      Database to be used
     *  @param  query   Query being searched.
     */
//    std::map<std::string, long int> collection_termfreq(const Xapian::Database & db, const Xapian::Query & query);

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
//    double calculate_f1(const Xapian::Query & query, std::map<std::string, long int> & tf, char ch);

//    double calculate_f2(const Xapian::Query & query, std::map<std::string, long int> & tf, std::map<std::string, long int> & doc_length, char ch);

//    double calculate_f3(const Xapian::Query & query, std::map<std::string, double> & idf, char ch);

//    double calculate_f4(const Xapian::Query & query, std::map<std::string, long int> & tf, std::map<std::string, long int> & coll_len, char ch);

//    double calculate_f5(const Xapian::Query & query, std::map<std::string, long int> & tf, std::map<std::string, double> & idf, std::map<std::string, long int> & doc_length, char ch);

//    double calculate_f6(const Xapian::Query & query, std::map<std::string, long int> & tf, std::map<std::string, long int> & doc_length, std::map<std::string, long int> & coll_tf, std::map<std::string, long int> & coll_length, char ch);

    /** Gives the scores to each item of initial mset using the trained model. Note: It assigns a score to each document only so user needs to sort that map
     *  as descending order of the value of map.
     *
     *  @return Letor score corresponding to each document in mset as map<docid, score> format.
     */
    std::map<Xapian::docid, double> letor_score(const Xapian::MSet & mset);

    /** In this method the model is learnt and stored in 'model.txt' file using training file 'train.txt'. It is required that libsvm is
     *  installed in the system. The SVM model is learnt using libsvm.
     *
     *  @param  s       svm_type (default s=4). In libsvm-3.1,
     *          1 -- C-SVC
     *          1 -- nu-SVC
     *          2 -- one-class SVM
     *          3 -- epsilon-SVR
     *          4 -- nu-SVR
     *  @param  k       kernel_type (default k=0). In libsvm-3.1,
     *          0 -- linear
     *          1 -- polynomial
     *          2 -- radial basis function
     *          3 -- sigmoid
     *          4 -- precomputed kernel
     *
     */
    void letor_learn_model();

    /** This method prepares the 'train.txt' file in the current working directory. This file is used to train a model which in turn will be used to
     *  assign scores to the documents based of Learning-to-Rank model. File 'train.txt' is created in the standard format of Letor training file
     *  as below:
     *
     *  0 qid:102 1:0.130742 2:0.000000 3:0.333333 4:0.000000 ... 18:0.750000 19:1.000000 #docid = 13566007
     *  1 qid:102 1:0.593640 2:1.000000 3:0.000000 4:0.000000 ... 18:0.500000 19:0.000000 #docid = 0740276
     *
     *  where first column is relevance judgement of the document with docid as shown in the last column. The second column is query id and in between
     *  there are 19 feature values.
     *
     *  @param  query_file      Here you have to give a path to the file (in free text form)  containing training queries in specified format.
     *  @param  qrel_file       Here supply the path to the qrel file (in free text form) containing the relevance judgements for the
     *          queries in the training file. This file should be in standard format specified.
     *  @param  msetsize   This is the mset size used for the first retrieval for training queries. It should be selected depending on the qrel file
     *          and database size.
     */
    void prepare_training_file(const std::string & query_file, const std::string & qrel_file, Xapian::doccount msetsize);
    
    void prepare_training_file_listwise(const std::string & query_file, int num_features);
    
    void create_ranker(int ranker_type);
};

}

#endif /* XAPIAN_INCLUDED_LETOR_H */
