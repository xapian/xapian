/* da_database.h: C++ class definition for DA access routines */

#ifndef _da_database_h_
#define _da_database_h_

#include <map>
#include <vector>

#include "omassert.h"

#include "database.h"

#include "errno.h"

// Anonymous declarations (definitions in daread.h)
struct postings;
struct DAfile;
struct terminfo;
struct termvec;

// But it turns out we need to include this anyway
// FIXME - try and sort this out sometime.
#include "daread.h"

class DAPostList : public virtual DBPostList {
    friend class DADatabase;
    private:
	struct postings * postlist;
	docid  currdoc;

	doccount termfreq;

	DAPostList(const IRDatabase *,
		   struct postings *,
		   doccount,
		   termid,
		   const RSet * = NULL);
    public:
	~DAPostList();

	doccount get_termfreq() const;

	docid  get_docid() const;     // Gets current docid
	weight get_weight() const;    // Gets current weight
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

	// Gets passed current database, for termid lookups, _NOT_ root
	DATermList(const DADatabase *database, struct termvec *tv);
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

inline bool DATermList::at_end() const
{
    Assert(have_started);
    if(pos == terms.end()) return true;
    return false;
}




class DATerm {
    friend class DADatabase;
    private:
	DATerm(struct terminfo *ti_new, termname name_new, struct DAfile * DA_t_new = NULL) {
	    if (ti_new) {
		ti = *ti_new;
	    } else {
		ti.freq = 0;
	    }
	    name = name_new;
	    DA_t = DA_t_new;
	}
        struct terminfo * get_ti() {
	    if (ti.freq == 0) {
		int len = name.length();
		if(len > 255) abort();
		byte * k = (byte *) malloc(len + 1);
		if(k == NULL) throw OmError(strerror(ENOMEM));
		k[0] = len + 1;
		name.copy((char*)(k + 1), len);

		int found = DAterm(k, &ti, DA_t);
		free(k);

		if(found == 0) abort();
	    }
	    return &ti;
	}
        struct terminfo ti;
        struct DAfile * DA_t;
    public:
	termname name;
};


class DADatabase : public virtual IRDatabase {
    friend class DATermList;
    private:
	bool   opened;
	struct DAfile * DA_r;
	struct DAfile * DA_t;
	string path;

	mutable map<termname, termid> termidmap;
	mutable vector<DATerm> termvec;

	// Stop copy / assignment being allowed
	DADatabase& operator=(const DADatabase&);
	DADatabase(const DADatabase&);

        termid term_name_to_id_lazy(const termname &name) const;
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

	const string get_database_path() const;
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

inline termname
DADatabase::term_id_to_name(termid id) const
{
    Assert(opened);
    Assert(id > 0 && id <= termvec.size());
    //printf("Looking up termid %d: name = `%s'\n", id, termvec[id - 1].name.c_str());
    return termvec[id - 1].name;
}

inline const string
DADatabase::get_database_path() const {
    Assert(opened);
    return path;
}

#endif /* _da_database_h_ */
