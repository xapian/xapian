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
	struct termvec * tvec;
	DADatabase * dbase;

	DATermList(DADatabase *db, struct termvec *tv);
    public:
	~DATermList();

	termid get_termid();
	void   next();
	bool   at_end();
};

class DATerm {
    friend class DADatabase;
    private:
	DATerm(struct terminfo *, termname);
    public:
	struct terminfo ti;
	termname name;
};

inline DATerm::DATerm(struct terminfo *ti_new, termname name_new)
{
    ti = *ti_new;
    name = name_new;
}



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

	void open(string pathname, bool readonly);
	void close();

	PostList * open_post_list(termid id);
	TermList * open_term_list(docid id);

	termid term_name_to_id(termname);
	termname term_id_to_name(termid);
};

#endif /* _da_database_h_ */
