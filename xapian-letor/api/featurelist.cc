/** @file featurelist.cc
 *  @brief Definition of FeatureList class
 */
/* Copyright (C) 2016 Ayush Tomar
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

#include "xapian-letor/featurelist.h"
#include "xapian-letor/feature.h"
#include "xapian-letor/featurevector.h"
#include "debuglog.h"

#include <iostream>

using namespace std;

namespace Xapian {

FeatureList::FeatureList() {

    LOGCALL_CTOR(API, "FeatureList", NO_ARGS);
    feature.push_back(new TfFeature());
    feature.push_back(new TfDoclenFeature());
    feature.push_back(new IdfFeature());
    feature.push_back(new CollTfCollLenFeature());
    feature.push_back(new TfIdfDoclenFeature());
    feature.push_back(new TfDoclenCollTfCollLenFeature());
}

FeatureList::FeatureList(const std::vector<Feature*> & f) {

    LOGCALL_CTOR(API, "FeatureList", f);
    feature = f;
}

FeatureList::~FeatureList() {
    LOGCALL_DTOR(API, "FeatureList");
    for (std::vector<Feature*>::iterator it = feature.begin() ; it != feature.end(); ++it) {
	delete (*it);
    }
    feature.clear();
}

void
FeatureList::normalise(std::vector<FeatureVector> & fvec) const {

    LOGCALL_VOID(API, "FeatureList::normalise", fvec);
    // find the max value for each feature gpr all the FeatureVectors in the vector.
    int num_features = fvec[0].get_fcount();
    double temp = 0.0;
    double max[num_features];

    for(int i=0; i<num_features; ++i)
	max[i] = 0.0;

    int num_fv = fvec.size();
    for(int i=0; i < num_fv; ++i) {
	for(int j=0; j<num_features; ++j) {
	    double fval = fvec[i].get_fvals()[j];
	    if (max[j] < fval)
		max[j] = fval;
	}
    }

    /* We have the maximum value of each feature overall.
       Now we need to normalize each feature value of a
       featureVector by dividing it by the corresponding max of the feature value
    */

    for(int i=0; i < num_fv; ++i) {
	for(int j=0; j<num_features; ++j) {
	    temp = fvec[i].get_feature_value(j);
	    temp /= max[j];
	    if (max[j] == 0) // Skip if dividing by zero
		continue;
	    fvec[i].set_feature_value(j, temp);
	}
    }
}

std::vector<FeatureVector>
FeatureList::create_feature_vectors(const Xapian::MSet & mset, const Xapian::Query & letor_query,
				    const Xapian::Database & letor_db) const
{
    LOGCALL(API, std::vector<FeatureVector>, "FeatureList::create_feature_vectors", mset | letor_query | letor_db);
    std::vector<FeatureVector> fvec;

    for (Xapian::MSetIterator i = mset.begin(); i != mset.end(); ++i) {

	Xapian::Document doc = i.get_document();

	std::vector<double> fVals;

	for (std::vector<Feature*>::const_iterator it = feature.begin() ; it != feature.end(); ++it) {

	    (*it)->set_database(letor_db);
	    (*it)->set_query(letor_query);
	    (*it)->set_doc(doc);

	    vector<double> values = (*it)->get_values();
	    fVals.insert(fVals.end(), values.begin(), values.end()); // Append feature values

	}

	double wt = i.get_weight();

	/// Weight is added as a feature by default.
	fVals.push_back(wt);

	Xapian::docid did = doc.get_docid();

	Xapian::FeatureVector fv(did, fVals); // construct a FeatureVector object using did & fVals

	fvec.push_back(fv);

    }
    normalise(fvec);
    return fvec;
}

}
