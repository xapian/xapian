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
# error "Never use <xapian/cluster.h> directly; include <xapian.h> instead."
#endif

#include <xapian/visibility.h>
#include <xapian/types.h>
#include <xapian/error.h>
#include <xapian/mset.h>

#include <map>
#include <tr1/unordered_map>
#include <vector>
#include <set>

namespace Xapian {

class Document;

// Class representing a source of documents to be provided to the Clusterer
class XAPIAN_VISIBILITY_DEFAULT DocumentSource {
  public:
    // Returns the next document in the DocumentSource
    virtual Document next_document() = 0;

    // Checks whether the DocumentSource index is at its end
    virtual bool at_end() const = 0;

    // Returns the size of the DocumentSource
    virtual doccount size() = 0;
};

// Class representing a source of documents created from an MSet
class XAPIAN_VISIBILITY_DEFAULT MSetDocumentSource : public DocumentSource {
    MSet mset;
    Xapian::doccount maxitems;
    Xapian::doccount index;
  public:
    /** Constructor :
     *  Constructs DocumentSource directly from the MSet
     */
    MSetDocumentSource(const MSet &mset);

    /** Constructor :
     *  Constructs DocumentSource directly from the MSet with only maxitems_
     *  number of items
     */
    MSetDocumentSource(const MSet &mset, Xapian::doccount maxitems_);

    Document next_document();

    bool at_end() const;

    Xapian::doccount size();
};

class DocumentSetIterator;

// Class representing a set of documents in a cluster
class XAPIAN_VISIBILITY_DEFAULT DocumentSet {
    friend class DocumentSetIterator;
    // Vector storing the documents for this DocumentSet
    std::vector<Document> docs;
  public:
    // Returns the size of the DocumentSet
    int size() {
	return docs.size();
    }

    // Returns the document at index i
    Xapian::Document operator[](Xapian::doccount i);

    /* Returns an iterator to the start of the DocumentSet to iterate
     * through all the documents
     */
    DocumentSetIterator begin();

    // Returns an iterator to the end of the DocumentSet
    DocumentSetIterator end();

    // Adds a document to the DocumentSet
    void add_document(Document doc);
};

// A class used to iterate through the DocumentSet
class XAPIAN_VISIBILITY_DEFAULT DocumentSetIterator {
    friend class DocumentSet;
    DocumentSet docs;
  public:
    int index;
    DocumentSetIterator(DocumentSet &docs_, int index_)
	: docs(docs_), index(index_) {}

    // Returns the document referred to by the current iterator position
    Xapian::Document get_document();

    // Prefix increment operator for this iterator
    DocumentSetIterator & operator++() {
	index++;
	return *this;
    }

    // Postfix increment operator for this iterator
    DocumentSetIterator operator++(int) {
	DocumentSetIterator d = *this;
	index++;
	return d;
    }

    // Prefix decrement operator for this iterator
    DocumentSetIterator & operator--() {
	index--;
	return *this;
    }

    // Postfix decrement operator for this iterator
    DocumentSetIterator operator--(int) {
	DocumentSetIterator d = *this;
	index--;
	return d;
    }
};

// Base class used for construction of the TermLists
class XAPIAN_VISIBILITY_DEFAULT FreqSource {
  public:
    /** This contains a map of the terms and its corresponding term frequencies.
     *  The term frequency of a term stands for the number of documents it indexes
     */
    std::tr1::unordered_map<std::string, Xapian::doccount> termfreq;

    // Destructor
    virtual ~FreqSource();

    // Gets the term frequency of a particular term 'tname'
    virtual Xapian::doccount get_termfreq(const std::string &tname) = 0;

    // Gets the number of documents
    virtual Xapian::doccount get_doccount() = 0;
};

// A class for dummy frequency source for construction of termlists
class XAPIAN_VISIBILITY_DEFAULT DummyFreqSource : public FreqSource {
  public:
    Xapian::doccount get_termfreq(const std::string &);

    Xapian::doccount get_doccount();
};

// A class for construction of termlists
class XAPIAN_VISIBILITY_DEFAULT TermListGroup : public FreqSource {
  public:
    /** Adds document terms and their statistics to the FreqSource for
     *  construction of termlists
     */
    doccount docs_num;

    void add_document(const Document &doc);

