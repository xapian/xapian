/* inmemory_database.h: C++ class definition for multiple database access
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2002,2003,2004 Olly Betts
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 * -----END-LICENCE-----
 */

#ifndef OM_HGUARD_INMEMORY_DATABASE_H
#define OM_HGUARD_INMEMORY_DATABASE_H

#include "omdebug.h"
#include "utils.h"
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
	string tname;
	vector<Xapian::termpos> positions; // Sorted vector of positions
	Xapian::termcount wdf;

	// Merge two postings (same term/doc pair, new positional info)
	void merge(const InMemoryPosting & post) {
	    Assert(did == post.did);
	    Assert(tname == post.tname);

	    positions.insert(positions.end(),
			     post.positions.begin(),
			     post.positions.end());
	    // FIXME - inefficient - use merge (and list<>)?
	    sort(positions.begin(), positions.end());
	}
};

// Compare by document ID
class InMemoryPostingLessByDocId {
    public:
	int operator() (const InMemoryPosting &p1, const InMemoryPosting &p2)
	{
	    return p1.did < p2.did;
	}
};

// Compare by termname
class InMemoryPostingLessByTermName {
    public:
	int operator() (const InMemoryPosting &p1, const InMemoryPosting &p2)
	{
	    return p1.tname < p2.tname;
	}
};

// Class representing a term and the documents indexing it
class InMemoryTerm {
    public:
	vector<InMemoryPosting> docs;// Sorted list of documents indexing term

	Xapian::termcount collection_freq;

	InMemoryTerm() : collection_freq(0) {}

	void add_posting(const InMemoryPosting & post) {
	    // Add document to right place in list
	    vector<InMemoryPosting>::iterator p;
	    p = lower_bound(docs.begin(), docs.end(),
			    post, InMemoryPostingLessByDocId());
	    if (p == docs.end() || InMemoryPostingLessByDocId()(post, *p)) {
		docs.insert(p, post);
	    } else {
		(*p).merge(post);
	    }
	}
};

/// Class representing a document and the terms indexing it
class InMemoryDoc {
    public:
	bool is_valid;
	vector<InMemoryPosting> terms;// Sorted list of terms indexing document

	/* Initialise valid */
	InMemoryDoc() : is_valid(true) {}
	void add_posting(const InMemoryPosting & post) {
	    // Add document to right place in list
	    vector<InMemoryPosting>::iterator p;
	    p = lower_bound(terms.begin(), terms.end(),
			    post, InMemoryPostingLessByTermName());
	    if (p == terms.end() || InMemoryPostingLessByTermName()(post, *p)) {
		terms.insert(p, post);
	    } else {
		(*p).merge(post);
	    }
	}
};

/** A PostList in an inmemory database.
 */
class InMemoryPostList : public LeafPostList {
    friend class InMemoryDatabase;
    private:
	vector<InMemoryPosting>::const_iterator pos;
	vector<InMemoryPosting>::const_iterator end;
	string tname;
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

	bool   at_end() const;        // True if we're off the end of the list

	string get_description() const;
};

// Term List
class InMemoryTermList : public LeafTermList {
    friend class InMemoryDatabase;
    private:
	vector<InMemoryPosting>::const_iterator pos;
	vector<InMemoryPosting>::const_iterator end;
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
	bool   at_end() const;
	Xapian::PositionIterator positionlist_begin() const;
};

/** A database held entirely in memory.
 *
 *  This is a prototype database, mainly used for debugging and testing.
 */
class InMemoryDatabase : public Xapian::Database::Internal {
    private:
	map<string, InMemoryTerm> postlists;
	vector<InMemoryDoc> termlists;
	vector<std::string> doclists;
	vector<std::map<Xapian::valueno, string> > valuelists;

	vector<Xapian::doclength> doclengths;

	Xapian::doccount totdocs;

	Xapian::doclength totlen;

	bool indexing; // Whether we have started to index to the database

