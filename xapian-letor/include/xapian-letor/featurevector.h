/** @file
 * @brief The class for transforming the document into the feature space.
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

#ifndef XAPIAN_INCLUDED_FEATUREVECTOR_H
#define XAPIAN_INCLUDED_FEATUREVECTOR_H

#include <xapian.h>
#include <xapian/intrusive_ptr.h>
#include <xapian/types.h>
#include <xapian/visibility.h>

#include <xapian-letor/letor_error.h>

#include <vector>

namespace Xapian {

class XAPIAN_VISIBILITY_DEFAULT FeatureVector {
    /// @private @internal Class representing the FeatureVector internals.
    class Internal;
    /// @private @internal Reference counted internals.
    Xapian::Internal::intrusive_ptr<Internal> internal;

  public:
    /// Default constructor.
    FeatureVector();

    /// Constructor creating an object instantiated with docid and fvals
    FeatureVector(const Xapian::docid & did, const std::vector<double> & fvals);

    /// Copy constructor.
    FeatureVector(const FeatureVector & o);

    /// Assignment.
    FeatureVector & operator=(const FeatureVector & o);

    /// Destructor.
    ~FeatureVector();

    /// Set docid corresponding to the FeatureVector object
    void set_did(Xapian::docid did);

    /// Set training label corresponding to the FeatureVector object
    void set_label(double label);

    /// Set score corresponding to the FeatureVector object
    void set_score(double score);

    /// Set vector of feature values returned by Feature objects
    void set_fvals(const std::vector<double> & fvals);

    /// Set individual feature value from vector of fvals, by index.
    void set_feature_value(int index, double value);

    /// Get number of feature values corresponding to the FeatureVector object
    int get_fcount() const;

    /// Get score value corresponding to the FeatureVector object
    double get_score() const;

    /// Get label value corresponding to the FeatureVector object
    double get_label() const;

    /// Get docid value corresponding to the FeatureVector object
    Xapian::docid get_did() const;

    /// Get vector of feature values corresponding to the FeatureVector object
    std::vector<double> get_fvals() const;

    /// Get individual feature value from vector of fvals, by index.
    double get_feature_value(int index) const;
};

}

#endif /* XAPIAN_INCLUDED_FEATUREVECTOR_H */
