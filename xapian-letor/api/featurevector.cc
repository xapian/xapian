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

FeatureVector::FeatureVector() : internal(new FeatureVector::Internal)
{
}

FeatureVector::FeatureVector(const FeatureVector & o) : internal(o.internal)
{
}

FeatureVector &
FeatureVector::operator=(const FeatureVector & o)
{
    internal = o.internal;
    return *this;
}

FeatureVector::~FeatureVector()
{
}

bool
FeatureVector::before(const Xapian::FeatureVector& c1, const Xapian::FeatureVector& c2)
{
    return c1.internal->score < c2.internal->score;
}

map<string, map<string, int> >
FeatureVector::load_relevance(const std::string & qrel_file)
{
    return internal->load_relevance(qrel_file);
}


void
FeatureVector::set_did(const Xapian::docid & did1) {
    internal->did=did1;
}

void
FeatureVector::set_fcount(int fcount1) {
    internal->fcount=fcount1;
}

void
FeatureVector::set_feature_value(int index, double value) {
    internal->fvals[index] = value;
}

void
FeatureVector::set_label(double label1) {
    internal->label=label1;
}

void
FeatureVector::set_fvals(map<int,double> & fvals1) {
    internal->fvals=fvals1;
}

int
FeatureVector::get_fcount(){
    return internal->fcount;
}

double
FeatureVector::get_score() const{
    return internal->score;
}

double
FeatureVector::get_label() const{
    return internal->label;
}

std::map<int,double>
FeatureVector::get_fvals() {
    return internal->fvals;
}

Xapian::docid
FeatureVector::get_did() {
    return internal->did;
}

double
FeatureVector::get_feature_value(int index) {
    return internal->get_feature_value(index);
}

void
FeatureVector::set_score(double score1) {
    internal->score=score1;
}

int
FeatureVector::get_nonzero_num(){
    return internal->get_nonzero_num();
}
