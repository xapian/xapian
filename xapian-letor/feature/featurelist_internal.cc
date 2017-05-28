/** @file feature_internal.cc
 * @brief Definition of Feature::Internal class.
 */
/* Copyright (C) 2012 Parth Gupta
 * Copyright (C) 2016 Ayush Tomar
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301
 * USA
 */

#include <config.h>

#include "xapian-letor/featurelist.h"
#include "featurelist_internal.h"

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>

using namespace std;
using namespace Xapian;

void
FeatureList::Internal::set_database(const Xapian::Database & db)
{
    LOGCALL_VOID(API, "FeatureList::set_database", db);
    featurelist_db = db;
}

void
FeatureList::Internal::set_query(const Xapian::Query & query)
{
    LOGCALL_VOID(API, "FeatureList::set_query", query);
    if (query.empty()) {
        throw Xapian::InvalidArgumentError("Can't initialise with an empty query string");
    }
    featurelist_query = query;
}

void
FeatureList::Internal::set_doc(const Xapian::Document & doc)
{
    LOGCALL_VOID(API, "FeatureList::set_doc", doc);
    featurelist_doc = doc;
}


void
FeatureList::Internal::compute_statistics(const Xapian::Query & letor_query, const Xapian::Document & letor_doc,
                                          const Xapian::Database & letor_db) const
{
    set_query(letor_query);
    set_doc(letor_doc);
    set_db(letor_db);

    termfreq = compute_termfreq();
    inverse_doc_freq = compute_inverse_doc_freq();
    doc_length = compute_doc_length();
    collection_length = compute_collection_length();
    collection_termfreq = compute_collection_termfreq();
}


std::map<string, long int>
FeatureList::Internal::compute_termfreq() const
{
    std::map<string, long int> tf;

    Xapian::TermIterator docterms = featurelist_doc.termlist_begin();
    for (Xapian::TermIterator qt = featurelist_query.get_terms_begin(); qt != featurelist_query.get_terms_end(); ++qt) {
        docterms.skip_to(*qt);
        if (docterms != featurelist_doc.termlist_end() && *qt == *docterms) {
            tf[*qt] = docterms.get_wdf();
        } else {
            tf[*qt] = 0;
        }
    }
    return tf;
}

std::map<string, double>
FeatureList::Internal::compute_inverse_doc_freq() const
{
    std::map<string, double> idf;

    for (Xapian::TermIterator qt = featurelist_query.get_terms_begin(); qt != featurelist_query.get_terms_end(); ++qt) {
        if (feature_db.term_exists(*qt)) {
            long int totaldocs = featurelist_db.get_doccount();
            long int df = featurelist_db.get_termfreq(*qt);
            idf[*qt] = log10(totaldocs / (1 + df));
        } else {
            idf[*qt] = 0;
        }
    }
    return idf;
}

std::map<string, long int>
FeatureList::Internal::compute_doc_length() const
{
    std::map<string, long int> len;

    long int temp_count = 0;
    Xapian::TermIterator dt = featurelist_doc.termlist_begin();
    // reach the iterator to the start of the title terms i.e. prefix "S"
    dt.skip_to("S");
    for ( ; dt != featurelist_doc.termlist_end(); ++dt) {
        if ((*dt)[0] != 'S') {
            // We've reached the end of the S-prefixed terms.
            break;
        }
        temp_count += dt.get_wdf();
    }
    len["title"] = temp_count;
    len["whole"] = featurelist_db.get_doclength(featurelist_doc.get_docid());
    len["body"] = len["whole"] - len["title"];
    return len;
}

std::map<string, long int>
FeatureList::Internal::compute_collection_length() const
{
    std::map<string, long int> len;

    if (!featurelist_db.get_metadata("collection_len_title").empty() &&
            !featurelist_db.get_metadata("collection_len_body").empty() &&
            !featurelist_db.get_metadata("collection_len_whole").empty()) {
        len["title"] = atol(featurelist_db.get_metadata("collection_len_title").c_str());
        len["body"] = atol(featurelist_db.get_metadata("collection_len_body").c_str());
        len["whole"] = atol(featurelist_db.get_metadata("collection_len_whole").c_str());
    } else {
        long int temp_count = 0;
        Xapian::TermIterator dt = featurelist_db.allterms_begin("S");
        for ( ; dt != featurelist_db.allterms_end("S"); ++dt) {
            //  because we don't want the unique terms so we want their
            // original frequencies and i.e. the total size of the title collection.
            temp_count += featurelist_db.get_collection_freq(*dt);
        }
        len["title"] = temp_count;
        len["whole"] = featurelist_db.get_avlength() * featurelist_db.get_doccount();
        len["body"] = len["whole"] - len["title"];
    }
    return len;
}

std::map<string, long int>
FeatureList::Internal::compute_collection_termfreq() const
{
    std::map<string, long int> tf;

    for (Xapian::TermIterator qt = featurelist_query.get_terms_begin(); qt != featurelist_query.get_terms_end(); ++qt) {
        if (featurelist_db.term_exists(*qt))
            tf[*qt] = featurelist_db.get_collection_freq(*qt);
        else
            tf[*qt] = 0;
    }
    return tf;
}
void
FeatureList::Internal::compute_populate_feature(Feature *feature)
{
    stat_flag stats_needed =stat_flags(feature->get_stats());
    if (stats_needed & TERM_FREQUENCY)
        feature->termfreq = termfreq;
    if (stats_needed & INVERSE_DOCUMENT_FREQUENCY)
        feature->inverse_doc_freq = inverse_doc_freq;
    if (stats_needed & DOCUMENT_LENGTH)
        feature->doc_length = doclength;
    if (stats_needed & COLLECTION_LENGTH)
        feature->collection_length = collection_length;
    if (stats_needed & COLLECTION_TERM_FREQ)
        feature->collection_termfreq = collection_termfreq;
}
