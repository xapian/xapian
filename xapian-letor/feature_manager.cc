
#include "featuremanager.h"
#include "feature.h"

#include <cstring>
#include <cstdlib>
#include <fstream>

#include <map>

using namespace Xapian;

FeatureManager::FeatureManager(const Xapian::Database & database_,
vector<Feature::FeatureBase> features_) {
    database = database_;
    update_database_details();

    feature = Feature.create(*this, features_);
}

Xapian::Database &
FeatureManager::get_database() {
    return database;
}

Xapian::Query &
FeatureManager::get_query() {
    return query;
}

Xapian::MSet
FeatureManager::get_mset() {
    return mset;
}

Feature &
FeatureManager::get_feature() {
    return feature;
}

int
FeatureManager::get_features_num() {
    return feature.get_features_num();
}

void
FeatureManager::update_database_details() {
    if (!database.get_metadata("collection_len_title").empty()
            && !database.get_metadata("collection_len_bogy").empty()
            && !database.get_metadata("collection_len_whole").empty()) {
        database_details[0] = 
            atol(database.get_metadata("collection_len_title").c_str());
        data_details[1] =
            atol(database.get_metadata("collection_len_body").c_str());
        data_details[2] = 
            atol(database.get_metadata("collection_len_whole").c_str());
    }
    else {
        long int title_length = 0;
        long int whole_length = database.get_avlength() *
        database.get_doccount();

        Xapian::TermIterator term_it = database.allterms_begin("S");
        for (; term_it != database.allterms_end("S"); ++term_it)
            title_length += database.get_collection_freq(*term_it);
        d_details[0] = title_length;
        d_details[1] = whole_length;
        d_details[2] = whole_length - title_length;
    }
}

vector<long int> &
FeatureManager::get_database_details() {
    return database_details;
}

void
FeatureManager::update_query_term_frequency_database() {
    query_term_frequency_database.reserve(term_length);

    
    for (int i = 0, Xapian::TermIterator q_term_it = query.get_terms_begin();
            q_term_it != query.get_terms_end() && i < query_term_length;
            ++i, ++q_term_it) {
        if (database.term_exists(*q_term_it))
            query_term_frequency_database[i] =
                database.get_collection_freq(*q_term_it);
        else
            query_term_frequency_database[i] = 0;
    }
}

vector<long int> &
FeatureManager::get_query_term_frequency_database() {
    return query_term_frequency_database;
}

void
FeatureManager::update_query_inverse_doc_frequency_database() {
    query_inverse_doc_frequency_database.reserve(term_length);

    for (int i = 0, Xapian::TermIterator q_term_it = query.get_terms_begin();
            q_term_it != query.get_terms_end() && i < query_term_length;
            ++i, ++q_term_it) {
        if (database.term_exists(*q_term_it))
            query_inverse_doc_frequency_database[i] =
                log10( databse.get_doccount() / (1 +
                database.get_termfreq(*q_term_it)));
        else
            query_inverse_doc_frequency_database[i] = 0;
    }
}

vector<double> &
FeatureManager::get_query_inverse_doc_frequency_database() {
    return query_inverse_doc_frequency_database;
}


vector<long int>
FeatureManager::get_query_term_frequency_doc(Xapian::Document doc_) {
    vector<long int> term_freq;
    term_freq.reserve(query_term_length);

    Xapian::TermIterator doc_term_it = doc.termlist_begin();
    for (int i = 0, Xapain::TermIterator q_term_it = query.get_terms_begin();
            q_term_it != query.get_terms_end() && i < query_term_length;
            ++i, ++q_term_it) {
        doc_term_it.skip_to(*q_term_it);
        if (q_term_it != doc.termlist_end() && *q_term_it == *doc_item_it)
            term_freq.push_back(doc_term_it.get_wdf());
        else
            term_freq.push_back(0);
    }

    return term_freq;
}

vector<long int>
FeatureManager::get_doc_details(Xapian::Document doc_) {
    vector<long int> d_details(3);

    long int title_length = 0;
    long int whole_length = database.get_doclength(doc.get_docid());
    Xapian::TermIterator d_term_it = doc.termlist_begin();

    dt.skip_to("S");
    for (; d_term_it != doc.terlist_end(); ++d_term_it) {
        if ((*d_term_it)[0] != 'S')
            break;
        title_length += dt.get_wdf();
    }
    d_details[0] = title_length;
    d_details[1] = whole_length;
    d_details[2] = whole_length - title_length;

    return d_details;
}

void
FeatureManager::update_state(Xapian::Query query_, Xapian::MSet mset_) {
    query = query_;
    query_term_length = query.get_length();
    update_query_term_frequency_database();
    update_query_inverse_doc_frequency_database();

    mset = mset_;
}

void 
FeatureManager::update_mset(vector<Xapian::MSet::letor_item> & letor_items_) {
}
