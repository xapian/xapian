/** @file colltfcolllenfeature.cc
 * @brief CollTfCollLenFeature class
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
CollTfCollLenFeature::name() const
{
    return "CollTfCollLenFeature";
}

vector<double>
CollTfCollLenFeature::get_values() const
{
    LOGCALL(API, vector<double>, "CollTfCollLenFeature::get_values", NO_ARGS);

    vector<double> values;
    double value = 0;
    double coll_len = 0;
    map<string, Xapian::termcount>::const_iterator coll_len_iterator =
	    collection_length.find("title");
    if (coll_len_iterator != collection_length.end())
	coll_len = (double)coll_len_iterator->second;
    else
	coll_len = 0;

    for (Xapian::TermIterator qt = feature_query.get_unique_terms_begin();
	 qt != feature_query.get_terms_end(); ++qt) {
	if ((*qt).substr(0, 1) == "S" || (*qt).substr(1, 1) == "S") {
	    map<string, Xapian::termcount>::const_iterator coll_tf_iterator =
		    collection_termfreq.find(*qt);
	    if (coll_tf_iterator != collection_termfreq.end()) {
		value += log10(1 + (coll_len /
				    (double)(1 + coll_tf_iterator->second)));
	    }
	    else
		value += log10(1 + coll_len);
	}
	else
	    value += 0;
    }
    values.push_back(value);
    value = 0;
    coll_len_iterator = collection_length.find("body");
    if (coll_len_iterator != collection_length.end())
	coll_len = (double)coll_len_iterator->second;
    else
	coll_len = 0;

    for (Xapian::TermIterator qt = feature_query.get_unique_terms_begin();
	 qt != feature_query.get_terms_end(); ++qt) {
	if ((*qt).substr(0, 1) != "S" && (*qt).substr(1, 1) != "S") {
	    map<string, Xapian::termcount>::const_iterator coll_tf_iterator =
		    collection_termfreq.find(*qt);
	    if (coll_tf_iterator != collection_termfreq.end()) {
		value += log10(1 + (coll_len /
				    (double)(1 + coll_tf_iterator->second)));
	    }
	    else
		value += log10(1 + coll_len);
	}
	else
	    value += 0;
    }
    values.push_back(value);
    value = 0;
    coll_len_iterator = collection_length.find("body");
    if (coll_len_iterator != collection_length.end())
	coll_len = (double)coll_len_iterator->second;
    else
	coll_len = 0;

    for (Xapian::TermIterator qt = feature_query.get_unique_terms_begin();
	 qt != feature_query.get_terms_end(); ++qt) {
	map<string, Xapian::termcount>::const_iterator coll_tf_iterator =
		collection_termfreq.find(*qt);
	if (coll_tf_iterator != collection_termfreq.end()) {
	    value += log10(1 + (coll_len /
				 (double)(1 + coll_tf_iterator->second)));
	}
	else
	    value += log10(1 + coll_len);
    }
    values.push_back(value);

    return values;
}

}
