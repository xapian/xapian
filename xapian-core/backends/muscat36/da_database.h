/* da_database.h: C++ class definition for DA access routines
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2002,2003,2004 Olly Betts
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

#ifndef OM_HGUARD_DA_DATABASE_H
#define OM_HGUARD_DA_DATABASE_H

#include <stdio.h>
#include <map>
#include <vector>
#include <errno.h>
#include "document.h"

#include "utils.h"
#include "omdebug.h"

#include "leafpostlist.h"
#include "termlist.h"
#include "database.h"

// FIXME - try and sort it out so that we don't need to include this.
#include "daread.h"

/** A posting list for a DA Database */
class DAPostList : public LeafPostList {
    friend class DADatabase;
    private:
	struct DA_postings * postlist;
	Xapian::docid  currdoc;

	string tname;
	Xapian::doccount termfreq;

	Xapian::Internal::RefCntPtr<const DADatabase> this_db; // Just used to keep a reference

	DAPostList(const string & tname_,
		   struct DA_postings * postlist_,
		   Xapian::doccount termfreq_,
		   Xapian::Internal::RefCntPtr<const DADatabase> this_db_);
    public:
	~DAPostList();

	Xapian::doccount get_termfreq() const;

	Xapian::docid  get_docid() const;     // Gets current docid
	Xapian::doclength get_doclength() const; // Get length of current document
        Xapian::termcount get_wdf() const;    // Within Document Frequency
	PositionList *read_position_list(); // Gets positions
	PositionList * open_position_list() const; // Gets positions
	PostList *next(Xapian::weight w_min);          // Moves to next docid
	PostList *skip_to(Xapian::docid did, Xapian::weight w_min);  // Moves to next docid >= specified docid
	bool   at_end() const;        // True if we're off the end of the list

	std::string get_description() const;
};

inline Xapian::doccount
DAPostList::get_termfreq() const
{
    DEBUGLINE(DB, "DAPostList::get_termfreq() = " << termfreq);
    return termfreq;
}

inline Xapian::docid
DAPostList::get_docid() const
{
    Assert(!at_end());
    Assert(currdoc != 0);
    DEBUGLINE(DB, "DAPostList::get_docid() = " << currdoc);
    return currdoc;
}

inline Xapian::doclength
DAPostList::get_doclength() const
{
    // FIXME: return database->get_doclength()
    DEBUGLINE(DB, "DAPostList::get_doclength() = " << 1.0);
    return 1;
}

inline Xapian::termcount
DAPostList::get_wdf() const
{
    DEBUGLINE(DB, "DAPostList::get_wdf() = " << postlist->wdf);
    return postlist->wdf;
}

inline bool
DAPostList::at_end() const
{
    DEBUGCALL(DB, bool, "DAPostList::at_end()", "");
    Assert(currdoc != 0);
    RETURN(currdoc == MAXINT);
}

inline std::string
DAPostList::get_description() const
{
    return tname + ":" + om_tostring(termfreq);
}



class DATermListItem {
    public:
	string tname;
	Xapian::termcount wdf;
	Xapian::doccount termfreq;

	DATermListItem(string tname_, Xapian::termcount wdf_,
		       Xapian::doccount termfreq_)
		: tname(tname_), wdf(wdf_), termfreq(termfreq_)
	{ }
};

/** A term list for a DA Database */
class DATermList : public LeafTermList {
    friend class DADatabase;
    private:
	std::vector<DATermListItem>::iterator pos;
	std::vector<DATermListItem> terms;
	bool have_started;
	Xapian::doccount dbsize;

	Xapian::Internal::RefCntPtr<const DADatabase> this_db;

	DATermList(struct termvec *tv, Xapian::doccount dbsize_,
		   Xapian::Internal::RefCntPtr<const DADatabase> this_db_);
    public:
	Xapian::termcount get_approx_size() const;

	OmExpandBits get_weighting() const; // Gets weight info of current term
	string get_termname() const;
	Xapian::termcount get_wdf() const; // Number of occurrences of term in current doc
	Xapian::doccount get_termfreq() const;  // Number of docs indexed by term
	TermList * next();
	bool   at_end() const;
};

inline Xapian::termcount DATermList::get_approx_size() const
{
    return terms.size();
}