	// Stop copy / assignment being allowed
	InMemoryDatabase& operator=(const InMemoryDatabase &);
	InMemoryDatabase(const InMemoryDatabase &);

	void make_term(const string & tname);

	bool doc_exists(Xapian::docid did) const;
	Xapian::docid make_doc(const string & docdata);

	/* The common parts of add_doc and replace_doc */
	void finish_add_doc(Xapian::docid did, const Xapian::Document &document);
	void add_values(Xapian::docid did, const map<Xapian::valueno, string> &values_);

	void make_posting(const string & tname,
			  Xapian::docid did,
			  Xapian::termpos position,
			  Xapian::termcount wdf,
			  bool use_position = true);

	//@{
	/** Implementation of virtual methods: see Database for details.
	 */
	void flush();

	Xapian::docid add_document(const Xapian::Document & document);
	void delete_document(Xapian::docid did);
	void replace_document(Xapian::docid did, const Xapian::Document & document);
	//@}

    public:
#if 0
	/** Hack put in to cause an error when calling next, for testing
	 *  purposes.
	 */
	int error_in_next;
	int abort_in_next;
#endif

	/** Create and open an in-memory database.
	 *
	 *  @exception Xapian::DatabaseOpeningError thrown if database can't be opened.
	 */
	InMemoryDatabase();

	~InMemoryDatabase();

	Xapian::doccount  get_doccount() const;
	Xapian::docid get_lastdocid() const;
	Xapian::doclength get_avlength() const;
	Xapian::doclength get_doclength(Xapian::docid did) const;

	Xapian::doccount get_termfreq(const string & tname) const;
	Xapian::termcount get_collection_freq(const string & tname) const;
	bool term_exists(const string & tname) const;

	LeafPostList * do_open_post_list(const string & tname) const;
	LeafTermList * open_term_list(Xapian::docid did) const;
	Xapian::Document::Internal * open_document(Xapian::docid did, bool lazy = false) const;
	PositionList * open_position_list(Xapian::docid did,
					  const string & tname) const;
	TermList * open_allterms() const;
};

//////////////////////////////////////////////
// Inline function definitions for postlist //
//////////////////////////////////////////////

inline
InMemoryPostList::InMemoryPostList(Xapian::Internal::RefCntPtr<const InMemoryDatabase> db_,
				   const InMemoryTerm & term)
	: pos(term.docs.begin()),
	  end(term.docs.end()),
	  tname(pos->tname),
	  termfreq(term.docs.size()),
	  started(false),
	  db(db_)
{
    // InMemoryPostLists cannot be empty
    Assert(pos != end);
}

inline Xapian::doccount
InMemoryPostList::get_termfreq() const
{
    return termfreq;
}

inline Xapian::docid
InMemoryPostList::get_docid() const
{
    //DebugMsg(tname << ".get_docid()");
    Assert(started);
    Assert(!at_end());
    //DebugMsg(" = " << (*pos).did << endl);
    return (*pos).did;
}

inline PostList *
InMemoryPostList::next(Xapian::weight /*w_min*/)
{
#if 0
    if (db->error_in_next) {
	// Nasty cast, but this is only in testcase code anyway.
	(const_cast<InMemoryDatabase *>(db.get()))->error_in_next--;
	if (db->error_in_next == 0)
	    throw Xapian::DatabaseCorruptError("Fake error - this should only be thrown when testing error handling.");
    }

    if (db->abort_in_next) {
	(const_cast<InMemoryDatabase *>(db.get()))->abort_in_next--;
	if (db->abort_in_next == 0)
	    abort();
    }
#endif

    if (started) {
	Assert(!at_end());
	pos++;
    } else {
	started = true;
    }
    return NULL;
}

