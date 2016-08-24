/** @file tfidfdoclenfeature.cc
 * @brief TfIdfDoclenFeature class
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

#include "xapian-letor/feature.h"
#include "feature_internal.h"
#include "debuglog.h"

using namespace std;

namespace Xapian {

std::string
TfIdfDoclenFeature::name() const {
    return "TfIdfDoclenFeature";
}

vector<double>
TfIdfDoclenFeature::get_values() const {
    LOGCALL(API, std::vector<double>, "TfIdfDoclenFeature::get_values", NO_ARGS);

    Query query = Feature::internal->feature_query;
    map<string, long int> tf = Feature::internal->termfreq();
    map<string, long int> doc_len = Feature::internal->doc_length();
    map<string, double> idf = Feature::internal->inverse_doc_freq();

    vector<double> values;
    double value = 0;

    for (Xapian::TermIterator qt = query.get_terms_begin(); qt != query.get_terms_end(); ++qt) {
	if ((*qt).substr(0, 1) == "S" || (*qt).substr(1, 1) == "S")
	    value += log10(1 + ((double)(tf[*qt] * idf[*qt]) / (1 + (double)doc_len["title"])));
	else
	    value += 0;
    }
    values.push_back(value);
    value = 0;

    for (Xapian::TermIterator qt = query.get_terms_begin(); qt != query.get_terms_end(); ++qt) {
	if ((*qt).substr(0, 1) != "S" && (*qt).substr(1, 1) != "S")
	    value += log10(1 + ((double)(tf[*qt] * idf[*qt]) / (1 + (double)doc_len["body"])));
	else
	    value += 0;
    }
    values.push_back(value);
    value = 0;

    for (Xapian::TermIterator qt = query.get_terms_begin(); qt != query.get_terms_end(); ++qt) {
	value += log10(1 + ((double)(tf[*qt] * idf[*qt]) / (1 + (double)doc_len["whole"])));
    }
    values.push_back(value);

    return values;
}

}
