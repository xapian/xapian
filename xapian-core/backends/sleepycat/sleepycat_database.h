/* sleepy_database.h: C++ class definition for sleepycat access routines */

#ifndef _sleepy_database_h_
#define _sleepy_database_h_

#include "omassert.h"
#include "database.h"
#include "postlist.h"
#include "termlist.h"
#include <stdlib.h>

// Postlist - a list of documents indexed by a given term
class SleepyPostList : public virtual DBPostList {
    friend class SleepyDatabase;
    private:
	doccount pos;
	docid *data;

	doccount termfreq;

	SleepyPostList(const IRDatabase *,
		       docid *,
		       doccount,
		       termid tid,
		       const RSet *rset);
    public:
	~SleepyPostList();

	doccount   get_termfreq() const;// Number of docs indexed by this term

	docid      get_docid() const;   // Current docid
	weight     get_weight() const;  // Current weight
        PostList * next(weight w_min);  // Move next docid
	bool       at_end() const;      // True if we're off the end of the list
};



// Termlist - a list of terms indexing a given document
class SleepyTermList : public virtual TermList {
    friend class SleepyDatabase;
    private:
	termcount pos;
	termid *data;
	termcount terms;

	const SleepyDatabase *db;

	SleepyTermList(const IRDatabase *,
		       const SleepyDatabase *,
		       termid *, termcount);
    public:
	~SleepyTermList();
	termcount get_approx_size() const;

	weight get_weight() const;  // Gets weight of current term
	termname get_termname() const;  // Current term
	termcount get_wdf() const;  // Occurences of current term in doc
	doccount get_termfreq() const;  // Docs indexed by current term
	TermList * next();
	bool   at_end() const;
};



class SleepyDatabaseInternals;

class SleepyDatabase : public virtual IRDatabase {
    private:
	SleepyDatabaseInternals * internals;
	string path;
	bool opened;
    public:
	SleepyDatabase();
	~SleepyDatabase();

	termid term_name_to_id(const termname &) const;
	termname term_id_to_name(termid) const;

	termid add_term(const termname &);
	docid add_doc(IRDocument &);
	void add(termid, docid, termpos);

	void open(const string &pathname, bool readonly);
	void close();

	doccount  get_doccount() const;
	doclength get_avlength() const;

	DBPostList * open_post_list(termid id, RSet *) const;
	TermList * open_term_list(docid id) const;
	IRDocument * open_document(docid id) const;

	const string get_database_path() const;
};



///////////////////////////////////////////
// Inline definitions for SleepyPostList //
///////////////////////////////////////////

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
SleepyPostList::next(weight w_min)
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

///////////////////////////////////////////
// Inline definitions for SleepyTermList //
///////////////////////////////////////////

inline termcount
SleepyTermList::get_approx_size() const
{
    return terms;
}

inline weight
SleepyTermList::get_weight() const {
    Assert(!at_end());
    Assert(pos != 0);
    return 1.0; // FIXME
}

inline termname
SleepyTermList::get_termname() const
{
    Assert(!at_end());
    Assert(pos != 0);
    return db->term_id_to_name(data[pos]);
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

///////////////////////////////////////////
// Inline definitions for SleepyDatabase //
///////////////////////////////////////////

inline doccount
SleepyDatabase::get_doccount() const
{
    Assert(opened);
    return 1;
}

inline doclength
SleepyDatabase::get_avlength() const
{
    Assert(opened);
    return 1;
}

inline const string
SleepyDatabase::get_database_path() const {
    Assert(opened);
    return path;
}

#endif /* _sleepy_database_h_ */