inline string DATermList::get_termname() const
{
    Assert(!at_end());
    Assert(have_started);
    return pos->tname;
}

inline Xapian::termcount DATermList::get_wdf() const
{
    Assert(!at_end());
    Assert(have_started);
    return pos->wdf;
}

inline Xapian::doccount DATermList::get_termfreq() const
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
    DEBUGCALL(DB, bool, "DATermList::at_end()", "");
    Assert(have_started);
    RETURN(pos == terms.end());
}




class DATerm : public Xapian::Internal::RefCntBase {
    friend class DADatabase;
    private:
	DATerm(struct DA_term_info * ti_,
	       string tname_,
	       struct DA_file * DA_t_ = NULL);
        struct DA_term_info * get_ti() const;

	mutable bool terminfo_initialised;
        mutable struct DA_term_info ti;
        mutable struct DA_file * DA_t;
    public:
	string tname;
};

inline
DATerm::DATerm(struct DA_term_info * ti_,
	       string tname_,
	       struct DA_file * DA_t_)
	: terminfo_initialised(false)
{
    if (ti_) {
	ti = *ti_;
	terminfo_initialised = true;
    }
    tname = tname_;
    DA_t = DA_t_;
}

inline struct DA_term_info *
DATerm::get_ti() const
{
    if (!terminfo_initialised) {
	DEBUGLINE(DB, "Getting terminfo");
	std::string::size_type len = tname.length();
	if(len > 255) abort();
	byte * k = (byte *) malloc(len + 1);
	if(k == NULL) throw std::bad_alloc();
	k[0] = len + 1;
	tname.copy((char*)(k + 1), len, 0);

	int found = DA_term(k, &ti, DA_t);
	free(k);

	if(found == 0) abort();
	terminfo_initialised = true;
    }
    return &ti;
}

/** A DA Database */
class DADatabase : public Xapian::Database::Internal {
    friend class DADocument;
    private:
	struct DA_file * DA_r;
	struct DA_file * DA_t;

	FILE * valuefile;

	mutable std::map<string, Xapian::Internal::RefCntPtr<const DATerm> > termmap;

	bool heavy_duty;

	// Stop copy / assignment being allowed
	DADatabase& operator=(const DADatabase&);
	DADatabase(const DADatabase&);

	// Look up term in database
	Xapian::Internal::RefCntPtr<const DATerm> term_lookup(const string & tname) const;

	// Get a record
	struct record * get_record(Xapian::docid did) const;

	/** Get a value from valuefile (will return empty value if valuefile
	 *  not open.
	 */
	string get_value(Xapian::docid did, Xapian::valueno valueid) const;

	/** Internal method for opening postlists.
	 */
	LeafPostList * open_post_list_internal(const string & tname) const;

    public:
	/** Create and open a DA database.
	 *
	 *  @exception Xapian::DatabaseOpeningError thrown if database can't be
	 *      opened.
	 *
	 *  @param filename_r Filename of the record file (usually called "R").
	 *  @param filename_t Filename of the term file (usually called "T").
	 *  @param filename_v Filename of the value file (or "" if none).
	 *  @param heavy_duty_ True if lengths are 3 bytes, false if they're 2.
	 */
	DADatabase(const string &filename_r, const string &filename_t,
		   const string &filename_v, bool heavy_duty_);

	~DADatabase();

	/// Get the database size.
	Xapian::doccount  get_doccount() const;
	/// Get the average length of a document in the database.
	Xapian::doclength get_avlength() const;
	Xapian::doclength get_doclength(Xapian::docid did) const;

	Xapian::doccount get_termfreq(const string & tname) const;
	Xapian::termcount get_collection_freq(const string & /*tname*/) const {
	    throw Xapian::UnimplementedError(
		"DADatabase::get_collection_freq() not implemented: data not stored in database.");
	}
	bool term_exists(const string & tname) const;

	LeafPostList * do_open_post_list(const string & tname) const;
	LeafTermList * open_term_list(Xapian::docid did) const;
	Xapian::Document::Internal * open_document(Xapian::docid did, bool lazy = false) const;
	PositionList * open_position_list(Xapian::docid did,
					  const string & tname) const;
	TermList * open_allterms() const;
};

#endif /* OM_HGUARD_DA_DATABASE_H */
