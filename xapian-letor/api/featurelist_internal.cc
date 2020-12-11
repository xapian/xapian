/** @file
 * @brief Definition of Feature::Internal class.
 */
/* Copyright (C) 2012 Parth Gupta
 * Copyright (C) 2016 Ayush Tomar
 * Copyright (C) 2019 Vaibhav Kansagara
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
#include "feature_internal.h"

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "debuglog.h"

using namespace std;
using namespace Xapian;

void
FeatureList::Internal::set_data(const Xapian::Query & letor_query,
				const Xapian::Database & letor_db,
				const Xapian::Document & letor_doc)
{
    set_query(letor_query);
    set_doc(letor_doc);
    set_database(letor_db);
}

std::map<std::string, Xapian::termcount>
FeatureList::Internal::compute_termfreq() const
{
    std::map<std::string, Xapian::termcount> tf;

    Xapian::TermIterator docterms = featurelist_doc.termlist_begin();
    for (Xapian::TermIterator qt = featurelist_query.get_unique_terms_begin();
	 qt != featurelist_query.get_terms_end(); ++qt) {
	docterms.skip_to(*qt);
	if (docterms != featurelist_doc.termlist_end() && *qt == *docterms)
	    tf[*qt] = docterms.get_wdf();
    }
    return tf;
}

std::map<std::string, double>
FeatureList::Internal::compute_inverse_doc_freq() const
{
    std::map<std::string, double> idf;
    Xapian::doccount totaldocs = featurelist_db.get_doccount();

    for (Xapian::TermIterator qt = featurelist_query.get_unique_terms_begin();
	 qt != featurelist_query.get_terms_end(); ++qt) {
	Xapian::doccount df = featurelist_db.get_termfreq(*qt);
	if (df != 0)
	    idf[*qt] = log10((double)totaldocs / (double)(1 + df));
    }
    return idf;
}

std::map<std::string, Xapian::termcount>
FeatureList::Internal::compute_doc_length() const
{
    std::map<std::string, Xapian::termcount> len;

    Xapian::termcount title_len = 0;
    Xapian::TermIterator dt = featurelist_doc.termlist_begin();
    // reach the iterator to the start of the title terms i.e. prefix "S"
    dt.skip_to("S");
    for ( ; dt != featurelist_doc.termlist_end(); ++dt) {
	if ((*dt)[0] != 'S') {
	    // We've reached the end of the S-prefixed terms.
	    break;
	}
	title_len += dt.get_wdf();
    }
    len["title"] = title_len;
    Xapian::termcount whole_len =
	    featurelist_db.get_doclength(featurelist_doc.get_docid());
    len["whole"] = whole_len;
    len["body"] = whole_len - title_len;
    return len;
}

std::map<std::string, Xapian::termcount>
FeatureList::Internal::compute_collection_length() const
{
    std::map<std::string, Xapian::termcount> len;

    if (!featurelist_db.get_metadata("collection_len_title").empty() &&
	!featurelist_db.get_metadata("collection_len_body").empty() &&
	!featurelist_db.get_metadata("collection_len_whole").empty()) {
	len["title"] =
	    atol(featurelist_db.get_metadata("collection_len_title").c_str());
	len["body"] =
	    atol(featurelist_db.get_metadata("collection_len_body").c_str());
	len["whole"] =
	    atol(featurelist_db.get_metadata("collection_len_whole").c_str());
    } else {
	Xapian::termcount title_len = 0;
	Xapian::TermIterator dt = featurelist_db.allterms_begin("S");
	for ( ; dt != featurelist_db.allterms_end("S"); ++dt) {
	    //  because we don't want the unique terms so we want their
	    // original frequencies and i.e. the total size of the title collection.
	    title_len += featurelist_db.get_collection_freq(*dt);
	}
	len["title"] = title_len;
	Xapian::termcount whole_len = featurelist_db.get_avlength() *
		featurelist_db.get_doccount();
	len["whole"] = whole_len;
	len["body"] = whole_len - title_len;
    }
    return len;
}

std::map<std::string, Xapian::termcount>
FeatureList::Internal::compute_collection_termfreq() const
{
    std::map<std::string, Xapian::termcount> tf;

    for (Xapian::TermIterator qt = featurelist_query.get_unique_terms_begin();
	 qt != featurelist_query.get_terms_end(); ++qt) {
	Xapian::termcount coll_tf = featurelist_db.get_collection_freq(*qt);
	if (coll_tf != 0)
	    tf[*qt] = coll_tf;
    }
    return tf;
}

void
FeatureList::Internal::populate_feature_internal(Feature::Internal*
						 internal_feature)
{
    if (stats_needed & TERM_FREQUENCY) {
	internal_feature->set_termfreq(compute_termfreq());
    }
    if (stats_needed & INVERSE_DOCUMENT_FREQUENCY) {
	internal_feature->set_inverse_doc_freq(compute_inverse_doc_freq());
    }
    if (stats_needed & DOCUMENT_LENGTH) {
	internal_feature->set_doc_length(compute_doc_length());
    }
    if (stats_needed & COLLECTION_LENGTH) {
	internal_feature->set_collection_length(compute_collection_length());
    }
    if (stats_needed & COLLECTION_TERM_FREQ) {
	internal_feature->set_collection_termfreq(
			  compute_collection_termfreq());
    }
}
