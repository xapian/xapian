/* featurevector_internal.cc: Internals of FeatureVector Class
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

#include <config.h>
#include <xapian.h>

#include "xapian-letor/featurevector.h"
#include "xapian-letor/featuremanager.h"
#include "featurevector_internal.h"

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
using namespace Xapian;


map<string, map<string, int> >
FeatureVector::Internal::load_relevance(const std::string & qrel_file)
{
    map<string, map<string, int>> qrel;     // < qid, <docid, relevance_judgement> >

    string inLine;
    ifstream myfile(qrel_file.c_str(), ifstream::in);
    string token[4];
    if (myfile.is_open()) {
    while (myfile.good()) {
        getline(myfile, inLine);        // read a file line by line
        char * str;
        char * x1;
        x1 = const_cast<char*>(inLine.c_str());
        str = strtok(x1, " ,.-");
        int i = 0;
        while (str != NULL) {
        token[i] = str;     // store tokens in a string array
        ++i;
        str = strtok(NULL, " ,.-");
        }

        qrel[token[0]].insert(make_pair(token[2], atoi(token[3].c_str())));
    }
    myfile.close();
    }
    return qrel;
}

double
FeatureVector::Internal::get_score() {
    return score;
}

double
FeatureVector::Internal::get_label() {
    return label;
}

std::map<int,double>
FeatureVector::Internal::get_fvals() {
    return fvals;
}

Xapian::docid
FeatureVector::Internal::get_did() {
    return did;
}

double
FeatureVector::Internal::get_feature_value(int index) {
    map<int,double>::const_iterator iter;
    iter = fvals.find(index);
    if(iter == fvals.end())
    return 0;
    else
    return (*iter).second;
}

int
FeatureVector::Internal::get_nonzero_num(){
    int nonzero = 0;
    int fvalsize=fvals.size();
    for(int i = 1; i <= fvalsize; ++i){
        if(fvals[i] != 0){
            //cout << "index: "<< i << "fvals in fv: " <<fvals[i]<< endl;
            nonzero++;
        }
    }
    //cout << "nonzero: "<< nonzero <<endl;
    return nonzero;
}
