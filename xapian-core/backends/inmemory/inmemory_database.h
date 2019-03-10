/** @file inmemory_database.h
 * @brief C++ class definition for inmemory database access
 */
/* Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2002,2003,2004,2005,2006,2007,2008,2009,2010,2011,2014,2015 Olly Betts
 * Copyright 2006,2009 Lemur Consulting Ltd
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

#ifndef XAPIAN_INCLUDED_INMEMORY_DATABASE_H
#define XAPIAN_INCLUDED_INMEMORY_DATABASE_H

#include "api/leafpostlist.h"
#include "api/smallvector.h"
#include "api/termlist.h"
#include "backends/backends.h"
#include "backends/databaseinternal.h"
#include "backends/valuestats.h"
#include <map>
#include <vector>
#include <algorithm>
#include <xapian/document.h>
#include "inmemory_positionlist.h"
#include "internaltypes.h"
#include "omassert.h"

using namespace std;

// Class representing a posting (a term/doc pair, and
// all the relevant positional information, is a single posting)
class InMemoryPosting {
    public:
	Xapian::docid did;
	bool valid;
	Xapian::VecCOW<Xapian::termpos> positions; // Sorted vector of positions
	Xapian::termcount wdf;

	// Merge two postings (same term/doc pair, new positional info)
	void merge(const InMemoryPosting & post) {
	    Assert(did == post.did);

	    positions.reserve(positions.size() + post.positions.size());
	    for (auto&& pos : post.positions) {
		positions.push_back(pos);
	    }
	    inplace_merge(positions.begin(),
			  positions.begin() + post.positions.size(),
			  positions.end());
	}
};

class InMemoryTermEntry {
    public:
	string tname;
	Xapian::VecCOW<Xapian::termpos> positions; // Sorted vector of positions
	Xapian::termcount wdf;

	// Merge two postings (same term/doc pair, new positional info)
	void merge(const InMemoryTermEntry & post) {
	    Assert(tname == post.tname);

	    positions.reserve(positions.size() + post.positions.size());
	    for (auto&& pos : post.positions) {
		positions.push_back(pos);
	    }
	    inplace_merge(positions.begin(),
			  positions.begin() + post.positions.size(),
			  positions.end());
	}
};

// Compare by document ID
class InMemoryPostingLessThan {
    public:
	int operator() (const InMemoryPosting &p1,
			const InMemoryPosting &p2) const
	{
	    return p1.did < p2.did;
	}
};

// Compare by termname
class InMemoryTermEntryLessThan {
    public:
	int operator() (const InMemoryTermEntry&p1,
			const InMemoryTermEntry&p2) const
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

	void add_posting(InMemoryPosting&& post);
};

/// Class representing a document and the terms indexing it.
class InMemoryDoc {
    public:
	bool is_valid;
	// Sorted list of terms indexing this document.
	vector<InMemoryTermEntry> terms;

	/* Initialise invalid by default, so that resizing the termlist array
	 * doesn't create valid documents. */
	InMemoryDoc() : is_valid(false) {}

	// Initialise specifying validity.
	explicit InMemoryDoc(bool is_valid_) : is_valid(is_valid_) {}

	void add_posting(InMemoryTermEntry&& post);
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

	Xapian::Internal::intrusive_ptr<const InMemoryDatabase> db;

	InMemoryPostList(Xapian::Internal::intrusive_ptr<const InMemoryDatabase> db,
			 const InMemoryTerm & imterm, const std::string & term_);
    public:
	Xapian::doccount get_termfreq() const;

	Xapian::docid get_docid() const;     // Gets current docid
	Xapian::termcount get_wdf() const;	   // Within Document Frequency
	PositionList * read_position_list();
	PositionList * open_position_list() const;

	PostList *next(double w_min); // Moves to next docid

	PostList *skip_to(Xapian::docid did, double w_min); // Moves to next docid >= specified docid

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

	Xapian::Internal::intrusive_ptr<const InMemoryDatabase> db;

	InMemoryAllDocsPostList(Xapian::Internal::intrusive_ptr<const InMemoryDatabase> db);
    public:
	Xapian::doccount get_termfreq() const;

	Xapian::docid get_docid() const;     // Gets current docid
	Xapian::termcount get_doclength() const; // Length of current document
	Xapian::termcount get_unique_terms() const; // number of terms in current document
	Xapian::termcount get_wdf() const;       // Within Document Frequency
	PositionList * read_position_list();
	PositionList * open_position_list() const;

	PostList *next(double w_min);      // Moves to next docid

	PostList *skip_to(Xapian::docid did, double w_min); // Moves to next docid >= specified docid

	// True if we're off the end of the list
	bool at_end() const;

	string get_description() const;
};

