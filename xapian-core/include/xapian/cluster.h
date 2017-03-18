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

namespace Xapian {

/** Class representing a set of documents in a cluster
 */
class XAPIAN_VISIBILITY_DEFAULT DocumentSet {

  private:

    /// Vector storing the documents for this DocumentSet
    std::vector<Document> docs;

  public:

    /// This method returns the size of the DocumentSet
    int size() const;

    /// This method returns the Document in the DocumentSet at index i
    Xapian::Document operator[](Xapian::doccount i);

    /// This method adds a new Document to the DocumentSet
    void add_document(Document doc);
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

/** Base class for TermListGroup
 *  This class stores and provides terms that are contained in a document
 *  and their respective term frequencies
 */
class XAPIAN_VISIBILITY_DEFAULT FreqSource {

  protected:

    /** This contains a map of the terms and its corresponding term frequencies.
     *  The term frequency of a term stands for the number of documents it indexes
     */
    std::unordered_map<std::string, doccount> termfreq;

  public:

    /// Destructor
    virtual ~FreqSource();

    /// This method returns the term frequency of a particular term 'tname'
    virtual doccount get_termfreq(const std::string &tname) = 0;

    /// This method returns the number of documents
    virtual doccount get_doccount() const = 0;
};

/** A class for dummy frequency source for construction of termlists
 *  This returns 1 as the term frequency for any term
 */
class XAPIAN_VISIBILITY_DEFAULT DummyFreqSource : public FreqSource {

  public:

    /// This method returns the value 1 as a dummy term frequency
    doccount get_termfreq(const std::string &);

    /// This method returns the total number of documents
    doccount get_doccount() const;
};

/** A class for construction of termlists which store the terms for a
 *  document along with the number of documents it indexes i.e. term
 *  frequency
 */
class XAPIAN_VISIBILITY_DEFAULT TermListGroup : public FreqSource {

  private:

    /// Number of documents added to the termlist
    doccount docs_num;

    /// This method adds a single document and calculates its corresponding stats
    void add_document(const Document &doc);

  public:

    /// This method adds a number of documents from the MSet
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

  protected:

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

    /// This method adds the value 'value' to the mapping of a term
    void add_value(std::string term, double value);

    /// This method sets the value 'value' to the mapping of a term
    void set_value(std::string term, double value);

  public:

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

    /// This method returns the size of the termlist
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

    /// Constructor
    Point() { magnitude = 0; }

    /// This method initializes the point with terms and corresponding term weights
    void initialize(TermListGroup &tlg, const Document &doc);

    /// This method returns the document corresponding to this Point
    Document get_document() const;
};

/** Class to represents a Cluster which contains Points and
 *  of the Cluster
 */
class XAPIAN_VISIBILITY_DEFAULT Cluster {

  private:

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
    Point get_index(unsigned int index) const;

    /// This method returns the documents that are contained within the cluster
    DocumentSet get_documents();
};

/** Class for storing the results returned by the Clusterer
 */
class XAPIAN_VISIBILITY_DEFAULT ClusterSet {

    /** A vector storing the clusters that have been created
     *  by the respective clusterers. Each cluster contains points.
     */
    std::vector<Cluster> clusters;

  public:

    /// This method adds a cluster to the cluster set
    void add_cluster(Cluster &c);

    /// This method returns the Cluster at position 'index'
    Cluster get_cluster(unsigned int index) const;

    /// This method adds the point the the cluster at position 'index'
    void add_to_cluster(const Point &x, unsigned int index);

    /// This method returns the number of clusters
    Xapian::doccount size() const;

    /// This method is used to check the cluster at index 'i'
    Cluster operator[](Xapian::doccount i);

    /// This method is used to clear all the Clusters in the ClusterSet
    void clear_clusters();
};
}
#endif
