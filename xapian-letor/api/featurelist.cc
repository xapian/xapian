/** @file
 *  @brief Definition of FeatureList class
 */
/* Copyright (C) 2016 Ayush Tomar
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
#include "xapian-letor/feature.h"
#include "xapian-letor/featurevector.h"
#include "featurelist_internal.h"
#include "feature_internal.h"

#include "debuglog.h"
#include "omassert.h"

using namespace std;

namespace Xapian {

FeatureList::FeatureList() : internal(new FeatureList::Internal())
{
    LOGCALL_CTOR(API, "FeatureList", NO_ARGS);
    internal->feature.push_back(new TfFeature());
    internal->feature.push_back(new TfDoclenFeature());
    internal->feature.push_back(new IdfFeature());
    internal->feature.push_back(new CollTfCollLenFeature());
    internal->feature.push_back(new TfIdfDoclenFeature());
    internal->feature.push_back(new TfDoclenCollTfCollLenFeature());
    for (Feature* it : internal->feature) {
	internal->stats_needed = Internal::stat_flags(internal->stats_needed |
						      it->get_stats());
    }
}

FeatureList::FeatureList(const std::vector<Feature*> & f)
{
    LOGCALL_CTOR(API, "FeatureList", f);
    if (f.empty()) {
	throw InvalidArgumentError("FeatureList cannot be empty");
    }
    internal = new Internal();
    internal->feature = f;
    for (Feature* it : internal->feature) {
	internal->stats_needed = Internal::stat_flags(internal->stats_needed |
						      it->get_stats());
    }
}

FeatureList::~FeatureList()
{
    LOGCALL_DTOR(API, "FeatureList");
    for (Feature* it : internal->feature)
	delete it;
    internal->feature.clear();
}

void
FeatureList::normalise(std::vector<FeatureVector>& fvec) const
{
    LOGCALL_VOID(API, "FeatureList::normalise", fvec);
    int num_features = fvec[0].get_fcount();
    for (int j = 0; j < num_features; ++j) {
	// Find the maximum value of this feature.
	double max_fval = 0.0;
	for (const auto& v : fvec) {
	    max_fval = max(max_fval, v.get_fvals()[j]);
	}

	if (max_fval == 0.0) {
	    // Skip scaling step if we'd divide by zero.
	    continue;
	}

	// Scale all values of this feature such that the max is 1.
	for (auto& v : fvec) {
	    v.set_feature_value(j, v.get_feature_value(j) / max_fval);
	}
    }
}

std::vector<FeatureVector>
FeatureList::create_feature_vectors(const Xapian::MSet & mset,
				    const Xapian::Query & letor_query,
				    const Xapian::Database & letor_db) const
{
    LOGCALL(API, std::vector<FeatureVector>, "FeatureList::create_feature_vectors", mset | letor_query | letor_db);
    if (mset.empty())
	return vector<FeatureVector>();
    std::vector<FeatureVector> fvec;
    Assert(!internal->feature.empty());

    for (Xapian::MSetIterator i = mset.begin(); i != mset.end(); ++i) {
	Xapian::Document doc = i.get_document();
	std::vector<double> fvals;
	internal->set_data(letor_query, letor_db, doc);
	auto internal_feature = new Feature::Internal(letor_db,
						      letor_query, doc);
	// Computes and populates the Feature::Internal with required stats.
	internal->populate_feature_internal(internal_feature);
	for (Feature* it : internal->feature) {
	    it->internal = internal_feature;
	    const vector<double>& values = it->get_values();
	    // Append feature values
	    fvals.insert(fvals.end(), values.begin(), values.end());
	}
	double wt = i.get_weight();
	// Weight is added as a feature by default.
	fvals.push_back(wt);
	Xapian::docid did = doc.get_docid();
	// construct a FeatureVector object using did and fvals.
	Xapian::FeatureVector fv(did, fvals);
	fvec.push_back(fv);
    }
    normalise(fvec);
    return fvec;
}

}
