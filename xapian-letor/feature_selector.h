/* feature_selector.h: FeatureSelector is responsible for features selection.
 *
 * Copyright (C) 2014 Jiarong Wei
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

#ifndef FEATURE_SELECTOR_H
#define FEATURE_SELECTOR_H


#include <xapian.h>
#include <xapian/intrusive_ptr.h>
#include <xapian/types.h>
#include <xapian/visibility.h>

#include "feature.h"
#include "feature_vector.h"
#include "ranklist.h"
#include "normalizer.h"

#include <vector>
#include <string>

using std::vector;

namespace Xapian {

class XAPIAN_VISIBILITY_DEFAULT FeatureSelector {

public:

    struct density_function {
        int m;
        double h;
        vector<double> x_train;
    };

    struct sort_struct {
        double val;
        Feature::feature_t idx;
    };
    typedef struct sort_struct sort_ele;

    struct comp {
        bool operator() (sort_ele i, sort_ele j) {
            return i.val > j.val;
        }
    } comp_obj;

    FeatureSelector() {}

    ~FeatureSelector() {}

    // Probablity density function of normaal distribution 
    double normal_pdf(double x, double m, double s);

    // Probablity density function of standard normaal distribution 
    double std_normal_pdf(double x);

    // Standard deviation
    double std(vector<double> & data);

    // The estimated probability density function using kernel density estimation
    double kde_pdf(double x, int m, double h, vector<double> & x_train);

    // The version of kde_pdf which takes struct as input argument
    double kde_pdf(double x, struct density_function f);

    // KL divergence
    double d_kl(double p, double q);

    // KL divergence for Jensen-Shannon divergence
    double d_kl_for_js(struct density_function P, struct density_function Q, int steps);

    // Jensen-Shannon divergence
    double d_js(struct density_function P, struct density_function Q, int steps);

    // Jensen-Shannon divergence
    double d_js(double p, double q);

    // Compute weights for features
    vector<double> compute(vector<Feature::feature_t> features, vector<FeatureVector> training_data, vector<FeatureVector> validation_data);

    // Fetch top k features
    vector<Feature::feature_t> fetch(vector<double> weights, int k);

    // Select top k features
    vector<Feature::feature_t> select(vector<Feature::feature_t> features, int k, vector<FeatureVector> training_data, vector<FeatureVector> validation_data);
};

}

#endif // FEATURE_SELECTOR_H
