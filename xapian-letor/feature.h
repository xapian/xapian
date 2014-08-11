
#ifndef FEATURE_H
#define FEATURE_H


#include <xapian.h>
#include <xapian/intrusive_ptr.h>
#include <xapian/types.h>
#include <xapian/visibility.h>

#include "feature_manager.h"
#include "feature_vector.h"

#include <vector>
#include <string>

using std::vector;
using std::string;

namespace Xapian {

class FeatureManager;
class FeatureVector;

class XAPIAN_VISIBILITY_DEFAULT Feature {

public:
    // The feature type. The index starts from 1.
    typedef unsigned int feature_t;


private:
    Xapian::FeatureManager * feature_manager;
    vector<Xapian::Feature::feature_t> features;

    double get_feature(const Xapian::Feature::feature_t & feature_base_, const Xapian::MSetIterator & mset_it_);

    double feature_1(Xapian::Document doc_);
    double feature_2(Xapian::Document doc_);
    double feature_3(Xapian::Document doc_);
    double feature_4(Xapian::Document doc_);
    double feature_5(Xapian::Document doc_);
    double feature_6(Xapian::Document doc_);
    double feature_7();
    double feature_8();
    double feature_9();
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
    void set_featuremanager(Xapian::FeatureManager & feature_manager_);

    // Set features to be used
    void set_features(const vector<Xapian::Feature::feature_t> & features_);

    // Get document id
    string get_did(const Xapian::Document & doc);

    // Generate FeatureVector without score and label
    FeatureVector generate_feature_vector(const Xapian::MSetIterator & mset_it_);

    // Return the number of features used
    int get_features_num();

    // Check if features are valid
    static bool is_valid(const vector<Xapian::Feature::feature_t> & features);

    // Restore Feature from file
    static vector<Xapian::Feature::feature_t> read_from_file(string file);
};

}

#endif /* FEATURES_H */
