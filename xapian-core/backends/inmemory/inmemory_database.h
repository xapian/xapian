/* inmemory_database.h: C++ class definition for multiple database access
 *
 * ----START-LICENCE----
 * Copyright 1999,2000 BrightStation PLC
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
#include "indexer.h"
#include <stdlib.h>
#include <map>
#include <vector>
#include <algorithm>
#include "om/omdocument.h"
#include "om/omindexdoc.h"
#include "inmemory_positionlist.h"

// Class representing a posting (a term/doc pair, and
// all the relevant positional information, is a single posting)
class InMemoryPosting {
    public:
	om_docid did;
	om_termname tname;
	std::vector<om_termpos> positions; // Sorted vector of positions
	om_termcount wdf;

	// Merge two postings (same term/doc pair, new positional info)
	void merge(const InMemoryPosting & post) {
	    Assert(did == post.did);
	    Assert(tname == post.tname);

	    positions.insert(positions.end(),
			     post.positions.begin(),
			     post.positions.end());
	    // FIXME - inefficient - use merge (and list<>)?
	    std::sort(positions.begin(), positions.end());
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

// Compare by term ID
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
	std::vector<InMemoryPosting> docs;// Sorted list of documents indexing term
	void add_posting(const InMemoryPosting & post) {
	    // Add document to right place in list
	    std::vector<InMemoryPosting>::iterator p;
	    p = std::lower_bound(docs.begin(), docs.end(),
			    post,
			    InMemoryPostingLessByDocId());
	    if(p == docs.end() || InMemoryPostingLessByDocId()(post, *p)) {
		docs.insert(p, post);
	    } else {
		(*p).merge(post);
	    }
	}
};

// Class representing a document and the terms indexing it
class InMemoryDoc {
    public:
	std::vector<InMemoryPosting> terms;// Sorted list of terms indexing document
	void add_posting(const InMemoryPosting & post) {
	    // Add document to right place in list
	    std::vector<InMemoryPosting>::iterator p;
	    p = std::lower_bound(terms.begin(), terms.end(),
			    post,
			    InMemoryPostingLessByTermName());
	    if(p == terms.end() || InMemoryPostingLessByTermName()(post, *p)) {
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
	std::vector<InMemoryPosting>::const_iterator pos;
	std::vector<InMemoryPosting>::const_iterator end;
	om_termname tname;
	om_doccount termfreq;
	bool started;

	/** List of positions of the current term.
	 *  This list is populated when get_position_list() is called.
	 */
	InMemoryPositionList mypositions;

	RefCntPtr<const InMemoryDatabase> this_db;

	InMemoryPostList(RefCntPtr<const InMemoryDatabase> db,
			 const InMemoryTerm & term);
    public:
	om_doccount get_termfreq() const;

	om_docid       get_docid() const;     // Gets current docid
	om_weight      get_weight() const;    // Gets current weight
	om_doclength   get_doclength() const; // Length of current document
        om_termcount   get_wdf() const;	      // Within Document Frequency
	PositionList *get_position_list();

	PostList *next(om_weight w_min); // Moves to next docid

	PostList *skip_to(om_docid did, om_weight w_min); // Moves to next docid >= specified docid

	bool   at_end() const;        // True if we're off the end of the list

	std::string intro_term_description() const;
};


// Term List
class InMemoryTermList : public LeafTermList {
    friend class InMemoryDatabase;
    private:
	std::vector<InMemoryPosting>::const_iterator pos;
	std::vector<InMemoryPosting>::const_iterator end;
	om_termcount terms;
	bool started;

	RefCntPtr<const InMemoryDatabase> this_db;
	om_doclength document_length;

	InMemoryTermList(RefCntPtr<const InMemoryDatabase> db,
			 const InMemoryDoc & doc,
			 om_doclength len);
    public:
	om_termcount get_approx_size() const;

	OmExpandBits get_weighting() const;
	const om_termname get_termname() const;
	om_termcount get_wdf() const; // Number of occurences of term in current doc
	om_doccount get_termfreq() const;  // Number of docs indexed by term
	TermList * next();
	bool   at_end() const;
};


/** A database held entirely in memory.
 *
 *  This is a prototype database, mainly used for debugging and testing.
 */
class InMemoryDatabase : public Database {
    friend class DatabaseBuilder;
    private:
	std::map<om_termname, InMemoryTerm> postlists;
	std::vector<InMemoryDoc> termlists;
	std::vector<std::string> doclists;
	std::vector<std::map<om_keyno, OmKey> > keylists;

	std::vector<om_doclength> doclengths;

	om_totlength totlen;

	bool indexing; // Whether we have started to index to the database

	/// If true, we're not allowed to add documents.
	bool readonly;

	// Stop copy / assignment being allowed
	InMemoryDatabase& operator=(const InMemoryDatabase &);
	InMemoryDatabase(const InMemoryDatabase &);