inline PostList *
InMemoryPostList::skip_to(Xapian::docid did, Xapian::weight w_min)
{
    //DebugMsg(tname << ".skip_to(" << did << ")" << endl);
    // FIXME - see if we can make more efficient, perhaps using better
    // data structure.  Note, though, that a binary search of
    // the remaining list may NOT be a good idea (search time is then
    // O(log {length of list}), as opposed to O(distance we want to skip)
    // Since we will frequently only be skipping a short distance, this
    // could well be worse.
    Assert(!at_end());
    started = true;
    while (!at_end() && (*pos).did < did) {
	(void) next(w_min);
    }
    return NULL;
}

inline bool
InMemoryPostList::at_end() const
{
    return (pos == end);
}

inline string
InMemoryPostList::get_description() const
{
    return tname + ":" + om_tostring(termfreq);
}

//////////////////////////////////////////////
// Inline function definitions for termlist //
//////////////////////////////////////////////

inline
InMemoryTermList::InMemoryTermList(Xapian::Internal::RefCntPtr<const InMemoryDatabase> db_,
				   Xapian::docid did_,
				   const InMemoryDoc & doc,
				   Xapian::doclength len)
	: pos(doc.terms.begin()), end(doc.terms.end()), terms(doc.terms.size()),
	  started(false), db(db_), did(did_)
{
    DEBUGLINE(DB, "InMemoryTermList::InMemoryTermList(): " <<
	          terms << " terms starting from " << pos->tname);
    document_length = len;
    return;
}

inline Xapian::termcount
InMemoryTermList::get_approx_size() const
{
    return terms;
}

inline OmExpandBits
InMemoryTermList::get_weighting() const
{
    Assert(started);
    Assert(!at_end());
    Assert(wt != NULL);

    return wt->get_bits(InMemoryTermList::get_wdf(), document_length,
			InMemoryTermList::get_termfreq(),
			db->get_doccount());
}

inline string
InMemoryTermList::get_termname() const
{
    Assert(started);
    Assert(!at_end());
    return (*pos).tname;
}

inline Xapian::termcount
InMemoryTermList::get_wdf() const
{
    Assert(started);
    Assert(!at_end());
    return (*pos).wdf;
}

inline Xapian::doccount
InMemoryTermList::get_termfreq() const
{
    Assert(started);
    Assert(!at_end());

    return db->get_termfreq((*pos).tname);
}

inline TermList *
InMemoryTermList::next()
{
    if (started) {
	Assert(!at_end());
	pos++;
    } else {
	started = true;
    }
    return NULL;
}

inline bool
InMemoryTermList::at_end() const
{
    Assert(started);
    return (pos == end);
}

inline Xapian::PositionIterator
InMemoryTermList::positionlist_begin() const
{
    return Xapian::PositionIterator(db->open_position_list(did, (*pos).tname));
}

//////////////////////////////////////////////
// Inline function definitions for database //
//////////////////////////////////////////////

inline Xapian::doccount
InMemoryDatabase::get_doccount() const
{
    return totdocs;
}

inline Xapian::docid
InMemoryDatabase::get_lastdocid() const
{
    return termlists.size();
}

inline Xapian::doclength
InMemoryDatabase::get_avlength() const
{
    Xapian::doccount docs = InMemoryDatabase::get_doccount();
    if (docs == 0) return 0;
    return ((Xapian::doclength) totlen) / docs;
}

inline Xapian::doccount
InMemoryDatabase::get_termfreq(const string & tname) const
{
    map<string, InMemoryTerm>::const_iterator i = postlists.find(tname);
    if (i == postlists.end()) return 0;
    return i->second.docs.size();
}

inline Xapian::termcount
InMemoryDatabase::get_collection_freq(const string &tname) const
{
    map<string, InMemoryTerm>::const_iterator i = postlists.find(tname);
    if (i == postlists.end()) return 0;
    return i->second.collection_freq;
}

inline Xapian::doclength
InMemoryDatabase::get_doclength(Xapian::docid did) const
{
    if (!doc_exists(did)) {
	throw Xapian::DocNotFoundError(string("Docid ") + om_tostring(did) +
				 string(" not found"));
    }
    return doclengths[did - 1];
}

#endif /* OM_HGUARD_INMEMORY_DATABASE_H */
