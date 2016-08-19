/** @file ranker.h
 * @brief Ranker class
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

#ifndef RANKER_H
#define RANKER_H


#include <xapian.h>
#include <xapian/intrusive_ptr.h>
#include <xapian/types.h>
#include <xapian/visibility.h>

#include "featurevector.h"

#include <list>
#include <iostream>
#include <map>
#include <vector>

namespace Xapian {

class XAPIAN_VISIBILITY_DEFAULT Ranker : public Xapian::Internal::intrusive_base {


    std::vector<Xapian::FeatureVector> traindata;

  public:

    /// Default constructor
    Ranker();

    /// Virtual destructor since we have virtual methods.
    virtual ~Ranker();

    /// Returns a vector of RankLists i.e. the training data
    std::vector<Xapian::FeatureVector> get_traindata();

    /// Sets the training data (vector of RankLists)
    void set_training_data(std::vector<Xapian::FeatureVector> training_data);

    /// Method to train the model. Overrided in ranker subclass.
    virtual void train_model()=0;

    /** Method to save model as an external file. Overrided in ranker subclass.
     *  @param output_filename      Filename by which model is to be stored.
     */
    virtual void save_model_to_file(const char* output_filename)=0;

    /// Method to load model as an external file. Overrided in ranker subclass.
    virtual void load_model_from_file(const char* model_filename)=0;

    /** Method to re-rank a list of FeatureVectors (each representing a Xapian::Document) by using the model.
     *  Overrided in ranker subclass.
     */
    virtual std::vector<Xapian::FeatureVector> rank(const std::vector<Xapian::FeatureVector> & fvv)=0;

    /// Compare function used to sort std::vector<Xapian::FeatureVector> by score values.
    static bool scorecomparer(const FeatureVector & firstfv, const FeatureVector& secondfv);

    /// Compare function used to sort std::vector<Xapian::FeatureVector> by label values.
    static bool labelcomparer(const FeatureVector & firstfv, const FeatureVector& secondfv);

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

    /// Learning rate (Default is 0.0001)
    double learning_rate;

    /// Number of iterations (Default is 15)
    int iterations;

  public:

    /* Construct ListNet instance
     * @param learn_rate       Learning rate
     * @param num_interations  Number of iterations
     */
    ListNETRanker(double learn_rate = 0.001, int num_interations = 15):
		 learning_rate(learn_rate), iterations(num_interations) {
	std::cout << "Initializing ListNETRanker with Learning Rate: " << learn_rate;
	std::cout << ", No. of Iterations: " << iterations << std::endl;
    }

    /// Destructor
    ~ListNETRanker();

    /// Method to train the model.
    void train_model();

    /** Method to save ListNET model as an external file.
     *  ListNET model file gets stored with each parameter value in a new line. e.g.
     *
     *  0.000920817564536697
     *  0.000920817564536697
     *  0
     *  -1.66533453693773e-19
     *
     *  @param output_filename      Filename by which model is to be stored.
     */
    void save_model_to_file(const char* output_filename);

    /// Method to load model as an external file.
    void load_model_from_file(const char* model_filename);

    /// Method to re-rank a std::vector<Xapian::FeatureVector> by using the model.
    std::vector<Xapian::FeatureVector> rank(const std::vector<Xapian::FeatureVector> & fvv);

};

}

#endif /* RANKER_H */
