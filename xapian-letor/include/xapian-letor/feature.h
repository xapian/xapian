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
    /// Xapian::Database using which features will be calculated.
    Database feature_db;

    /// Xapian::Query using which features will be calculated.
    Query feature_query;

    /// Xapian::Document using which features will be calculated.
    Document feature_doc;

    /// Frequency of the Query Terms in the specified documents.
    std::map<std::string, Xapian::termcount> termfreq;

    /// Inverse Document Frequency of Query terms in the database.
    std::map<std::string, double> inverse_doc_freq;

    /// Length of the document as number of "terms"
    std::map<std::string, Xapian::termcount> doc_length;

    /** Length of the collection in number of terms for different parts
     *  like 'title', 'body' and 'whole'
     */
    std::map<std::string, Xapian::termcount> collection_length;

    /// Frequency of the Query Terms in the whole database
    std::map<std::string, Xapian::termcount> collection_termfreq;

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

  public:
    /// Default constructor
    Feature();

    /// Virtual destructor because we have virtual methods.
    virtual ~Feature();

    /** Specify the database to use for feature building.
     *
     *  This will be used by FeatureList::Internal class.
     */
    void set_database(const Xapian::Database & db);

    /** Specify the query to use for feature building.
     *
     *  This will be used by FeatureList::Internal class.
     *
     *  @param query  Xapian::Query which has to be queried
     */
    void set_query(const Xapian::Query & query);

    /** Specify the document to use for feature building.
     *
     *  This will be used by FeatureList::Internal class.
     */
    void set_doc(const Xapian::Document & doc);

    /** Sets the termfrequency that is going to be used for
     *  Feature building.
     *
     *  This is used by FeatureList::Internal while populating Statistics.
     */
    void set_termfreq(const std::map<std::string, Xapian::termcount> &tf);

    /** Sets the inverse_doc_freq that is going to be used for
     *  Feature building.
     *
     *  This is used by FeatureList::Internal while populating Statistics.
     */
    void set_inverse_doc_freq(const std::map<std::string, double> & idf);

    /** Sets the doc_length that is going to be used for Feature building.
     *
     *  This is used by FeatureList::Internal while populating Statistics.
     */
    void set_doc_length(const std::map<std::string,
			Xapian::termcount> & doc_len);

    /** Sets the collection_length that is going to be used for
     *  Feature building.
     *
     *  This is used by FeatureList::Internal while populating Statistics.
     */
    void set_collection_length(const std::map<std::string,
			       Xapian::termcount> & collection_len);

    /** Sets the collection_termfreq that is going to be used
     *  for Feature building.
     *
     *  This is used by FeatureList::Internal while populating Statistics.
     */
    void set_collection_termfreq(const std::map<std::string,
				 Xapian::termcount> & collection_tf);

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
