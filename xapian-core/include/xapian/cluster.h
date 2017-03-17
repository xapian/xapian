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
}
#endif
