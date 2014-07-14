#include <xapian.h>
#include <xapian/intrusive_ptr.h>
#include <xapian/types.h>
#include <xapian/visibility.h>

#include <featurevector.h>

#include <list>
#include <map>
#include <vector>
#include <algorithm>

using namespace Xapian;

using std::string;
using std::vector;


RankList::RankList() : feature_num(-1) {}


RankList::~RankList() {}


void
RankList::set_qid(string qid_) {
	qid = qid_;
}


void
RankList::set_feature_vector_list(vector<FeatureVector> feature_vector_list_) {
	vector<FeatureVector>::iterator fv_list_it = feature_vector_list_.begin();
	feature_num = *(fv_list_it).size();
	for (; fv_list_it != feature_vector_list_.end(); ++fv_list_it) {
		if (feature_num != *(fv_list_it).size()) {
			printf("Feature num is not compatible.\n");
			return 1;
		}
	}

	feature_vector_list = feature_vector_list_;
}


void
RankList::add_feature_vector(FeatureVector fvector_) {
	if (feature_num == -1) {
		feature_num = fvector_.size();
	}
	else
		if (feature_num != fvector_.size()) {
			printf("Feature num is not compatible.\n");
			return 1;
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
	string txt = (*fv_list_it).get_label_feature_values_text();
	++fv_list_it;
	
	for (; fv_list_it != feature_vector_list.end(); ++fv_list_it) {
		txt.append("\n" + (*fv_list_it).get_label_feature_values_text());
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
		Xapian::MSet::letor_item l_item = fvectors_it->create_letor_item(rank);
		letor_items.push_back(l_item);
	}
	return letor_items;
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