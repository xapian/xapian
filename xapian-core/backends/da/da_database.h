/* da_database.h: C++ class definition for DA access routines
 *
 * ----START-LICENCE----
 * Copyright 1999,2000 Dialog Corporation
 * 
 * This program is free software; you can redistribute it and/or 
 * modify it under the terms of the GNU General Public License as 
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 * -----END-LICENCE-----
 */

#ifndef _da_database_h_
#define _da_database_h_

#include <map>
#include <vector>

#include "utils.h"
#include "omassert.h"

#include "dbpostlist.h"
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

	termname tname;
	doccount termfreq;

	DAPostList(const termname & tname_,
		   struct postings * postlist_,
		   doccount termfreq_);
    public:
	~DAPostList();

	doccount get_termfreq() const;

	docid  get_docid() const;     // Gets current docid
	weight get_weight() const;    // Gets current weight
	PostList *next(weight w_min);          // Moves to next docid
	PostList *skip_to(docid did, weight w_min);  // Moves to next docid >= specified docid
	bool   at_end() const;        // True if we're off the end of the list

	string intro_term_description() const;
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
    return currdoc;
}

inline bool
DAPostList::at_end() const
{
    Assert(currdoc != 0);
    if (currdoc == MAXINT) return true;
    return false;
}

inline string
DAPostList::intro_term_description() const
{
    return tname + ":" + inttostring(termfreq);
}



class DATermListItem {
    public:
	termname tname;
	termcount wdf;
	doccount termfreq;

	DATermListItem(termname tname_,
		       termcount wdf_,
		       doccount termfreq_)
		: tname(tname_),
		  wdf(wdf_),
		  termfreq(termfreq_)
	{ return; }
};
 
class DATermList : public virtual DBTermList {
    friend class DADatabase;
    private:
	vector<DATermListItem>::iterator pos;
	vector<DATermListItem> terms;
	bool have_started;
	doccount dbsize;

	DATermList(struct termvec *tv, doccount dbsize_);
    public:
	termcount get_approx_size() const;

	OMExpandBits get_weighting() const; // Gets weight info of current term
	const termname get_termname() const;
	termcount get_wdf() const; // Number of occurences of term in current doc
	doccount get_termfreq() const;  // Number of docs indexed by term
	TermList * next();
	bool   at_end() const;
};

inline termcount DATermList::get_approx_size() const
{
    return terms.size();
}

inline const termname DATermList::get_termname() const
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
	DebugMsg("TERMLIST " << this << " ENDED " << endl);
	return true;
    }
    return false;
}




class DATerm {
    friend DADatabase;
    private:
	DATerm(struct terminfo * ti_,
	       termname tname_,
	       struct DAfile * DA_t_ = NULL);
        struct terminfo * get_ti() const;

	mutable bool terminfo_initialised;
        mutable struct terminfo ti;
        mutable struct DAfile * DA_t;
    public:
	termname tname;
};

inline
DATerm::DATerm(struct terminfo * ti_,
	       termname tname_,
	       struct DAfile * DA_t_)
	: terminfo_initialised(false)
{
    if (ti_) {
	ti = *ti_;
	terminfo_initialised = true;
    }
    tname = tname_;
    DA_t = DA_t_;
}

inline struct terminfo *
DATerm::get_ti() const
{
    if (!terminfo_initialised) {
	DebugMsg("Getting terminfo" << endl);
	int len = tname.length();
	if(len > 255) abort();
	byte * k = (byte *) malloc(len + 1);
	if(k == NULL) throw bad_alloc();
	k[0] = len + 1;
	tname.copy((char*)(k + 1), len);

	int found = DAterm(k, &ti, DA_t);
	free(k);

	if(found == 0) abort();
	terminfo_initialised = true;
    }
    return &ti;
}

class DADatabase : public virtual IRDatabase {
    friend class DatabaseBuilder;
    friend class DADocument;
    private:
	bool   opened;
	struct DAfile * DA_r;
	struct DAfile * DA_t;

	mutable map<termname, DATerm> termmap;

	// Stop copy / assignment being allowed
	DADatabase& operator=(const DADatabase&);
	DADatabase(const DADatabase&);

	// Look up term in database
	const DATerm * term_lookup(const termname & tname) const;

	// Get a record
	struct record * get_record(docid did) const;

	DADatabase();
	void open(const DatabaseBuilderParams & params);
    public:
	~DADatabase();

	doccount  get_doccount() const;
	doclength get_avlength() const;

	doccount get_termfreq(const termname & tname) const;
	bool term_exists(const termname & tname) const;

	DBPostList * open_post_list(const termname & tname, RSet * rset) const;
	DBTermList * open_term_list(docid did) const;
	IRDocument * open_document(docid did) const;

	void make_term(const termname & tname) {
	    throw OmUnimplemented("DADatabase::make_term() not implemented");
	}
	docid make_doc(const docname & ) {
	    throw OmUnimplemented("DADatabase::make_doc() not implemented");
	}
	void make_posting(const termname &, unsigned int, unsigned int) {
	    throw OmUnimplemented("DADatabase::make_posting() not implemented");
	}
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
DADatabase::get_termfreq(const termname & tname) const
{
    PostList *pl = open_post_list(tname, NULL);
    doccount freq = 0;
    if(pl) freq = pl->get_termfreq();
    delete pl;
    return freq;
}

#endif /* _da_database_h_ */