    // Add a number of documents from the DocumentSource
    void add_documents(MSetDocumentSource docs);

    Xapian::doccount get_termfreq(const std::string &tname);

    Xapian::doccount get_doccount();
};

/** Structure to store term and corresponding wdf.
 *  This is used with PointTermIterator to return wdf
 */
struct XAPIAN_VISIBILITY_DEFAULT Wdf {
    std::string term;
    double wdf;
    Wdf(std::string term_, double wdf_) : term(term_), wdf(wdf_) {}
};

/** Abstract class representing a point in the VSM
 *  For K-Means : Two types, Point and Centroid
 */
class XAPIAN_VISIBILITY_DEFAULT PointType {
  public:
    std::vector<struct Wdf> termlist;
    std::tr1::unordered_map<std::string, double> values;
    double magnitude;
    TermIterator termlist_begin();
    TermIterator termlist_end();
    bool contains(std::string term);
    double get_value(std::string term);
    double get_magnitude();
    void add_value(std::string term, double value);
    void set_value(std::string term, double value);
    int termlist_size();
};

// Represent a single point in the vector space
class XAPIAN_VISIBILITY_DEFAULT Point : public PointType {
    Document doc;
  public:
    Point() {
	magnitude = 0;
	values.max_load_factor(0.25);
	values.rehash(4096);
    }

    // Initialize the point with terms and corresponding term weights
    void initialize(TermListGroup &tlg, const Document &doc);

    // Returns the document corresponding to this Point
    Document get_document();
};

// Represents cluster centroids in the vector space
class XAPIAN_VISIBILITY_DEFAULT Centroid : public PointType {
    int id;
  public:
    Centroid(int id_) : id(id_) { magnitude = 0; }

    // Initialize the values of a centroid to the Point 'x'
    void set_to_point(Point &x);

    // Divide the weight of terms in the centroid by 'size'
    void divide(int size);

    // Returns the unqiue index of the centroid
    int get_index();

    // Clears the terms and corresponding values of the centroid
    void clear();

    // Recalculates the magnitude of the centroid
    void recalc_magnitude();
};

// Base class for calculating the similarity between documents
class XAPIAN_VISIBILITY_DEFAULT Similarity {
  public:
    // Destructor
    virtual ~Similarity();

    // Calculates the similarity between the two documents
    virtual double similarity(PointType &a, PointType &b) = 0;

    // Returns description of the similarity metric being used
    virtual std::string get_description() = 0;
};

class XAPIAN_VISIBILITY_DEFAULT EuclidianDistance {
  public:
    double similarity(PointType &a, PointType &b);

    std::string get_description();
};

// Class for calculating the cosine distance between two documents
class XAPIAN_VISIBILITY_DEFAULT CosineDistance : public Similarity {
  public:
    double similarity(PointType &a, PointType &b);

    std::string get_description();
};

// Class representing a Cluster
class XAPIAN_VISIBILITY_DEFAULT Cluster {
    friend class Point;
    //Documents (or Points in the vector space) within the cluster
    std::vector<Point> cluster_docs;
    // Point or Document representing the cluster centroid
    Centroid centroid;
  public:
    Cluster(Centroid centroid_) : centroid(centroid_) {}

    // Returns size of the cluster
    Xapian::doccount size();

    // Returns the docid of the centroid
    Centroid get_centroid();

    // Sets the centroid of the Cluster to centroid_
    void set_centroid(const Centroid centroid_);

    // Recalculates the cluster centroid to be the mean of document vectors
    void recalculate();

    // Add a document to the Cluster 
    void add_cluster(Point &doc);

    // Clears the cluster values
    void clear();

    // Returns the point at the given index in the cluster
    Point get_index(int index);

    // Returns the documents that are contained within the cluster
    DocumentSet get_documents();

    /** Calculates the average distance between the points and the
     *  centroids within a cluster
     */
    double advdc();
};

class ClusterSetIterator;

// Class for storing the results returned by the Clusterer
class XAPIAN_VISIBILITY_DEFAULT ClusterSet {
    friend class ClusterSetIterator;
    /** A map storing the clusterid and its corresponding
     *  Document cluster
     */
    std::vector<Cluster> clusters;
  public:
    // Adds a cluster to the cluster set
    void add_cluster(Cluster &c);

    // Returns a vector of documents
    Cluster get_cluster(clusterid id);

