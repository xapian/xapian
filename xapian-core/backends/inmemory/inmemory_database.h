/* textfile_database.h: C++ class definition for multiple database access */

#ifndef _textfile_database_h_
#define _textfile_database_h_

#include "omassert.h"
#include "database.h"
#include "postlist.h"
#include "termlist.h"
#include <stdlib.h>
#include <map>
#include <vector>
#include <list>
#include <algorithm>

/*
class TextfilePostList : public virtual DBPostList {
    friend class TextfileDatabase;
    private:
	bool   finished;
	docid  currdoc;

	mutable bool freq_initialised;
	mutable doccount termfreq;

	weight termweight;

	TextfilePostList(const IRDatabase *, list<MultiPostListInternal> &);
    public:
	~TextfilePostList();

	doccount get_termfreq() const;

	weight recalc_maxweight();

	docid  get_docid() const;     // Gets current docid
	weight get_weight() const;    // Gets current weight
	weight get_maxweight() const;    // Gets max weight
	PostList *next(weight);          // Moves to next docid
	//PostList *skip_to(docid, weight);// Moves to next docid >= specified docid
	bool   at_end() const;        // True if we're off the end of the list
};

inline doccount
TextfilePostList::get_termfreq() const
{
    if(freq_initialised) return termfreq;
    printf("Calculating multiple term frequencies\n");

    // Calculate and remember the termfreq
    list<MultiPostListInternal>::const_iterator i = postlists.begin();
    termfreq = 0;
    while(i != postlists.end()) {
	termfreq += (*i).pl->get_termfreq();
	i++;
    }

    freq_initialised = true;
    return termfreq;
}

inline docid
MultiPostList::get_docid() const
{   
    Assert(!at_end());
    Assert(currdoc != 0);
    //printf("%p:DocID %d\n", this, currdoc);
    return currdoc;
}

inline bool
MultiPostList::at_end() const
{
    return finished;
}

inline weight
MultiPostList::recalc_maxweight()
{
    return MultiPostList::get_maxweight();
}





class MultiTermList : public virtual TermList {
    friend class MultiDatabase;
    private:
	TermList *tl;
	const IRDatabase *termdb;
	const IRDatabase *rootdb;
	double termfreq_factor;

	MultiTermList(TermList *tl,
		      const IRDatabase *termdb,
		      const IRDatabase *rootdb);
    public:
	termid get_termid() const;
	termcount get_wdf() const; // Number of occurences of term in current doc
	doccount get_termfreq() const;  // Number of docs indexed by term
	TermList * next();
	bool   at_end() const;

	~MultiTermList();
};

inline MultiTermList::MultiTermList(TermList *tl_new,
				    const IRDatabase *termdb_new,
				    const IRDatabase *rootdb_new)
	: tl(tl_new), termdb(termdb_new), rootdb(rootdb_new)
{
    termfreq_factor = ((double)(rootdb->get_doccount())) /
		      (termdb->get_doccount());
printf("Approximation factor for termfrequency: %f\n", termfreq_factor);
}

inline MultiTermList::~MultiTermList()
{
    delete tl;
}

inline termid MultiTermList::get_termid() const
{
    termid tid = tl->get_termid();
    // FIXME - inefficient (!!!)
    return rootdb->term_name_to_id(termdb->term_id_to_name(tid));
}

inline termcount MultiTermList::get_wdf() const
{
    return tl->get_wdf();
}

inline doccount MultiTermList::get_termfreq() const
{
    // Approximate term frequency
    return (doccount) (tl->get_termfreq() * termfreq_factor);
}

inline TermList * MultiTermList::next()
{
    return tl->next();
}

inline bool MultiTermList::at_end() const
{
    return tl->at_end();
}
*/


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
		printf("Not found - adding\n");
		docs.insert(p, post);
	    } else {
		printf("Found - merging\n");
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
		printf("Not found - adding\n");
		terms.insert(p, post);
	    } else {
		printf("Found - merging\n");
		(*p).merge(post);
	    }
	}
};

class TextfileDatabase : public virtual IRDatabase {
    private:
	mutable map<termname, termid> termidmap;
	mutable vector<termname> termvec;

	vector<TextfileTerm> termlists;
	vector<TextfileDoc> postlists;

	doccount docs;
	doclength avlength;

	bool opened; // Whether we have opened the database
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

	DBPostList * open_post_list(termid id) const;
	TermList * open_term_list(docid id) const;
	IRDocument * open_document(docid id) const;
};

inline doccount
TextfileDatabase::get_doccount() const
{
    Assert(opened);
    return docs;
}

inline doclength
TextfileDatabase::get_avlength() const
{
    Assert(opened);
    return avlength;
}

termname
TextfileDatabase::term_id_to_name(termid id) const
{
    Assert(opened);
    Assert(id > 0 && id <= termvec.size());
    //printf("Looking up termid %d: name = `%s'\n", id, termvec[id - 1].name.c_str());
    return termvec[id - 1];
}

#endif /* _textfile_database_h_ */
