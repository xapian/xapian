/* da_database.h: C++ class definition for DA access routines */

#ifndef _da_database_h_
#define _da_database_h_

#include "omassert.h"

#include "database.h"
#include "daread.h"

class DAPostList : public virtual DBPostList {
    friend class DADatabase;
    private:
	struct postings * postlist;
	docid  currdoc;

	doccount termfreq;

	DAPostList(const IRDatabase *, struct postings *, doccount);
    public:
	~DAPostList();

	doccount get_termfreq() const;

        weight recalc_maxweight();

	docid  get_docid() const;     // Gets current docid
	weight get_weight() const;    // Gets current weight
	weight get_maxweight() const;    // Gets max weight
	PostList *next(weight w_min);          // Moves to next docid
	PostList *skip_to(docid, weight w_min);  // Moves to next docid >= specified docid
	bool   at_end() const;        // True if we're off the end of the list
};

inline doccount
DAPostList::get_termfreq() const
{
    return termfreq;
}

inline docid
DAPostList::get_docid() const
{
    Assert(!at_end());
    Assert(currdoc != 0);
    //printf("%p:DocID %d\n", this, currdoc);
    return currdoc;
}

inline bool
DAPostList::at_end() const
{
    Assert(currdoc != 0);
    if (currdoc == MAXINT) return true;
    return false;
}

inline weight
DAPostList::recalc_maxweight()
{
    return DAPostList::get_maxweight();
}




class DATermListItem {
    public:
	termid id;
	termcount wdf;
	doccount termfreq;
	DATermListItem(termid id_new,
		       termcount wdf_new,
		       doccount termfreq_new) {
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
	bool have_started;

	DATermList(const IRDatabase *root, struct termvec *tv);
    public:
	termid get_termid() const;
	termcount get_wdf() const; // Number of occurences of term in current doc
	doccount get_termfreq() const;  // Number of docs indexed by term
	TermList * next();
	bool   at_end() const;
};

inline termid DATermList::get_termid() const
{
    Assert(!at_end());
    Assert(have_started);
    return pos->id;
}

inline termcount DATermList::get_wdf() const
{
    Assert(!at_end());
    Assert(have_started);
    return pos->wdf;
}

inline doccount DATermList::get_termfreq() const
{
    Assert(!at_end());
    Assert(have_started);
    return pos->termfreq;
}

inline TermList * DATermList::next()
{
    if(have_started) {
	Assert(!at_end());
	pos++;
    } else {
	have_started = true;
    }
    return NULL;
}

inline bool   DATermList::at_end() const
{
    Assert(have_started);
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

	mutable map<termname, termid> termidmap;
	mutable vector<DATerm> termvec;

	// Stop copy / assignment being allowed
	DADatabase& operator=(const DADatabase&);
	DADatabase(const DADatabase&);
    public:
	DADatabase();
	~DADatabase();

	termid term_name_to_id(const termname &) const;
	termname term_id_to_name(termid) const;

	termid add_term(const termname &) {
	    throw OmError("DADatabase.add_term() not implemented");
	}
	docid add_doc(IRDocument &) {
	    throw OmError("DADatabase.add_doc() not implemented");
	}
	void add(termid, docid, termpos) {
	    throw OmError("DADatabase.add() not implemented");
	}

	void open(const string &pathname, bool readonly);
	void close();

	doccount  get_doccount() const;
	doclength get_avlength() const;

	DBPostList * open_post_list(termid id) const;
	TermList * open_term_list(docid id) const;

	IRDocument * open_document(docid id) const;
};

inline doccount
DADatabase::get_doccount() const
{
    Assert(opened);
    return DA_r->itemcount;
}

inline doclength
DADatabase::get_avlength() const
{
    Assert(opened);
    return 1;
}

#endif /* _da_database_h_ */
