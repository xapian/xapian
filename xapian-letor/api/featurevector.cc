/** @file
 * @brief The file responsible for transforming the document into the feature space.
 */
/* Copyright (C) 2012 Parth Gupta
 * Copyright (C) 2016 Ayush Tomar
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

#include "xapian-letor/featurevector.h"
#include "featurevector_internal.h"

#include "debuglog.h"

#include <vector>

using namespace Xapian;

FeatureVector::FeatureVector() : internal(new FeatureVector::Internal)
{
    LOGCALL_CTOR(API, "FeatureVector", NO_ARGS);
}

FeatureVector::FeatureVector(const Xapian::docid & did,
			     const std::vector<double> & fvals)
			     : internal(new FeatureVector::Internal)
{
    LOGCALL_CTOR(API, "FeatureVector", did | fvals);
    internal->did_ = did;
    internal->fvals_ = fvals;
}

FeatureVector::FeatureVector(const FeatureVector & o) : internal(o.internal)
{
    LOGCALL_CTOR(API, "FeatureVector", o);
}

FeatureVector &
FeatureVector::operator=(const FeatureVector & o)
{
    LOGCALL(API, FeatureVector &, "FeatureVector::operator=", o);
    internal = o.internal;
    return *this;
}

FeatureVector::~FeatureVector()
{
    LOGCALL_DTOR(API, "FeatureVector");
}

void
FeatureVector::set_did(Xapian::docid did)
{
    LOGCALL_VOID(API, "FeatureVector::set_did", did);
    internal->did_ = did;
}

void
FeatureVector::set_label(double label)
{
    LOGCALL_VOID(API, "FeatureVector::set_label", label);
    internal->label_ = label;
}

void
FeatureVector::set_score(double score)
{
    LOGCALL_VOID(API, "FeatureVector::set_score", score);
    internal->score_ = score;
}

void
FeatureVector::set_fvals(const std::vector<double> & fvals)
{
    LOGCALL_VOID(API, "FeatureVector::set_fvals", fvals);
    internal->fvals_ = fvals;
}

void
FeatureVector::set_feature_value(int index, double value)
{
    LOGCALL_VOID(API, "FeatureVector::set_feature_value", index | value);
    internal->fvals_[index] = value;
}

int
FeatureVector::get_fcount() const
{
    LOGCALL(API, int, "FeatureVector::get_fcount", NO_ARGS);
    return internal->fvals_.size();
}

double
FeatureVector::get_score() const
{
    LOGCALL(API, double, "FeatureVector::get_score", NO_ARGS);
    return internal->score_;
}

double
FeatureVector::get_label() const
{
    LOGCALL(API, double, "FeatureVector::get_label", NO_ARGS);
    return internal->label_;
}

std::vector<double>
FeatureVector::get_fvals() const
{
    LOGCALL(API, std::vector<double>, "FeatureVector::get_fvals", NO_ARGS);
    return internal->fvals_;
}

Xapian::docid
FeatureVector::get_did() const
{
    LOGCALL(API, Xapian::docid, "FeatureVector::get_did", NO_ARGS);
    return internal->did_;
}

double
FeatureVector::get_feature_value(int index) const
{
    LOGCALL(API, double, "FeatureVector::get_feature_value", index);
    return internal->fvals_[index];
}
