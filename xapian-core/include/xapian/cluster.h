/** \file cluster.h
 * \brief API for clustering groups of documents.
 */
/* Copyright 2007 Lemur Consulting
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 */

#ifndef XAPIAN_INCLUDED_CLUSTER_H
#define XAPIAN_INCLUDED_CLUSTER_H

#include <xapian/database.h>
#include <xapian/enquire.h>
#include <xapian/expanddecider.h>
#include <xapian/visibility.h>

#include <map>
#include <string>
#include <vector>

namespace Xapian {

class Document;

/** Base class for a source of Documents
 *
 *  This is used to supply a set of Documents to a clustering algorithm.
 *
 *  @todo This should probably be a C++ iterator.
 */
class XAPIAN_VISIBILITY_DEFAULT DocumentSource {
  public:
    virtual ~DocumentSource();

    /** Get the next document from the source.
     *
     *  `at_end()` should always be checked before calling next_document().
     */
    virtual Document next_document() = 0;

    /** Check if we're at the end of the list of documents.
     *
     *  If we are, this returns True, and `next_document()` should no longer be
     *  called.
     */
    virtual bool at_end() const = 0;
};

/** A source of documents from an MSet.
 */
class XAPIAN_VISIBILITY_DEFAULT MSetDocumentSource : public DocumentSource {
    /// The mset to read documents from.
    const MSet mset;

    /// The maximum number of items to read.
    Xapian::doccount maxitems;

    /// The index of the next item to return.
    Xapian::doccount index;
  public:

    /** Construct a source of documents from an MSet.
     *
     *  @param mset_ The mset to take the documents from.  All the hits held in
     *  the MSet are used.
     */
    MSetDocumentSource(const MSet & mset_);

    /** Construct a source of documents from part of an MSet.
     *
     *  @param mset_ The mset to take documents from.
     *
     *  @param maxitems_ The maximum number of items to take from the MSet.  If
     *  the MSet contains more items than this, only the first items in the
     *  MSet will be returned.
     */
    MSetDocumentSource(const MSet & mset_, Xapian::doccount maxitems_);

    /** Get the next document from the source.
     *
     *  `at_end()` should always be checked before calling next_document().
     */
    Document next_document();

    /** Check if we're at the end of the list of documents.
     *
     *  If we are, this returns True, and `next_document()` should no longer be
     *  called.
     */
    bool at_end() const;
};

/** Base class of classes which provide term frequencies.
 */
class XAPIAN_VISIBILITY_DEFAULT TermFreqSource {
  public:
    virtual ~TermFreqSource();

    /** Get the frequency of a term.
     */
    virtual Xapian::doccount get_termfreq(const std::string &tname) const = 0;

    /** Get the number of documents being considered.
     */
    virtual Xapian::doccount get_doccount() const = 0;
};

/** A class which always claims that the term frequency is 1.
 */
class XAPIAN_VISIBILITY_DEFAULT DummyTermFreqSource : public TermFreqSource{
  public:

    /** Get the frequency of a term.  This always returns 1.
     */
    Xapian::doccount get_termfreq(const std::string &) const;

    /** Get the number of documents being considered.  This always returns 1.
     */
    Xapian::doccount get_doccount() const;
};

/** A class which gets term frequencies from a database.
 */
class XAPIAN_VISIBILITY_DEFAULT DatabaseTermFreqSource : public TermFreqSource {
    /// The database from which term frequencies are retrieved.
    Database db;
  public:

    /** Construct a DatabaseTermFreqSource from a database.
     *
     *  @param db The database to retrieve term frequencies from.  A copy of
     *  this database object will be held by the DatabaseTermFreqSource.
     */
    DatabaseTermFreqSource(const Database & db_) : db(db_) {}

    /** Get the frequency of a term.
     */
    Xapian::doccount get_termfreq(const std::string &tname) const;

    /** Get the number of documents in the database.
     */
    Xapian::doccount get_doccount() const;
};

/// A class holding a term and a WDF value for that term.
struct XAPIAN_VISIBILITY_DEFAULT TermWdf {
    /** The term name.
     */
    std::string tname;

    /** The wdf of the term.
     *
     *  The wdf (within document frequency) is the number of occurences
     *  of a term in a particular document.
     */
    Xapian::termcount wdf;

