/** @file
 * @brief Ranker class - weighting scheme based on Learning to Rank
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

#ifndef XAPIAN_INCLUDED_RANKER_H
#define XAPIAN_INCLUDED_RANKER_H

#include <xapian.h>
#include <xapian/intrusive_ptr.h>
#include <xapian/types.h>
#include <xapian/visibility.h>

#include <xapian-letor/featurelist.h>
#include <xapian-letor/featurevector.h>
#include <xapian-letor/letor_error.h>

#include <string>
#include <vector>

namespace Xapian {

/** Utility function that prepares the training data.
 *
 *  The training data file is used to train a model which in turn will be used
 *  to assign scores to the documents based of Learning-to-Rank model.
 *
 *  The file is created in the standard format of Letor training file
 *  as below:
 *
 *  0 qid:102 1:0.130742 2:0.000000 3:0.333333 4:0.000000 ... 18:0.750000
 *            19:1.000000 #docid = 13566007
 *  1 qid:102 1:0.593640 2:1.000000 3:0.000000 4:0.000000 ... 18:0.500000
 *            19:0.000000 #docid = 0740276
 *
 *  where first column is relevance judgement of the document with docid as
 *  shown in the last column. The second column is query id and in between
 *  there are 19 feature values.
 *
 *  @param  db_path	Path to Xapian::Database to be used.
 *  @param  query_file	Here you have to give a path to the file
 *			(in free text form) containing training queries
 *			in specified format.
 *  @param  qrel_file	Here supply the path to the qrel file
 *			(in free text form) containing the relevance
 *			judgements for the queries in the training
 *			file. This file should be in standard format
 *			specified.
 *  @param  msetsize	This is the mset size used for the first
 *			retrieval for training queries.
 *			It should be selected depending on the qrel
 *			file and database size.
 *  @param  filename	Filename path where the training file has to
 *			be stored.
 *  @param  flist	Xapian::FeatureList object defining what set
 *			of features to use for preparing the training
 *			file. It is initialised by DEFAULT set of
 *			Features by default. To use a custom set of
 *			features, pass a customised Xapian::FeatureList
 *			object.
 *
 *  @exception FileNotFoundError will be thrown if file not found at
 *	       supplied path
 *  @exception LetorParseError will be thrown if query file or qrel file
 *	       could not be parsed
 */
XAPIAN_VISIBILITY_DEFAULT
void
prepare_training_file(const std::string & db_path,
		      const std::string & query_file,
		      const std::string & qrel_file,
		      Xapian::doccount msetsize,
		      const std::string & filename,
		      const Xapian::FeatureList & flist = FeatureList());

class XAPIAN_VISIBILITY_DEFAULT Ranker : public Xapian::Internal::intrusive_base {
    /// Path to Xapian::Database instance to be used.
    std::string db_path;
    /// Xapian::Query to be ranked using Ranking model.
    Xapian::Query letor_query;

  public:
    /// Default constructor
    Ranker();

    /// Virtual destructor since we have virtual methods.
    virtual ~Ranker();

    /** Specify path to Xapian::Database to be used for Ranker methods.
     *
     *  @param  dbpath      Path to Xapian::Database to be used.
     */
    void set_database_path(const std::string & dbpath);

    /** Get path to Xapian::Database that has been set for ranking.
     *
     *  @return	  Path to Xapian::Database that has been set for ranking.
     */
    std::string get_database_path();

    /** Specify Xapian::Query that is to be used for ranking.
     *
     *  @param  query      Xapian::Query to be ranked using ranking model.
     */
    void set_query(const Xapian::Query & query);

    /** Learns the model using the training file.
     *
     *  Model file is saved as DB metadata.
     *
     *  @param  input_filename   Path to training file.
     *
     *  @param  model_key	 Metadata key using which the model is to be
     *				 loaded. If no model_key is supplied, ranker
     *				 subclass uses its default key
     *				 e.g. ListNET_default_key.
     *
     *  @exception FileNotFoundError will be thrown if file not found at
     *		   supplied path
     */
    void train_model(const std::string & input_filename,
		     const std::string & model_key = std::string());

