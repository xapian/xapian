
#ifndef FEATURE_MANAGER_H
#define FEATURE_MANAGER_H

#include <xapian.h>
#include <xapian/intrusive_ptr.h>
#include <xapian/types.h>
#include <xapian/visibility.h>

#include "feature.h"

#include <vector>
#include <string>

using std::vector;

namespace Xapian {

class Feature;

class XAPIAN_VISIBILITY_DEFAULT FeatureManager {

    Xapian::Database & database;
    Feature feature;
    Xapian::Query & query;
    Xapian::MSet & mset;

    int query_term_length;
    vector<long int> query_term_frequency_database;
    vector<double> query_inverse_doc_frequency_database;
    vector<long int> database_details(3);

    vector<Xapian::MSet::letor_item> generate_letor_info();

    void update_database_details();
    void update_query_term_frequency_database();
    void update_query_inverse_doc_frequency_database();

public:

    Xapian::Database & get_database();

    Xapian::Query & get_query();

    Xapian::MSet & get_mset();

    Feature & get_feature();

    int get_features_num();

    vector<long int> & get_database_details();

    vector<long int> & get_q_term_freq_db();

    vector<long int> & get_q_inv_doc_freq_db();

    vector<long int> & get_database_details();

    vector<long int> get_q_term_freq_doc(const Xapain::Document doc_);

    vector<long int> get_doc_details(const Xapian::Docuement doc_);

    void update_context(const Xapian::Database database_, const vector<Feature::FeatureBase> features_);

    void update_state(const Xapian::Query query_, const Xapian::MSet mset_);

    void update_mset(const vector<Xapian::MSet::letor_item> & letor_items_);
}

#endif // FEATURE_MANAGER_H
