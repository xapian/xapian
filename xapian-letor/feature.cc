
#include "feature.h"

#include <vector>

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>

using namespace Xapian;
using std::vector;

double
Feature::feature_1(Xapian::Document doc_) {
    double value = 0;
    vector<long int> & q_term_freq_doc = feature_manager.get_q_term_freq_doc(doc_);
    for (Xapian::TermIterator term_it = query.get_terms_begin(), vector<long int>::iterator tf_it = q_term_freq_doc.begin();
            term_it != query.get_terms_end() && tf_it != q_term_freq_doc.end();
            ++term_it, ++tf_it) {
        if ((*term_it)[0] == "S" || (*term_it)[1] == "S")
            value += log10(1 + *tf_it);
    }
    return value;
}

double
Feature::feature_2(Xapian::Document doc_) {
    doubel value = 0;
    vector<long int> & q_term_freq_doc = feature_manager.get_q_term_freq_doc(doc_);
    for (Xapian::TermIterator term_it = query.get_terms_begin(), vector<long int>::iterator tf_it = q_term_freq_doc.begin();
            term_it != query.get_terms_end() && tf_it != q_term_freq_doc.end();
            ++term_it, ++tf_it) {
        if ((*term_it)[0] != "S" && (*term_it)[1] != "S")
            value += log10(1 + *tf_it);
    }
    return value;
}

double
Feature::feature_3(Xapian::Document doc_) {
    double value = 0;
    vector<long int> & q_term_freq_doc = feature_manager.get_q_term_freq_doc(doc_);
    for (vector<long int>::iterator tf_it = q_term_freq_doc.begin();
            tf_it != q_term_freq_doc.end();
            ++tf_it) {
        value += log10(1 + *tf_it);
    }
    return value;
}

double
Feature::feature_4(Xapian::Document doc_) {
    double value = 0;
    vector<long int> & q_term_freq_doc = feature_manager.get_q_term_freq_doc(doc_);
    vector<long int> doc_details = feature_manager.get_doc_details(doc_);
    for (Xapian::TermIterator term_it = query.get_terms_begin(), vector<long int>::iterator tf_it = q_term_freq_doc.begin();
            term_it != query.get_terms_end() && tf_it != q_term_freq_doc.end();
            ++term_it, ++tf_it) {
        if ((*term_it)[0] == "S" || (*term_it)[1] == "S")
            value += log10(1 + (double)(*tf_it) / (double)(1 + doc_details[0]));
    }
    return value;
}

double
Feature::feature_5(Xapian::Document doc_) {
    double value = 0;
    vector<long int> & q_term_freq_doc = feature_manager.get_q_term_freq_doc(doc_);
    vector<long int> doc_details = feature_manager.get_doc_details(doc_);
    for (Xapian::TermIterator term_it = query.get_terms_begin(), vector<long int>::iterator tf_it = q_term_freq_doc.begin();
            term_it != query.get_terms_end() && tf_it != q_term_freq_doc.end();
            ++term_it, ++tf_it) {
        if ((*term_it)[0] != "S" && (*term_it)[1] != "S")
            value += log10(1 + (double)(*tf_it) / (double)(1 + doc_details[1]));
   }
    return value;
 }

double
Feature::feature_6(Xapian::Document doc_) {
    double value = 0;
    vector<long int> & q_term_freq_doc = feature_manager.get_q_term_freq_doc(doc_);
    vector<long int> doc_details = feature_manager.get_doc_details(doc_);
    for (vector<long int>::iterator tf_it = q_term_freq_doc.begin();
            tf_it != q_term_freq_doc.end();
            ++tf_it) {
        value += log10(1 + (double)(*tf_it) / (1 + (double)doc_details[2]));
    }
    return value;
}

double
Feature::feature_7(Xapian::Document doc_) {
    double value = 0;
    vector<long int> & q_inv_doc_freq_db = feature_manager.get_inv_doc_freq_db();
    for (Xapian::TermIterator term_it = query.get_terms_begin(), vector<long int>::iterator tf_it = q_inv_doc_freq_db.begin();
            term_it != query.get_terms_end() && tf_it != q_inv_doc_freq_db.end();
            ++term_it, ++tf_it) {
        if ((*term_it)[0] == "S" || (*term_it)[1] == "S")
            value += log10(1 + *tf_it);
    }
    return value;
}

double
Feature::feature_8(Xapian::Document doc_) {
    doubel value = 0;
    vector<long int> & q_inv_doc_freq_db = feature_manager.get_inv_doc_freq_db();
    for (Xapian::TermIterator term_it = query.get_terms_begin(), vector<long int>::iterator tf_it = q_inv_doc_freq_db.begin();
            term_it != query.get_terms_end() && tf_it != q_inv_doc_freq_db.end();
            ++term_it, ++tf_it) {
       if ((*term_it)[0] != "S" && (*term_it)[1] != "S")
            value += log10(1 + *tf_it);
    }
    return value;
}

