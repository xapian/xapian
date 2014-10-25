/* featurevector.h: The file responsible for transforming the document into the feature space.
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

#ifndef FEATURE_VECTOR_H
#define FEATURE_VECTOR_H


#include <xapian.h>
#include <xapian/base.h>
#include <xapian/types.h>
#include <xapian/visibility.h>

#include "featuremanager.h"

#include <list>
#include <map>

using namespace std;

namespace Xapian {

class XAPIAN_VISIBILITY_DEFAULT FeatureVector {

  public:
  
    double label;
    double score;
    std::map<int,double> fvals;
    int fcount;
    string did;

    FeatureVector();

    FeatureVector(const FeatureVector & o);
    
    virtual ~FeatureVector() {};

    /** This method takes the document from the MSet as input and gives the feature representation
     * as vector in the form of 
     * map<int,double>
     */

    static bool before( const Xapian::FeatureVector& c1, const Xapian::FeatureVector& c2 ) { return c1.score < c2.score; }

    map<string, map<string, int> > load_relevance(const std::string & qrel_file);

    void set_did(const std::string & did1);
    
    void set_label(double label1);
    void set_fvals(map<int,double> fvals1);
    double get_score();
    void set_score(double score1);
    std::map<int,double> get_fvals();
    double get_feature_value(int index);
    
};

}
#endif /* FEATURE_VECTOR_H */
