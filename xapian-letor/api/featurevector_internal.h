/* featurevector_internal.h: Internals of FeatureVector class.
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

#ifndef FEATURE_VECTOR_INTERNAL_H
#define FEATURE_VECTOR_INTERNAL_H

#include <config.h>

#include <xapian.h>

#include "xapian-letor/featurevector.h"
#include "xapian-letor/featuremanager.h"

#include <list>
#include <map>

#include "str.h"
#include "stringutils.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "safeerrno.h"
#include "safeunistd.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>


using namespace std;

namespace Xapian{

class FeatureVector::Internal : public Xapian::Internal::intrusive_base
{
    friend class FeatureVector;
    double label;
    double score;
    std::map<int,double> fvals;
    int fcount;
    Xapian::docid did;

  public:
    map<string, map<string, int> > load_relevance(const std::string & qrel_file);
    double get_score();
    double get_label();
    std::map<int,double> get_fvals();
    Xapian::docid get_did();
    double get_feature_value(int index);
    int get_nonzero_num();

};

}

#endif // FEATURE_VECTOR_INTERNAL_H