double
Feature::feature_9(Xapian::Document doc_) {
    doubel value = 0;
    vector<long int> & q_inv_doc_freq_db = feature_manager.get_inv_doc_freq_db();
    for (vector<long int>::iterator tf_it = q_inv_doc_freq_db.begin();
            tf_it != q_inv_doc_freq_db.end();
            ++tf_it) {
        value += log10(1 + *tf_it);
    }
    return value;
}

double
Feature::feature_10(Xapian::Document doc_) {
    double value = 0;
    vector<long int> & q_term_freq_db = feature_manager.get_q_term_freq_db();
    vector<long int> & database_details = feature_manager.get_database_datails();
    for (Xapian::TermIterator term_it = query.get_terms_begin(), vector<long int>::iterator tf_it = q_term_freq_db.begin();
            term_it != query.get_terms_end() && tf_it != q_term_freq_db.end();
            ++term_it, ++tf_it) {
        if ((*term_it)[0] == "S" || (*term_it)[1] == "S")
            value += log10 / (1 + (double)doc_details[0] / (double)(1 + *tf_it));
    }
    return value;
}

double
Feature::feature_11(Xapian::Document doc_) {
    double value = 0;
    vector<long int> & q_term_freq_db = feature_manager.get_q_term_freq_db();
    vector<long int> & database_details = feature_manager.get_database_datails();
    for (Xapian::TermIterator term_it = query.get_terms_begin(), vector<long int>::iterator tf_it = q_term_freq_db.begin();
            term_it != query.get_terms_end() && tf_it != q_term_freq_db.end();
            ++term_it, ++tf_it) {
        if ((*term_it)[0] != "S" && (*term_it)[1] != "S")
            value += log10 / (1 + (double)doc_details[1] / (double)(1 + *tf_it));
    }
    return value;
 }

double
Feature::feature_12(Xapian::Document doc_) {
    double value = 0;
    vector<long int> & q_term_freq_db = feature_manager.get_q_term_freq_db();
    vector<long int> & database_details = feature_manager.get_database_datails();
    for (vector<long int>::iterator tf_it = q_term_freq_db.begin();
            tf_it != q_term_freq_db.end();
            ++tf_it) {
        value += log10 / (1 + (double)doc_details[2] / (double)(1 + *tf_it));
    }
    return value;
}

double
feature::feature_13(xapian::document doc_) {
    double value = 0;
    vector<long int> & q_term_freq_doc = feature_manager.get_q_term_freq_doc(doc_);
    vector<long int> & q_inv_doc_freq_db = feature_manager.get_inv_doc_freq_db();
    vector<long int> doc_details = feature_manager.get_doc_details(doc_);

    for (Xapian::termiterator term_it = query.get_terms_begin(),
            vector<long int>::iterator tf_it = q_term_freq_db.begin(),
            vector<long int>::iterator idf_it = q_inv_freq_db.begin();
            term_it != query.get_terms_end() && tf_it != q_term_freq_db.end() && idf_it != q_inv_freq_db.end();
            ++term_it, ++tf_it, ++idf_it) {
        if ((*term_it)[0] == "s" || (*term_it)[1] == "s")
            value += log10(1 + ((double)((*tf_it)*(*idf_it)) / (double)(1 + doc_details[0])));
    }
    return value;
}

double
feature::feature_14(xapian::document doc_) {
    double value = 0;
    vector<long int> & q_term_freq_doc = feature_manager.get_q_term_freq_doc(doc_);
    vector<long int> & q_inv_doc_freq_db = feature_manager.get_inv_doc_freq_db();
    vector<long int> doc_details = feature_manager.get_doc_details(doc_);

    for (Xapian::TermIterator term_it = query.get_terms_begin(),
            vector<long int>::iterator tf_it = q_term_freq_db.begin(),
            vector<long int>::iterator idf_it = q_inv_freq_db.begin();
            term_it != query.get_terms_end() && tf_it != q_term_freq_db.end() && idf_it != q_inv_freq_db.end();
            ++term_it, ++tf_it, ++idf_it) {
        if ((*term_it)[0] != "s" && (*term_it)[1] != "s")
            value += log10(1 + ((double)((*tf_it)*(*idf_it)) / (double)(1 + doc_details[1])));
    }
    return value;
 }

