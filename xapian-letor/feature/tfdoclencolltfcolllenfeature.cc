/** @file tfdoclencolltfcolllenfeature.cc
 * @brief TfDoclenCollTfCollLenFeature class
 */
/* Copyright (C) 2012 Parth Gupta
 * Copyright (C) 2016 Ayush Tomar
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include <config.h>

#include "xapian-letor/feature.h"

#include "debuglog.h"

using namespace std;

namespace Xapian {

string
TfDoclenCollTfCollLenFeature::name() const
{
    return "TfDoclenCollTfCollLenFeature";
}

vector<double>
TfDoclenCollTfCollLenFeature::get_values() const
{
    LOGCALL(API, vector<double>, "TfDoclenCollTfCollLenFeature::get_values", NO_ARGS);

    vector<double> values;
    double value = 0;
    double coll_len;
    double doc_len;
    auto coll_len_iterator = collection_length.find("title");
    if (coll_len_iterator != collection_length.end())
	coll_len = (double)coll_len_iterator->second;
    else
	coll_len = 0;
    auto doc_len_iterator = doc_length.find("title");
    if (doc_len_iterator != doc_length.end())
	doc_len = (double)doc_len_iterator->second;
    else
	doc_len = 0;

    for (TermIterator qt = feature_query.get_unique_terms_begin();
	 qt != feature_query.get_terms_end(); ++qt) {
	if (is_title_term((*qt))) {
	    double tf;
	    double coll_tf;
	    auto tf_iterator = termfreq.find(*qt);
	    auto coll_tf_iterator = collection_termfreq.find(*qt);
	    if (tf_iterator != termfreq.end())
		tf = (double)tf_iterator->second;
	    else
		tf = 0;
	    if (coll_tf_iterator != collection_termfreq.end())
		coll_tf = (double)coll_tf_iterator->second;
	    else
		coll_tf = 0;
	    value += log10(1 + ((tf * coll_len) / (1 + (doc_len * coll_tf))));
	} else {
	    value += 0;
	}
    }
    values.push_back(value);
    value = 0;
    coll_len_iterator = collection_length.find("body");
    if (coll_len_iterator != collection_length.end())
	coll_len = (double)coll_len_iterator->second;
    else
	coll_len = 0;
    doc_len_iterator = doc_length.find("body");
    if (doc_len_iterator != doc_length.end())
	doc_len = (double)doc_len_iterator->second;
    else
	doc_len = 0;

    for (Xapian::TermIterator qt = feature_query.get_unique_terms_begin();
	 qt != feature_query.get_terms_end(); ++qt) {
	if (!is_title_term((*qt))) {
	    double tf;
	    double coll_tf;
	    auto tf_iterator = termfreq.find(*qt);
	    auto coll_tf_iterator = collection_termfreq.find(*qt);
	    if (tf_iterator != termfreq.end())
		tf = (double)tf_iterator->second;
	    else
		tf = 0;
	    if (coll_tf_iterator != collection_termfreq.end())
		coll_tf = (double)coll_tf_iterator->second;
	    else
		coll_tf = 0;
	    value += log10(1 + ((tf * coll_len) / (1 + (doc_len * coll_tf))));
	} else {
	    value += 0;
	}
    }
    values.push_back(value);
    value = 0;
    coll_len_iterator = collection_length.find("whole");
    if (coll_len_iterator != collection_length.end())
	coll_len = (double)coll_len_iterator->second;
    else
	coll_len = 0;
    doc_len_iterator = doc_length.find("whole");
    if (doc_len_iterator != doc_length.end())
	doc_len = (double)doc_len_iterator->second;
    else
	doc_len = 0;

    for (Xapian::TermIterator qt = feature_query.get_unique_terms_begin();
	 qt != feature_query.get_terms_end(); ++qt) {
	double tf;
	double coll_tf;
	auto tf_iterator = termfreq.find(*qt);
	auto coll_tf_iterator = collection_termfreq.find(*qt);
	if (tf_iterator != termfreq.end())
	    tf = (double)tf_iterator->second;
	else
	    tf = 0;
	if (coll_tf_iterator != collection_termfreq.end())
	    coll_tf = (double)coll_tf_iterator->second;
	else
	    coll_tf = 0;
	value += log10(1 + ((tf * coll_len) / (1 + (doc_len * coll_tf))));
    }
    values.push_back(value);

    return values;
}

}
