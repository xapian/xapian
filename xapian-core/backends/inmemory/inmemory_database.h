/* textfile_database.h: C++ class definition for multiple database access */

#ifndef _textfile_database_h_
#define _textfile_database_h_

#include "omassert.h"
#include "database.h"
#include "postlist.h"
#include "termlist.h"
#include "indexer.h"
#include <stdlib.h>
#include <map>
#include <vector>
#include <list>
#include <algorithm>


// Class representing a posting (a term/doc pair, and
// all the relevant positional information, is a single posting)
class TextfilePosting {
    public:
	docid did;
	termid tid;
	vector<termcount> positions; // Sorted list of positions

	// Merge two postings (same term/doc pair, new positional info)
	void merge(const TextfilePosting & post) {
	    Assert(did == post.did);
	    Assert(tid == post.tid);

	    positions.insert(positions.end(),
			     post.positions.begin(),
			     post.positions.end());
	    // FIXME - inefficient
	    sort(positions.begin(), positions.end());
	}
};

// Compare by document ID
class TextfilePostingLessByDocId {
    public:
	int operator() (const TextfilePosting &p1, const TextfilePosting &p2)
	{
	    return p1.did < p2.did;
	}
};

// Compare by term ID
class TextfilePostingLessByTermId {
    public:
	int operator() (const TextfilePosting &p1, const TextfilePosting &p2)
	{
	    return p1.tid < p2.tid;
	}
};

// Class representing a term and the documents indexing it
class TextfileTerm {
    public:
	vector<TextfilePosting> docs;// Sorted list of documents indexing term
	void add_posting(const TextfilePosting & post) {
	    // Add document to right place in list
	    vector<TextfilePosting>::iterator p;
	    p = lower_bound(docs.begin(), docs.end(),
			    post,
			    TextfilePostingLessByDocId());
	    if(p == docs.end() || TextfilePostingLessByDocId()(post, *p)) {
		docs.insert(p, post);
	    } else {
		(*p).merge(post);
	    }
	}
};

// Class representing a document and the terms indexing it
class TextfileDoc {
    public:
	vector<TextfilePosting> terms;// Sorted list of terms indexing document
	void add_posting(const TextfilePosting & post) {
	    // Add document to right place in list
	    vector<TextfilePosting>::iterator p;
	    p = lower_bound(terms.begin(), terms.end(),
			    post,
			    TextfilePostingLessByTermId());
	    if(p == terms.end() || TextfilePostingLessByTermId()(post, *p)) {
		terms.insert(p, post);
	    } else {
		(*p).merge(post);
	    }
	}
};




// Post List
class TextfilePostList : public virtual DBPostList {
    friend class TextfileDatabase;
    private:
	vector<TextfilePosting>::const_iterator pos;
	vector<TextfilePosting>::const_iterator end;
	doccount termfreq;
	bool started;

	const TextfileDatabase * this_db;

	TextfilePostList(const IRDatabase *,
			 const TextfileDatabase *,
			 const TextfileTerm &,
			 termid tid,
			 const RSet *rset);
    public:
	doccount get_termfreq() const;

	docid  get_docid() const;     // Gets current docid
	weight get_weight() const;    // Gets current weight
	PostList *next(weight);          // Moves to next docid

	//PostList *skip_to(docid, weight);// Moves to next docid >= specified docid
	// FIXME - implement a skip_to.  Note, though, that a binary search of
	// the remaining list may NOT be a good idea (search time is then
	// O(log {length of list}), as opposed to O(distance we want to skip)
	// Since we will frequently only be skipping a short distance, this
	// could be worse.

	bool   at_end() const;        // True if we're off the end of the list
};


// Term List
class TextfileTermList : public virtual TermList {
    friend class TextfileDatabase;
    private:
	vector<TextfilePosting>::const_iterator pos;
	vector<TextfilePosting>::const_iterator end;
	termcount terms;
	bool started;

	const TextfileDatabase * this_db;

	TextfileTermList(const TextfileDatabase *, const TextfileDoc &);
    public:
	termcount get_approx_size() const;

	weight get_weight() const;
	termid get_termid() const;
	termcount get_wdf() const; // Number of occurences of term in current doc
	doccount get_termfreq() const;  // Number of docs indexed by term
	TermList * next();
	bool   at_end() const;
};


