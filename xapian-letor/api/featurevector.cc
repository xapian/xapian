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

#include "featurevector_internal.h"

#include <vector>

using namespace Xapian;

FeatureVector::FeatureVector() : internal(new FeatureVector::Internal)
{
}

FeatureVector::FeatureVector(const Xapian::docid & did, const std::vector<double> & fvals)
                             : internal(new FeatureVector::Internal)
{
    internal->did_=did;
    internal->fvals_=fvals;
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

void
FeatureVector::set_did(const Xapian::docid & did) {
    internal->did_=did;
}

void
FeatureVector::set_label(const double label) {
    internal->label_=label;
}

void
FeatureVector::set_score(const double score) {
    internal->score_=score;
}

void
FeatureVector::set_fvals(const std::vector<double> & fvals) {
    internal->fvals_=fvals;
}

void
FeatureVector::set_feature_value(int index, double value) {
    internal->fvals_[index] = value;
}

int
FeatureVector::get_fcount() const {
    return internal->fvals_.size();
}

double
FeatureVector::get_score() const{
    return internal->score_;
}

double
FeatureVector::get_label() const{
    return internal->label_;
}

std::vector<double>
FeatureVector::get_fvals() const {
    return internal->fvals_;
}

Xapian::docid
FeatureVector::get_did() const {
    return internal->did_;
}

double
FeatureVector::get_feature_value(int index) const {
    return internal->fvals_[index];
}
