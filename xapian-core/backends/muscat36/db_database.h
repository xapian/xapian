/* db_database.h: C++ class definition for DB access routines
 *
 * ----START-LICENCE----
 * Copyright 1999,2000 BrightStation PLC
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

#ifndef OM_HGUARD_DB_DATABASE_H
#define OM_HGUARD_DB_DATABASE_H

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
#include "dbread.h"

/** A posting list for a DB Database */
class DBPostList : public LeafPostList {
    friend class DBDatabase;
    private:
	struct DB_postings * postlist;
	om_docid  currdoc;

	om_termname tname;
	om_doccount termfreq;

	DBPostList(const om_termname & tname_,
		   struct DB_postings * postlist_,
		   om_doccount termfreq_);
    public:
	~DBPostList();

	om_doccount get_termfreq() const;

	om_docid  get_docid() const;     // Gets current docid
	om_weight get_weight() const;    // Gets current weight
	om_doclength get_doclength() const; // Get length of current document
        om_termcount get_wdf() const;    // Within Document Frequency
	PositionList *get_position_list(); // Gets positions
	PostList *next(om_weight w_min);          // Moves to next docid
	PostList *skip_to(om_docid did, om_weight w_min);  // Moves to next docid >= specified docid
	bool   at_end() const;        // True if we're off the end of the list

	std::string intro_term_description() const;
};

inline om_doccount
DBPostList::get_termfreq() const
{
    DEBUGLINE(DB, "DAPostList::get_termfreq() = " << termfreq);
    return termfreq;
}

inline om_docid
DBPostList::get_docid() const
{
    Assert(!at_end());
    Assert(currdoc != 0);
    DEBUGLINE(DB, "DAPostList::get_doclength() = " << 1.0);
    return currdoc;
}

inline om_doclength
DBPostList::get_doclength() const
{
    // FIXME: return database->get_doclength()
    return 1;
}

inline om_termcount
DBPostList::get_wdf() const
{
    DEBUGLINE(DB, "DAPostList::get_wdf() = " << postlist->wdf);
    return postlist->wdf;
}

inline bool
DBPostList::at_end() const
{
    Assert(currdoc != 0);
    if (currdoc == MAXINT) {
	DEBUGLINE(DB, "DAPostList::at_end() = true");
	return true;
    }
    DEBUGLINE(DB, "DAPostList::at_end() = false");
    return false;
}

inline std::string
DBPostList::intro_term_description() const
{
    return tname + ":" + om_tostring(termfreq);
}



class DBTermListItem {
    public:
	om_termname tname;
	om_termcount wdf;
	om_doccount termfreq;

	DBTermListItem(om_termname tname_,
		       om_termcount wdf_,
		       om_doccount termfreq_)
		: tname(tname_),
		  wdf(wdf_),
		  termfreq(termfreq_)
	{ return; }
};

class DBTermList : public LeafTermList {
    friend class DBDatabase;
    private:
	std::vector<DBTermListItem>::iterator pos;
	std::vector<DBTermListItem> terms;
	bool have_started;
	om_doccount dbsize;

	DBTermList(struct termvec *tv, om_doccount dbsize_);
    public:
	om_termcount get_approx_size() const;

	OmExpandBits get_weighting() const; // Gets weight info of current term
	const om_termname get_termname() const;
	om_termcount get_wdf() const; // Number of occurences of term in current doc
	om_doccount get_termfreq() const;  // Number of docs indexed by term
	TermList * next();
	bool   at_end() const;
};

inline om_termcount DBTermList::get_approx_size() const
{
    return terms.size();
}

inline const om_termname DBTermList::get_termname() const
{
    Assert(!at_end());
    Assert(have_started);
    return pos->tname;
}

inline om_termcount DBTermList::get_wdf() const
{
    Assert(!at_end());
    Assert(have_started);
    return pos->wdf;
}

inline om_doccount DBTermList::get_termfreq() const
{
    Assert(!at_end());
    Assert(have_started);
    return pos->termfreq;
}

inline TermList * DBTermList::next()
{
    if(have_started) {
	Assert(!at_end());
	pos++;
    } else {
	have_started = true;
    }
    return NULL;
}

inline bool DBTermList::at_end() const
{
    Assert(have_started);
    if(pos == terms.end()) {
	DEBUGLINE(DB, "TERMLIST " << this << " ENDED ");
	return true;
    }
    return false;
}




class DBTerm : public OmRefCntBase {
    friend DBDatabase;
    private:
	DBTerm(struct DB_term_info * ti_,
	       om_termname tname_,
	       struct DB_file * DB_ = NULL);
        struct DB_term_info * get_ti() const;

	mutable bool terminfo_initialised;
        mutable struct DB_term_info ti;
        mutable struct DB_file * DB;
    public:
	om_termname tname;
};

