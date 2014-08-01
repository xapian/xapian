/* feature_vector.cc: FeatureVector is the feature-based representation of documents.
 *
 * Copyright (C) 2012 Parth Gupta
 * Copyright (C) 2014 Jiarong Wei
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
FeatureVector::set_index(Xapian::doccount index_) {
    index = index_;
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

Xapian::doccount
FeatureVector::get_index() {
    return index;
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
        cerr << "Feature index out of range.\n";
        exit(1);
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

Xapian::MSet::letor_item
FeatureVector::create_letor_item(Xapian::doccount idx) {
    Xapian::MSet::letor_item l_item;
    l_item.feature_vector = fvals;
    l_item.score = score;
    l_item.label = label;
    l_item.idx   = index;
    return l_item;
}

vector<FeatureVector>
FeatureVector::read_from_file(string file) {
    ifstream data_file;
    data_file.open(file);

    if (data_file.is_open()) {
        vector<FeatureVector> fvectors;
        string line;
        int num_feature = -1;

        while (getline(data_file, line)) {
            if (line.find("qid") != string::npos) {
                if (line.find("#") != string::npos) {
                    line = line.substr(0, line.find("#"));
                }

                line = StringHelper.trim(line);
                vector<string> s_vector = StringHelper.split(line);

                if (s_vector.size() > 2) {
                    FeatureVector fvector;
                    vector<double> fvals;

                    fvector.set_label = StringHelper.to_double(s_vector[0]);
                    // int qid = StringHelper.to_int(s_vector[1].substr(4, s_vector[1].size()-4));

                    for (int i = 2; i < s_vector.size(); ++i) {
                        int found = s_vector[i].find(":");
                        if (found != string::npos) {
                            // int f_idx = StringHelper.to_int(0, found);
                            double f_val = StringHelper.to_double(found+1, f_s_vector[i].size()-found-1);
                            fvals.push_back(f_val);
                        }
                        else {
                            cerr << "The file is broken!\n";
                            exit(1);
                        }
                    }

                    if (num_feature == -1) {
                        num_feature = fvals.size();
                    }
                    else if (num_feature != fvals.size()) {
                        cout << "The number of features is not compatible!\n";
                        exit(1);
                    }

                    fvector.set_feature_values(fvals);

                    fvectors.push_back(fvector);
                }
            }
        }
        return fvectors;
    }
    else {
        cerr << "Can't open file " << file << '\n';
        exit(1);
    }
}

static vector<double>
FeatureVector::extract(const vector<FeatureVector> & fvectors, double relevance, int f_idx) {
    vector<double> fvals;
    for (vector<FeatureVector>::iterator it = fvectors.begin(); it != fvectors.end(); ++it) {
        if (relevance == it->get_label()) {
            if (f_idx < 1 || f_idx > it->get_feature_num()) {
                cerr << "The feature doesn't exist!\n";
                exit(1);
            }
            fvals.push_back( it->get_feature_value_of(f_idx) );
        }
    }
}