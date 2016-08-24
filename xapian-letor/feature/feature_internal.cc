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

#include "xapian-letor/feature.h"
#include "feature_internal.h"

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>

using namespace std;
using namespace Xapian;

std::map<string, long int>
Feature::Internal::termfreq() const {

    std::map<string, long int> tf;

    Xapian::TermIterator docterms = feature_doc.termlist_begin();
    for (Xapian::TermIterator qt = feature_query.get_terms_begin();
     qt != feature_query.get_terms_end(); ++qt) {
    docterms.skip_to(*qt);
    if (docterms != feature_doc.termlist_end() && *qt == *docterms) {
	tf[*qt] = docterms.get_wdf();
    } else {
	tf[*qt] = 0;
    }
    }
    return tf;
}

std::map<string, double>
Feature::Internal::inverse_doc_freq() const {

    std::map<string, double> idf;

    for (Xapian::TermIterator qt = feature_query.get_terms_begin();
     qt != feature_query.get_terms_end(); ++qt) {
    if (feature_db.term_exists(*qt)) {
	long int totaldocs = feature_db.get_doccount();
	long int df = feature_db.get_termfreq(*qt);
	idf[*qt] = log10(totaldocs / (1 + df));
    } else {
	idf[*qt] = 0;
    }
    }
    return idf;
}

std::map<string, long int>
Feature::Internal::doc_length() const {

    std::map<string, long int> len;

    long int temp_count = 0;
    Xapian::TermIterator dt = feature_doc.termlist_begin();
    // reach the iterator to the start of the title terms i.e. prefix "S"
    dt.skip_to("S");
    for ( ; dt != feature_doc.termlist_end(); ++dt) {
    if ((*dt)[0] != 'S') {
	// We've reached the end of the S-prefixed terms.
	break;
    }
    temp_count += dt.get_wdf();
    }
    len["title"] = temp_count;
    len["whole"] = feature_db.get_doclength(feature_doc.get_docid());
    len["body"] = len["whole"] - len["title"];
    return len;
}

std::map<string, long int>
Feature::Internal::collection_length() const {

    std::map<string, long int> len;

    if (!feature_db.get_metadata("collection_len_title").empty() && !feature_db.get_metadata("collection_len_body").empty() && !feature_db.get_metadata("collection_len_whole").empty()) {
    len["title"] = atol(feature_db.get_metadata("collection_len_title").c_str());
    len["body"] = atol(feature_db.get_metadata("collection_len_body").c_str());
    len["whole"] = atol(feature_db.get_metadata("collection_len_whole").c_str());
    } else {
    long int temp_count = 0;
    Xapian::TermIterator dt = feature_db.allterms_begin("S");
    for ( ; dt != feature_db.allterms_end("S"); ++dt) {
	temp_count += feature_db.get_collection_freq(*dt);  //  because we don't want the unique terms so we want their original frequencies and i.e. the total size of the title collection.
    }
    len["title"] = temp_count;
    len["whole"] = feature_db.get_avlength() * feature_db.get_doccount();
    len["body"] = len["whole"] - len["title"];
    }
    return len;
}

std::map<string, long int>
Feature::Internal::collection_termfreq() const {

    std::map<string, long int> tf;

    for (Xapian::TermIterator qt = feature_query.get_terms_begin();
     qt != feature_query.get_terms_end(); ++qt) {
    if (feature_db.term_exists(*qt))
	tf[*qt] = feature_db.get_collection_freq(*qt);
    else
	tf[*qt] = 0;
    }
    return tf;
}
