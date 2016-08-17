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

#include "featurelist.h"

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

    /** Core ranking function. Re-ranks the initial mset using trained model.
     *
     *  @param mset     Xapian::MSet that is to be re-ranked
     *  @param flist    Xapian::FeatureList object definining what set of features to use for ranking.
     *                  It is initialised by DEFAULT set of Features by default.
     *                  Note: Make sure that this FeatureList object is the same as what was used during
     *                  preparation of the training file. //TODO: Replace this by a "feature.config" file prepared while training.
     *  @param  output_filename  Path to model file. Default is "./parameters.txt".
     *  @return A vector of docids after ranking.
     */
    std::vector<Xapian::docid> letor_rank(const Xapian::MSet & mset,
					 Xapian::FeatureList & flist = * new Xapian::FeatureList(),
					 const char* model_filename = "./parameters.txt");

    /** Learns the model using the training file.
     *  Model file is saved as an external file in the working directory.
     *  @param  input_filename   Path to training file. Default is "./train.txt".
     *  @param  output_filename  Path to file where model parameters will be stored. Default is "./parameters.txt".
     */
    void letor_learn_model(const char* input_filename = "./train.txt",
			   const char* output_filename = "./parameters.txt");

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
     *  @param  filename   Filename path where the training file has to be stored. Default is "./train.txt".
     *  @param  flist      Xapian::FeatureList object definining what set of features to use for preparing the training file.
     *          It is initialised by DEFAULT set of Features by default.
     *          To use a custom set of features, pass a customised Xapian::FeatureList object.
     */
    void prepare_training_file(const std::string & query_file,
			       const std::string & qrel_file,
			       Xapian::doccount msetsize,
			       const char* filename = "./train.txt",
			       Xapian::FeatureList & flist = * new Xapian::FeatureList());

    void create_ranker(int ranker_type, int metric_type); // TODO: Remove function and update as command line utility. Same for scorers as well.

};

}

#endif /* XAPIAN_INCLUDED_LETOR_H */
