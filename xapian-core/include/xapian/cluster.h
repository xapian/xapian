/** @file cluster.h
 *  @brief Cluster API
 */
/* Copyright (C) 2010 Richard Boulton
 * Copyright (C) 2016 Richhiey Thomas
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
#error "Never use <xapian/cluster.h> directly; include <xapian.h> instead."
#endif

#include <xapian/mset.h>
#include <xapian/types.h>
#include <xapian/visibility.h>

#include <unordered_map>
#include <vector>

/** Maximum number of times KMeans algorithm will iterate
 *  till it converges
 */
#define MAX_ITERS 1000

namespace Xapian {

/** Class representing a set of documents in a cluster
 */
class XAPIAN_VISIBILITY_DEFAULT DocumentSet {
  public:
    class Internal;
    /// @private @internal Reference counted internals.
    Xapian::Internal::intrusive_ptr<Internal> internal;

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
    void operator=(const DocumentSet &other);

    /// Constructor
    DocumentSet();

     /// Destructor
    ~DocumentSet();

    /// Return the size of the DocumentSet
    unsigned int size() const;

    /// Return the Document in the DocumentSet at index i
    Xapian::Document operator[](Xapian::doccount i);

    /** Add a new Document to the DocumentSet
     *
     *  @param doc	Document object that is to be added to
     *			the DocumentSet
     */
    void add_document(const Document &doc);
};

/** Base class for TermListGroup
 *  Stores and provides terms that are contained in a document and
 *  their respective term frequencies
 */
class XAPIAN_VISIBILITY_DEFAULT FreqSource
    : public Xapian::Internal::opt_intrusive_base {

  private:

    /// Don't allow assignment.
    void operator=(const FreqSource &);

    /// Don't allow copying.
    FreqSource(const FreqSource &);

  public:

    /// Constructor
    FreqSource() {}

    /// Destructor
    virtual ~FreqSource();

    /** Return the term frequency of a particular term 'tname'
     *
     *  @param tname	The term for which we return the frequency value
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

/** A class for dummy frequency source for construction of termlists
 *  This returns 1 as the term frequency for any term
 */
class XAPIAN_VISIBILITY_DEFAULT DummyFreqSource : public FreqSource {

  public:

    /// Return the value 1 as a dummy term frequency
    doccount get_termfreq(const std::string &) const;

    doccount get_doccount() const;
};

/** A class for construction of termlists which store the terms for a
 *  document along with the number of documents it indexes i.e. term
 *  frequency
 */
class XAPIAN_VISIBILITY_DEFAULT TermListGroup : public FreqSource {

  private:

    /** Map of the terms and its corresponding term frequencies.
     *  The term frequency of a term stands for the number of documents it indexes
     */
    std::unordered_map<std::string, doccount> termfreq;

    /// Number of documents added to the termlist
    doccount docs_num;

    /** Add a single document and calculates its corresponding term frequencies
     *
     *  @param doc	Adds a document and updates the TermListGroup based on the
     *			terms found in the document
     */
    void add_document(const Document &doc);

  public:
    /** Constructor
     *
     *  @params docs	MSet object used to construct the TermListGroup
     */
    explicit TermListGroup(const MSet &docs);

    /** Return the number of documents that the term 'tname' exists in
     *  or the number of documents that a certain term indexes
     *
     *  @param tname	The term for which we return the frequency value
     */
    doccount get_termfreq(const std::string &tname) const;

    doccount get_doccount() const;
};

/** Abstract class representing a point in the VSM
 */
class XAPIAN_VISIBILITY_DEFAULT PointType {

  protected:

    /** Implement a map to store the terms within a document
     *  and their pre-computed TF-IDF values
     */
    std::unordered_map<std::string, double> values;

    /// Store the squared magnitude of the PointType
    double magnitude;

    /// Set the value 'value' to the mapping of a term
    void set_value(const std::string &term, double value);

  public:
    /// Constructor
    PointType() { magnitude = 0; }

    /// Return a TermIterator to the beginning of the termlist
    TermIterator termlist_begin() const;

    /// Return a TermIterator to the end of the termlist
    TermIterator termlist_end() const {
	return TermIterator(NULL);
    }

    /** Validate whether a certain term exists in the termlist
     *  or not by performing a lookup operation in the existing values
     */
    bool contains(const std::string &term) const;

    /// Return the TF-IDF weight associated with a certain term
    double get_value(const std::string &term) const;

    /// Add the value 'value' to the mapping of a term
    void add_value(const std::string &term, double value);

    /// Return the pre-computed squared magnitude
    double get_magnitude() const;

    /// Return the size of the termlist
    int termlist_size() const;
};

/** Class to represent a document as a point in the Vector Space
 *  Model
 */
class XAPIAN_VISIBILITY_DEFAULT Point : public PointType {

  private:

    /// The document which is being represented by the Point
    Document doc;

  public:

    /// Initialize the point with terms and corresponding term weights
    void initialize(const TermListGroup &tlg, const Document &doc);

    /// Returns the document corresponding to this Point
    Document get_document() const;
};

/** Class to represent cluster centroids in the vector space
*/
class Centroid : public PointType {

  public:

    // Constructor
    Centroid();

    /// Constructor with Point argument
    explicit Centroid(Point &x);

    /// Divide the weight of terms in the centroid by 'size'
    void divide(int size);

    /// Clear the terms and corresponding values of the centroid
    void clear();

    /// Recalculate the magnitude of the centroid
    void recalc_magnitude();
};

/** Class to represents a Cluster which contains Points and
 *  of the Cluster
 */
class XAPIAN_VISIBILITY_DEFAULT Cluster {

  public:
    class Internal;
    /// @private @internal Reference counted internals.
    Xapian::Internal::intrusive_ptr<Internal> internal;

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
    void operator=(const Cluster &other);

    /// Constructor
    Cluster(const Centroid centroid_);

    /// Constructor
    Cluster();

     /// Destructor
    ~Cluster();

    /// Returns size of the cluster
    Xapian::doccount size() const;

    /// Add a document to the Cluster
    void add_point(const Point &doc);

    /// Clear the cluster values
    void clear();

    /// Return the point at the given index in the cluster
    Point get_index(unsigned int index) const;

    /// Return the documents that are contained within the cluster
    DocumentSet get_documents();

    /// Return the current centroid of the cluster
    Centroid get_centroid() const;

    /// Set the centroid of the Cluster to centroid_
    void set_centroid(const Centroid centroid_);

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
    Xapian::Internal::intrusive_ptr<Internal> internal;

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
    void operator=(const ClusterSet &other);

    /// Constructor
    ClusterSet();

    /// Destructor
    ~ClusterSet();

    /// Add a cluster to the cluster set
    void add_cluster(const Cluster &c);

    /// Return the Cluster at position 'index'
    Cluster get_cluster(unsigned int index) const;

    /// Add the point the the cluster at position 'index'
    void add_to_cluster(const Point &x, unsigned int index);

    /// Return the number of clusters
    Xapian::doccount size() const;

    /// Return the cluster at index 'i'
    Cluster& operator[](Xapian::doccount i);

    /// Clear all the clusters in the ClusterSet
    void clear_clusters();

    /** Recalculate the centroids for all the centroids
     *  in the ClusterSet
     */
    void recalculate_centroids();
};

/** Base class for calculating the similarity between documents
 */
class XAPIAN_VISIBILITY_DEFAULT Similarity {

  public:

    /// Destructor
    virtual ~Similarity();

    /// Calculates the similarity between the two documents
    virtual double similarity(const PointType &a, const PointType &b) const = 0;

    /// Returns description of the similarity metric being used
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

    /// Returns the description of Cosine Similarity
    std::string get_description() const;
};

/** Class representing an abstract class for a clusterer to be implemented
 */
class XAPIAN_VISIBILITY_DEFAULT Clusterer {

  public:

    /// Destructor
    virtual ~Clusterer();

    /* Implement the required clustering algorithm in the subclass and
     *  and return clustered output as ClusterSet
     */
    virtual ClusterSet cluster(MSet &mset) = 0;

    /// Returns a description of the clusterer being used
    virtual std::string get_description() const = 0;
};

/** Round Robin clusterer:
 *  This clusterer is a minimal clusterer which will cluster documents as -
 *  ith document goes to the (i % k)th cluster where k is the number of clusters and
 *  0 <= i < N; where N is the number of documents
 */
class XAPIAN_VISIBILITY_DEFAULT RoundRobin : public Clusterer {

    /// Number of clusters to be formed by the clusterer
    unsigned int num_of_clusters;

  public:

    /// Constructor
    RoundRobin(unsigned int num_of_clusters_) : num_of_clusters(num_of_clusters_) {}

    /// Implements the RoundRobin clustering
    ClusterSet cluster(MSet &mset);

    /// Returns the description of the clusterer
    std::string get_description() const;
};

/** Kmeans clusterer:
 *  This clusterer implements the K-Means clustering algorithm
 */
class XAPIAN_VISIBILITY_DEFAULT KMeans : public Clusterer {

    /// Contains the initialized points that are to be clustered
    std::vector<Point> docs;

    /// Specifies that the clusterer needs to form 'k' clusters
    unsigned int k;

    /// Specifies the maximum number of iterations that KMeans will have
    unsigned int max_iters;

    /** Initialize 'k' clusters by randomly selecting 'k' centroids
     *  and assigning them to different clusters
     */
    void initialize_clusters(ClusterSet &cset);

    /** Initialize the 'Points' to be fed into the Clusterer with the DocumentSource.
     *  The TF-IDF weights for the points are calculated and stored within the
     *  Points to be used later during distance calculations
     */
    void initialize_points(const MSet &docs);

  public:

    /// Constructor specifying number of clusters and maximum iterations
    explicit KMeans(unsigned int k_, unsigned int max_iters_ = MAX_ITERS);

    /// Implements the KMeans clustering algorithm
    ClusterSet cluster(MSet &mset);

    /// Returns the description of the clusterer
    std::string get_description() const;
};
}
#endif
