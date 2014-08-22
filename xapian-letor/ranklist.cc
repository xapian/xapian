/* ranklist.cc: RankList which stors list of feature vectors.
 *
 * Copyright (C) 2012 Parth Gupta
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

#include <xapian.h>
#include <xapian/intrusive_ptr.h>
#include <xapian/types.h>
#include <xapian/visibility.h>

#include "ranklist.h"

#include <iostream>
#include <string>
#include <vector>

#include <cstdlib>

using std::string;
using std::vector;
using std::cerr;

namespace Xapian {

RankList::RankList() : feature_num(-1) {}


RankList::~RankList() {}


void
RankList::set_qid(string qid_) {
	qid = qid_;
}


void
RankList::set_feature_vector_list(vector<FeatureVector> & feature_vector_list_) {
	vector<FeatureVector>::iterator fv_list_it = feature_vector_list_.begin();
	feature_num = fv_list_it->get_feature_num();
	for (; fv_list_it != feature_vector_list_.end(); ++fv_list_it) {
		if (feature_num != fv_list_it->get_feature_num()) {
			cerr << "Feature num is not compatible." << '\n';
			exit(1);
		}
	}

	feature_vector_list = feature_vector_list_;
}


void
RankList::add_feature_vector(FeatureVector fvector_) {
	if (feature_num == -1) {
		feature_num = fvector_.get_feature_num();
	}
	else
		if (feature_num != fvector_.get_feature_num()) {
			cerr << "Feature num is not compatible." << '\n';
			exit(1);
		}

	feature_vector_list.push_back(fvector_);
}


string
RankList::get_qid() {
	return qid;
}


int
RankList::get_num() {
	return feature_vector_list.size();
}


int
RankList::get_feature_num() {
	return feature_num < 0 ? 0 : feature_num;
}


vector<FeatureVector> &
RankList::get_feature_vector_list() {
	return feature_vector_list;
}


string
RankList::get_label_feature_values_text() {
	vector<FeatureVector>::iterator fv_list_it = feature_vector_list.begin();
	string txt = fv_list_it->get_label_feature_values_did_text(qid);
	++fv_list_it;
	
	for (; fv_list_it != feature_vector_list.end(); ++fv_list_it) {
		txt.append("\n" + fv_list_it->get_label_feature_values_did_text(qid));
	}
	return txt;
}


vector<Xapian::MSet::letor_item>
RankList::create_letor_items() {
	vector<Xapian::MSet::letor_item> letor_items;
	vector<FeatureVector>::iterator fvectors_it = feature_vector_list.begin();
	Xapian::doccount rank = 0;
	for (; fvectors_it != feature_vector_list.end(); ++fvectors_it) {
		++rank;
		fvectors_it->set_index(rank);
		Xapian::MSet::letor_item l_item = fvectors_it->create_letor_item();
		letor_items.push_back(l_item);
	}
	return letor_items;
}

}

/*
void
RankList::normalise() {

	vector<FeatureVector>::iterator fv_list_it = feature_vector_list.begin();
	int feature_num = (*fv_list_it).size();

	for (; fv_list_it != feature_vector_list.end(); ++fv_list_it) {
		if ((*fv_list_it).size() != feature_num) {
			cout << "The number of features used for RankList don't matched.\n";
			exit(1);
		}
	}

	double feature_max_value[feature_num];
	memset(feature_max_value, 0, sizeof(double)*feature_num);

	// look for the max value for each feature
	for (fv_list_it = feature_vector_list.begin();
			fv_list_it != feature_vector_list.end(); ++fv_list_it) {
		for (int feature_idx = 0; feature_idx < feature_num; ++feature_idx) {
			if ((*fv_list_it)[feature_idx] > feature_max_value[feature_idx])
				feature_max_value[feature_idx] = (*fv_list_it)[feature_idx];
		}
	}

	for (fv_list_it = feature_vector_list.begin();
			fv_list_it != feature_vector_list.end(); ++fv_list_it) {
		for (int feature_idx = 0; feature_idx < feature_num; ++feature_idx) {
			if (feature_max_value[feature_idx] != 0)
				(*fv_list_it)[feature_idx] /= feature_max_value[feature_idx];
		}
	}

	normalized = true;
}
*/