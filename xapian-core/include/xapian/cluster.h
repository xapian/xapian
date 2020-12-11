/** @file
 *  @brief Cluster API
 */
/* Copyright (C) 2010 Richard Boulton
 * Copyright (C) 2016 Richhiey Thomas
 * Copyright (C) 2018 Uppinder Chugh
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

#ifndef XAPIAN_INCLUDED_CLUSTER_H
#define XAPIAN_INCLUDED_CLUSTER_H

#if !defined XAPIAN_IN_XAPIAN_H && !defined XAPIAN_LIB_BUILD
#error Never use <xapian/cluster.h> directly; include <xapian.h> instead.
#endif

#include <xapian/attributes.h>
#include <xapian/mset.h>
#include <xapian/queryparser.h>
#include <xapian/types.h>
#include <xapian/visibility.h>

#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace Xapian {

/// Stopper subclass which checks for both stemmed and unstemmed stopwords
class XAPIAN_VISIBILITY_DEFAULT StemStopper : public Xapian::Stopper {
  public:
    /// Stemming strategies
    typedef enum {
	STEM_NONE, STEM_SOME, STEM_ALL, STEM_ALL_Z, STEM_SOME_FULL_POS
    } stem_strategy;

    /** Constructor
     *
     *  @param stemmer	The Xapian::Stem object to set.
     *  @param strategy The stemming strategy to be used.
     */
    explicit StemStopper(const Xapian::Stem &stemmer, stem_strategy strategy = STEM_SOME);

    std::string get_description() const;

    bool operator()(const std::string & term) const {
	return stop_words.find(term) != stop_words.end();
    }

    /// Add a single stop word and its stemmed equivalent
    void add(const std::string &term);

  private:
    stem_strategy stem_action;
    std::unordered_set<std::string> stop_words;
    Xapian::Stem stemmer;
};

/** Class representing a set of documents in a cluster
 */
class XAPIAN_VISIBILITY_DEFAULT DocumentSet {
  public:
    class Internal;
    /// @private @internal Reference counted internals.
    Xapian::Internal::intrusive_ptr_nonnull<Internal> internal;

    /** Copying is allowed.  The internals are reference counted, so
     *  copying is cheap.
     *
     *  @param other	The object to copy.
     */
    DocumentSet(const DocumentSet &other);

    /** Assignment is allowed.  The internals are reference counted,
     *  so assignment is cheap.
     *
     *  @param other	The object to copy.
     */
    DocumentSet & operator=(const DocumentSet &other);

    /** Move constructor.
     *
     * @param other	The object to move.
     */
    DocumentSet(DocumentSet && other);

    /** Move assignment operator.
     *
     * @param other	The object to move.
     */
    DocumentSet & operator=(DocumentSet && other);

    /// Default constructor
    DocumentSet();

    /// Destructor
    ~DocumentSet();

    /// Return the size of the DocumentSet
    Xapian::doccount size() const;

    /// Return the Document in the DocumentSet at index i
    Xapian::Document& operator[](Xapian::doccount i);

    /// Return the Document in the DocumentSet at index i
    const Xapian::Document& operator[](Xapian::doccount i) const;

    /** Add a new Document to the DocumentSet
     *
     *  @param document		Document object that is to be added to
     *				the DocumentSet
     */
    void add_document(const Document &document);
};

/** Base class for TermListGroup
 *  Stores and provides terms that are contained in a document and
 *  their respective term frequencies
 */
class XAPIAN_VISIBILITY_DEFAULT FreqSource
    : public Xapian::Internal::opt_intrusive_base {
    /// Don't allow assignment.
    void operator=(const FreqSource &) = delete;

    /// Don't allow copying.
    FreqSource(const FreqSource &) = delete;

  public:
    /// Default constructor
    FreqSource() {}

    /// Destructor
    virtual ~FreqSource();

    /** Return the term frequency of a particular term 'tname'
     *
     *  @param tname	The term for which to return the term frequency
     */
    virtual doccount get_termfreq(const std::string &tname) const = 0;

    /// Return the number of documents within the MSet
    virtual doccount get_doccount() const = 0;

    /** Start reference counting this object.
     *
     *  You can hand ownership of a dynamically allocated FreqSource
     *  object to Xapian by calling release() and then passing the object to a
     *  Xapian method.  Xapian will arrange to delete the object once it is no
     *  longer required.
     */
    FreqSource * release() {
	opt_intrusive_base::release();
	return this;
    }

    /** Start reference counting this object.
     *
     *  You can hand ownership of a dynamically allocated FreqSource
     *  object to Xapian by calling release() and then passing the object to a
     *  Xapian method.  Xapian will arrange to delete the object once it is no
     *  longer required.
     */
    const FreqSource * release() const {
	opt_intrusive_base::release();
	return this;
    }
};

