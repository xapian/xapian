/* sleepy_database.h: C++ class definition for sleepycat access routines */

#ifndef _sleepy_database_h_
#define _sleepy_database_h_

#include "omassert.h"
#include "database.h"
#include <stdlib.h>

// Postlist - a list of documents indexed by a given term
class SleepyPostList : public virtual PostList {
    friend class SleepyDatabase;
    private:
	doccount pos;
	docid *data;
	doccount termfreq;

	SleepyPostList(docid *, doccount);
    public:
	~SleepyPostList();

	doccount get_termfreq() const; // Number of docs indexed by this term

	docid    get_docid() const;    // Current docid
	weight   get_weight() const;   // Current weight
	PostList * next();             // Move next docid
	bool     at_end() const;       // True if we're off the end of the list
};

inline doccount
SleepyPostList::get_termfreq() const
{
    return termfreq;
}

inline docid
SleepyPostList::get_docid() const
{
    Assert(!at_end());
    Assert(pos != 0);
    return data[pos - 1];
}

inline PostList *
SleepyPostList::next()
{
    Assert(!at_end());
    pos ++;
    return NULL;
}

inline bool
SleepyPostList::at_end() const
{
    if(pos > termfreq) return true;
    return false;
}





// Termlist - a list of terms indexing a given document
class SleepyTermList : public virtual TermList {
    friend class SleepyDatabase;
    private:
	termcount pos;
	termid *data;
	termcount terms;

	SleepyTermList(termid *, termcount);
	~SleepyTermList();
    public:
	termid get_termid() const;  // Current termid
	termcount get_wdf() const;  // Occurences of current term in doc
	doccount get_termfreq() const;  // Docs indexed by current term
	TermList * next();
	bool   at_end() const;
};

inline termid
SleepyTermList::get_termid() const
{
    Assert(!at_end());
    Assert(pos != 0);
    return data[pos];
}

inline termcount
SleepyTermList::get_wdf() const
{
    Assert(!at_end());
    Assert(pos != 0);
    return 1;
}

inline doccount
SleepyTermList::get_termfreq() const
{
    Assert(!at_end());
    Assert(pos != 0);
    return 1;
}   

inline TermList *
SleepyTermList::next()
{
    Assert(!at_end());
    pos ++;
    return NULL;
}

inline bool
SleepyTermList::at_end() const
{
    if(pos > terms) return true;
    return false;
}





class SleepyDatabaseInternals;

class SleepyDatabase : public virtual IRDatabase {
    private:
	SleepyDatabaseInternals * internals;
    public:
	SleepyDatabase();
	~SleepyDatabase();

	termid term_name_to_id(const termname &) const;
	termname term_id_to_name(termid) const;

	termid add_term(const termname &);
	docid add_doc(const docname &);
	void add(termid, docid);

	void open(const string &pathname, bool readonly);
	void close();

	PostList * open_post_list(termid id) const;
	TermList * open_term_list(docid id) const;
};

#endif /* _sleepy_database_h_ */
