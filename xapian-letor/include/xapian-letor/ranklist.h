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
#include <xapian/intrusive_ptr.h>
#include <xapian/types.h>
#include <xapian/visibility.h>

#include "featurevector.h"

#include <list>
#include <map>
#include <iostream>
#include <vector>

using namespace std;

namespace Xapian {

class FeatureVector;

class XAPIAN_VISIBILITY_DEFAULT RankList { //TODO: Update documentation

  public:

    std::vector<FeatureVector> fvv;

    std::string qid;

    RankList();

    void set_qid(std::string qid1);

    std::string get_qid();

    void set_fvv(std::vector<FeatureVector> & local_fvv);

    void add_feature_vector(const Xapian::FeatureVector fv);

    std::vector<FeatureVector> normalise();

    bool fvcompare(FeatureVector fv1, FeatureVector fv2);

    void sort_by_score();

    void sort_by_label();

    std::vector<FeatureVector> get_fvv();

};

}
#endif /* RANKLIST_H */
