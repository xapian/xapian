
#ifndef FEATURE_H
#define FEATURE_H

#include <xapian/letor.h>

#include <vector>

using std::vector;

namespace Xapian {

class XAPIAN_VISIBILITY_DEFAULT Feature {
    
    FeatureManaget & feature_manager;
    vector<feature_t> features;

    double get_feature(const feature_t & feature_base_, const Xapian::MSetIterator & mset_it_);

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
    // The feature type. The index starts from 1.
    typedef unsigned int feature_t;

    // Total feature number.
    static const int MAX_FEATURE_NUM = 19;
    //
    // Feature 1:
    // Feature 2:
    // Feature 3:
    // Feature 4:
    // Feature 5:
    // Feature 6:
    // Feature 7:
    // Feature 8:
    // Feature 9:
    // Feature 10:
    // Feature 11:
    // Feature 12:
    // Feature 13:
    // Feature 14:
    // Feature 15:
    // Feature 16:
    // Feature 17:
    // Feature 18:
    // Feature 19:

    // Set FeatureManager to use this Feature
    void set_featuremanager(const FeatureManager & feature_manager_);

    // Set features to be used
    void set_features(const vector<feature_t> & features_);

    // Get document id
    string get_did(const Document & doc);

    // Generate FeatureVector without score and label
    FeatureVector generate_feature_vector(const Xapian::MSetIterator & mset_it_);

    // Return the number of features used
    int get_features_num();

    // Check if features are valid
    static bool is_valid(const vector<feature_t> & features);

    // Restore Feature from file
    vector<feature_t> read_from_file(string file);
};

#endif /* FEATURES_H */
