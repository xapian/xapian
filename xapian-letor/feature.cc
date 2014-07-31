
#include "feature.h"

#include <vector>

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>

using std::vector;
using std::cerr;

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
Feature::get_feature(const feature_t feature_base_, const Xapian::MSetIterator & mset_it_) {
    Xapian::Document doc_ = mset_it_.get_document();
    
    switch (feature_base_) {
        case 1:
            return feature_1(doc_);
        case 2:
            return feature_2(doc_);
        case 3:
            return feature_3(doc_);
        case 4:
            return feature_4(doc_);
        case 5:
            return feature_5(doc_);
        case 6:
            return feature_6(doc_);
        case 7:
            return feature_7(doc_);
        case 8:
            return feature_8(doc_);
        case 9:
            return feature_9(doc_);
        case 10:
            return feature_10(doc_);
        case 11:
            return feature_11(doc_);
        case 12:
            return feature_12(doc_);
        case 13:
            return feature_13(doc_);
        case 14:
            return feature_14(doc_);
        case 15:
            return feature_15(doc_);
        case 16:
            return feature_16(doc_);
        case 17:
            return feature_17(doc_);
        case 18:
            return feature_18(doc_);
        case 19:
            return mset_it_.get_weight();
        default:
            cerr << "Feature type error!\n";
            exit(1);
    }
}

void
Feature::update(const Feature & feature_manager_, const vector<feature_t> & features_) {
    feature_manager = feature_manager_;
    features = features_;
}


string
Feature::get_did(const Document & doc) {
    string data = doc.get_data();
    string temp_id = data.substr(data.find("url=", 0), (data.find("sample=", 0) - data.find("url=", 0)));
    return temp_id.substr(temp_id.rfind('/') + 1, (temp_id.rfind('.') - temp_id.rfind('/') - 1));  //to parse the actual document name associated with the documents if any
}


FeatureVector
Feature::generate_feature_vector(const Xapian::MSetIterator & met_it_) {
    FeatureVector fvector;

    Xapian::Document doc = mset_it_->get_document();

    fvector.set_did( get_did(doc) );

    fvector.set_index( mset_it_->get_rank() );

    vector<double> feature_values( get_features_num() );
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

static bool
Feature::is_valid(const vector<feature_t> & features) {
    if (features.size() == 0)
        return false;
    for (vector<feature_t>::iterator it = features.begin(); it != features.end(); ++it) {
        if (*it < 1 || *it > MAX_FEATURE_NUM) {
            return false;
        }
    }
    return true;
}

vector<feature_t>
Feature::read_from_file(string file) {
    ifstream feature_file;
    string line;
    vector<feature_t> features;

    feature_file.open(file.c_str());
    if (feature_file.is_open()) {
        while (getline(feature_file, line)) {
            istringstream iss(line);
            feature_t feature_idx;
            iss >> feature_idx;
            features.push_back(feature_idx);
        }
    }
    else {
        cerr << "Can't open file " << file << "!\n";
        exit(1);
    }

    feature_file.close();

    if (Feature::is_valid(features))
        return features;
    else {
        cerr << "Features are not valid!\n";
        exit(1);
    }
}