/* ranker.h: The abstract ranker file.
 *
 * Copyright (C) 2012 Parth Gupta
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
#include <map>
#include <vector>

namespace Xapian {

class XAPIAN_VISIBILITY_DEFAULT Ranker {


    std::vector<Xapian::FeatureVector> traindata;

    int MAXPATHLEN;

  public:

    /// Default constructor
    Ranker();

    /// Constructor to initialise ranker a scoring metric.
    Ranker(int metric_type);

    /// Constructor to initialise ranker with a scoring metric, learning rate & no. of iterations
    Ranker(int metric_type, double learn_rate, int num_iterations);

    /// Returns a vector of RankLists i.e. the training data
    std::vector<Xapian::FeatureVector> get_traindata();

    /// Sets the training data (vector of RankLists)
    void set_training_data(vector<Xapian::FeatureVector> training_data);

    /// Method to get path of current working directory
    std::string get_cwd();

    /// Method to train the model. Overrided in ranker subclass.
    virtual void train_model()=0;

    /// Method to save model as an external file. Overrided in ranker subclass.
    virtual void save_model_to_file()=0;

    /// Method to load model as an external file. Overrided in ranker subclass.
    virtual void load_model_from_file(const std::string & model_file)=0;

    /// Method to re-rank a std::vector<Xapian::FeatureVector> by using letor weighting scheme. Overrided in ranker subclass.
    virtual std::vector<Xapian::FeatureVector> rank(std::vector<Xapian::FeatureVector> & fvv)=0;

    /// Compare function used to sort std::vector<Xapian::FeatureVector> by score values.
    static bool scorecomparer(const FeatureVector & firstfv, const FeatureVector& secondfv);

    /// Compare function used to sort std::vector<Xapian::FeatureVector> by label values.
    static bool labelcomparer(const FeatureVector & firstfv, const FeatureVector& secondfv);

};

}
#endif /* RANKER_H */
