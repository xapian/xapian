/* featuremanager.h: The feature manager file.
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

#ifndef FEATURE_MANAGER_H
#define FEATURE_MANAGER_H

#include <xapian.h>
#include <xapian/intrusive_ptr.h>
#include <xapian/types.h>
#include <xapian/visibility.h>

#include "letor_features.h"
#include "featurevector.h"
#include "ranklist.h"

#include <map>
#include <string>

using namespace std;

namespace Xapian {

class Features;
class RankList;//WHY???
class FeatureVector;//WHY???

class XAPIAN_VISIBILITY_DEFAULT FeatureManager {

public:

    Xapian::Features f;

    FeatureManager();
    
    std::map<int,double> transform(const Document &doc, double &weight);

    Xapian::RankList create_rank_list(const Xapian::MSet & mset, std::string & qid);
    Xapian::RankList create_rank_list_from_training_file(const Xapian::MSet & mset, std::string & qid);

    map<string, map<string,int> > load_relevance(const std::string & qrel_file);    
    
    Xapian::FeatureVector create_feature_vector(map<int,double> fvals, int &label, std::string & did);
    
    std::string get_did(const Document &doc);
    
    int get_label(map<string, map<string, int> > qrel, const Document &doc, std::string & qid);

    static const int fNum = 20;

    virtual ~FeatureManager() {};

    // accessors
    inline void set_database(const Database &db) { letor_db = db; update_collection_level();}
    inline const Database &get_database() const { return letor_db; }
    inline void set_query(const Query &query) { letor_query = query; update_query_level();}
    inline const Query &get_query() const { return letor_query; }

    // fill "dest" with the corresponding values NOTE: dest should have size NUM_FEATURES (vector safer?)
    // TODO: the definition of this should be generated (or the whole class inherited from this one)
//    void compute(const Document &doc, double *dest);



private:
    Database letor_db;
    Query letor_query;

    map<string,long int> coll_len;
    map<string,long int> coll_tf;
    map<string,double> idf;

    map<string, map<string, int> > qrel;

    // update collection-level measures
    void update_collection_level();

    // update query-level measures
    void update_query_level();

};

}



#endif // FEATURE_MANAGER_H