    /** Ranking function.
     *
     *  Re-ranks the initial mset using trained model.
     *
     *  @param  mset       Xapian::MSet corresponding to Xapian::Query that
     *			   is to be re-ranked.
     *  @param  model_key  DB metadata key from which the ranking model is to
     *			   be loaded. If no model_key is provided,
     *			   ranker subclass uses its default model_key
     *			   e.g. ListNET_default_key.
     *  @param flist       Xapian::FeatureList object defining what set of
     *			   features to use for ranking. It is initialised by
     *			   DEFAULT set of Features by default.
     *                     Note: Make sure that this FeatureList object is the
     *			   same as what was used during preparation of the
     *			   training file.
     */
    void rank(Xapian::MSet & mset,
	      const std::string & model_key = std::string(),
	      const Xapian::FeatureList & flist = Xapian::FeatureList());

    /** Method to score the ranking.
     *
     *  @param query_file    Query file containing test queries in letor
     *			     specified format.
     *  @param qrel_file     Qrel file containing relevance judgements for the
     *			     queries in letor specified format.
     *  @param model_key     Model to check for ranking quality.
     *  @param output_file   Output file noting scoring results.
     *  @param msetsize      MSet size of retrieved documents.
     *  @param scorer_type   Scorer algorithm to use. By default, NDCGScore is
     *			     used.
     *                       Available algorithms:
     *                       1. NDCGScore
     *  @param flist         Xapian::FeatureList object defining what set of
     *			     features to use.
     *			     Note: Make sure that it is same as what was used
     *			     while training the model being used.
     *
     *  @exception FileNotFoundError will be thrown if file not found at
     *		   supplied path
     *  @exception LetorParseError will be thrown if query file or qrel file
     *		   could not be parsed
     */
    void score(const std::string & query_file,
	       const std::string & qrel_file,
	       const std::string & model_key,
	       const std::string & output_file,
	       Xapian::doccount msetsize,
	       const std::string & scorer_type = "NDCGScore",
	       const Xapian::FeatureList & flist = Xapian::FeatureList());

  protected:
    /// Method to train the model. Overridden in ranker subclass.
    virtual void
    train(const std::vector<Xapian::FeatureVector> & training_data) = 0;

    /** Method to save model as db metadata. Overridden in ranker subclass.
     *
     *  Note: Make sure that there is no active writer on the database.
     *        Since this method writes to database, it may cause database
     *	      exceptions.
     *
     *  @param model_key     Key by which model is to be stored.
     *			     If empty, default key is used by the respective
     *			     subclass.
     */
    virtual void save_model_to_metadata(const std::string & model_key) = 0;

    /** Method to load model from db metadata. Overridden in ranker subclass.
     *
     *  @param model_key     Key by which model is to be loaded.
     *			     If empty, default key is used by the respective
     *			     subclass.
     */
    virtual void load_model_from_metadata(const std::string & model_key) = 0;

    /** Method to re-rank a list of FeatureVectors
     *	(each representing a Xapian::Document) by using the model.
     *
     *  Overridden in ranker subclass.
     */
    virtual std::vector<Xapian::FeatureVector>
    rank_fvv(const std::vector<Xapian::FeatureVector> & fvv) const = 0;

    /** Compare function used to sort std::vector<Xapian::FeatureVector>
     *  by score values.
     */
    static bool scorecomparer(const FeatureVector & firstfv,
			      const FeatureVector & secondfv);

    /** Compare function used to sort std::vector<Xapian::FeatureVector>
     *  by label values.
     */
    static bool labelcomparer(const FeatureVector & firstfv,
			      const FeatureVector & secondfv);

  private:
    /// Don't allow assignment.
    void operator=(const Ranker &);

    /// Don't allow copying.
    Ranker(const Ranker & o);
};

/// ListNet Ranker class
class XAPIAN_VISIBILITY_DEFAULT ListNETRanker: public Ranker {
    /// Ranker parameters
    std::vector<double> parameters;
    /// Learning rate (Default is 0.001)
    double learning_rate;
    /// Number of iterations (Default is 15)
    int iterations;

    /** Method to train the model.
     *
     * @exception LetorInternalError will be thrown if training data is null.
     */
    void train(const std::vector<Xapian::FeatureVector> & training_data);

    /** Method to save ListNET model as db metadata.
     *
     *  ListNET model file gets stored with each parameter value in a new line.
     *	 e.g.
     *
     *  0.000920817564536697
     *  0.000920817564536697
     *  0
     *  -1.66533453693773e-19
     *
     *  @param model_key	Metadata key using which model is to be stored.
     */
    void save_model_to_metadata(const std::string & model_key);

