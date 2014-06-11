
#ifndef FEATURE_H
#define FEATURE_H

#include <xapian/letor.h>

#include <vector>

using std::vector;

namespace Xapian {

class XAPIAN_VISIBILITY_DEFAULT Feature {
    
    const FeatureManaget & feature_manager;
    vector<Feature::FeatureBase> features;


    double get_feature(const Feature::FeatureBase & feature_base_, const Xapian::MSetIterator & mset_it_);

    double feature_1(Xapian::Document doc_);
    double feature_2(Xapian::Document doc_);
    double feature_3(Xapian::Document doc_);
    double feature_4(Xapian::Document doc_);
    double feature_5(Xapian::Document doc_);
    double feature_6(Xapian::Document doc_);
    double feature_7(Xapian::Document doc_);
    double feature_8(Xapian::Document doc_);
    double feature_9(Xapian::Document doc_);
    double feature_10(Xapian::Document doc_);
    double feature_11(Xapian::Document doc_);
    double feature_12(Xapian::Document doc_);
    double feature_13(Xapian::Document doc_);
    double feature_14(Xapian::Document doc_);
    double feature_15(Xapian::Document doc_);
    double feature_16(Xapian::Document doc_);
    double feature_17(Xapian::Document doc_);
    double feature_18(Xapian::Document doc_);

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

    void update(const FeatureManager & feature_manager_, const vector<Feature::FeatureBase> & features_);

    vector<double> generate_feature_vector(const Xapian::MSetIterator & mset_it_);

    int get_features_num();
}

#endif /* FEATURES_H */
