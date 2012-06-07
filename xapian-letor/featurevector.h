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
    double label;
    std::map<int,double> fvals;
    int fcount;

  public:

    FeatureVector(const FeatureVector & o);
    
    virtual ~FeatureVector() {};

    /** This method takes the document from the MSet as input and gives the feature representation
     * as vector in the form of 
     * map<int,double>
     */
    std::map<int,double> transform(const Xapian::Document & doc);


    map<string, map<string, int> > load_relevance(const std::string & qrel_file);

    void set_qid(const std::string & qid);

    void set_did(const std::string & did);
    
};

}
#endif /* FEATURE_VECTOR_H */