/** A class for construction of termlists which store the terms for a
 *  document along with the number of documents it indexes i.e. term
 *  frequency
 */
class XAPIAN_VISIBILITY_DEFAULT TermListGroup : public FreqSource {
    /** Map of the terms and its corresponding term frequencies.
     *  The term frequency of a term stands for the number of documents it indexes
     */
    std::unordered_map<std::string, doccount> termfreq;

    /// Number of documents added to the termlist
    doccount num_of_documents;

    /** Add a single document and calculates its corresponding term frequencies
     *
     *  @param document		Adds a document and updates the TermListGroup
     *				based on the terms found in the document
     *  @param stopper		Xapian::Stopper object to identify stopwords
     */
    void add_document(const Document &document, const Stopper *stopper = NULL);

  public:
    /** Constructor
     *
     *  @param docs	MSet object used to construct the TermListGroup
     *  @param stopper	Xapian::Stopper object to identify stopwords
     */
    explicit TermListGroup(const MSet &docs, const Stopper *stopper = NULL);

    /** Return the number of documents that the term 'tname' exists in
     *
     *  @param tname	The term for which to return the term frequency
     */
    doccount get_termfreq(const std::string &tname) const;

    doccount get_doccount() const;
};

/** Abstract class representing a point in the VSM
 */
class XAPIAN_VISIBILITY_DEFAULT PointType
    : public Xapian::Internal::opt_intrusive_base {
  protected:
    /** Implement a map to store the terms within a document
     *  and their pre-computed TF-IDF weights
     */
    std::unordered_map<std::string, double> weights;

    /// Store the squared magnitude of the PointType
    double magnitude;

    /** Set the weight 'weight' to the mapping of a term
     *
     *  @param term	Term for which the weight is supposed
     *			to be changed
     *  @param weight	The weight to which the mapping of the
     *			term is to be set
     */
    void set_weight(const std::string &term, double weight);

  public:
    /// Default constructor
    PointType() : magnitude(0.0) {}

    /// Return a TermIterator to the beginning of the termlist
    TermIterator termlist_begin() const;

    /// Return a TermIterator to the end of the termlist
    TermIterator termlist_end() const noexcept {
	return TermIterator(NULL);
    }

    /** Validate whether a certain term exists in the termlist
     *  or not by performing a lookup operation in the existing values
     *
     *  @param term	Term which is to be searched
     */
    bool contains(const std::string &term) const;

    /** Return the TF-IDF weight associated with a certain term
     *
     *  @param term	Term for which TF-IDF weight is returned
     */
    double get_weight(const std::string &term) const;

    /** Add the weight 'weight' to the mapping of a term
     *
     *  @param term	Term to which the weight is to be added
     *  @param weight	Weight which has to be added to the existing
     *			mapping of the term
     */
    void add_weight(const std::string &term, double weight);

    /// Return the pre-computed squared magnitude
    double get_magnitude() const;

    /// Return the size of the termlist
    Xapian::termcount termlist_size() const;

    /** Start reference counting this object.
     *
     *  You can hand ownership of a dynamically allocated PointType
     *  object to Xapian by calling release() and then passing the object to a
     *  Xapian method.  Xapian will arrange to delete the object once it is no
     *  longer required.
     */
    PointType * release() {
	opt_intrusive_base::release();
	return this;
    }

    /** Start reference counting this object.
     *
     *  You can hand ownership of a dynamically allocated PointType
     *  object to Xapian by calling release() and then passing the object to a
     *  Xapian method.  Xapian will arrange to delete the object once it is no
     *  longer required.
     */
    const PointType * release() const {
	opt_intrusive_base::release();
	return this;
    }
};

/** Class to represent a document as a point in the Vector Space
 *  Model
 */
class XAPIAN_VISIBILITY_DEFAULT Point : public PointType {
    /// The document which is being represented by the Point
    Document document;

  public:
    /** Constructor
     *  Initialise the point with terms and corresponding TF-IDF weights
     *
     *  @param freqsource	FreqSource object which provides the term
     *				frequencies.  It is used for TF-IDF weight
     *				calculations
     *  @param document		The Document object over which the Point object
     *				will be initialised
     */
    Point(const FreqSource& freqsource, const Document& document);

    /// Returns the document corresponding to this Point
    Document get_document() const;
};

/** Class to represent cluster centroids in the vector space
*/
class XAPIAN_VISIBILITY_DEFAULT Centroid : public PointType {
  public:
    /// Default constructor
    Centroid();

    /** Constructor with Point argument
     *
     *  @param point	Point object to which Centroid object is
     *			initialised. The document vector and the
     *			magnitude are made equal
     */
    explicit Centroid(const Point &point);

