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

#include <xapian/intrusive_ptr.h>
#include <xapian/mset.h>
#include <xapian/types.h>
#include <xapian/visibility.h>

#include <tr1/unordered_map>
#include <vector>

namespace Xapian {

class TermListGroup;

class Document;

/// Class representing a source of documents to be provided to the Clusterer
class DocumentSource {

  public:

    /// Returns the next document in the DocumentSource
    virtual Document next_document() = 0;

    /// Checks whether the DocumentSource index is at its end
    virtual bool at_end() const = 0;

    /// Returns the size of the DocumentSource
    virtual doccount size() = 0;
};

/// Class representing a source of documents created from an MSet
class MSetDocumentSource : public DocumentSource {

    /// This represents the MSet used to build this DocumentSource
    MSet mset;

    /** This represents the maximum number of items that can be stored
     *  in this DocumentSource
     */
    doccount maxitems;

    /** This represents the index in the MSet. It is used to return the
     *  the document at the current index value in MSet
     */
    doccount index;

  public:

    /** Constructor :
     *  Constructs MSetDocumentSource directly from the MSet
     */
    MSetDocumentSource(const MSet &mset);

    /** Constructor :
     *  Constructs MSetDocumentSource directly from the MSet with only maxitems_
     *  number of items
     */
    MSetDocumentSource(const MSet &mset, doccount maxitems_);

    /** This method returns the next document in the MSet according to index.
     *  index will be incremented everytime this method is called.
     *  This will work only till index < (maxitems - 1)
     */
    Document next_document();

    /// This method checks whether the MSetDocumentSource is at end of MSet or not
    bool at_end() const;

    /// This method returns the size of the MSetDocumentSource
    doccount size();
};

/** Structure to store term and corresponding wdf. This is used with
 *  PointTermIterator to return wdf
 */
struct XAPIAN_VISIBILITY_DEFAULT Wdf {

    /// Represents term that is to be stored
    std::string term;

    /// Represents the wdf corresponding to the term
    double wdf;

    // Constructor
    Wdf(std::string term_, double wdf_) : term(term_), wdf(wdf_) {}
};

class DocumentSetIterator;

/// Class representing a set of documents in a cluster
class XAPIAN_VISIBILITY_DEFAULT DocumentSet {

    friend class DocumentSetIterator;

    /// Vector storing the documents for this DocumentSet
    std::vector<Document> docs;

  public:

    /// This method returns the size of the DocumentSet
    int size();

    /// This method returns the Document in the DocumentSet at index i
    Xapian::Document operator[](Xapian::doccount i);

    /* This method returns an iterator to the start of the DocumentSet to
     * iterate through all the documents
     */
    DocumentSetIterator begin();

    /// This method returns an iterator to the end of the DocumentSet
    DocumentSetIterator end();

    /// This method adds a new Document to the DocumentSet
    void add_document(Document doc);
};

/// Class used to iterate through the DocumentSet
class XAPIAN_VISIBILITY_DEFAULT DocumentSetIterator {

    friend class DocumentSet;

    /// This represents the DocumentSet over which we will iterate
    DocumentSet docs;

  public:

    /// This represents the current index of the iterator
    int index;

    /// Constructor
    DocumentSetIterator(const DocumentSet &docs_, int index_)
	: docs(docs_), index(index_) {}

    /// This method returns the document referred to by the current iterator position
    Xapian::Document get_document();

    /// Prefix increment operator for this iterator
    DocumentSetIterator & operator++() {
	index++;
	return *this;
    }

    /// Postfix increment operator for this iterator
    DocumentSetIterator operator++(int) {
	DocumentSetIterator d = *this;
	index++;
	return d;
    }

    /// Prefix decrement operator for this iterator
    DocumentSetIterator & operator--() {
	index--;
	return *this;
    }

    /// Postfix decrement operator for this iterator
    DocumentSetIterator operator--(int) {
	DocumentSetIterator d = *this;
	index--;
	return d;
    }
};

/** Abstract class representing a point in the VSM
 *  For K-Means : Two types -
 *  1) Point
 *  2) Centroid
 */
class PointType {

  public:

    /** This contains the termlist which is used by our
     *  TermIterator working over the PointType
     *  It contains terms and their corresponding wdf's
     */
    std::vector<struct Wdf> termlist;

    /** This implements a map to store the terms within a document
     *  and their pre-computed TF-IDF values
     */
    std::tr1::unordered_map<std::string, double> values;

    /// This stores the squared magnitude of the PointType
    double magnitude;

    /// This method returns a TermIterator to the beginning of the termlist
    TermIterator termlist_begin();

    /// This method returns a TermIterator to the end of the termlist
    TermIterator termlist_end();

    /** This method validates whether a certain term exists in the termlist
     *  or not by performing a lookup operation in the existing values
     */
    bool contains(std::string term);

    /// This method returns the TF-IDF weight associated with a certain term
    double get_value(std::string term);

    /// This method returns the pre-computed squared magnitude
    double get_magnitude();

    /// This method adds the value 'value' to the mapping of a term
    void add_value(std::string term, double value);

    /// This method sets the value 'value' to the mapping of a term
    void set_value(std::string term, double value);

    /// This method returns the size of the termlist
    int termlist_size();
};

/** Class to represent a document as a point in the Vector Space
 *  Model
 */
class Point : public PointType {

    /// The document which is being represented by the Point
    Document doc;

  public:

    /// Constructor
    Point() { magnitude = 0; }

    /// This method initializes the point with terms and corresponding term weights
    void initialize(TermListGroup &tlg, const Document &doc);

    /// This method returns the document corresponding to this Point
    Document get_document();
};

// Class to represent cluster centroids in the vector space
class Centroid : public PointType {

