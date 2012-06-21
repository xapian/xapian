/* featurevector.cc: The file responsible for transforming the document into the feature space.
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
#include <xapian/base.h>
#include <xapian/types.h>
#include <xapian/visibility.h>

#include "featurevector.h"
#include "featuremanager.h"

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

FeatureVector::FeatureVector() {
}

FeatureVector::FeatureVector(const FeatureVector & /*o*/) {

}

map<string, map<string, int> >
FeatureVector::load_relevance(const std::string & qrel_file) {
    typedef map<string, int> Map1;		//docid and relevance judjement 0/1
    typedef map<string, Map1> Map2;		// qid and map1
    Map2 qrel;

    string inLine;
    ifstream myfile(qrel_file.c_str(), ifstream::in);
    string token[4];
    if (myfile.is_open()) {
	while (myfile.good()) {
	    getline(myfile, inLine);		//read a file line by line
	    char * str;
	    char * x1;
	    x1 = const_cast<char*>(inLine.c_str());
	    str = strtok(x1, " ,.-");
	    int i = 0;
	    while (str != NULL)	{
		token[i] = str;		//store tokens in a string array
		++i;
		str = strtok(NULL, " ,.-");
	    }
	    qrel.insert(make_pair(token[0], Map1()));
	    qrel[token[0]].insert(make_pair(token[2], atoi(token[3].c_str())));
	}
	myfile.close();
    }
    return qrel;
}

void
FeatureVector::set_did(const std::string & did1) {
    this->did=did1;
}

void
FeatureVector::set_label(double label1) {
    this->label=label1;
}

void
FeatureVector::set_fvals(map<int,double> fvals1) {
    this->fvals=fvals1;
}