// Database
class TextfileDatabase : public virtual IRDatabase,
			 public virtual IndexerDestination {
    private:
	string path;

	map<termname, termid> termidmap;
	vector<termname> termvec;

	vector<TextfileTerm> postlists;
	vector<TextfileDoc> termlists;
	vector<string> doclists;

	vector<doclength> doclengths;

	totlength totlen;

	bool opened; // Whether we have opened the database
	bool indexing; // Whether we have started to index to the database
    public:
	TextfileDatabase();
	~TextfileDatabase();

	void set_root(IRDatabase *);

	termid term_name_to_id(const termname &) const;
	termname term_id_to_name(termid) const;

	termid add_term(const termname &);
	docid add_doc(IRDocument &);
	void add(termid, docid, termpos);

	void open(const string &pathname, bool readonly);
	void close();

	doccount  get_doccount() const;
	doclength get_avlength() const;
	doccount get_termfreq(termid) const;  // Number of docs indexed by term

	DBPostList * open_post_list(termid, RSet *) const;
	TermList * open_term_list(docid) const;
	IRDocument * open_document(docid) const;

	doclength get_doclength(docid) const;

	termid make_term(const termname &);
	docid make_doc(const docname &);
	void make_posting(termid, docid, termcount);

	const string get_database_path() const;
};




//////////////////////////////////////////////
// Inline function definitions for postlist //
//////////////////////////////////////////////

inline
TextfilePostList::TextfilePostList(const IRDatabase *root,
				   const TextfileDatabase *db,
				   const TextfileTerm &term,
				   termid tid,
				   const RSet *rset)
	: pos(term.docs.begin()),
	  end(term.docs.end()),
	  termfreq(term.docs.size()),
	  started(false),
	  this_db(db)
{
    own_wt.set_stats(root, get_termfreq(), tid, rset);
}

inline doccount
TextfilePostList::get_termfreq() const
{
    return termfreq;
}

inline docid
TextfilePostList::get_docid() const
{   
    Assert(started);
    Assert(!at_end());
    return (*pos).did;
}

inline PostList *
TextfilePostList::next(weight w_min)
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
TextfilePostList::at_end() const
{
    Assert(started);
    if(pos != end) return false;
    return true;
}




//////////////////////////////////////////////
// Inline function definitions for termlist //
//////////////////////////////////////////////

inline TextfileTermList::TextfileTermList(const TextfileDatabase *db,
					  const TextfileDoc &doc)
	: pos(doc.terms.begin()),
	  end(doc.terms.end()),
	  terms(doc.terms.size()),
	  started(false),
	  this_db(db)
{}

inline termcount TextfileTermList::get_approx_size() const
{
    return terms;
}

inline weight TextfileTermList::get_weight() const
{
    Assert(started);
    Assert(!at_end());
    return 1.0;  // FIXME
}

inline termid TextfileTermList::get_termid() const
{
    Assert(started);
    Assert(!at_end());
    return (*pos).tid;
}

inline termcount TextfileTermList::get_wdf() const
{
    Assert(started);
    Assert(!at_end());
    return (*pos).positions.size();
}

inline doccount TextfileTermList::get_termfreq() const
{
    Assert(started);
    Assert(!at_end());

    return this_db->get_termfreq(get_termid());
}

inline TermList * TextfileTermList::next()
{
    if(started) {
	Assert(!at_end());
	pos++;
    } else {
	started = true;
    }
    return NULL;
}

inline bool TextfileTermList::at_end() const
{
    Assert(started);
    if(pos != end) return false;
    return true;
}





inline doccount
TextfileDatabase::get_doccount() const
{
    Assert(opened);
    return postlists.size();
}

inline doclength
TextfileDatabase::get_avlength() const
{
    Assert(opened);
    doccount docs = TextfileDatabase::get_doccount();
    Assert(docs != 0);
    return ((doclength) totlen) / docs;
}

inline doccount
TextfileDatabase::get_termfreq(termid tid) const
{
    Assert(opened);
    Assert(tid > 0 && tid <= postlists.size());

    return postlists[tid - 1].docs.size();
}

inline doclength
TextfileDatabase::get_doclength(docid did) const
{
    Assert(opened);
    Assert(did > 0 && did <= termlists.size());

    return (doclength) doclengths[did - 1];
}

inline termname
TextfileDatabase::term_id_to_name(termid tid) const
{
    Assert(opened);
    Assert(tid > 0 && tid <= termvec.size());
    //printf("Looking up termid %d: name = `%s'\n", tid, termvec[tid - 1].name.c_str());
    return termvec[tid - 1];
}

inline const string
TextfileDatabase::get_database_path() const {
    Assert(opened);
    return path;
}

#endif /* _textfile_database_h_ */
