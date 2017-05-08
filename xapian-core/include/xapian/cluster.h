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

#include <map>
#include <string>
#include <vector>

#include <xapian/visibility.h>
#include <xapian/types.h>
#include <xapian/mset.h>

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

    doccount size();
};

class DocumentSetIterator;

// Class representing a set of documents in a cluster
class XAPIAN_VISIBILITY_DEFAULT DocumentSet {
    friend class DocumentSetIterator;
    //Vector storing the documents for this DocumentSet
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
    void add_document(Document &doc);
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
    std::map<std::string, Xapian::doccount> termfreq;

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
    void add_documents(DocumentSource &docs);

    Xapian::doccount get_termfreq(const std::string &tname);

    Xapian::doccount get_doccount();
};

// Base class for calculating the similarity between documents
class XAPIAN_VISIBILITY_DEFAULT Similarity {
  public:
    // Destructor
    virtual ~Similarity();

    // Calculates the similarity between the two documents
    virtual double similarity(TermListGroup tlg, TermIterator a_begin, TermIterator a_end, TermIterator b_begin, TermIterator b_end) = 0;

   // Returns description of the similarity metric being used
    virtual std::string get_description() = 0;
};

// Class for caluclating the euclidian distance between two documents
class XAPIAN_VISIBILITY_DEFAULT EuclidianDistance : public Similarity {
  public:
    double similarity(TermListGroup tlg, TermIterator a_begin, TermIterator a_end, TermIterator b_begin, TermIterator b_end);

    std::string get_description();
};

class ClusterSetIterator;

// Class for storing the results returned by the Clusterer
class XAPIAN_VISIBILITY_DEFAULT ClusterSet {
    friend class ClusterSetIterator;
    // A map storing the clusterid and its corresponding Document cluster
    std::map<clusterid, std::vector<Document> > clusters;
  public:
    // Returns a vector of documents
    DocumentSet get_cluster(clusterid id);

    // Adds a document to a cluster
    void add_document(clusterid id, Document &doc);

    // Returns the number of clusters
    Xapian::doccount num_of_clusters();

    // Returns the size of a cluster with clusterid 'cid'
    Xapian::doccount cluster_size(clusterid cid);

    /** Returns an iterator to the start of the clusters to analyze each
     *  cluster individually
     */
    ClusterSetIterator begin();

    // Returns an iterator to the end of the clusters
    ClusterSetIterator end();

    // Used to check the cluster at index 'i'
    ClusterSetIterator operator[](Xapian::doccount i);
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

    DocumentSet get_cluster();

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
    ClusterSet cluster(MSet &mset, unsigned int k);

    // Returns the description of the clusterer being used
    std::string get_description();
};
}
#endif