inline
DBTerm::DBTerm(struct DB_term_info * ti_,
	       om_termname tname_,
	       struct DB_file * DB_)
	: terminfo_initialised(false)
{
    if (ti_) {
	ti = *ti_;
	terminfo_initialised = true;
    }
    tname = tname_;
    DB = DB_;
}

inline struct DB_term_info *
DBTerm::get_ti() const
{
    if (!terminfo_initialised) {
	DEBUGLINE(DB, "Getting terminfo");
	std::string::size_type len = tname.length();
	if(len > 255) abort();
	byte * k = (byte *) malloc(len + 1);
	if(k == NULL) throw std::bad_alloc();
	k[0] = len + 1;
	tname.copy((char*)(k + 1), len, 0);

	int found = DB_term(k, &ti, DB);
	free(k);

	if(found == 0) abort();
	terminfo_initialised = true;
    }
    return &ti;
}

class DBDatabase : public IRDatabase {
    friend class DatabaseBuilder;
    friend class DBDocument;
    private:
	OmLock mutex;

	struct DB_file * DB;

	FILE * keyfile;

	mutable std::map<om_termname, OmRefCntPtr<const DBTerm> > termmap;

	// Stop copy / assignment being allowed
	DBDatabase& operator=(const DBDatabase&);
	DBDatabase(const DBDatabase&);

	// Look up term in database
	OmRefCntPtr<const DBTerm> term_lookup(const om_termname & tname) const;

	// Get a record
	struct record * get_record(om_docid did) const;

	/** Get a key from keyfile (will return empty value if keyfile
	 *  not open).
	 */
	OmKey get_key(om_docid did, om_keyno keyid) const;

	/// Internal method for getting the database size.
	om_doccount  get_doccount_internal() const;

	/** Internal method for getting the average length of a document in
	 *  the database.
	 */
	om_doclength get_avlength_internal() const;

	/** Internal method for opening postlists.
	 */
	LeafPostList * open_post_list_internal(const om_termname & tname) const;

	/** Internal method for checking term existence.
	 */
	bool term_exists_internal(const om_termname & tname) const;

	/** Create and open a DB database.
	 *
	 *  @exception OmOpeningError thrown if database can't be opened.
	 *
	 *  @param params Parameters supplied by the user to specify the
	 *                location of the database to open.
	 */
	DBDatabase(const OmSettings & params, bool readonly);
    public:
	~DBDatabase();

	om_doccount  get_doccount() const;
	om_doclength get_avlength() const;
	om_doclength get_doclength(om_docid did) const;

	om_doccount get_termfreq(const om_termname & tname) const;
	bool term_exists(const om_termname & tname) const;

	LeafPostList * do_open_post_list(const om_termname & tname) const;
	LeafTermList * open_term_list(om_docid did) const;
	LeafDocument * open_document(om_docid did) const;

	//@{
	/** DBDatabase is a readonly database type, and thus this method is
	 *  not supported: if called an exception will be thrown.
	 */
	void do_begin_session(om_timeout timeout) {
	    throw OmUnimplementedError(
		"DBDatabase::begin_session() not implemented: readonly database type");
	};

	void do_end_session() {
	    throw OmUnimplementedError(
		"DBDatabase::do_end_session() not implemented: readonly database type");
	};

	void do_flush() {
	    throw OmUnimplementedError(
		"DBDatabase::flush() not implemented: readonly database type");
	};

	void do_begin_transaction() {
	    throw OmUnimplementedError(
		"DBDatabase::begin_transaction() not implemented: readonly database type");
	};

	void do_commit_transaction() {
	    throw OmUnimplementedError(
		"DBDatabase::commit_transaction() not implemented: readonly database type");
	};

	void do_cancel_transaction() {
	    throw OmUnimplementedError(
		"DBDatabase::cancel_transaction() not implemented: readonly database type");
	};

	om_docid do_add_document(const struct OmDocumentContents & document) {
	    throw OmUnimplementedError(
		"DBDatabase::add_document() not implemented: readonly database type");
	}

	void do_delete_document(om_docid did) {
	    throw OmUnimplementedError(
		"DBDatabase::delete_document() not implemented: readonly database type");
	};

	void do_replace_document(om_docid did,
				 const OmDocumentContents & document) {
	    throw OmUnimplementedError(
		"DBDatabase::replace_document() not implemented: readonly database type");
	};

	//@}

	/** Get a document from the database.
	 *  FIXME: implement this method.
	 */
	OmDocumentContents do_get_document(om_docid did) {
	    throw OmUnimplementedError(
		"DBDatabase::get_document() not yet implemented");
	};
};

#endif /* OM_HGUARD_DB_DATABASE_H */