	/** Create and open an in-memory database.
	 *
	 *  @exception OmOpeningError thrown if database can't be opened.
	 *
	 *  @param params Parameters supplied by the user to specify the
	 *                location of the database to open.
	 */
	InMemoryDatabase(const OmSettings & params, bool readonly);

	void make_term(const om_termname & tname);

	om_docid make_doc(const OmData & docdata);
	void add_keys(om_docid did,
		      const OmDocumentContents::document_keys &keys_);

	void make_posting(const om_termname & tname,
			  om_docid did,
			  om_termpos position,
			  om_termcount wdf);

	//@{
	/** Implementation of virtual methods: see Database for details.
	 */
	void do_begin_session(om_timeout timeout);
	void do_end_session();
	void do_flush();

	void do_begin_transaction();
	void do_commit_transaction();
	void do_cancel_transaction();

	om_docid do_add_document(const OmDocumentContents & document);
	void do_delete_document(om_docid did);
	void do_replace_document(om_docid did,
				 const OmDocumentContents & document);
	OmDocumentContents do_get_document(om_docid did);
	//@}

    public:
	~InMemoryDatabase();

	om_doccount  get_doccount() const;
	om_doclength get_avlength() const;
	om_doclength get_doclength(om_docid did) const;

	om_doccount get_termfreq(const om_termname & tname) const;
	bool term_exists(const om_termname & tname) const;

	LeafPostList * do_open_post_list(const om_termname & tname) const;
	LeafTermList * open_term_list(om_docid did) const;
	LeafDocument * open_document(om_docid did) const;
};




//////////////////////////////////////////////
// Inline function definitions for postlist //
//////////////////////////////////////////////

inline
InMemoryPostList::InMemoryPostList(RefCntPtr<const InMemoryDatabase> db,
				   const InMemoryTerm & term)
	: pos(term.docs.begin()),
	  end(term.docs.end()),
	  tname(pos->tname),
	  termfreq(term.docs.size()),
	  started(false),
	  this_db(db)
{
    // InMemoryPostLists cannot be empty
    Assert(pos != end);
}

inline om_doccount
InMemoryPostList::get_termfreq() const
{
    return termfreq;
}

inline om_docid
InMemoryPostList::get_docid() const
{
    //DebugMsg(tname << ".get_docid()");
    Assert(started);
    Assert(!at_end());
    //DebugMsg(" = " << (*pos).did << endl);
    return (*pos).did;
}

inline PostList *
InMemoryPostList::next(om_weight w_min)
{
    //DebugMsg(tname << ".next()" << endl);
    if(started) {
	Assert(!at_end());
	pos++;
    } else {
	started = true;
    }
    return NULL;
}

inline PostList *
InMemoryPostList::skip_to(om_docid did, om_weight w_min)
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
    if(pos != end) return false;
    return true;
}


inline std::string
InMemoryPostList::intro_term_description() const
{
    return tname + ":" + om_tostring(termfreq);
}


//////////////////////////////////////////////
// Inline function definitions for termlist //
//////////////////////////////////////////////

inline
InMemoryTermList::InMemoryTermList(RefCntPtr<const InMemoryDatabase> db,
				   const InMemoryDoc & doc,
				   om_doclength len)
	: pos(doc.terms.begin()),
	  end(doc.terms.end()),
	  terms(doc.terms.size()),
	  started(false),
	  this_db(db)
{
    document_length = len;
    return;
}

inline om_termcount
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

    return wt->get_bits(InMemoryTermList::get_wdf(),
			document_length,
			InMemoryTermList::get_termfreq(),
			this_db->get_doccount());
}

inline const om_termname
InMemoryTermList::get_termname() const
{
    Assert(started);
    Assert(!at_end());
    return (*pos).tname;
}

inline om_termcount
InMemoryTermList::get_wdf() const
{
    Assert(started);
    Assert(!at_end());
    return (*pos).wdf;
}

inline om_doccount
InMemoryTermList::get_termfreq() const
{
    Assert(started);
    Assert(!at_end());

    return this_db->get_termfreq((*pos).tname);
}

inline TermList *
InMemoryTermList::next()
{
    if(started) {
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




//////////////////////////////////////////////
// Inline function definitions for database //
//////////////////////////////////////////////

inline om_doccount
InMemoryDatabase::get_doccount() const
{
    return termlists.size();
}

inline om_doclength
InMemoryDatabase::get_avlength() const
{
    om_doccount docs = InMemoryDatabase::get_doccount();
    if (docs == 0) {
	return 0;
    } else {
	return ((om_doclength) totlen) / docs;
    }
}

inline om_doccount
InMemoryDatabase::get_termfreq(const om_termname & tname) const
{
    std::map<om_termname, InMemoryTerm>::const_iterator i = postlists.find(tname);
    if(i == postlists.end()) return 0;
    return i->second.docs.size();
}

inline om_doclength
InMemoryDatabase::get_doclength(om_docid did) const
{
    Assert(did > 0 && did <= termlists.size());
    return doclengths[did - 1];
}

#endif /* OM_HGUARD_INMEMORY_DATABASE_H */
