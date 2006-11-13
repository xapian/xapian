/* inmemory_database.h: C++ class definition for multiple database access
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2002,2003,2004,2005,2006 Olly Betts
 * Copyright 2006 Richard Boulton
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

#ifndef OM_HGUARD_INMEMORY_DATABASE_H
#define OM_HGUARD_INMEMORY_DATABASE_H

#include "leafpostlist.h"
#include "termlist.h"
#include "database.h"
#include <stdlib.h>
#include <map>
#include <vector>
#include <algorithm>
#include <xapian/document.h>
#include "inmemory_positionlist.h"

using namespace std;

// Class representing a posting (a term/doc pair, and
// all the relevant positional information, is a single posting)
class InMemoryPosting {
    public:
	Xapian::docid did;
	bool valid;
	vector<Xapian::termpos> positions; // Sorted vector of positions
	Xapian::termcount wdf;

	// Merge two postings (same term/doc pair, new positional info)
	void merge(const InMemoryPosting & post) {
	    Assert(did == post.did);

	    positions.insert(positions.end(),
			     post.positions.begin(),
			     post.positions.end());
	    // FIXME - inefficient - use merge (and list<>)?
	    sort(positions.begin(), positions.end());
	}
};

class InMemoryTermEntry {
    public:
	string tname;
	vector<Xapian::termpos> positions; // Sorted vector of positions
	Xapian::termcount wdf;

	// Merge two postings (same term/doc pair, new positional info)
	void merge(const InMemoryTermEntry & post) {
	    Assert(tname == post.tname);

	    positions.insert(positions.end(),
			     post.positions.begin(),
			     post.positions.end());
	    // FIXME - inefficient - use merge (and list<>)?
	    sort(positions.begin(), positions.end());
	}
};

// Compare by document ID
class InMemoryPostingLessThan {
    public:
	int operator() (const InMemoryPosting &p1, const InMemoryPosting &p2)
	{
	    return p1.did < p2.did;
	}
};

// Compare by termname
class InMemoryTermEntryLessThan {
    public:
	int operator() (const InMemoryTermEntry&p1, const InMemoryTermEntry&p2)
	{
	    return p1.tname < p2.tname;
	}
};

// Class representing a term and the documents indexing it
class InMemoryTerm {
    public:
	// Sorted list of documents indexing this term.
	vector<InMemoryPosting> docs;

	Xapian::termcount term_freq;
	Xapian::termcount collection_freq;

	InMemoryTerm() : term_freq(0), collection_freq(0) {}

	void add_posting(const InMemoryPosting & post);
};

/// Class representing a document and the terms indexing it.
class InMemoryDoc {
    public:
	bool is_valid;
	// Sorted list of terms indexing this document.
	vector<InMemoryTermEntry> terms;

	/* Initialise valid */
	InMemoryDoc() : is_valid(true) {}

	void add_posting(const InMemoryTermEntry & post);
};

class InMemoryDatabase;

/** A PostList in an inmemory database.
 */
class InMemoryPostList : public LeafPostList {
    friend class InMemoryDatabase;
    private:
	vector<InMemoryPosting>::const_iterator pos;
	vector<InMemoryPosting>::const_iterator end;
	Xapian::doccount termfreq;
	bool started;

	/** List of positions of the current term.
	 *  This list is populated when read_position_list() is called.
	 */
	InMemoryPositionList mypositions;

	Xapian::Internal::RefCntPtr<const InMemoryDatabase> db;

	InMemoryPostList(Xapian::Internal::RefCntPtr<const InMemoryDatabase> db,
			 const InMemoryTerm & term);
    public:
	Xapian::doccount get_termfreq() const;

	Xapian::docid       get_docid() const;     // Gets current docid
	Xapian::doclength   get_doclength() const; // Length of current document
        Xapian::termcount   get_wdf() const;	      // Within Document Frequency
	PositionList * read_position_list();
	PositionList * open_position_list() const;

	PostList *next(Xapian::weight w_min); // Moves to next docid

	PostList *skip_to(Xapian::docid did, Xapian::weight w_min); // Moves to next docid >= specified docid

	// True if we're off the end of the list.
	bool at_end() const;

	string get_description() const;
};

/** A PostList over all docs in an inmemory database.
 */
class InMemoryAllDocsPostList : public LeafPostList {
    friend class InMemoryDatabase;
    private:
	Xapian::docid did;

	Xapian::Internal::RefCntPtr<const InMemoryDatabase> db;

