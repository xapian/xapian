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
#include "ranklist.h"

#include <map>
#include <string>

using namespace std;

namespace Xapian {

class RankList;
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

    /** Calculates ranklist (0 qid:10032 1:0.130742 2:0.000000 ... 18:0.750000 19:1.000000 #docid = 1123323)
     *  for an mset retrieved from a query. Feature 19 is the BM25 weight.
     *
     *  @ param  train      Set to 1 while training.
     */
    Xapian::RankList create_rank_list(const Xapian::MSet & mset, std::string & qid, bool train);

    /** Loads up a qrel file.
     *  @return a map: < qid <docid, relevance_judgement> >
     */
    map<string, map<string,int> > load_relevance(const std::string & qrel_file);

    /** Creates instance of FeatureVector class that stores features_array, relevance label and corresponding document id
     *  @return a FeatureVector instance containing features_array, relevance label and corresponding document id
     */
    Xapian::FeatureVector create_feature_vector(map<int,double> fvals, int &label, Xapian::docid & did);

    /** Creates instance of FeatureVector class that stores features_array, relevance label and corresponding document id
     *  @return a FeatureVector instance containing features_array, relevance label and corresponding document id
     */
    std::string getdid(const Document &doc);

    /** Get relvance label corresponding to a document (docid) and query (qid) in qrel
     *  @return Relavance label corresponding to docid and qid in qrel
     */
    int getlabel(map<string, map<string, int> > qrel, const Document &doc, std::string & qid);

    static const int fNum = 19;

    /// Specify the database to use for retrieval. This database will be used directly by internal class.
    void set_database(const Database &db);

    /// Specify the query. This will be used by the internal class.
    void set_query(const Query &query);

};

}

#endif // FEATURE_MANAGER_H
