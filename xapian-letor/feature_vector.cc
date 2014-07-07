
/* featurevector.cc: The file responsible for transforming the document into the feature space.
 *
 * Copyright (C) 2012 Parth Gupta
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

#include <xapian.h>
#include <xapian/intrusive_ptr.h>
#include <xapian/types.h>
#include <xapian/visibility.h>

#include "featurevector.h"
 
#include <vector>

#include "str.h"
#include "stringutils.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "safeerrno.h"
#include "safeunistd.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>


using std::string;
using std::vector;

using namespace Xapian;

FeatureVector::FeatureVector() {}

FeatureVector::FeatureVector(const FeatureVector & o) {}

void
FeatureVector::set_did(string did_) {
    did = did_;
}

void
FeatureVector::set_score(double score_) {
    score = score_;
}

void
FeatureVector::set_label(double label_) {
    label = label_;
}

void
FeatureVector::set_feature_values(vector<double> feature_values_) {
    feature_values = feature_values_;
}

string
FeatureVector::get_did() {
    return did;
}

double
FeatureVector::get_score() {
    return score;
}

double
FeatureVector::get_label() {
    return label;
}

int
FeatureVector::get_feature_num() {
    return feature_values.size();
}

vector<double>
FeatureVector::get_feature_values() {
    return feature_values;
}

double
FeatureVector::get_feature_value_of(int idx) {
    if (idx < 1 || idx > get_feature_num()) {
        printf("Feature index out of range.\n");
        return 1;
    }
    return feature_values[idx-1];
}

vector<double>
FeatureVector::get_label_feature_values() {
    vector<doubel> new_vector;
    new_vector.push_back(label);
    new_vector.insert(new_vector.end(), feature_values.begin(), feature_values.end());
    return new_vector;
}

vector<double>
FeatureVector::get_score_feature_values() {
    vector<doubel> new_vector;
    new_vector.push_back(score);
    new_vector.insert(new_vector.end(), feature_values.begin(), feature_values.end());
    return new_vector;
}

string
FeatureVector::get_feature_values_text() {
    string txt = "1:" + str(feature_values[0])
    for (int idx = 2; idx < get_feature_num(); ++idx) {
        txt.append(" " + str(idx) + ":" + str(feature_values[idx]));
    }
    return txt;
}

string
FeatureVector::get_label_feature_values_text() {
    string txt = str(label) + " ";
    txt.append( get_feature_values_text() );
    return txt;
}

string
FeatureVector::get_score_feature_values_text() {
    string txt = str(score) + " ";
    txt.append( get_feature_values_text() );
    return txt;
}