    /** Divide the weight of terms in the centroid by 'size' and
     *  recalculate the magnitude
     *
     *  @param cluster_size	Value by which Centroid document vector is
     *				divided
     */
    void divide(double cluster_size);

    /// Clear the terms and corresponding values of the centroid
    void clear();
};

/** Class to represents a Cluster which contains Points and Centroid
 *  of the Cluster
 */
class XAPIAN_VISIBILITY_DEFAULT Cluster {
  public:
    class Internal;
    /// @private @internal Reference counted internals.
    Xapian::Internal::intrusive_ptr_nonnull<Internal> internal;

    /** Copying is allowed.  The internals are reference counted, so
     *  copying is cheap.
     *
     *  @param other	The object to copy.
     */
    Cluster(const Cluster &other);

    /** Assignment is allowed.  The internals are reference counted,
     *  so assignment is cheap.
     *
     *  @param other	The object to copy.
     */
    Cluster& operator=(const Cluster &other);

    /** Move constructor.
     *
     * @param other	The object to move.
     */
    Cluster(Cluster && other);

    /** Move assignment operator.
     *
     * @param other	The object to move.
     */
    Cluster & operator=(Cluster && other);

    /** Constructor
     *
     *  @param centroid		The centroid of the cluster object is
     *				assigned to 'centroid'
     */
    explicit Cluster(const Centroid &centroid);

    /// Default constructor
    Cluster();

    /// Destructor
    ~Cluster();

    /// Return size of the cluster
    Xapian::doccount size() const;

    /** Add a document to the Cluster
     *
     *  @param point	The Point object representing the document which
     *			needs to be added to the cluster
     */
    void add_point(const Point &point);

    /// Clear the cluster weights
    void clear();

    /// Return the point at the given index in the cluster
    Point& operator[](Xapian::doccount i);

    /// Return the point at the given index in the cluster
    const Point& operator[](Xapian::doccount i) const;

    /// Return the documents that are contained within the cluster
    DocumentSet get_documents() const;

    /// Return the current centroid of the cluster
    const Centroid& get_centroid() const;

    /** Set the centroid of the Cluster to 'centroid'
     *
     *  @param centroid		Centroid object for the Cluster
     */
    void set_centroid(const Centroid &centroid);

    /** Recalculate the centroid of the Cluster after each iteration
     *  of the KMeans algorithm by taking the mean of all document vectors (Points)
     *  that belong to the Cluster
     */
    void recalculate();
};

/** Class for storing the results returned by the Clusterer
 */
class XAPIAN_VISIBILITY_DEFAULT ClusterSet {
  public:
    class Internal;
    /// @private @internal Reference counted internals.
    Xapian::Internal::intrusive_ptr_nonnull<Internal> internal;

    /** Copying is allowed.  The internals are reference counted, so
     *  copying is cheap.
     *
     *  @param other	The object to copy.
     */
    ClusterSet(const ClusterSet &other);

    /** Assignment is allowed.  The internals are reference counted,
     *  so assignment is cheap.
     *
     *  @param other	The object to copy.
     */
    ClusterSet& operator=(const ClusterSet &other);

    /** Move constructor.
     *
     * @param other	The object to move.
     */
    ClusterSet(ClusterSet && other);

    /** Move assignment operator.
     *
     * @param other	The object to move.
     */
    ClusterSet & operator=(ClusterSet && other);

    /// Default constructor
    ClusterSet();

    /// Destructor
    ~ClusterSet();

    /** Add a cluster to the ClusterSet
     *
     *  @param cluster	Cluster object which is to be added to the ClusterSet
     */
    void add_cluster(const Cluster &cluster);

    /** Add the point to the cluster at position 'index'
     *
     *  @param point	Point object which needs to be added to
     *			a Cluster within the ClusterSet
     *  @param index	Index of the Cluster within the ClusterSet to
     *			which the Point is to be added
     */
    void add_to_cluster(const Point &point, unsigned int index);

    /// Return the number of clusters
    Xapian::doccount size() const;

    /// Return the cluster at index 'i'
    Cluster& operator[](Xapian::doccount i);

    /// Return the cluster at index 'i'
    const Cluster& operator[](Xapian::doccount i) const;

    /// Clear all the clusters in the ClusterSet
    void clear_clusters();

    /** Recalculate the centroid for all the clusters in the ClusterSet */
    void recalculate_centroids();
};

/** Base class for calculating the similarity between documents
 */
class XAPIAN_VISIBILITY_DEFAULT Similarity {
  public:
    /// Destructor
    virtual ~Similarity();

    /** Calculates the similarity between the two documents
     *
     *  @param a	First point object for distance calculation
     *  @param b	Second point object for distance calculation
     */
    virtual double similarity(const PointType &a, const PointType &b) const = 0;

    /// Returns a string describing the similarity metric being used
    virtual std::string get_description() const = 0;
};

