/* sleepy_database.h: C++ class definition for sleepycat access routines */

#ifndef _sleepy_database_h_
#define _sleepy_database_h_

#include "omassert.h"
#include "database.h"

class SleepyPostList : public virtual PostList {
    friend class SleepyDatabase;
    private:
	docid  currdoc;

	SleepyPostList();
    public:
	~SleepyPostList();

	doccount get_termfreq() const; // Number of docs indexed by this term

	docid    get_docid() const;    // Current docid
	weight   get_weight() const;   // Current weight
	void     next();               // Move next docid
	bool     at_end() const;       // True if we're off the end of the list
};

inline docid
SleepyPostList::get_docid() const
{
    Assert(!at_end());
    return currdoc;
}

inline bool
SleepyPostList::at_end() const
{
    if(currdoc == 0) return true;
    return false;
}




class SleepyTermList : public virtual TermList {
    friend class SleepyDatabase;
    private:
	vector<termid>::iterator pos;
	vector<termid> ids;

	SleepyTermList(struct proto_db *db);
    public:
	termid get_termid();
	termcount get_wdf();  // Number of occurences of term in current doc
	termcount get_termfreq();  // Number of docs indexed by term
	void   next();
	bool   at_end();
};


class SleepyDatabaseInternals;

class SleepyDatabase : public virtual IRDatabase {
    private:
	SleepyDatabaseInternals * internals;
    public:
	SleepyDatabase();
	~SleepyDatabase();

	void open(const string &pathname, bool readonly);
	void close();

	PostList * open_post_list(termid id);
	TermList * open_term_list(docid id);

	termid term_name_to_id(const termname &);
	termname term_id_to_name(termid);
};

#endif /* _sleepy_database_h_ */
