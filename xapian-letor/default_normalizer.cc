/* default_normalizer.cc: The default normalizer.
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110=1301
 * USA
 */

#include <xapian.h>
#include <xapian/intrusive_ptr.h>
#include <xapian/types.h>
#include <xapian/visibility.h>

#include "default_normalizer.h"

#include <string>
#include <vector>

#include <cstring>

using std::string;
using std::vector;

namespace Xapian {

DefaultNormalizer::~DefaultNormalizer() {}


RankList
DefaultNormalizer::normalize(RankList rlist_) {
	RankList rlist;

	rlist.set_qid( rlist_.get_qid() );

	vector<FeatureVector> feature_vector_list = rlist_.get_feature_vector_list();

	int feature_num = rlist_.get_feature_num();

	double feature_max_value[feature_num];
	memset(feature_max_value, 0, sizeof(double)*feature_num);

	// look for the max value for each feature
	for (vector<FeatureVector>::iterator fv_list_it = feature_vector_list.begin();
			fv_list_it != feature_vector_list.end(); ++fv_list_it) {
		for (int feature_idx = 1; feature_idx <= feature_num; ++feature_idx) {
			if (fv_list_it->get_feature_value_of(feature_idx) > feature_max_value[feature_idx])
				feature_max_value[feature_idx] = fv_list_it->get_feature_value_of(feature_idx);
		}
	}

	// perform the normalization
	for (vector<FeatureVector>::iterator fv_list_it = feature_vector_list.begin();
			fv_list_it != feature_vector_list.end(); ++fv_list_it) {
		vector<double> f_values;
		for (int feature_idx = 0; feature_idx < feature_num; ++feature_idx) {
			if (feature_max_value[feature_idx] != 0)
				f_values.push_back( fv_list_it->get_feature_value_of(feature_idx) / feature_max_value[feature_idx] );
			else
				f_values.push_back(0);
		}
		fv_list_it->set_feature_values(f_values);
	}

	rlist.set_feature_vector_list(feature_vector_list);

	return rlist;
}

}