
#ifndef FEATURE_MANAGER_H
#define FEATURE_MANAGER_H

#include <xapian.h>
#include <xapian/intrusive_ptr.h>
#include <xapian/types.h>
#include <xapian/visibility.h>

#include "feature.h"

#include <map>
#include <string>

using namespace std;

namespace Xapian {

class Feature;

class XAPIAN_VISIBILITY_DEFAULT FeatureManager {

    Xapian::Database & database;
    Feature feature;
    Xapian::Query query;
    Xapian::MSet mset;

    int query_term_length;
    vector<long int> query_term_frequency_database;
    vector<double> query_inverse_doc_frequency_database;
    vector<long int> database_details(3);

    FeatureManager(Xapian::Database database_, vector<Feature::FeatureBase>
    feature_base_);

    vector<Xapian::MSet::letor_item> generate_letor_info();

    void update_database_details();
    void update_query_term_frequency_database();
    void update_query_inverse_doc_frequency_database();

public:

    static FeatureManager create(Xapian::Database database_,
    vector<Feature::FeatureBase> features_);

    Xapian::Database & get_database();

    Xapian::Query & get_query();

    Xapian::MSet get_mset();

    Feature & get_feature();

    int get_features_num();

    vector<long int> & get_database_details();

    vector<long int> & get_query_term_frequency_database();

    vector<long int> & get_query_inverse_doc_frequency_database();

    vector<long int> & get_database_details();

    vector<long int> get_query_term_frequency_doc(Xapain::Document doc_);

    vector<long int> get_doc_details(Xapian::Docuement doc_);

    void update_state(Xapian::Query query_, Xapian::MSet mset_);

    void update_mset(Xapian::MSet mset_, vector<Xapian::MSet::letor_item> &
    letor_items_);
}

#endif // FEATURE_MANAGER_H
