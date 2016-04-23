/* featuremanager_internal.h: The feature manager internal file.
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

#ifndef FEATURE_MANAGER_INTERNAL_H
#define FEATURE_MANAGER_INTERNAL_H

#include <xapian.h>

#include "xapian-letor/featuremanager.h"
#include "xapian-letor/letor.h"
#include "xapian-letor/letor_features.h"
#include "xapian-letor/featurevector.h"
#include "xapian-letor/ranklist.h"

#include <map>
#include <string>

using namespace std;

namespace Xapian {

class RankList;
class FeatureVector;

class FeatureManager::Internal : public Xapian::Internal::intrusive_base
{
    friend class FeatureManager;

    Xapian::Features f;

    Database letor_db;
    Query letor_query;

    map<string,long int> coll_len;
    map<string,long int> coll_tf;
    map<string,double> idf;

    map<string, map<string, int> > qrel;

  public:

    std::map<int,double> transform(const Document &doc, double &weight_);

    Xapian::RankList create_rank_list(const Xapian::MSet & mset,std::string & qid);

    map<string, map<string,int> > load_relevance(const std::string & qrel_file);

    Xapian::FeatureVector create_feature_vector(map<int,double> fvals, int &label, std::string & did);

    std::string getdid(const Document &doc);

    int getlabel(map<string, map<string, int> > qrel, const Document &doc, std::string & qid);

    static const int fNum = 20;

  private:
    // update collection-level measures
    void update_collection_level();

    // update query-level measures
    void update_query_level();

};

}

#endif // FEATURE_MANAGER_INTERNAL_H
