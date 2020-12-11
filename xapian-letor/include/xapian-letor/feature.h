/** @file
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

#include <xapian-letor/letor_error.h>

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
    /// Stats which FeatureList can use.
    typedef enum {
	/// Frequency of the Query Terms in the specified documents.
	TERM_FREQUENCY = 1,
	/// Inverse Document Frequency of Query terms in the database.
	INVERSE_DOCUMENT_FREQUENCY = 2,
	/// Length of the document as number of "terms".
	DOCUMENT_LENGTH = 4,
	/// Length of the collection in number of term
	COLLECTION_LENGTH = 8,
	/// Frequency of the Query Terms in the whole database.
	COLLECTION_TERM_FREQ = 16,
    } stat_flags;

    /** Tell Xapian that your subclass will want a particular statistic.
     *
     *  Some of the statistics can be costly to fetch or calculate, so
     *  Xapian needs to know which are actually going to be used.  You
     *  should call need_stat() from your constructor for each such
     *  statistic.
     *
     * @param flag  The stat_flags value for a required statistic.
     */
    void need_stat(stat_flags flag) {
	stats_needed = stat_flags(stats_needed | flag);
    }

    /// A bitmask of the statistics this Feature needs.
    stat_flags stats_needed;

    /// Get termfreq
    Xapian::termcount get_termfreq(const std::string& term) const;

    /// Get inverse_doc_freq
    double get_inverse_doc_freq(const std::string& term) const;

    /// Get doc_length
    Xapian::termcount get_doc_length(const std::string& term) const;

    /// Get collection_length
    Xapian::termcount get_collection_length(const std::string& term) const;

    /// Get collection_termfreq
    Xapian::termcount get_collection_termfreq(const std::string& term) const;

  public:
    /// @internal Class representing the Feature internals.
    class Internal;
    /// @internal Reference counted internals.
    Xapian::Internal::intrusive_ptr<Internal> internal;

    /// Default constructor
    Feature();

    /// Virtual destructor because we have virtual methods.
    virtual ~Feature();

    /// Returns the stats needed by a subclass
    stat_flags get_stats() {
	return stats_needed;
    }

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
    TfFeature()
    {
	need_stat(TERM_FREQUENCY);
    }
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
    TfDoclenFeature() {
	need_stat(TERM_FREQUENCY);
	need_stat(DOCUMENT_LENGTH);
    }
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
    IdfFeature() {
	need_stat(INVERSE_DOCUMENT_FREQUENCY);
    }
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
    CollTfCollLenFeature() {
	need_stat(COLLECTION_TERM_FREQ);
	need_stat(COLLECTION_LENGTH);
    }
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
    TfIdfDoclenFeature() {
	need_stat(TERM_FREQUENCY);
	need_stat(DOCUMENT_LENGTH);
	need_stat(INVERSE_DOCUMENT_FREQUENCY);
    }
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
    TfDoclenCollTfCollLenFeature() {
	need_stat(TERM_FREQUENCY);
	need_stat(DOCUMENT_LENGTH);
	need_stat(COLLECTION_TERM_FREQ);
	need_stat(COLLECTION_LENGTH);
    }
    std::vector<double> get_values() const;
    std::string name() const;
};

}

#endif /* XAPIAN_INCLUDED_FEATURE_H */
