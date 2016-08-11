/** @file featuremanager.h
 *  @brief The feature manager file
 */
/* Copyright (C) 2012 Parth Gupta
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

#ifndef FEATURE_MANAGER_H
#define FEATURE_MANAGER_H

#include <xapian.h>
#include <xapian/intrusive_ptr.h>
#include <xapian/types.h>
#include <xapian/visibility.h>

#include "letor.h"
#include "letor_features.h"
#include "featurevector.h"

#include <map>
#include <string>
#include <vector>

using namespace std;

namespace Xapian {

class FeatureVector;

class XAPIAN_VISIBILITY_DEFAULT FeatureManager {

  public:
    /// @private @internal Class representing the FeatureManager internals.
    class Internal;
    /// @private @internal Reference counted internals.
    Xapian::Internal::intrusive_ptr<Internal> internal;

    /// Default constructor
    FeatureManager();

    /// Copy constructor
    FeatureManager(const FeatureManager & o);

    /// Assignment
    FeatureManager & operator=(const FeatureManager & o);

    /// Destructor
    ~FeatureManager();

    /** Calculates features array for a each document paired with a query
     *  @return features array of length 19 in <int, double> format
     */
    std::map<int,double> transform(const Document &doc, double &weight_);

    /** Returns a vector of FeatureVectors for each document in the MSet for a given query.
     *
     *  @ param  mset      MSet for which the vector<FeatureVector> is to be returned
     */
    std::vector<Xapian::FeatureVector> create_feature_vectors(const Xapian::MSet & mset);

    /** Creates instance of FeatureVector class that stores features_array, relevance label and corresponding document id
     *  @return a FeatureVector instance containing features_array, relevance label and corresponding document id
     */
    Xapian::FeatureVector create_feature_vector(map<int,double> fvals, Xapian::docid & did);

    static const int fNum = 19;

    /// Specify the database to use for retrieval. This database will be used directly by internal class.
    void set_database(const Database &db);

    /// Specify the query. This will be used by the internal class.
    void set_query(const Query &query);

};

}

#endif // FEATURE_MANAGER_H
