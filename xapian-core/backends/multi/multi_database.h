/* multi_database.h: C++ class definition for multiple database access */

#ifndef _multi_database_h_
#define _multi_database_h_

#include "omassert.h"
#include "postlist.h"
#include "termlist.h"
#include "database.h"
#include <stdlib.h>
#include <set>
#include <vector>
#include <list>

class MultiPostListInternal {
    public:
	DBPostList * pl;
	docid currdoc;

	doccount offset;
	doccount multiplier;

	MultiPostListInternal(DBPostList * pl_new,
			      doccount off,
			      doccount mult)
		: pl(pl_new), currdoc(0), offset(off), multiplier(mult) {}
};

class MultiPostList : public virtual DBPostList {
    friend class MultiDatabase;
    private:
	list<MultiPostListInternal> postlists;

	bool   finished;
	docid  currdoc;

	mutable bool freq_initialised;
	mutable doccount termfreq;

	weight termweight;

	MultiPostList(list<MultiPostListInternal> &);
    public:
	~MultiPostList();

	void set_termweight(const IRWeight *); // Sets term weight

	doccount get_termfreq() const;

	docid  get_docid() const;     // Gets current docid
	weight get_weight() const;    // Gets current weight
	PostList *next(weight);          // Moves to next docid
	PostList *skip_to(docid, weight);// Moves to next docid >= specified docid
	bool   at_end() const;        // True if we're off the end of the list
};

inline void
MultiPostList::set_termweight(const IRWeight * wt)
{
    // Set in base class, so that get_maxweight() works
    DBPostList::set_termweight(wt);
    list<MultiPostListInternal>::const_iterator i = postlists.begin();
    while(i != postlists.end()) {
	(*i).pl->set_termweight(wt);
	i++;
    }
}

inline doccount
MultiPostList::get_termfreq() const
{
    if(freq_initialised) return termfreq;
#ifdef DEBUG
    cout << "Calculating multiple term frequencies" << endl;
#endif /* DEBUG */

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
#ifdef DEBUG
    //cout << this << ":DocID " << currdoc << endl;
#endif /* DEBUG */
    return currdoc;
}

inline bool
MultiPostList::at_end() const
{
    return finished;
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
	termcount get_approx_size() const;

	weight get_weight() const;
	termname get_termname() const;
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
#ifdef DEBUG
    cout << "Approximation factor for termfrequency: " <<
	    termfreq_factor << endl;
#endif /* DEBUG */
}

inline MultiTermList::~MultiTermList()
{
    delete tl;
}

inline termcount
MultiTermList::get_approx_size() const
{
    return tl->get_approx_size();
}

inline weight
MultiTermList::get_weight() const {
    return tl->get_weight();
}

inline termname MultiTermList::get_termname() const
{
    return tl->get_termname();
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





class MultiDatabase : public virtual IRGroupDatabase {
    private:
	mutable set<termname> terms;

	vector<IRDatabase *> databases;

	mutable bool length_initialised;
	mutable doclength avlength;

	bool opened; // Whether we have opened the database (ie, added a subDB)
	mutable bool used;// Have we used the database (if so, can't add more DBs)
    public:
	MultiDatabase();
	~MultiDatabase();

	void set_root(IRDatabase *);

	termid term_name_to_id(const termname &) const;
	termname term_id_to_name(termid) const;

	void open(om_database_type,
		  const string &pathname,
		  bool readonly);
	void close();

	doccount  get_doccount() const;
	doclength get_avlength() const;

	doccount get_termfreq(const termname &) const;
	bool term_exists(const termname &) const;

	DBPostList * open_post_list(const termname&, RSet *) const;
	TermList * open_term_list(docid id) const;
	IRDocument * open_document(docid id) const;

        // Introspection methods
	IRDatabase * get_database_of_doc(docid id) const;
	const string get_database_path() const { return ""; }
};

inline doccount
MultiDatabase::get_doccount() const
{
    // FIXME - lazy evaluation?
    Assert(opened);
    Assert((used = true) == true);

    doccount docs = 0;

    vector<IRDatabase *>::const_iterator i = databases.begin();
    while(i != databases.end()) {
	docs += (*i)->get_doccount();
	i++;
    }

    return docs;
}

inline doclength
MultiDatabase::get_avlength() const
{
    // FIXME - lazy evaluation?
    Assert(opened);
    Assert((used = true) == true);

    if(!length_initialised) {
	doccount docs = 0;
	doclength totlen = 0;

	vector<IRDatabase *>::const_iterator i = databases.begin(); 
	while(i != databases.end()) {
	    doccount db_doccount = (*i)->get_doccount();
	    docs += db_doccount;
	    totlen += (*i)->get_avlength() * db_doccount;
	    i++;
	}

	avlength = totlen / docs;
	length_initialised = true;
    }

    return avlength;
}

inline doccount
MultiDatabase::get_termfreq(const termname &tname) const
{
    if(!term_exists(tname)) return 0;
    PostList *pl = open_post_list(tname, NULL);
    doccount freq = 0;
    if(pl) freq = pl->get_termfreq();
    delete pl;
    return freq;
}

inline termname
MultiDatabase::term_id_to_name(termid tid) const {
    Assert(false);
}

#endif /* _multi_database_h_ */
