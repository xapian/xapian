/* multi_database.h: C++ class definition for multiple database access */

#ifndef _multi_database_h_
#define _multi_database_h_

#include "omassert.h"
#include "database.h"
#include <stdlib.h>
#include <list>

class MultiPostListInternal {
    public:
	PostList * pl;
	docid currdoc;

	MultiPostListInternal(PostList * pl_new) {
	    pl = pl_new;
	    currdoc = 0;
	}
};

class MultiPostList : public virtual PostList {
    friend class MultiDatabase;
    private:
	list<MultiPostListInternal> postlists;

	bool   finished;
	docid  currdoc;

	mutable bool freq_initialised;
	mutable doccount termfreq;

	mutable bool weight_initialised;
	mutable weight termweight;

	MultiPostList(list<MultiPostListInternal> &);
    public:
	~MultiPostList();

	doccount get_termfreq() const;

	docid  get_docid() const;     // Gets current docid
	weight get_weight() const;    // Gets current weight
	weight get_maxweight() const;    // Gets max weight
	PostList *next(weight);          // Moves to next docid
	//PostList *skip_to(docid, weight);// Moves to next docid >= specified docid
	bool   at_end() const;        // True if we're off the end of the list
};

inline doccount
MultiPostList::get_termfreq() const
{
    if(freq_initialised) return termfreq;

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





class MultiTerm {
    friend class MultiDatabase;
    private:
	MultiTerm(termname name_new) {
	    name = name_new;
	}
    public:
	termname name;
};


class MultiDatabase : public virtual IRDatabase {
    private:
	mutable map<termname, termid> termidmap;
	mutable vector<MultiTerm> termvec;

	list<IRDatabase *> databases;

	bool opened; // Whether we have opened the database (ie, added a subDB)
	mutable bool used;// Have we used the database (if so, can't add more DBs)
    public:
	MultiDatabase();
	~MultiDatabase();

	termid term_name_to_id(const termname &) const;
	termname term_id_to_name(termid) const;

	termid add_term(const termname &);
	docid add_doc(IRDocument &);
	void add(termid, docid, termpos);

	void open(const string &pathname, bool readonly) {
	    throw OmError("open() not valid for MultiDatabase\n");
	}

	// MultiDatabase will take care of closing and deleting the
	// database.  FIXME find appropriate structure to make this
	// implicit in the interface.
	void open_subdatabase(IRDatabase *,
			      const string &pathname, bool readonly);
	void close();

	PostList * open_post_list(termid id) const;
	TermList * open_term_list(docid id) const;
};

#endif /* _sleepy_database_h_ */
