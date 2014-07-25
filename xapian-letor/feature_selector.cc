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
FeatureSelector::std(const vector<double> & data) {
	double sum = 0;
	for (vector<double>::iterator it = data.begin(); it != data.end(); ++it)
		sum += *it;

	double avg = sum / data.size();
	double ans = 0;
	for (vector<double>::iterator it = data.begin(); it != data.end(); ++it)
		ans += (*it - avg) * (*it - avg);

	return sqrt(data.size() / ans);
}


double
FeatureSelector::kde_pdf(double x, int m, double h, const vector<double> & x_train) {
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
FeatureSelector::d_kl(double p, double q) {
	return p * log2(p/q);
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
FeatureSelector::d_js(struct density_function P, struct density_function Q, int steps) {
	return 0.5 * d_kl_for_js(P, Q, steps) + 0.5 * d_kl_for_js(Q, P);
}


double
FeatureSelector::d_js(double p, double q) {
	double m = 0.5*(p+q);
	return 0.5*d_kl(p, m) + 0.5*d_kl(q, m);
}


vector<double>
FeatureSelector::compute(vector<int> features, vector<FeatureVector> training_data, vector<FeatureVector> validation_data) {
	vector<double> weights;
	map<double, vector<FeatureVector>> relevance_training_data, relevance_validation_data;	// map from relevance level to feature vectors

	// Separate FeatureVector by relevance level for training data
	for (vector<FeatureVector>::iterator fvector_it = training_data.begin();
			fvector_it != training_data.end(); ++fvector_it) {

		double relevance = fvector_it->get_label();
		if (relevance_training_data.find(relevance) != relevance_training_data.end()) {
			relevance_training_data[relevance].push_back( fvector_it->get_feature_value_of(i) );
		}
		else {
			vector<FeatureVector> fvectors;
			fvectors.push_back( fvector_it->get_feature_value_of(i) );
			relevance_training_data[relevance] = fvectors;
		}
	}

	// Separate FeatureVector by relevance level for validation data
	for (vector<FeatureVector>::iterator fvector_it = validation_data.begin();
			fvector_it != validation_data.end(); ++fvector_it) {

		double relevance = fvector_it->get_label();
		if (relevance_validation_data.find(relevance) != relevance_validation_data.end()) {
			relevance_validation_data[relevance].push_back( fvector_it->get_feature_value_of(i) );
		}
		else {
			vector<FeatureVector> fvectors;
			fvectors.push_back( fvector_it->get_feature_value_of(i) );
			relevance_validation_data[relevance] = fvectors;
		}
	}

	// Calculate score for each feature
	for (int i = 1; i <= features.size(); ++i) {
		vector<struct density_function> pdfs;
		vector<double> rels;

		// Generate probability density functions for each relevance level
		for (relevance_training_data::iterator it = relevance_training_data.begin();
				it != relevance_training_data.end(); ++it) {

			vector<double> x_train = FeatureVector::extract(training_data, it->first, i);

			struct density_function pdf;
			pdf.m = training_data.size();
			pdf.h = 1.06 * std(x_train) * pow(pdf.m, -0.2); 
			pdf.x_train = x_train;

			pdfs.push_back(pdf);
			rels.push_back(it->first);
		}

		// Calculate weight of feature
		double temp_w = 0;

		// Calculate feature importance
		// TO DO

		// Calculate discrimination
		for (vector<FeatureVector>::iterator it = validation_data.begin();
				it != validation_data.end(); ++it) {

			double x = it->get_feature_value_of(i);		// Get the value of ith feature

			vector<double> pdf_vals;
			for (int i = 0; i<rels.size(); ++i) {
				pdf_vals.push_back( kde_pdf(x, pdfs[i]) );
			}

			for (int i = 0; i<rels.size(); ++i)
				for (int j = i+1; j<rels.size(); ++j) {
					temp_w += (rels[j] - rels[i]) * d_kl(pdf_vals[j], pdf_vals[i]);
				}
		}

		weights.push_back(temp_w);
	}

	return weights;
}


vector<int>
FeatureSelector::fetch(vector<double> weights, int k) {
	struct data {
		double val;
		int idx;
	};

	struct comp {
		bool operator() (struct data i, struct data j) {
			return i.val > j.val;
		}
	} comp_obj;

	vector<struct data> sorted_data;
	for (vector<double>::iterator it = weights.begin(), int i = 0;
			it != weights.end(); ++it, ++i) {
		struct data ele;
		ele.val = *it;
		ele.idx = i;
		sorted_data.push_back(ele);
	}
	sort(sorted_data.begin(), sorted_data.end(), comp_obj);

	vector<int> f_idx;
	for (vector<struct data>::iterator it = sorted_data.begin();
			it != sorted_data.end(); ++it) {
		f_idx.push_back(it->idx);
	}

	return f_idx;
}


vector<int>
FeatureSelector::select(vector<int> features, int k, vector<FeatureVector> training_data, vector<FeatureVector> validation_data) {
	vector<double> weights = compute(features, training_data, validation_data);
	return fetch(weights, k);
}