double
feature::feature_15(xapian::document doc_) {
    double value = 0;
    vector<long int> & q_term_freq_doc = feature_manager.get_q_term_freq_doc(doc_);
    vector<long int> & q_inv_doc_freq_db = feature_manager.get_inv_doc_freq_db();
    vector<long int> doc_details = feature_manager.get_doc_details(doc_);

    for (vector<long int>::iterator tf_it = q_term_freq_db.begin(),
            vector<long int>::iterator idf_it = q_inv_freq_db.begin();
            tf_it != q_term_freq_db.end() && idf_it != q_inv_freq_db.end();
            ++tf_it, ++idf_it) {
        value += log10(1 + ((double)((*tf_it)*(*idf_it)) / (double)(1 + doc_details[2])));
    }
    return value;
}

double
feature::feature_16(xapian::document doc_) {
    double value = 0;
    vector<long int> & q_term_freq_db = feature_manager.get_q_term_freq_db();
    vector<long int> & database_details = feature_manager.get_database_datails();
    vector<long int> q_term_freq_doc = feature_manager.get_q_term_freq_doc(doc_);
    vector<long int> doc_details = feature_manager.get_doc_details(doc_);

    for (Xapian::TermIterator term_it = query.get_terms_begin(),
            vector<long int>::iterator db_tf_it = q_term_freq_db.begin(),
            vector<long int>::iterator doc_tf_it = q_term_freq_doc.begin();
            term_it != query.get_terms_end() && db_tf_it != q_term_freq_db.end() && doc_tf_it != q_term_freq_doc.end();
            ++term_it, ++doc_tf_it, ++db_tf_it) {
        if ((*term_it)[0] == "s" || (*term_it)[1] == "s")
            value += log10(1 + ((double)(*doc_tf_it) * (double)database_details[0] / (double)(1 + (double)doc_details[0] * (double)(*db_tf_it))));
    }

    return value;
}

double
feature::feature_17(xapian::document doc_) {
    double value = 0;
    vector<long int> & q_term_freq_db = feature_manager.get_q_term_freq_db();
    vector<long int> & database_details = feature_manager.get_database_datails();
    vector<long int> q_term_freq_doc = feature_manager.get_q_term_freq_doc(doc_);
    vector<long int> doc_details = feature_manager.get_doc_details(doc_);

    for (Xapian::TermIterator term_it = query.get_terms_begin(),
            vector<long int>::iterator db_tf_it = q_term_freq_db.begin(),
            vector<long int>::iterator doc_tf_it = q_term_freq_doc.begin();
            term_it != query.get_terms_end() && db_tf_it != q_term_freq_db.end() && doc_tf_it != q_term_freq_doc.end();
            ++term_it, ++doc_tf_it, ++db_tf_it) {
        if ((*term_it)[0] != "s" && (*term_it)[1] != "s")
            value += log10(1 + ((double)(*doc_tf_it) * (double)database_details[1] / (double)(1 + (double)doc_details[1] * (double)(*db_tf_it))));
    }
    
    return value;
 }

double
feature::feature_18(xapian::document doc_) {
    double value = 0;
    vector<long int> & q_term_freq_db = feature_manager.get_q_term_freq_db();
    vector<long int> & database_details = feature_manager.get_database_datails();
    vector<long int> q_term_freq_doc = feature_manager.get_q_term_freq_doc(doc_);
    vector<long int> doc_details = feature_manager.get_doc_details(doc_);

    for (vector<long int>::iterator db_tf_it = q_term_freq_db.begin(),
            vector<long int>::iterator doc_tf_it = q_term_freq_doc.begin();
            db_tf_it != q_term_freq_db.end() && doc_tf_it != q_term_freq_doc.end();
            ++doc_tf_it, ++db_tf_it) {
        value += log10(1 + ((double)(*doc_tf_it) * (double)database_details[2] / (double)(1 + (double)doc_details[2] * (double)(*db_tf_it))));
    }

    return value;
}

double
Feature::get_feature(const FeatureBase & feature_base_, const Xapian::MSetIterator & mset_it_) {
    Xapian::Document doc_ = mset_it_.get_document();
    
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
            return mset_it_.get_weight();
            break;
    }
}

void
Feature::update(const Feature & feature_manager_, const vector<Feature::FeatureBase> & features_) {
    feature_manager = feature_manager_;
    features = features_;
}

FeatureVector
Feature::generate_feature_vector(const Xapian::MSetIterator & met_it_) {
    FeatureVector fvector;

    Xapian::Document doc = mset_it_.get_document();

    fvector.set_did( doc.get_did() );

    vector<double> feature_values(get_features_num());
    for (int i=0; i<get_features_num(); i++) {
        feature_values[i] = get_feature(features[i], mset_it_);
    }
    fvector.set_feature_values( feature_values );

    return fvector;
}

int
Feature::get_features_num() {
    return features.size();
}
