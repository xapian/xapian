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
        mutable weight termweight;
	doccount dbsize;
    
        mutable bool weight_initialised;

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




class DATermListItem {
    public:
	termid id;
	termcount wdf;
	doccount termfreq;
	DATermListItem(termid id_new, termcount wdf_new, doccount termfreq_new) {
	    id = id_new;
	    wdf = wdf_new;
	    termfreq = termfreq_new;
	}
	bool operator< (const DATermListItem& a) const {
	    return this->id < a.id;
	}
};
 
class DATermList : public virtual TermList {
    friend class DADatabase;
    private:
	vector<DATermListItem>::iterator pos;
	vector<DATermListItem> terms;

	DATermList(DADatabase *db, struct termvec *tv);
    public:
	termid get_termid();
	termcount get_wdf();       // Number of occurences of term in current doc
	termcount get_termfreq();  // Number of docs indexed by term
	void   next();
	bool   at_end();
};

inline termid DATermList::get_termid()
{
    Assert(!at_end());
    return pos->id;
}

inline termcount DATermList::get_wdf()
{
    Assert(!at_end());
    return pos->wdf;
}

inline doccount DATermList::get_termfreq()
{
    Assert(!at_end());
    return pos->termfreq;
}

inline void   DATermList::next()
{
    Assert(!at_end());
    pos++;
}

inline bool   DATermList::at_end()
{
    if(pos == terms.end()) return true;
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


class DARecord;

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
	DARecord * get_document(docid id);

	termid term_name_to_id(const termname &);
	termname term_id_to_name(termid);
};

#endif /* _da_database_h_ */