	InMemoryAllDocsPostList(Xapian::Internal::RefCntPtr<const InMemoryDatabase> db);
    public:
	Xapian::doccount get_termfreq() const;

	Xapian::docid       get_docid() const;     // Gets current docid
	Xapian::doclength   get_doclength() const; // Length of current document
	Xapian::termcount   get_wdf() const;       // Within Document Frequency
	PositionList * read_position_list();
	PositionList * open_position_list() const;

	PostList *next(Xapian::weight w_min);      // Moves to next docid

	PostList *skip_to(Xapian::docid did, Xapian::weight w_min); // Moves to next docid >= specified docid

	// True if we're off the end of the list
	bool at_end() const;

	string get_description() const;
};

// Term List
class InMemoryTermList : public LeafTermList {
    friend class InMemoryDatabase;
    private:
	vector<InMemoryTermEntry>::const_iterator pos;
	vector<InMemoryTermEntry>::const_iterator end;
	Xapian::termcount terms;
	bool started;

	Xapian::Internal::RefCntPtr<const InMemoryDatabase> db;
	Xapian::docid did;
	Xapian::doclength document_length;

	InMemoryTermList(Xapian::Internal::RefCntPtr<const InMemoryDatabase> db, Xapian::docid did,
			 const InMemoryDoc & doc,
			 Xapian::doclength len);
    public:
	Xapian::termcount get_approx_size() const;

	OmExpandBits get_weighting() const;
	string get_termname() const;
	Xapian::termcount get_wdf() const; // Number of occurrences of term in current doc
	Xapian::doccount get_termfreq() const;  // Number of docs indexed by term
	TermList * next();
	bool at_end() const;
	Xapian::termcount positionlist_count() const;
	Xapian::PositionIterator positionlist_begin() const;
};

/** A database held entirely in memory.
 *
 *  This is a prototype database, mainly used for debugging and testing.
 */
class InMemoryDatabase : public Xapian::Database::Internal {
    friend class InMemoryAllDocsPostList;
    private:
	map<string, InMemoryTerm> postlists;
	vector<InMemoryDoc> termlists;
	vector<std::string> doclists;
	vector<std::map<Xapian::valueno, string> > valuelists;

	vector<Xapian::doclength> doclengths;

	Xapian::doccount totdocs;

	Xapian::doclength totlen;

	bool positions_present;

	// Stop copy / assignment being allowed
	InMemoryDatabase& operator=(const InMemoryDatabase &);
	InMemoryDatabase(const InMemoryDatabase &);

	void make_term(const string & tname);

	bool doc_exists(Xapian::docid did) const;
	Xapian::docid make_doc(const string & docdata);

	/* The common parts of add_doc and replace_doc */
	void finish_add_doc(Xapian::docid did, const Xapian::Document &document);
	void add_values(Xapian::docid did, const map<Xapian::valueno, string> &values_);

	void make_posting(InMemoryDoc * doc,
			  const string & tname,
			  Xapian::docid did,
			  Xapian::termpos position,
			  Xapian::termcount wdf,
			  bool use_position = true);

	//@{
	/** Implementation of virtual methods: see Database for details.
	 */
	void flush();
	void cancel();

	Xapian::docid add_document(const Xapian::Document & document);
	void delete_document(Xapian::docid did);
	void replace_document(Xapian::docid did, const Xapian::Document & document);
	//@}

    public:
	/** Create and open an in-memory database.
	 *
	 *  @exception Xapian::DatabaseOpeningError thrown if database can't be opened.
	 */
	InMemoryDatabase();

	~InMemoryDatabase();

	Xapian::doccount get_doccount() const;

	Xapian::docid get_lastdocid() const;

	Xapian::doclength get_avlength() const;
	Xapian::doclength get_doclength(Xapian::docid did) const;

	Xapian::doccount get_termfreq(const string & tname) const;
	Xapian::termcount get_collection_freq(const string & tname) const;
	bool term_exists(const string & tname) const;
	bool has_positions() const;

	LeafPostList * do_open_post_list(const string & tname) const;
	LeafTermList * open_term_list(Xapian::docid did) const;
	Xapian::Document::Internal * open_document(Xapian::docid did, bool lazy = false) const;
	PositionList * open_position_list(Xapian::docid did,
					  const string & tname) const;
	TermList * open_allterms() const;
};

#endif /* OM_HGUARD_INMEMORY_DATABASE_H */
