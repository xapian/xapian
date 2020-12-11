/** @file
 * @brief CollTfCollLenFeature class
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
#include "api/feature_internal.h"

#include "debuglog.h"
#include "stringutils.h"

using namespace std;

namespace Xapian {

string
CollTfCollLenFeature::name() const
{
    return "CollTfCollLenFeature";
}

/** A helper function for feature->get_value()
 *
 *  Checks if the term belongs to the title or is stemmed from the title.
 */
static inline bool
is_title_term(const std::string& term)
{
    return startswith(term, 'S') || startswith(term, "ZS");
}

vector<double>
CollTfCollLenFeature::get_values() const
{
    LOGCALL(API, vector<double>, "CollTfCollLenFeature::get_values", NO_ARGS);

    vector<double> values;
    double value = 0;
    double coll_len = internal->get_collection_length("title");

    Xapian::Query feature_query = internal->get_query();
    for (Xapian::TermIterator qt = feature_query.get_unique_terms_begin();
	 qt != feature_query.get_terms_end(); ++qt) {
	if (is_title_term((*qt))) {
	    double tf = internal->get_collection_termfreq(*qt);
	    value += log10(1 + (coll_len / (1 + tf)));
	}
    }
    values.push_back(value);
    value = 0;
    coll_len = internal->get_collection_length("body");

    for (Xapian::TermIterator qt = feature_query.get_unique_terms_begin();
	 qt != feature_query.get_terms_end(); ++qt) {
	if (!is_title_term((*qt))) {
	    double tf = internal->get_collection_termfreq(*qt);
	    value += log10(1 + (coll_len / (1 + tf)));
	}
    }
    values.push_back(value);
    value = 0;
    coll_len = internal->get_collection_length("whole");

    for (Xapian::TermIterator qt = feature_query.get_unique_terms_begin();
	 qt != feature_query.get_terms_end(); ++qt) {
	double tf = internal->get_collection_termfreq(*qt);
	value += log10(1 + (coll_len / (1 + tf)));
    }
    values.push_back(value);

    return values;
}

}