// Term List
class InMemoryTermList : public TermList {
    friend class InMemoryDatabase;
    private:
	vector<InMemoryTermEntry>::const_iterator pos;
	vector<InMemoryTermEntry>::const_iterator end;
	Xapian::termcount terms;
	bool started;

	Xapian::Internal::intrusive_ptr<const InMemoryDatabase> db;
	Xapian::docid did;
	Xapian::termcount document_length;

	InMemoryTermList(Xapian::Internal::intrusive_ptr<const InMemoryDatabase> db,
			 Xapian::docid did,
			 const InMemoryDoc & doc,
			 Xapian::termcount len);
    public:
	Xapian::termcount get_approx_size() const;

	/// Collate weighting information for the current term.
	void accumulate_stats(Xapian::Internal::ExpandStats & stats) const;

	string get_termname() const;
	Xapian::termcount get_wdf() const; // Number of occurrences of term in current doc
	Xapian::doccount get_termfreq() const;  // Number of docs indexed by term
	TermList * next();
	TermList * skip_to(const std::string & term);
	bool at_end() const;
	Xapian::termcount positionlist_count() const;
	PositionList* positionlist_begin() const;
};

class InMemoryDocument;

/** A database held entirely in memory.
 *
 *  This is a prototype database, mainly used for debugging and testing.
 */
class InMemoryDatabase : public Xapian::Database::Internal {
    friend class InMemoryAllDocsPostList;
    friend class InMemoryDocument;

    map<string, InMemoryTerm> postlists;
    vector<InMemoryDoc> termlists;
    vector<std::string> doclists;
    vector<std::map<Xapian::valueno, string>> valuelists;
    std::map<Xapian::valueno, ValueStats> valuestats;

    vector<Xapian::termcount> doclengths;

    std::map<string, string> metadata;

    Xapian::doccount totdocs;

    Xapian::totallength totlen;

    bool positions_present;

    // Flag, true if the db has been closed.
    bool closed;

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
    void commit();
    void cancel();

    Xapian::docid add_document(const Xapian::Document & document);
    // Stop the default implementation of delete_document(term) and
    // replace_document(term) from being hidden.  This isn't really
    // a problem as we only try to call them through the base class
    // (where they aren't hidden) but some compilers generate a warning
    // about the hiding.
    using Xapian::Database::Internal::delete_document;
    using Xapian::Database::Internal::replace_document;
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

    bool reopen();
    void close();
    bool is_closed() const { return closed; }

    Xapian::doccount get_doccount() const;

    Xapian::docid get_lastdocid() const;

    Xapian::totallength get_total_length() const;
    Xapian::termcount get_doclength(Xapian::docid did) const;
    Xapian::termcount get_unique_terms(Xapian::docid did) const;

    void get_freqs(const string & term,
		   Xapian::doccount * termfreq_ptr,
		   Xapian::termcount * collfreq_ptr) const;
    Xapian::doccount get_value_freq(Xapian::valueno slot) const;
    std::string get_value_lower_bound(Xapian::valueno slot) const;
    std::string get_value_upper_bound(Xapian::valueno slot) const;
    Xapian::termcount get_doclength_lower_bound() const;
    Xapian::termcount get_doclength_upper_bound() const;
    Xapian::termcount get_wdf_upper_bound(const std::string& term) const;
    bool term_exists(const string & tname) const;
    bool has_positions() const;

    PostList * open_post_list(const string & tname) const;
    LeafPostList* open_leaf_post_list(const string& term,
				      bool need_read_pos) const;
    TermList * open_term_list(Xapian::docid did) const;
    TermList * open_term_list_direct(Xapian::docid did) const;
    Xapian::Document::Internal * open_document(Xapian::docid did, bool lazy) const;

    std::string get_metadata(const std::string & key) const;
    TermList * open_metadata_keylist(const std::string &prefix) const;
    void set_metadata(const std::string & key, const std::string & value);

    Xapian::termcount positionlist_count(Xapian::docid did,
					 const string & tname) const;
    PositionList * open_position_list(Xapian::docid did,
				      const string & tname) const;
    TermList * open_allterms(const string & prefix) const;

    [[noreturn]]
    static void throw_database_closed();

    int get_backend_info(string * path) const {
	if (path) *path = string();
	return BACKEND_INMEMORY;
    }

    std::string get_description() const;
};

#ifdef DISABLE_GPL_LIBXAPIAN
# error GPL source we cannot relicense included in libxapian
#endif

#endif /* XAPIAN_INCLUDED_INMEMORY_DATABASE_H */
