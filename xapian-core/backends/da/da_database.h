/* da_database.h: C++ class definition for DA access routines */

#ifndef _da_database_h_
#define _da_database_h_

#include "omassert.h"

#include "database.h"
#include "daread.h"

class DAPostList : public virtual PostList {
    friend class DADatabase;
    private:
	struct postings * postlist;
	docid  currdoc;
	doccount termfreq;
	weight termweight;

	DAPostList(struct postings *pl, doccount termf, doccount dbsize);
    public:
	~DAPostList();

	doccount get_termfreq() const;

	docid  get_docid() const;     // Gets current docid
	weight get_weight() const;    // Gets current weight
	void   next();          // Moves to next docid
	void   skip_to(docid);  // Moves to next docid >= specified docid
	bool   at_end() const;        // True if we're off the end of the list
};

inline doccount
DAPostList::get_termfreq() const
{
    Assert(!at_end());
    return termfreq;
}

inline docid
DAPostList::get_docid() const
{
    Assert(!at_end());
    //printf("%p:DocID %d\n", this, postlist->Doc);
    return postlist->Doc;
}

inline bool
DAPostList::at_end() const
{
    if(postlist->Doc == MAXINT) return true;
    return false;
}




 
class DATermList : public virtual TermList {
    friend class DADatabase;
    private:
	vector<termid>::iterator pos;
	vector<termid> ids;

	DATermList(DADatabase *db, struct termvec *tv);
    public:
	termid get_termid();
	void   next();
	bool   at_end();
};

inline termid DATermList::get_termid()
{
    Assert(!at_end());
    return *pos;
}

inline void   DATermList::next()
{
    Assert(!at_end());
    pos++;
}

inline bool   DATermList::at_end()
{
    if(pos == ids.end()) return true;
    return false;
}




class DATerm {
    friend class DADatabase;
    private:
	DATerm(struct terminfo *ti_new, termname name_new) {
	    ti = *ti_new;
	    name = name_new;
	}
    public:
	struct terminfo ti;
	termname name;
};



class DADatabase : public virtual IRDatabase {
    private:
	bool   opened;
	struct DAfile * DA_r;
	struct DAfile * DA_t;
	doccount dbsize;

	termid max_termid;
	map<termname, termid> termidmap;
	vector<DATerm> termvec;
    public:
	DADatabase();
	~DADatabase();

	void open(const string &pathname, bool readonly);
	void close();

	PostList * open_post_list(termid id);
	TermList * open_term_list(docid id);

	termid term_name_to_id(const termname &);
	termname term_id_to_name(termid);
};

#endif /* _da_database_h_ */
