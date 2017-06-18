/** @file featurelist_internal.cc
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
#include "debuglog.h"

using namespace std;
using namespace Xapian;

void
FeatureList::Internal::set_database(const Xapian::Database & db)
{
    featurelist_db = db;
}

void
FeatureList::Internal::set_query(const Xapian::Query & query)
{
    featurelist_query = query;
}

void
FeatureList::Internal::set_doc(const Xapian::Document & doc)
{
    featurelist_doc = doc;
}

void
FeatureList::Internal::compute_statistics(const Xapian::Query & letor_query,
					  const Xapian::Database & letor_db,
					  const Xapian::Document & letor_doc)
{
    set_query(letor_query);
    set_doc(letor_doc);
    set_database(letor_db);

    compute_termfreq();
    compute_inverse_doc_freq();
    compute_doc_length();
    compute_collection_length();
    compute_collection_termfreq();
}

void
FeatureList::Internal::compute_termfreq()
{
    std::map<std::string, Xapian::termcount> tf;

    Xapian::TermIterator docterms = featurelist_doc.termlist_begin();
    for (Xapian::TermIterator qt = featurelist_query.get_unique_terms_begin();
	 qt != featurelist_query.get_terms_end(); ++qt) {
	docterms.skip_to(*qt);
	if (docterms != featurelist_doc.termlist_end() && *qt == *docterms)
	    tf[*qt] = docterms.get_wdf();
    }
    std::swap(termfreq, tf);
}

void
FeatureList::Internal::compute_inverse_doc_freq()
{
    std::map<std::string, double> idf;

    for (Xapian::TermIterator qt = featurelist_query.get_unique_terms_begin();
	 qt != featurelist_query.get_terms_end(); ++qt) {
	Xapian::doccount totaldocs = featurelist_db.get_doccount();
	Xapian::doccount df = featurelist_db.get_termfreq(*qt);
	if (df != 0)
	    idf[*qt] = log10((double)totaldocs / (double)(1 + df));
    }
    std::swap(inverse_doc_freq, idf);
}

void
FeatureList::Internal::compute_doc_length()
{
    std::map<std::string, Xapian::termcount> len;

    Xapian::termcount title_len = 0;
    Xapian::termcount whole_len = 0;
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
    whole_len = featurelist_db.get_doclength(featurelist_doc.get_docid());
    len["whole"] = whole_len;
    len["body"] = whole_len - title_len;
    std::swap(doc_length, len);
}

void
FeatureList::Internal::compute_collection_length()
{
    std::map<std::string, Xapian::termcount> len;

    if (!featurelist_db.get_metadata("collection_len_title").empty() &&
	    !featurelist_db.get_metadata("collection_len_body").empty() &&
	    !featurelist_db.get_metadata("collection_len_whole").empty()) {
	len["title"] = atol(featurelist_db.get_metadata("collection_len_title").c_str());
	len["body"] = atol(featurelist_db.get_metadata("collection_len_body").c_str());
	len["whole"] = atol(featurelist_db.get_metadata("collection_len_whole").c_str());
    } else {
	Xapian::termcount title_len = 0;
	Xapian::termcount whole_len = 0;
	Xapian::TermIterator dt = featurelist_db.allterms_begin("S");
	for ( ; dt != featurelist_db.allterms_end("S"); ++dt) {
	    //  because we don't want the unique terms so we want their
	    // original frequencies and i.e. the total size of the title collection.
	    title_len += featurelist_db.get_collection_freq(*dt);
	}
	len["title"] = title_len;
	whole_len = featurelist_db.get_avlength() * featurelist_db.get_doccount();
	len["whole"] = whole_len;
	len["body"] = whole_len - title_len;
    }
    std::swap(collection_length, len);
}

void
FeatureList::Internal::compute_collection_termfreq()
{
    std::map<std::string, Xapian::termcount> tf;

    for (Xapian::TermIterator qt = featurelist_query.get_unique_terms_begin();
	 qt != featurelist_query.get_terms_end(); ++qt) {
	Xapian::termcount coll_tf = featurelist_db.get_collection_freq(*qt);
	if (coll_tf != 0)
	    tf[*qt] = coll_tf;
    }
    std::swap(collection_termfreq, tf);
}

void
FeatureList::Internal::populate_feature(Feature *feature_)
{
    stat_flags stats_needed = stat_flags(feature_->get_stats());
    if (stats_needed & TERM_FREQUENCY)
	feature_->set_termfreq(termfreq);
    if (stats_needed & INVERSE_DOCUMENT_FREQUENCY)
	feature_->set_inverse_doc_freq(inverse_doc_freq);
    if (stats_needed & DOCUMENT_LENGTH)
	feature_->set_doc_length(doc_length);
    if (stats_needed & COLLECTION_LENGTH)
	feature_->set_collection_length(collection_length);
    if (stats_needed & COLLECTION_TERM_FREQ)
	feature_->set_collection_termfreq(collection_termfreq);
}