    /** Method to load model from an external file.
     *
     *  @param model_key         Metadata key using which model is to be
     *				 loaded.
     *
     *  @exception LetorInternalError will be thrown if no model exists
     *		   corresponding to the supplied key
     */
    void load_model_from_metadata(const std::string & model_key);

    /** Method to re-rank a std::vector<Xapian::FeatureVector> by using the
     *  model.
     *
     *  @param fvv	vector<FeatureVector> that will be re-ranked.
     *
     *  @exception	LetorInternalError will be thrown if model file
     *			is not compatible.
     */
    std::vector<Xapian::FeatureVector>
    rank_fvv(const std::vector<Xapian::FeatureVector> & fvv) const;

  public:
    /* Construct ListNet instance
     * @param learn_rate       Learning rate
     * @param num_iterations   Number of iterations
     */
    explicit ListNETRanker(double learn_rate = 0.001,
			   int num_iterations = 15)
	: learning_rate(learn_rate), iterations(num_iterations) { }

    /// Destructor
    ~ListNETRanker();
};

/// SVMRanker class
class XAPIAN_VISIBILITY_DEFAULT SVMRanker: public Ranker {
    /// Model data string
    std::string model_data;

    /** Method to train the model.
     *
     *  @exception LetorInternalError will be thrown if training data is null.
     */
    void train(const std::vector<Xapian::FeatureVector> & training_data);

    /** Method to save SVMRanker model as db metadata.
     *
     *  @param model_key      Metadata key using which model is to be stored.
     */
    void save_model_to_metadata(const std::string & model_key);

    /** Method to load model from an external file.
     *
     *  @param model_key        Metadata key using which model is to be
     *				loaded.
     *
     *  @exception LetorInternalError will be thrown if no model exists
     *		   corresponding to the supplied key
     */
    void load_model_from_metadata(const std::string & model_key);

    /** Method to re-rank a std::vector<Xapian::FeatureVector> by using the
     *  model.
     *
     *  @param fvv vector<FeatureVector> that will be re-ranked
     */
    std::vector<Xapian::FeatureVector>
    rank_fvv(const std::vector<Xapian::FeatureVector> & fvv) const;

  public:
    /* TODO: Pass struct svm_parameter* to constructor to be able to configure
     * libsvm params at run time.
     */
    /// Constructor
    SVMRanker();

    /// Destructor
    ~SVMRanker();
};

class XAPIAN_VISIBILITY_DEFAULT ListMLERanker : public Ranker {
    /// Ranker parameters
    std::vector<double> parameters;
    /// Learning rate (Default is 0.001)
    double learning_rate;
    /// Number of iterations (Default is 10)
    int iterations;

    /** Train the model.
     *
     * @exception InvalidArgumentError will be thrown if training_data is
     *            empty.
     */
    void train(const std::vector<Xapian::FeatureVector>& training_data);

    /** Save ListMLE model as db metadata.
     *
     *  The model is serialised as a binary blob.
     *
     *  @param model_key	Metadata key using which model is to be stored.
     */
    void save_model_to_metadata(const std::string& model_key);

    /** Load model from db metadata.
     *
     *  @param model_key         Metadata key using which model is to be
     *				 loaded.
     *
     *  @exception InvalidArgumentError will be thrown if no model exists
     *		   corresponding to the supplied key
     */
    void load_model_from_metadata(const std::string& model_key);

    /** Re-rank a std::vector<Xapian::FeatureVector> by using the
     *  model.
     *
     *  @param fvv	vector<FeatureVector> that will be re-ranked.
     *
     *  @exception	InvalidArgumentError will be thrown if model file
     *			is not compatible.
     */
    std::vector<Xapian::FeatureVector>
    rank_fvv(const std::vector<Xapian::FeatureVector>& fvv) const;

  public:
    /* Construct ListMLE instance
     * @param learn_rate       Learning rate (Default is 0.001)
     * @param num_iterations   Number of iterations (Default is 10)
     */
    explicit ListMLERanker(double learn_rate = 0.001,
			   int num_iterations = 10)
	: learning_rate(learn_rate), iterations(num_iterations) { }

    /// Destructor
    ~ListMLERanker();
};

}

#endif /* XAPIAN_INCLUDED_RANKER_H */
