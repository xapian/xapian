
#ifndef FEATURE_H
#define FEATURE_H

#include <xapian/letor.h>

#include <list>
#include <map>


namespace Xapian {

class XAPIAN_VISIBILITY_DEFAULT Feature {
    
    FeatureManaget & feature_manager;
    vector<Feature::FeatureBase> features;
    map<String, long int> query_term_frequency_doc;
    map<String, long int> query_term_frequency_db;

    Feature(const FeatureManager & feature_manager_, vector<Feature::FeatureBase>
    features_);

    double get_feature(Feature::FeatureBase feature_base_, Xapian::Document
    doc_);

    double feature_1(Xapian::Query query_, Xapian::Document doc_);
    double feature_2(Xapian::Query query_, Xapian::Document doc_);
    double feature_3(Xapian::Query query_, Xapian::Document doc_);
    double feature_4(Xapian::Query query_, Xapian::Document doc_);
    double feature_5(Xapian::Query query_, Xapian::Document doc_);
    double feature_6(Xapian::Query query_, Xapian::Document doc_);
    double feature_7(Xapian::Query query_, Xapian::Document doc_);
    double feature_8(Xapian::Query query_, Xapian::Document doc_);
    double feature_9(Xapian::Query query_, Xapian::Document doc_);
    double feature_10(Xapian::Query query_, Xapian::Document doc_);
    double feature_11(Xapian::Query query_, Xapian::Document doc_);
    double feature_12(Xapian::Query query_, Xapian::Document doc_);
    double feature_13(Xapian::Query query_, Xapian::Document doc_);
    double feature_14(Xapian::Query query_, Xapian::Document doc_);
    double feature_15(Xapian::Query query_, Xapian::Document doc_);
    double feature_16(Xapian::Query query_, Xapian::Document doc_);
    double feature_17(Xapian::Query query_, Xapian::Document doc_);
    double feature_18(Xapian::Query query_, Xapian::Document doc_);
    double feature_19(Xapian::Query query_, Xapian::Document doc_);

public:
    class enum {
        FEATURE_1,
        FEATURE_2,
        FEATURE_3,
        FEATURE_4,
        FEATURE_5,
        FEATURE_6,
        FEATURE_7,
        FEATURE_8,
        FEATURE_9,
        FEATURE_10,
        FEATURE_11,
        FEATURE_12,
        FEATURE_13,
        FEATURE_14,
        FEATURE_15,
        FEATURE_16,
        FEATURE_17,
        FEATURE_18,
        FEATURE_19
    } FeatureBase;

    static Feature create(FeatureManager feature_manager_,

    vector<Feature::FeatureBase> features_);

    void update_context(Xapian::Query query_, Xapian::MSet mset_);

    Xapian::FeatureVector generate_feature_vector(Xapian::Document doc_);

    vector<double> generate_feature_vector(Xapian::Document doc_);

    int get_features_num();
}
