/** @file feature.h
 * @brief Abstract base class for features in learning to rank
 */
/* Copyright (C) 2016 Ayush Tomar
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

#ifndef XAPIAN_INCLUDED_FEATURE_H
#define XAPIAN_INCLUDED_FEATURE_H

#include <xapian.h>
#include <xapian/intrusive_ptr.h>
#include <xapian/types.h>
#include <xapian/visibility.h>

#include "letor_error.h"

#include <string>
#include <limits>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>


namespace Xapian {

/// Abstract base class for features in learning to rank
class XAPIAN_VISIBILITY_DEFAULT Feature {
  protected:
    /// @internal Class representing the FeatureVector internals.
    class Internal;
    /// @internal Reference counted internals.
    Xapian::Internal::intrusive_ptr<Internal> internal;

  public:
    /// Default constructor
    Feature();

    /// Virtual destructor because we have virtual methods.
    virtual ~Feature();

    /// Specify the database to use for feature building. This will be used by the Internal class.
    void set_database(const Xapian::Database & db);

    /** Specify the query to use for feature building. This will be used by the Internal class.
     * @param query  Xapian::Query which has to be queried
     * @exception Xapian::InvalidArgumentError will be thrown if an empty
     *  query is supplied
     */
    void set_query(const Xapian::Query & query);

    /// Specify the document to use for feature building. This will be used by the Internal class.
    void set_doc(const Xapian::Document & doc);

    /// Calculate and return the feature values
    virtual std::vector<double> get_values() const = 0;

    /// Return name of the feature
    virtual std::string name() const = 0;

  private:

    /// Don't allow assignment.
    void operator=(const Feature &);

    /// Don't allow copying.
    Feature(const Feature & o);
};

/** Feature subclass returning feature value calculated as:
 *  @f[fval = \sum_{q_i \in Q \cap D} \log{\left( c(q_i,D) \right)}@f]
 *
 *  where:
 *         c(w,D) means that count of term w in Document D.
 *         C represents the Collection.
 *        'n' is the total number of terms in query.
 *        |.| is size-of function
 */
class XAPIAN_VISIBILITY_DEFAULT TfFeature : public Feature {
  public:
    std::vector<double> get_values() const;
    std::string name() const;
};

/** Feature subclass returning feature value calculated as:
 *  @f[fval = \sum_{i=1}^{n}\log{\left(1+\frac{c\left(q_i,D\right)}{|D|}\right)}@f]
 *
 *  where:
 *        c(w,D) means that count of term w in Document D.
 *        C represents the Collection.
 *        'n' is the total number of terms in query.
 *        |.| is size-of function
 */
class XAPIAN_VISIBILITY_DEFAULT TfDoclenFeature : public Feature {
  public:
    std::vector<double> get_values() const;
    std::string name() const;
};

/** Feature subclass returning feature value calculated as:
 *  @f[fval = \sum_{q_i \in Q \cap D} \log{\left(idf(q_i) \right) }@f]
 *
 *  where:
 *        idf(.) is the inverse-document-frequency.
 */
class XAPIAN_VISIBILITY_DEFAULT IdfFeature : public Feature {
  public:
    std::vector<double> get_values() const;
    std::string name() const;
};

/** Feature subclass returning feature value calculated as:
 *  @f[fval = \sum_{q_i \in Q \cap D} \log{\left( \frac{|C|}{c(q_i,C)} \right)}@f]
 *
 *  where:
 *        c(w,D) means that count of term w in Document D.
 *        C represents the Collection.
 *        'n' is the total number of terms in query.
 *        |.| is size-of function
 */
class XAPIAN_VISIBILITY_DEFAULT CollTfCollLenFeature : public Feature {
  public:
    std::vector<double> get_values() const;
    std::string name() const;
};

/** Feature subclass returning feature value calculated as:
 *  @f[fval = \sum_{i=1}^{n}\log{\left(1+\frac{c\left(q_i,D\right)}{|D|}idf(q_i)\right)}@f]
 *
 *  where:
 *        c(w,D) means that count of term w in Document D.
 *        C represents the Collection.
 *        'n' is the total number of terms in query.
 *        |.| is size-of function
 *        idf(.) is the inverse-document-frequency.
 */
class XAPIAN_VISIBILITY_DEFAULT TfIdfDoclenFeature : public Feature {
  public:
    std::vector<double> get_values() const;
    std::string name() const;
};

/** Feature subclass returning feature value calculated as:
 *  @f[fval = \sum_{i=1}^{n}\log{\left(1+\frac{c\left(q_i,D\right)}{|D|}\frac{|C|}{c(q_i,C)}\right)}@f]
 *
 *  where:
 *        c(w,D) means that count of term w in Document D.
 *        C represents the Collection.
 *        'n' is the total number of terms in query.
 *        |.| is size-of function
 */
class XAPIAN_VISIBILITY_DEFAULT TfDoclenCollTfCollLenFeature : public Feature {
  public:
    std::vector<double> get_values() const;
    std::string name() const;
};

}

#endif /* XAPIAN_INCLUDED_FEATURE_H */
