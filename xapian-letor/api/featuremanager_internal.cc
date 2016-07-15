/** @file featuremanager_internal.cc
 * @brief Internals of FeatureManager class
 */
/* Copyright (C) 2011 Parth Gupta
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

#include "xapian-letor/featuremanager.h"
#include "xapian-letor/featurevector.h"
#include "featuremanager_internal.h"

#include <cstring>
#include <cstdlib>
#include <fstream>

#include <map>
#include <string>


using namespace Xapian;
using namespace std;

struct FileNotFound { };

void
FeatureManager::Internal::normalise(std::vector<FeatureVector> & fvec) {

    // find the max value for each feature gpr all the FeatureVectors in the vector.
    int num_features = 19;
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
            temp = fvec[i].get_fvals()[j];
            temp /= max[j];
            fvec[i].set_feature_value(j, temp);
        }
    }
}

std::vector<FeatureVector>
FeatureManager::Internal::create_feature_vectors(const Xapian::MSet & mset)
{
    std::vector<FeatureVector> fvec;

    for (Xapian::MSetIterator i = mset.begin(); i != mset.end(); ++i) {

        Xapian::Document doc = i.get_document();

        // Here a weight vector can be created in future for different
        // weights of the document like BM25, LM etc.
        double weight = i.get_weight();

        map<int,double> fVals = transform(doc, weight);

        Xapian::docid did = doc.get_docid();

        Xapian::FeatureVector fv = create_feature_vector(fVals, did);

        fvec.push_back(fv);

    }
    normalise(fvec);
    return fvec;
}

Xapian::FeatureVector
FeatureManager::Internal::create_feature_vector(map<int,double> fvals, Xapian::docid & did)
{
    Xapian::FeatureVector fv;
    fv.set_did(did);
    fv.set_fvals(fvals);

    return fv;
}

std::map<int,double>
FeatureManager::Internal::transform(const Document &doc, double &weight_)
{
    map<int, double> fvals;
    map<string,long int> tf = f.termfreq(doc, letor_query);
    map<string, long int> doclen = f.doc_length(letor_db, doc);

    double val[20];// = new double[fCount+1];

    // storing the feature values from array index 1 to sync it with feature number.
    val[1]=f.calculate_f1(letor_query,tf,'t');
    val[2]=f.calculate_f1(letor_query,tf,'b');
    val[3]=f.calculate_f1(letor_query,tf,'w');

    val[4]=f.calculate_f2(letor_query,tf,doclen,'t');
    val[5]=f.calculate_f2(letor_query,tf,doclen,'b');
    val[6]=f.calculate_f2(letor_query,tf,doclen,'w');

    val[7]=f.calculate_f3(letor_query,idf,'t');
    val[8]=f.calculate_f3(letor_query,idf,'b');
    val[9]=f.calculate_f3(letor_query,idf,'w');

    val[10]=f.calculate_f4(letor_query,coll_tf,coll_len,'t');
    val[11]=f.calculate_f4(letor_query,coll_tf,coll_len,'b');
    val[12]=f.calculate_f4(letor_query,coll_tf,coll_len,'w');

    val[13]=f.calculate_f5(letor_query,tf,idf,doclen,'t');
    val[14]=f.calculate_f5(letor_query,tf,idf,doclen,'b');
    val[15]=f.calculate_f5(letor_query,tf,idf,doclen,'w');

    val[16]=f.calculate_f6(letor_query,tf,doclen,coll_tf,coll_len,'t');
    val[17]=f.calculate_f6(letor_query,tf,doclen,coll_tf,coll_len,'b');
    val[18]=f.calculate_f6(letor_query,tf,doclen,coll_tf,coll_len,'w');

// this weight can be either set on the outside how it is done right now
// or, better, extend Enquiry to support advanced ranking models
    val[19]=weight_;

    for(int i=0; i<=fNum;i++)
        fvals.insert(pair<int,double>(i,val[i]));

    return fvals;
}

void
FeatureManager::Internal::update_collection_level() {
    coll_len = f.collection_length(letor_db);
}

void
FeatureManager::Internal::update_query_level() {
    coll_tf = f.collection_termfreq(letor_db, letor_query);
    idf = f.inverse_doc_freq(letor_db, letor_query);
}