  public:

    /// Constructor
    Centroid() { magnitude = 0; }

    /// This method initializes the values of a centroid to the Point 'x'
    void set_to_point(Point &x);

    /// This method divides the weight of terms in the centroid by 'size'
    void divide(int size);

    /// This method clears the terms and corresponding values of the centroid
    void clear();

    /// This method recalculates the magnitude of the centroid
    void recalc_magnitude();
};

/** Class to represents a Cluster which contains Points and
 *  centroid of the Cluster
 */
class XAPIAN_VISIBILITY_DEFAULT Cluster {

    /// Documents (or Points in the vector space) within the cluster
    std::vector<Point> cluster_docs;

    /// Point or Document representing the cluster centroid
    Centroid centroid;

  public:

    /// Constructor
    Cluster(const Centroid centroid_) : centroid(centroid_) {}

    /// Constructor
    Cluster();

     /// Destructor
    ~Cluster();

    /// This method returns size of the cluster
    Xapian::doccount size();

    /// This method returns the current centroid of the cluster
    Centroid get_centroid();

    /// This method sets the centroid of the Cluster to centroid_
    void set_centroid(const Centroid centroid_);

    /** This method recalculates the centroid of the Cluster after each iteration
     *  of the KMeans algorithm by taking the mean of all document vectors (Points)
     *  that belong to the Cluster
     */
    void recalculate();

    /// This method adds a document to the Cluster
    void add_cluster(const Point &doc);

    /// This method clears the cluster values
    void clear();

    /// This method returns the point at the given index in the cluster
    Point get_index(int index);

    /// This method returns the documents that are contained within the cluster
    DocumentSet get_documents();

    /** This method returns the average distance between Centroid and Points in a
     *  Cluster. It can be used to check the compactness of the Cluster. The more
     *  compact a cluster is, the better is the quality of the cluster
     */
    double advdc();
};

// Class for storing the results returned by the Clusterer
class XAPIAN_VISIBILITY_DEFAULT ClusterSet {

    /** A map storing the clusterid and its corresponding
     *  Document cluster
     */
    std::vector<Cluster> clusters;

  public:

    /// This method adds a cluster to the cluster set
    void add_cluster(Cluster &c);

    /// This method returns a vector of documents
    Cluster get_cluster(clusterid id);

    /// This method adds the point the the cluster at index 'i'
    void add_to_cluster(const Point &x, clusterid i);

    /// This method returns the number of clusters
    Xapian::doccount size();

    /// This method returns the size of a cluster with clusterid 'cid'
    Xapian::doccount cluster_size(clusterid cid);

    /// This method is used to check the cluster at index 'i'
    Cluster operator[](Xapian::doccount i);

    /// This method is used to clear all the Clusters in the ClusterSet
    void clear_clusters();

    /** This methood recalculates the centroids for all the centroids
     *  in the ClusterSet
     */
    void recalculate_centroids();
};

/// This class represents an abstract class for a clusterer to be implemented
class XAPIAN_VISIBILITY_DEFAULT Clusterer {

  public:

    /// Destructor
    virtual ~Clusterer();

    /// This method helps implement the required clustering algorithm in the subclass
    virtual ClusterSet cluster(MSet &mset) = 0;

    /// This method returns a description of the clusterer being used
    virtual std::string get_description() = 0;
};

/** Round Robin clusterer:
 *  This clusterer is a minimal clusterer which will cluster documents as -
 *  ith document goes to the (i % k)th cluster where k is the number of clusters and
 *  0 <= i < N; where N is the number of documents
 */
class XAPIAN_VISIBILITY_DEFAULT RoundRobin : public Clusterer {

    /// This specifies the number of clusters to be formed by the clusterer
    unsigned int num_of_clusters;

  public:

    /// Constructor
    RoundRobin(unsigned int num_of_clusters_) : num_of_clusters(num_of_clusters_) {}

    /// This method implements the RoundRobin clustering
    ClusterSet cluster(MSet &mset);

    /// This method returns the description of the clusterer
    std::string get_description();
};

/** Kmeans clusterer:
 *  This clusterer implements the K-Means clustering algorithm
 */
class XAPIAN_VISIBILITY_DEFAULT KMeans : public Clusterer {

    /// This contains the initialized points that are to be clustered
    std::vector<Point> docs;

    /// This contains the state of 'k' centroids at every iteration
    std::vector<Centroid> centroids;

    /// This specifies that the clusterer needs to form 'k' clusters
    unsigned int k;

    /** This method checks whether the current state of KMeans has converged
     *  by checking for change in centroid of the clusters
     */
    bool converge(std::vector<Centroid> &previous, std::vector<Centroid> &current);

    /** This method initalizes of centroids using a certain specified method
     *  Current methods that are supported :
     *     -- random - Random Initialization
     *     -- kmeanspp - KMeans++ Initialization
     */
    void initialize_centroids(ClusterSet &cset);

    /** This method helps initialize the initial centroids to be passed
     *  to the KMeans clusterer in a random fashion by selecting 'k' points
     *  out of all the points in the clusterer
     */
    void initialize_random(ClusterSet &cset);

    /** Initialize the 'Points' to be fed into the Clusterer with the DocumentSource.
     *  The TF-IDF weights for the points are calculated and stored within the
     *  Points to be used later during distance calculations
     */
    void initialize_points(MSetDocumentSource docs, TermListGroup &tlg);

  public:

    /// Constructor specifying number of clusters
    KMeans(unsigned int k_) : k(k_) {}

    /// This method implements the KMeans clustering algorithm
    ClusterSet cluster(MSet &mset);

    /// This method returns the description of the clusterer
    std::string get_description();
};
};

#endif
