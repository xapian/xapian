/* feature_selector.cc: FeatureSelector is responsible for features selection.
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

#include "feature_selector.h"

#include "feature.h"
#include "feature_vector.h"
#include "rank_list.h"
#include "normalizer.h"

#include <vector>
#include <map>
#include <string>
#include <cmath>

using std::exp;
using std::vector;
using std::string;
using std::map;

double
FeatureSelector::normal_pdf(double x, double mu, double sigma) {
    static const double inv_sqrt_2pi = 0.3989422804014327;
    double n_x = (x - mu) / sigma;
    return inv_sqrt_2pi / sigma * exp(-0.5f * n_x * n_x);
}


double
FeatureSelector::std_normal_pdf(double x) {
	return normal_pdf(x, 0, 1);
}


double
FeatureSelector::kde_pdf(double x, int m, double h, vector<double> x_train) {
	double sum = 0;
	for (vector<double>::iterator it = x_train.begin(); it != x_train; ++it) {
		sum += std_normal_pdf( (x - *it) / h );
	}
	return sum / m / h;
}


double
FeatureSelector::kde_pdf(double x, struct density_function f) {
	return kde_pdf(f.m, f.h, x, f.x_train);
}


double
FeatureSelector::d_kl_for_js(struct density_function P, struct density_function Q, int steps) {
	double s = 0;
	double h = 1 / steps;
	double x = 0;
	double p = 0;
	double m = 0;
	double f_x = 0;
	double f_xh = 0;

	// Integration using trapezium method
	for (int i = 0; i < steps; ++i) {
		x = h * i;
		p = kde_pdf(x, P);
		m = 0.5 * (p + kde_pdf(x, Q));	// Since M = 1/2 * (P + Q)
		f_x = p * log2(p/m);

		x += h;
		p = kde_pdf(x, P);
		m = 0.5 * (p + kde_pdf(x, Q));	// Since M = 1/2 * (P + Q)
		f_xh = p * log2(p/m);

		s += 0.5 * (f_x + f_xh);
	}
	return h*s;
}


double
FeatureSelector::d_kl(struct density_function P, struct density_function Q, int steps) {
	return 0.5 * d_kl_for_js(P, Q, steps) + 0.5 * d_kl_for_js(Q, P);
}


vector<int>
FeatureSelector::select(vector<int> features, int k, int r, vector<FeatureVector> training_data, vector<FeatureVector> validation_data) {
	vector<double> weights;
	map<double, vector<FeatureVector>> relevance_training_data;

	// Separate FeatureVector by relevance level
	for (vector<FeatureVector>::iterator fvector_it = training_data.begin();
			fvector_it != training_data.end(); ++fvector_it) {

		double relevance = fvector_it->get_label();
		if (relevance_training_data.find(relevance) != relevance_training_data.end()) {
			relevance_training_data[relevance].push_back( fvector_it->get_feature_value_of(i) );
		}
		else {
			vector<FeatureVector> fvectors;
			relevance_training_data[relevance] = fvectors;
		}
	}

	// Generate probability density functions
	for (int i = 1; i <= features.size(); ++i) {
		struct density_function pdf;

		vector<double> x_train;
		for (vector<FeatureVector>::iterator fvector_it = training_data.begin();
				fvector_it != training_data.end(); ++fvector_it) {

			x_train.push_back( fvector_it->get_feature_value_of(i) );
		}

		pdf.m = training_data.size();
		// pdf.h = 
		pdf.x_train = x_train;

		pdfs.push_back(pdf);
	}

	// 


}