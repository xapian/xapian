/* da_database.h: C++ class definition for DA access routines */

#ifndef _da_database_h_
#define _da_database_h_

#include <map>
#include <vector>

#include "omassert.h"

#include "postlist.h"
#include "termlist.h"
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

	DAPostList(struct postings *, doccount);
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
	termname tname;
	termcount wdf;
	doccount termfreq;

	DATermListItem(termname tname_new,
		       termcount wdf_new,
		       doccount termfreq_new)
		: tname(tname_new),
		  wdf(wdf_new),
		  termfreq(termfreq_new)
	{ return; }
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
	termcount get_approx_size() const;

	weight get_weight() const; // Gets weight of current term
	termname get_termname() const;
	termcount get_wdf() const; // Number of occurences of term in current doc
	doccount get_termfreq() const;  // Number of docs indexed by term
	TermList * next();
	bool   at_end() const;
};

inline termcount DATermList::get_approx_size() const
{
    return terms.size();
}

inline termname DATermList::get_termname() const
{
    Assert(!at_end());
    Assert(have_started);
    return pos->tname;
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
    if(pos == terms.end()) {
#ifdef DEBUG
	cout << "TERMLIST " << this << " ENDED " << endl;
#endif
	return true;
    }
    return false;
}




class DATerm {
    friend class DADatabase;
    private:
	DATerm(struct terminfo *, termname, struct DAfile * = NULL);
        struct terminfo * get_ti();

	bool terminfo_initialised;
        struct terminfo ti;
        struct DAfile * DA_t;
    public:
	termname name;
};

inline
DATerm::DATerm(struct terminfo *ti_new,
	       termname name_new,
	       struct DAfile * DA_t_new)
	: terminfo_initialised(false)
{
    if (ti_new) {
	ti = *ti_new;
	terminfo_initialised = true;
    }
    name = name_new;
    DA_t = DA_t_new;
}

inline struct terminfo *
DATerm::get_ti()
{
    if (!terminfo_initialised) {
	cout << "Getting terminfo" << endl;
	int len = name.length();
	if(len > 255) abort();
	byte * k = (byte *) malloc(len + 1);
	if(k == NULL) throw OmError(strerror(ENOMEM));
	k[0] = len + 1;
	name.copy((char*)(k + 1), len);

	int found = DAterm(k, &ti, DA_t);
	free(k);

	if(found == 0) abort();
	terminfo_initialised = true;
    }
    return &ti;
}

class DADatabase : public virtual IRDatabase {
    friend class DatabaseBuilder;
    private:
	bool   opened;
	struct DAfile * DA_r;
	struct DAfile * DA_t;

	mutable map<termname, termid> termidmap;
	mutable vector<DATerm> termvec;

	// Stop copy / assignment being allowed
	DADatabase& operator=(const DADatabase&);
	DADatabase(const DADatabase&);

	DADatabase();
	void open(const DatabaseBuilderParams &);
    public:
	~DADatabase();

	termid term_name_to_id(const termname &) const;
	termname term_id_to_name(termid) const;

	doccount  get_doccount() const;
	doclength get_avlength() const;

	doccount get_termfreq(const termname &) const;
	bool term_exists(const termname &) const;

	DBPostList * open_post_list(const termname&, RSet *) const;
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

inline doccount
DADatabase::get_termfreq(const termname &tname) const
{
    PostList *pl = open_post_list(tname, NULL);
    doccount freq = 0;
    if(pl) freq = pl->get_termfreq();
    delete pl;
    return freq;
}

inline bool
DADatabase::term_exists(const termname &tname) const
{
    if(term_name_to_id(tname)) return true;
    return false;
}

inline termname
DADatabase::term_id_to_name(termid id) const
{
    Assert(opened);
    Assert(id > 0 && id <= termvec.size());
    //printf("Looking up termid %d: name = `%s'\n", id, termvec[id - 1].name.c_str());
    return termvec[id - 1].name;
}

#endif /* _da_database_h_ */