    // Add the point the the cluster at index 'i'
    void add_to_cluster(Point &x, clusterid i);

    // Returns the number of clusters
    Xapian::doccount size();

    // Returns the size of a cluster with clusterid 'cid'
    Xapian::doccount cluster_size(clusterid cid);

    // Used to check the cluster at index 'i'
    Cluster operator[](Xapian::doccount i);

    // Clears all the Clusters in the ClusterSet
    void clear_clusters();

    /** Recalculate the centroids for all the centroids
     *  in the ClusterSet
     */
    void recalculate_centroids();
};

// A class used to iterate through the ClusterSet
class XAPIAN_VISIBILITY_DEFAULT ClusterSetIterator {
    friend class ClusterSet;
  public:
    ClusterSet cset;
    Xapian::doccount index;

    ClusterSetIterator(ClusterSet &cset_, Xapian::doccount index_)
	: cset(cset_), index(index_) {}

    clusterid operator*() const;

    // Returns the id of the current cluster
    clusterid get_clusterid() {
	return index;
    }

    Cluster get_cluster();

    // Prefix increment operator for this iterator
    ClusterSetIterator & operator++() {
	index++;
	return *this;
    }

    // Postfix increment operator for this iterator
    ClusterSetIterator operator++(int) {
	ClusterSetIterator *c = this;
	index++;
	return *c;
    }

    // Prefix decrement operator for this iterator
    ClusterSetIterator & operator--() {
	index--;
	return *this;
    }

    // Postfix decrement operator for this iterator
    ClusterSetIterator operator--(int) {
	ClusterSetIterator c = *this;
	index--;
	return c;
    }
};

// Equality test for ClusterSetIterator
inline bool operator==(const ClusterSetIterator &a, const ClusterSetIterator &b) {
    return (a.index == b.index);
}

// Inequality test for ClusterSetIterator
inline bool operator!=(const ClusterSetIterator &a, const ClusterSetIterator &b) {
    return !(a == b);
}

// Equality test for DocumentSetIterator
inline bool operator==(const DocumentSetIterator &a, const DocumentSetIterator &b) {
    return (a.index == b.index);
}

// Inequality test for DocumentSetIterator
inline bool operator!=(const DocumentSetIterator &a, const DocumentSetIterator &b) {
    return !(a == b);
}

/** Round Robin clusterer:
 *  This clusterer is a minimal clusterer which will cluster documents as -
 *  ith document goes to the (i%N)th cluster where N is the number of clusters and
 *  0 <= i <= k; where k is the number of documents
 */
class XAPIAN_VISIBILITY_DEFAULT RoundRobin {
  public:
    ~RoundRobin();

    // Implements RoundRobin clustering
    ClusterSet cluster(MSet &mset, unsigned int num_of_clusters);

    // Returns the description of the clusterer
    std::string get_description();
};

/** Kmeans clusterer:
 *  This clusterer implements the K-Means clustering algorithm
 */
class XAPIAN_VISIBILITY_DEFAULT KMeans {
    std::vector<Point> docs;
    std::vector<Centroid> centroids;
    unsigned int k;
    std::string mode;
  public:
    KMeans(unsigned int k_) : k(k_) { mode = "random"; }
    KMeans(unsigned int k_, std::string mode_) : k(k_), mode(mode_) {}
    /** Checks whether the current state of KMeans has converged
     *  by checking for change in centroid of the clusters
     */
    bool converge(std::vector<Centroid> &previous, std::vector<Centroid> &current);

    /** Initialization of centroids using a certain specified method
     *  Current methods that are supported :
     *  random - Random Initialization
     *  kmeanspp - KMeans++ Initialization
     */
    void initialize_centroids(ClusterSet &cset);

    // Random initialization of cluster centroids
    void initialize_random(ClusterSet &cset);

    /** Uses KMeans++ initialization with roulette wheel selection
     *  for proportional fitness selection
     */
    void initialize_kmeanspp(ClusterSet &cset);

    /** Initialize the points in which contain the documents to be
     *  clusterid
     */
    void initialize_points(MSetDocumentSource docs, TermListGroup &tlg);

    // Implements KMeans clustering
    ClusterSet cluster(MSet &mset);

    // Returns the description of the clusterer
    std::string get_description();
};
};

#endif
