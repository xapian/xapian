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

#include <string>
#include <unordered_map>
#include <vector>

#include <xapian/visibility.h>
#include <xapian/types.h>
#include <xapian/mset.h>

namespace Xapian {

class DocumentSetIterator;

/** Class representing a set of documents in a cluster
 */
class XAPIAN_VISIBILITY_DEFAULT DocumentSet {

    friend class DocumentSetIterator;

    /// Vector storing the documents for this DocumentSet
    std::vector<Document> docs;

  public:

    /// This method returns the size of the DocumentSet
    int size() const;

    /// This method returns the Document in the DocumentSet at index i
    Xapian::Document operator[](Xapian::doccount i);

    /** This method returns an iterator to the start of the DocumentSet to
     * iterate through all the documents
     */
    DocumentSetIterator begin() const;

    /// This method returns an iterator to the end of the DocumentSet
    DocumentSetIterator end() const;

    /// This method adds a new Document to the DocumentSet
    void add_document(Document doc);
};

/** Class used to iterate through the DocumentSet
 */
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

/** Abstract class used for construction of the TermListGroup
 */
class XAPIAN_VISIBILITY_DEFAULT FreqSource {

  public:

    /** This contains a map of the terms and its corresponding term frequencies.
     *  The term frequency of a term stands for the number of documents it indexes
     */
    std::unordered_map<std::string, doccount> termfreq;

    /// Destructor
    virtual ~FreqSource();

    /// This method returns the term frequency of a particular term 'tname'
    virtual doccount get_termfreq(const std::string &tname) = 0;

    /// This method returns the number of documents
    virtual doccount get_doccount() const = 0;
};

/** Class for dummy frequency source for construction of termlists
 */
class DummyFreqSource : public FreqSource {

  public:

    /// This method returns the value 1 as a dummy term frequency
    doccount get_termfreq(const std::string &);

    /// This method returns the total number of documents
    doccount get_doccount() const;
};

/** A class for construction of termlists
 */
class XAPIAN_VISIBILITY_DEFAULT TermListGroup : public FreqSource {

  public:

    /** This method adds document terms and their statistics to the FreqSource
     *  for construction of termlists
     */
    doccount docs_num;

    /// This method adds a single document and calculates its corresponding stats
    void add_document(const Document &doc);

    /// This method adds a number of documents from the DocumentSource
    void add_documents(const MSet &docs);

    /** This method returns the number of documents that the term 'tname' exists in
     *  or the number of documents that a certain term indexes
     */
    doccount get_termfreq(const std::string &tname);

    /// This method returns the total number of documents
    doccount get_doccount() const;
};

/** Abstract class representing a point in the VSM
 */
class XAPIAN_VISIBILITY_DEFAULT PointType {

  public:

    /** This contains the termlist which is used by our
     *  TermIterator working over the PointType
     *  It contains terms and their corresponding wdf's
     */
    std::vector<struct Wdf> termlist;

    /** This implements a map to store the terms within a document
     *  and their pre-computed TF-IDF values
     */
    std::unordered_map<std::string, double> values;

    /// This stores the squared magnitude of the PointType
    double magnitude;

    /// This method returns a TermIterator to the beginning of the termlist
    TermIterator termlist_begin() const;

    /// This method returns a TermIterator to the end of the termlist
    TermIterator termlist_end() const;

    /** This method validates whether a certain term exists in the termlist
     *  or not by performing a lookup operation in the existing values
     */
    bool contains(std::string term);

    /// This method returns the TF-IDF weight associated with a certain term
    double get_value(std::string term);

    /// This method returns the pre-computed squared magnitude
    double get_magnitude() const;

    /// This method adds the value 'value' to the mapping of a term
    void add_value(std::string term, double value);

    /// This method sets the value 'value' to the mapping of a term
    void set_value(std::string term, double value);

    /// This method returns the size of the termlist
    int termlist_size() const;
};

/** Class to represent a document as a point in the Vector Space
 *  Model
 */
class XAPIAN_VISIBILITY_DEFAULT Point : public PointType {

    /// The document which is being represented by the Point
    Document doc;

  public:

    /// Constructor
    Point() { magnitude = 0; }

    /// This method initializes the point with terms and corresponding term weights
    void initialize(TermListGroup &tlg, const Document &doc);

    /// This method returns the document corresponding to this Point
    Document get_document() const;
};

/** Base class for calculating the similarity between documents
 */
class XAPIAN_VISIBILITY_DEFAULT Similarity {

  public:

    /// Destructor
    virtual ~Similarity();

    /// Calculates the similarity between the two documents
    virtual double similarity(PointType &a, PointType &b) const = 0;

    /// Returns description of the similarity metric being used
    virtual std::string get_description() const = 0;
};

/** Class for calculating Euclidian Distance between two points
 */
class XAPIAN_VISIBILITY_DEFAULT EuclidianDistance : public Similarity {

  public:

    /** This method calculates and returns the euclidian distance using the
     *  Euclidian distance formula
     */
    double similarity(PointType &a, PointType &b) const;

    /// This method returns the description of Euclidian Distance
    std::string get_description() const;
};

/** Class for calculating the cosine distance between two documents
 */
class XAPIAN_VISIBILITY_DEFAULT CosineDistance : public Similarity {

  public:

    /** This method calculates and returns the cosine similarity using the
     *  formula  cos(theta) = a.b/(|a|*|b|)
     */
    double similarity(PointType &a, PointType &b) const;

    /// This method returns the description of Cosine Similarity
    std::string get_description() const;
};

/** Class to represents a Cluster which contains the clustered documents
 */
class XAPIAN_VISIBILITY_DEFAULT Cluster {

    /// Documents (or Points in the vector space) within the cluster
    std::vector<Point> cluster_docs;

  public:

    /// Constructor
    Cluster();

    /// Destructor
    ~Cluster();

    /// This method returns size of the cluster
    Xapian::doccount size() const;

    /// This method adds a document to the Cluster
    void add_point(const Point &doc);

    /// This method clears the cluster values
    void clear();

    /// This method returns the point at the given index in the cluster
    Point get_index(int index) const;

    /// This method returns the documents that are contained within the cluster
    DocumentSet get_documents();
};

/** Class for storing the results returned by the Clusterer
 */
class XAPIAN_VISIBILITY_DEFAULT ClusterSet {

    /** A map storing the clusterid and its corresponding
     *  Document cluster
     */
    std::vector<Cluster> clusters;

  public:

    /// This method adds a cluster to the cluster set
    void add_cluster(Cluster &c);

    /// This method returns a vector of documents
    Cluster get_cluster(clusterid id) const;

    /// This method adds the point the the cluster at index 'i'
    void add_to_cluster(const Point &x, clusterid i);

    /// This method returns the number of clusters
    Xapian::doccount size() const;

    /// This method returns the size of a cluster with clusterid 'cid'
    Xapian::doccount cluster_size(clusterid cid) const;

    /// This method is used to check the cluster at index 'i'
    Cluster operator[](Xapian::doccount i);

    /// This method is used to clear all the Clusters in the ClusterSet
    void clear_clusters();
};

/** This class represents an abstract class for a clusterer to be implemented
 */
class XAPIAN_VISIBILITY_DEFAULT Clusterer {

  public:

    /// Destructor
    virtual ~Clusterer();

    /// This method helps implement the required clustering algorithm in the subclass
    virtual ClusterSet cluster(MSet &mset) = 0;

    /// This method returns a description of the clusterer being used
    virtual std::string get_description() const = 0;
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
    std::string get_description() const;
};
}
#endif
