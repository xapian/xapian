/* sleepy_database.h: C++ class definition for sleepycat access routines
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

#ifndef OM_HGUARD_SLEEPY_DATABASE_H
#define OM_HGUARD_SLEEPY_DATABASE_H

#include "utils.h"
#include "omassert.h"
#include "dbpostlist.h"
#include "termlist.h"
#include "database.h"
#include <stdlib.h>

// Postlist - a list of documents indexed by a given term
class SleepyPostList : public virtual DBPostList {
    friend class SleepyDatabase;
    private:
	doccount pos;
	docid *data;

	termname tname;
	doccount termfreq;

	SleepyPostList(const termname &tn, docid *data_new, doccount tf);
    public:
	~SleepyPostList();

	doccount   get_termfreq() const;// Number of docs indexed by this term

	docid      get_docid() const;   // Current docid
	weight     get_weight() const;  // Current weight
        PostList * next(weight w_min);  // Move to next docid
        PostList * skip_to(docid did, weight w_min);  // Skip to next docid >= docid
	bool       at_end() const;      // True if we're off the end of the list

	string intro_term_description() const;
};



class SleepyDatabaseTermCache;
// Termlist - a list of terms indexing a given document
class SleepyTermList : public virtual DBTermList {
    friend class SleepyDatabase;
    private:
	termcount pos;
	termid *data;
	termcount terms;
	doccount dbsize;

	const SleepyDatabaseTermCache *termcache;

	SleepyTermList(const SleepyDatabaseTermCache *tc_new,
		       termid *data_new,
		       termcount terms_new,
		       doccount dbsize_new);
    public:
	~SleepyTermList();
	termcount get_approx_size() const;

	OmExpandBits get_weighting() const;  // Gets weight of current term
	const termname get_termname() const;  // Current term
	termcount get_wdf() const;  // Occurences of current term in doc
	doccount get_termfreq() const;  // Docs indexed by current term
	TermList * next();
	bool   at_end() const;
};



class SleepyDatabaseInternals;

class SleepyDatabaseTermCache {
    friend class SleepyDatabase;
    private:
	SleepyDatabaseInternals * internals;
	SleepyDatabaseTermCache(SleepyDatabaseInternals *i) : internals(i) {}
    public:
	termname term_id_to_name(termid tid) const;
	termid term_name_to_id(const termname & tname) const;
};

class SleepyDatabase : public virtual IRDatabase {
    friend class DatabaseBuilder;
    private:
	SleepyDatabaseInternals * internals;
	bool opened;

	SleepyDatabaseTermCache * termcache;

	void open(const DatabaseBuilderParams & params);
	SleepyDatabase();
    public:
	~SleepyDatabase();

	doccount  get_doccount() const;
	doclength get_avlength() const;

	doccount get_termfreq(const termname & tname) const;
	bool term_exists(const termname & tname) const;

	DBPostList * open_post_list(const termname& tname, RSet * rset) const;
	DBTermList * open_term_list(docid did) const;
	IRDocument * open_document(docid did) const;

	void make_term(const termname &) {
	    throw OmError("DADatabase::make_term() not implemented");
	}
	docid make_doc(const docname &) {
	    throw OmError("DADatabase::make_doc() not implemented");
	}
	void make_posting(const termname &, unsigned int, unsigned int) {
	    throw OmError("DADatabase::make_posting() not implemented");
	}
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

inline PostList *
SleepyPostList::skip_to(docid did, weight w_min)
{
    Assert(!at_end());
    if(pos == 0) pos++;
    while (!at_end() && data[pos - 1] < did) {
	PostList *ret = next(w_min);
	if (ret) return ret;
    }
    return NULL;
}

inline bool
SleepyPostList::at_end() const
{
    if(pos > termfreq) return true;
    return false;
}

inline string
SleepyPostList::intro_term_description() const
{   
    return tname + ":" + inttostring(termfreq);
}


///////////////////////////////////////////
// Inline definitions for SleepyTermList //
///////////////////////////////////////////

inline termcount
SleepyTermList::get_approx_size() const
{
    return terms;
}

inline OmExpandBits
SleepyTermList::get_weighting() const {
    Assert(!at_end());
    Assert(pos != 0);
    Assert(wt != NULL);

    termcount wdf = 1; // FIXME - not yet stored in data structure
    doclength norm_len = 1.0; // FIXME - not yet stored in data structure

    return wt->get_bits(wdf, norm_len, SleepyTermList::get_termfreq(), dbsize);
}

inline const termname
SleepyTermList::get_termname() const
{
    Assert(!at_end());
    Assert(pos != 0);
    return termcache->term_id_to_name(data[pos]);
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

inline doccount
SleepyDatabase::get_termfreq(const termname &tname) const
{   
    PostList *pl = open_post_list(tname, NULL);
    doccount freq = 0;
    if(pl) freq = pl->get_termfreq();
    delete pl;
    return freq;
}

inline bool
SleepyDatabase::term_exists(const termname &tname) const
{
    if(termcache->term_name_to_id(tname)) return true;
    return false;
}

#endif /* OM_HGUARD_SLEEPY_DATABASE_H */
