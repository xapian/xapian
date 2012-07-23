/* ranklist.h: The ranklist -- list of feature vectors file.
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

#ifndef RANKLIST_H
#define RANKLIST_H


#include <xapian.h>
#include <xapian/base.h>
#include <xapian/types.h>
#include <xapian/visibility.h>

#include <featurevector.h>

#include <list>
#include <map>
#include <vector>

using namespace std;


namespace Xapian {

class FeatureVector;

class XAPIAN_VISIBILITY_DEFAULT RankList {
    

  public:
  
    std::vector<FeatureVector> rl;
    
    std::string qid;
    RankList();
    
    void set_qid(std::string qid1);
    
    void set_rl(std::vector<FeatureVector> local_rl);

    void add_feature_vector(const Xapian::FeatureVector fv);//was & fv initially,check back later

    void normalise();
    
    std::vector<FeatureVector> sort_by_score();
    
    std::vector<FeatureVector> get_data();

};

}
#endif /* RANKLIST_H */