/** Class for calculating the cosine distance between two documents
 */
class XAPIAN_VISIBILITY_DEFAULT CosineDistance : public Similarity {
  public:
    /** Calculates and returns the cosine similarity using the
     *  formula  cos(theta) = a.b/(|a|*|b|)
     */
    double similarity(const PointType &a, const PointType &b) const;

    /// Return a string describing this object
    std::string get_description() const;
};

/** Class representing an abstract class for a clusterer to be implemented
 */
class XAPIAN_VISIBILITY_DEFAULT Clusterer
    : public Xapian::Internal::opt_intrusive_base {
  public:
    /// Destructor
    virtual ~Clusterer();

    /** Implement the required clustering algorithm in the subclass and
     *  and return clustered output as ClusterSet
     *
     *  @param mset	The MSet object which contains the documents to be
     *			clustered
     */
    virtual ClusterSet cluster(const MSet &mset) = 0;

    /// Returns a string describing the clusterer being used
    virtual std::string get_description() const = 0;

    /** Start reference counting this object.
     *
     *  You can hand ownership of a dynamically allocated Clusterer
     *  object to Xapian by calling release() and then passing the object to a
     *  Xapian method.  Xapian will arrange to delete the object once it is no
     *  longer required.
     */
    Clusterer * release() {
	opt_intrusive_base::release();
	return this;
    }

    /** Start reference counting this object.
     *
     *  You can hand ownership of a dynamically allocated Clusterer
     *  object to Xapian by calling release() and then passing the object to a
     *  Xapian method.  Xapian will arrange to delete the object once it is no
     *  longer required.
     */
    const Clusterer * release() const {
	opt_intrusive_base::release();
	return this;
    }
};

/** Kmeans clusterer:
 *  This clusterer implements the K-Means clustering algorithm
 */
class XAPIAN_VISIBILITY_DEFAULT KMeans : public Clusterer {
    /// Contains the initialised points that are to be clustered
    std::vector<Point> points;

    /// Specifies that the clusterer needs to form 'k' clusters
    unsigned int k;

    /// Specifies the maximum number of iterations that KMeans will have
    unsigned int max_iters;

    /// Pointer to stopper object for identifying stopwords
    Xapian::Internal::opt_intrusive_ptr<const Xapian::Stopper> stopper;

    /** Initialise 'k' clusters by selecting 'k' centroids and assigning
     *  them to different clusters
     *
     *  @param cset		ClusterSet object to be initialised by assigning
     *				centroids to each cluster
     *  @param num_of_points	Number of points passed to clusterer
     */
    void initialise_clusters(ClusterSet &cset, Xapian::doccount num_of_points);

    /** Initialise the Points to be fed into the Clusterer with the MSet object
     *  'source'. The TF-IDF weights for the documents are calculated and stored
     *  within the Points to be used later during distance calculations
     *
     *  @param source	MSet object containing the documents which will be
     *			used to create document vectors that are represented
     *			as Point objects
     */
    void initialise_points(const MSet &source);

  public:
    /** Constructor specifying number of clusters and maximum iterations
     *
     *  @param k_		Number of required clusters
     *  @param max_iters_	The maximum number of iterations for which KMeans
     *				will run if it doesn't converge
     */
    explicit KMeans(unsigned int k_, unsigned int max_iters_ = 0);

    /** Implements the KMeans clustering algorithm
     *
     *  @param mset    MSet object containing the documents that are to
     *                 be clustered
     */
    ClusterSet cluster(const MSet &mset);

    /** Set the Xapian::Stopper object to be used for identifying stopwords.
     *
     *  Stopwords are discarded while calculating term frequency for terms.
     *
     *  @param stop	The Stopper object to set (default NULL, which means no
     *			stopwords)
     */
    void set_stopper(const Xapian::Stopper *stop = NULL);

    /// Return a string describing this object
    std::string get_description() const;
};

/** LCD clusterer:
 *  This clusterer implements the LCD clustering algorithm adapted from
 *  Modelling efficient novelty-based search result diversification in metric
 *  spaces Gil-Costa et al. 2013
 */
class XAPIAN_VISIBILITY_DEFAULT LCDClusterer : public Clusterer {
    /// Specifies that the clusterer needs to form 'k' clusters
    unsigned int k;

  public:
    /** Constructor specifying number of clusters
     *
     *  @param k_		Number of required clusters
     */
    explicit LCDClusterer(unsigned int k_);

    /** Implements the LCD clustering algorithm
     *
     *  @param mset    MSet object containing the documents that are to
     *                 be clustered
     */
    ClusterSet cluster(const MSet &mset);

    /// Return a string describing this object
    std::string get_description() const;
};
}
#endif // XAPIAN_INCLUDED_CLUSTER_H
