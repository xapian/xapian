/* da_database.h: C++ class definition for DA access routines */

#ifndef _da_database_h_
#define _da_database_h_

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

	doccount get_termfreq(); 

	docid  get_docid();     // Gets current docid
	weight get_weight();    // Gets current weight
	void   next();          // Moves to next docid
	void   skip_to(docid);  // Moves to next docid >= specified docid
	bool   at_end();        // True if we're off the end of the list
};

class DATermList : public virtual TermList {

};

class DADatabase : public virtual IRDatabase {
    private:
	bool   opened;
	struct DAfile * DA_r;
	struct DAfile * DA_t;
	doccount dbsize;

	termid max_termid;
	map<termname, termid> termidmap;
	vector<termname> termidvec;
    public:
	DADatabase();

	void open(string pathname, bool readonly);
	void close();

	PostList * open_post_list(termid id);
	TermList * open_term_list(docid id);

	termid term_name_to_id(termname);
	termname term_id_to_name(termid);
};

#endif /* _da_database_h_ */
