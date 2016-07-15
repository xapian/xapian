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
#include <xapian/intrusive_ptr.h>
#include <xapian/types.h>
#include <xapian/visibility.h>

#include "featuremanager.h"

#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <list>
#include <map>

using namespace std;


namespace Xapian {

class XAPIAN_VISIBILITY_DEFAULT FeatureVector { //TODO: Update documentation

  public:
    /// @private @internal Class representing the FeatureVector internals.
    class Internal;
    /// @private @internal Reference counted internals.
    Xapian::Internal::intrusive_ptr<Internal> internal;

    /// Default constructor.
    FeatureVector();

    /// Copy constructor.
    FeatureVector(const FeatureVector & o);

    /// Assignment.
    FeatureVector & operator=(const FeatureVector & o);

    /// Destructor.
    ~FeatureVector();

    static bool before(const Xapian::FeatureVector& c1, const Xapian::FeatureVector& c2);

    map<string, map<string, int> > load_relevance(const std::string & qrel_file);

    //TODO: Update documentation
    void set_did(const Xapian::docid & did1);
    void set_fcount(int fcount1);
    void set_label(double label1);
    void set_fvals(map<int,double> & fvals1);
    void set_score(double score1);
    void set_feature_value(int index, double value);
    int get_fcount();
    double get_score() const;
    double get_label() const;
    Xapian::docid get_did();
    std::map<int,double> get_fvals();
    double get_feature_value(int index);
    int get_nonzero_num();

};

}
#endif /* FEATURE_VECTOR_H */