    TermWdf(const std::string & tname_, Xapian::termcount wdf_)
	    : tname(tname_), wdf(wdf_)
    {}
};

/** A group of termlists.
 *
 *  This is supplied with termlists, and caches the termlists for future
 *  reference.
 *
 *  It also analyses the termlists to allow the number of times each term in
 *  the termlists occurs, and exports this information by implementing the
 *  TermFreqSource interface.
 */
class XAPIAN_VISIBILITY_DEFAULT TermListGroup : public TermFreqSource {
    /** A map holding the terms and their WDFs in each document.
     */
    std::map<Xapian::docid, std::vector<TermWdf> > termlists;

    /// A map holding the frequencies of the terms found in the termlists.
    std::map<std::string, Xapian::doccount> termfreqs;

  public:
    /** Add a document to the termlist group.
     *
     *  The document's termlist will be read and stored in the group.
     *
     *  @param document The document to read the termlist from.
     */
    void add_document(const Document & document)
    {
	add_document(document, NULL);
    }

    /** Add a document to the termlist group.
     *
     *  The document's termlist will be read and stored in the group.
     *
     *  @param document The document to read the termlist from.
     *  @param decider A decider to determine which terms should be stored.  If
     *                 NULL, all terms will be stored.
     */
    void add_document(const Document & document,
		      const ExpandDecider * decider);

    /** Add all documents from a document source to the termlist group.
     *
     *  All the documents supplied by the source will have their termlists read
     *  and stored in the group.
     *
     *  @param source The source to read the documents from.
     */
    void add_documents(DocumentSource & source)
    {
	add_documents(source, NULL);
    }

    /** Add all documents from a document source to the termlist group.
     *
     *  All the documents supplied by the source will have their termlists read
     *  and stored in the group.
     *
     *  @param source The source to read the documents from.
     *  @param decider A decider to determine which terms should be stored.  If
     *                 NULL, all terms will be stored.
     */
    void add_documents(DocumentSource & source,
		       const ExpandDecider * decider);

    /** Get the frequency of a term.
     *
     *  If the term is not found in the termlists (either because it wasn't
     *  present in the documents, or because it was excluded by the decider
     *  functor), returns 0.  Otherwise, returns the number of termlists which
     *  the term was found in.
     */
    Xapian::doccount get_termfreq(const std::string &tname) const;

    /** Get the number of documents in the database.
     */
    Xapian::doccount get_doccount() const;

    /// Iterator for the terms in a given document.
    TermIterator termlist_begin(Xapian::docid did) const;

    /// Equivalent end iterator for termlist_begin().
    TermIterator termlist_end(Xapian::docid) const {
	return TermIterator(NULL);
    }
};

/// Base class of document similarity calculation
class XAPIAN_VISIBILITY_DEFAULT DocSim {
  protected:
    const TermFreqSource * freqsource;
  public:
    DocSim() : freqsource(NULL) {}

    virtual ~DocSim();

    /** Set a TermFreqSource for the similarity calculation.
     *
     *  The caller must ensure that the freqsource remains valid for the lifetime
     *  of the DocSim object.
     */
    void set_termfreqsource(const TermFreqSource * freqsource_)
    {
	freqsource = freqsource_;
    }

    /// Calculate the similarity between two documents.
    double similarity(const Document & a, const Document & b) const
    {
	return similarity(a.termlist_begin(), a.termlist_end(),
			  b.termlist_begin(), b.termlist_end());
    }

    /** Calculate the similarity between the documents represented by two termlists.
     */
    virtual double similarity(TermIterator a_begin,
			      const TermIterator & a_end,
			      TermIterator b_begin,
			      const TermIterator & b_end) const = 0;

    /// Return a string describing this object.
    virtual std::string get_description() const = 0;
};

/// Modified cosine similarity match.
class XAPIAN_VISIBILITY_DEFAULT DocSimCosine : public DocSim {

  public:
    ~DocSimCosine();

    /// Calculate the similarity between two documents.
    double similarity(TermIterator a_begin,
		      const TermIterator & a_end,
		      TermIterator b_begin,
		      const TermIterator & b_end) const;

    /// Return a string describing this object.
    std::string get_description() const;
};

struct XAPIAN_VISIBILITY_DEFAULT ClusterAssignments {
    /// Map from document ID to a cluster ID.
    std::map<Xapian::docid, int> ids;

    int cluster(Xapian::docid did) const {
	std::map<Xapian::docid, int>::const_iterator i = ids.find(did);
	if (i == ids.end()) return -1;
	return i->second;
    }
};

class XAPIAN_VISIBILITY_DEFAULT ClusterSingleLink {
  public:
    void cluster(ClusterAssignments & clusters,
		 DocSimCosine & docsim,
		 DocumentSource & docsource,
		 const ExpandDecider * decider,
		 int num_clusters);
};

}

#endif // XAPIAN_INCLUDED_CLUSTER_H
