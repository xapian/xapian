
#include "feature.h"

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>

using namespace Xapian;

Feature::Feature(const Feature & feature_manager_,
vector<Feature::FeatureBase> features_) {
    feature_manager = feature_manager_;
    features = features_;
}

double
Feature::feature_1(Xapian::Document doc_) {
    double value = 0;
    vector<long int> & query_term_frequency_database = 
        feature_manager.get_query_term_frequency_database();
    for (Xapian::TermIterator term_it = query.get_terms_begin(), vector<long int>::iterator it = query_term_frequency_database.begin();
            it != query_term_frequency_database.end(); ++term_it, ++it) {
        if ((*term_it)[0] == "S" || (*term_it)[1] == "S")
            value += log10(1 + *it);
        else
            value += 0;
    }
    return value;
}

double
Feature::feature_2(Xapian::Document doc_) {
    doubel value = 0;
    vector<long int> & query_term_frequency_database = 
        feature_manager.get_query_term_frequency_database();
    for (Xapian::TermIterator term_it = query.get_terms_begin(), vector<long int>::iterator it = query_term_frequency_database.begin();
            it != query_term_frequency_database.end(); ++term_it, ++it) {
        if ((*term_it)[0] != "S" && (*term_it)[1] != "S")
            value += log10(1 + *it);
        else
            value += 0;
    }
    return value;
}

double
Feature::feature_3(Xapian::Document doc_) {
    double value = 0;
    vector<long int> & query_term_frequency_database = 
        feature_manager.get_query_term_frequency_database();
    for (vector<long int>::iterator it = query_term_frequency_database.begin();
            it != query_term_frequency_database.end(); ++it)
        value += log10(1 + *it);
    return value;
}

double
Feature::feature_4(Xapian::Document doc_) {
    double value = 0;
    vector<long int> & query_term_frequency_database = 
        feature_manager.get_query_term_frequency_database();
    vector<long int> doc_details = feature_manager.get_doc_details(doc_);
    for (Xapian::TermIterator term_it = query.get_terms_begin(), vector<long int>::iterator it = query_term_frequency_database.begin();
            it != query_term_frequency_database.end(); ++term_it, ++it) {
        if ((*term_it)[0] == "S" || (*term_it)[1] == "S")
            value += log10(1 + (double)(*it) / (1 + (double)doc_details[0]));
        else
            value += 0;
    }
    return value;
}

double
Feature::feature_5(Xapian::Document doc_) {
    double value = 0;
    vector<long int> & query_term_frequency_database = 
        feature_manager.get_query_term_frequency_database();
    vector<long int> doc_details = feature_manager.get_doc_details(doc_);
    for (Xapian::TermIterator term_it = query.get_terms_begin(), vector<long int>::iterator it = query_term_frequency_database.begin();
            it != query_term_frequency_database.end(); ++term_it, ++it) {
        if ((*term_it)[0] != "S" && (*term_it)[1] != "S")
            value += log10(1 + (double)(*it) / (1 + (double)doc_details[1]));
        else
            value += 0;
    }
    return value;
 }

double
Feature::feature_6(Xapian::Document doc_) {
    double value = 0;
    vector<long int> & query_term_frequency_database = 
        feature_manager.get_query_term_frequency_database();
    for (vector<long int>::iterator it = query_term_frequency_database.begin();
            it != query_term_frequency_database.end(); ++it)
        value += log10(1 + (double)(*it) / (1 + (double)doc_details[2]));
    return value;
}




double
Feature::get_feature(FeatureBase feature_base_, Xapian::Document doc_) {
    
    switch (feature_base_) {
        case FeatureBase::FEATURE_1:
            return feature_1(doc_);
            break;
        case FeatureBase::FEATURE_2:
            return feature_2(doc_);
            break;
        case FeatureBase::FEATURE_3:
            return feature_3(doc_);
            break;
        case FeatureBase::FEATURE_4:
            return feature_4(doc_);
            break;
        case FeatureBase::FEATURE_5:
            return feature_5(doc_);
            break;
        case FeatureBase::FEATURE_6:
            return feature_6(doc_);
            break;
        case FeatureBase::FEATURE_7:
            return feature_7(doc_);
            break;
        case FeatureBase::FEATURE_8:
            return feature_8(doc_);
            break;
        case FeatureBase::FEATURE_9:
            return feature_9(doc_);
            break;
        case FeatureBase::FEATURE_10:
            return feature_10(doc_);
            break;
        case FeatureBase::FEATURE_11:
            return feature_11(doc_);
            break;
        case FeatureBase::FEATURE_12:
            return feature_12(doc_);
            break;
        case FeatureBase::FEATURE_13:
            return feature_13(doc_);
            break;
        case FeatureBase::FEATURE_14:
            return feature_14(doc_);
            break;
        case FeatureBase::FEATURE_15:
            return feature_15(doc_);
            break;
        case FeatureBase::FEATURE_16:
            return feature_16(doc_);
            break;
        case FeatureBase::FEATURE_17:
            return feature_17(doc_);
            break;
        case FeatureBase::FEATURE_18:
            return feature_18(doc_);
            break;
        case FeatureBase::FEATURE_19:
            return feature_19(doc_);
            break;
    }
}

void
Feature::create(const FeatureManager feature_manager_,
vector<Feature::FeatureBase features_) {
    return Feature(feature_manager_, features_);
}

void
Feature::update_context(Xapian::Query query_, Xapian::MSet mset_) {
    query = query_;
}

vector<double>
Feature::generate_feature_vector(Xapian::Document doc_) {
    vector<double> feature_vector;
    feature_vector.reserve(get_features_num());
    for (int i=0; i<get_features_num(); i++) {
        feature_vector[i] = get_feature(features[i]);
    }
    return feature_vector;
}

int
Feature::get_features_num() {
    return features_.size();
}
