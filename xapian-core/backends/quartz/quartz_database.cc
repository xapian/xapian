/* quartz_database.cc: quartz database
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2001 Hein Ragas
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

#include <config.h>

#include "quartz_table_manager.h"

#include "quartz_database.h"
#include "utils.h"
#include "omdebug.h"
#include "autoptr.h"
#include <xapian/error.h>

#include "quartz_postlist.h"
#include "quartz_termlist.h"
#include "quartz_positionlist.h"
#include "quartz_utils.h"
#include "quartz_record.h"
#include "quartz_values.h"
#include "quartz_document.h"
#include "quartz_alltermslist.h"

#include <list>
#include <string>

using namespace std;
using namespace Xapian;

QuartzDatabase::QuartzDatabase(const string &quartz_dir)
{
    DEBUGCALL(DB, void, "QuartzDatabase", quartz_dir);
    // Open database manager
    tables.reset(new QuartzDiskTableManager(quartz_dir, OM_DB_READONLY, 0u));
}

QuartzDatabase::QuartzDatabase(AutoPtr<QuartzTableManager> tables_)
	: tables(tables_)
{
    DEBUGCALL(DB, void, "QuartzDatabase", "[tables_]");
}

QuartzDatabase::~QuartzDatabase()
{
    DEBUGCALL(DB, void, "~QuartzDatabase", "");
    try {
	internal_end_session();
    } catch (...) {
	// Ignore any exceptions, since we may be being called due to an
	// exception anyway.  internal_end_session() should have already
	// been called, in the normal course of events.
	DEBUGLINE(DB, "Ignoring exception in QuartzDatabase destructor.");
    }
}

void
QuartzDatabase::do_begin_session()
{
    DEBUGCALL(DB, void, "QuartzDatabase::do_begin_session", "");
    throw Xapian::InvalidOperationError(
	"Cannot begin a modification session: database opened readonly.");
}

void
QuartzDatabase::do_end_session()
{ Assert(false); }

void
QuartzDatabase::do_flush()
{ Assert(false); }

void
QuartzDatabase::do_begin_transaction()
{ Assert(false); }

void
QuartzDatabase::do_commit_transaction()
{ Assert(false); }

void
QuartzDatabase::do_cancel_transaction()
{ Assert(false); }

Xapian::docid
QuartzDatabase::do_add_document(const Xapian::Document & /*document*/)
{
    DEBUGCALL(DB, Xapian::docid, "QuartzDatabase::do_add_document", "");
    Assert(false);
    RETURN(0);
}

void
QuartzDatabase::do_delete_document(Xapian::docid /*did*/)
{ Assert(false); }

void
QuartzDatabase::do_replace_document(Xapian::docid /*did*/,
				    const Xapian::Document & /*document*/)
{ Assert(false); }

Xapian::doccount
QuartzDatabase::get_doccount() const
{
    DEBUGCALL(DB, Xapian::doccount, "QuartzDatabase::get_doccount", "");
    RETURN(QuartzRecordManager::get_doccount(*(tables->get_record_table())));
}

Xapian::doclength
QuartzDatabase::get_avlength() const
{
    DEBUGCALL(DB, Xapian::doclength, "QuartzDatabase::get_avlength", "");
    RETURN(QuartzRecordManager::get_avlength(*(tables->get_record_table())));
}

Xapian::doclength
QuartzDatabase::get_doclength(Xapian::docid did) const
{
    DEBUGCALL(DB, Xapian::doclength, "QuartzDatabase::get_doclength", did);
    Assert(did != 0);

    QuartzTermList termlist(0, tables->get_termlist_table(), did, 0);
    RETURN(termlist.get_doclength());
}

Xapian::doccount
QuartzDatabase::get_termfreq(const string & tname) const
{
    DEBUGCALL(DB, Xapian::doccount, "QuartzDatabase::get_termfreq", tname);
    Assert(!tname.empty());

    QuartzPostList pl(NULL, tables->get_postlist_table(), NULL, tname);
    RETURN(pl.get_termfreq());
}

Xapian::termcount
QuartzDatabase::get_collection_freq(const string & tname) const
{
    DEBUGCALL(DB, Xapian::termcount, "QuartzDatabase::get_collection_freq", tname);
    Assert(!tname.empty());

    Xapian::termcount collfreq = 0; // If not found, this value will be unchanged.
    QuartzPostList pl(NULL, tables->get_postlist_table(), NULL, tname);
    collfreq = pl.get_collection_freq();
    RETURN(collfreq);
}

bool
QuartzDatabase::term_exists(const string & tname) const
{
    DEBUGCALL(DB, bool, "QuartzDatabase::term_exists", tname);
    Assert(!tname.empty());
    const QuartzTable * table = tables->get_postlist_table();
    AutoPtr<QuartzCursor> cursor(table->cursor_get());
    // FIXME: nasty C&P from backends/quartz/quartz_postlist.cc
    string key = pack_string_preserving_sort(tname);
    return cursor->find_entry(key);
}


LeafPostList *
QuartzDatabase::do_open_post_list(const string& tname) const
{
    DEBUGCALL(DB, LeafPostList *, "QuartzDatabase::do_open_post_list", tname);
    Xapian::Internal::RefCntPtr<const QuartzDatabase> ptrtothis(this);

    RETURN(open_post_list_internal(tname, ptrtothis));
}

LeafPostList *
QuartzDatabase::open_post_list_internal(const string& tname,
				Xapian::Internal::RefCntPtr<const Xapian::Database::Internal> ptrtothis) const
{
    DEBUGCALL(DB, LeafPostList *, "QuartzDatabase::open_post_list_internal", tname << ", [ptrtothis]");
    Assert(!tname.empty());
    return(new QuartzPostList(ptrtothis,
			      tables->get_postlist_table(),
			      tables->get_positionlist_table(),
			      tname));
}

LeafTermList *
QuartzDatabase::open_term_list_internal(Xapian::docid did,
				Xapian::Internal::RefCntPtr<const Xapian::Database::Internal> ptrtothis) const
{
    DEBUGCALL(DB, LeafTermList *, "QuartzDatabase::open_term_list_internal",
	      did << ", [ptrtothis]");
    Assert(did != 0);
    return(new QuartzTermList(ptrtothis, tables->get_termlist_table(),
			      did, get_doccount()));
}

LeafTermList *
QuartzDatabase::open_term_list(Xapian::docid did) const
{
    DEBUGCALL(DB, LeafTermList *, "QuartzDatabase::open_term_list", did);
    Xapian::Internal::RefCntPtr<const QuartzDatabase> ptrtothis(this);
    RETURN(open_term_list_internal(did, ptrtothis));
}

Xapian::Document::Internal *
QuartzDatabase::open_document(Xapian::docid did, bool lazy) const
{
    DEBUGCALL(DB, Xapian::Document::Internal *, "QuartzDatabase::open_document",
	      did << ", " << lazy);
    Assert(did != 0);
    Xapian::Internal::RefCntPtr<const QuartzDatabase> ptrtothis(this);

    RETURN(new QuartzDocument(ptrtothis,
			      tables->get_value_table(),
			      tables->get_record_table(),
			      did, lazy));
}

PositionList *
QuartzDatabase::open_position_list(Xapian::docid did,
				   const string & tname) const
{
    Assert(did != 0);

    AutoPtr<QuartzPositionList> poslist(new QuartzPositionList());
    poslist->read_data(tables->get_positionlist_table(), did, tname);
    if (poslist->get_size() == 0) {
	// Check that term / document combination exists.
	Xapian::Internal::RefCntPtr<const QuartzDatabase> ptrtothis(this);
	// If the doc doesn't exist, this will throw Xapian::DocNotFoundError:
	AutoPtr<LeafTermList> ltl(open_term_list_internal(did, ptrtothis));
	ltl->skip_to(tname);
	if (ltl->at_end() || ltl->get_termname() != tname)
	    throw Xapian::RangeError("Can't open position list: requested term is not present in document.");
    }

    return poslist.release();
}

void
QuartzDatabase::do_reopen()
{
    DEBUGCALL(DB, void, "QuartzDatabase::do_reopen", "");
    tables->reopen();
}

TermList *
QuartzDatabase::open_allterms() const
{
    DEBUGCALL(DB, TermList *, "QuartzDatabase::open_allterms", "");
    QuartzTable *t = tables->get_postlist_table();
    AutoPtr<QuartzCursor> pl_cursor(t->cursor_get());
    RETURN(new QuartzAllTermsList(Xapian::Internal::RefCntPtr<const QuartzDatabase>(this),
				  pl_cursor, t->get_entry_count()));
}


QuartzWritableDatabase::QuartzWritableDatabase(const string &dir, int action,
					       int block_size)
	: buffered_tables(new QuartzBufferedTableManager(dir, action,
							 block_size)),
	  totlen_added(0),
	  totlen_removed(0),
	  freq_deltas(),
	  doclens(),
	  mod_plists(),
	  database_ro(AutoPtr<QuartzTableManager>(buffered_tables)),
	  changes_made(0)
{
    DEBUGCALL(DB, void, "QuartzWritableDatabase", dir << ", " << action << ", "
	      << block_size);
}

QuartzWritableDatabase::~QuartzWritableDatabase()
{
    DEBUGCALL(DB, void, "~QuartzWritableDatabase", "");
    // FIXME - release write lock if held
    try {
	internal_end_session();
    } catch (...) {
	// Ignore any exceptions, since we may be being called due to an
	// exception anyway.  internal_end_session() should have already
	// been called, in the normal course of events.
	DEBUGLINE(DB, "Ignoring exception in QuartzWritableDatabase destructor.");
    }
}

void
QuartzWritableDatabase::do_begin_session()
{
    DEBUGCALL(DB, void, "QuartzWritableDatabase::do_begin_session", "");
    Assert(buffered_tables != 0);

    // FIXME - get a write lock on the database
}

void
QuartzWritableDatabase::do_end_session()
{
    DEBUGCALL(DB, void, "QuartzWritableDatabase::do_end_session", "");
    Assert(buffered_tables != 0);

    do_flush();

    // FIXME - release write lock on the database (even if an apply() throws)
}

void
QuartzWritableDatabase::do_flush()
{
    return do_flush_const();
}

void
QuartzWritableDatabase::do_flush_const() const
{
    DEBUGCALL(DB, void, "QuartzWritableDatabase::do_flush_const", "");
    Assert(buffered_tables != 0);

    QuartzBufferedTable * pl_table = buffered_tables->get_postlist_table();
    QuartzPostList::merge_changes(pl_table, mod_plists, doclens, freq_deltas);

    // Update the total document length.
    QuartzRecordManager::modify_total_length(
	    *(buffered_tables->get_record_table()),
	    totlen_removed,
	    totlen_added);

    buffered_tables->apply();
    totlen_added = 0;
    totlen_removed = 0;
    freq_deltas.clear();
    doclens.clear();
    mod_plists.clear();
    changes_made = 0;
}

void
QuartzWritableDatabase::do_begin_transaction()
{
    DEBUGCALL(DB, void, "QuartzWritableDatabase::do_begin_transaction", "");
    throw Xapian::UnimplementedError("QuartzDatabase::do_begin_transaction() not yet implemented");
}

void
QuartzWritableDatabase::do_commit_transaction()
{
    DEBUGCALL(DB, void, "QuartzWritableDatabase::do_commit_transaction", "");
    throw Xapian::UnimplementedError("QuartzDatabase::do_commit_transaction() not yet implemented");
}

void
QuartzWritableDatabase::do_cancel_transaction()
{
    DEBUGCALL(DB, void, "QuartzWritableDatabase::do_cancel_transaction", "");
    throw Xapian::UnimplementedError("QuartzDatabase::do_cancel_transaction() not yet implemented");
}

Xapian::docid
QuartzWritableDatabase::do_add_document(const Xapian::Document & document)
{
    DEBUGCALL(DB, Xapian::docid,
	      "QuartzWritableDatabase::do_add_document", document);
    Assert(buffered_tables != 0);

    Xapian::docid did;
    try {
	// Set the record, and get the document ID to use.
	did = QuartzRecordManager::add_record(
		*(buffered_tables->get_record_table()),
		document.get_data());
	Assert(did != 0);

	// Set the values.
	{
	    Xapian::ValueIterator value = document.values_begin();
	    Xapian::ValueIterator value_end = document.values_end();
	    for ( ; value != value_end; ++value) {
		QuartzValueManager::add_value(
		    *(buffered_tables->get_value_table()),
		    *value, did, value.get_valueno());
	    }
	}

	quartz_doclen_t new_doclen = 0;
	{
	    Xapian::TermIterator term = document.termlist_begin();
	    Xapian::TermIterator term_end = document.termlist_end();
	    for ( ; term != term_end; ++term) {
		// Calculate the new document length
		new_doclen += term.get_wdf();
		string tname = *term;
		termcount wdf = term.get_wdf();

		map<string, pair<termcount_diff, termcount_diff> >::iterator i;
		i = freq_deltas.find(tname);
		if (i == freq_deltas.end()) {
		    freq_deltas.insert(make_pair(tname, make_pair(1, termcount_diff(wdf))));
		} else {
		    ++i->second.first;
		    i->second.second += wdf;
		}

		// Add did to tname's postlist
		map<string, map<docid, pair<char, termcount> > >::iterator j;
		j = mod_plists.find(tname);
		if (j == mod_plists.end()) {
		    map<docid, pair<char, termcount> > m;
		    j = mod_plists.insert(make_pair(tname, m)).first;
		}
		map<docid, pair<char, termcount> >::iterator k;
		k = j->second.find(did);
		if (k != j->second.end()) {
		    if (k->second.first == 'D') k->second.first = 'M';
		    k->second.second = wdf;
		} else {
		    j->second.insert(make_pair(did, make_pair('A', wdf)));
		}

		if (term.positionlist_begin() != term.positionlist_end()) {
		    QuartzPositionList::set_positionlist(
			buffered_tables->get_positionlist_table(), did, tname,
			term.positionlist_begin(), term.positionlist_end());
		}
	    }
	}

	// Set the termlist
	QuartzTermList::set_entries(buffered_tables->get_termlist_table(), did,
		document.termlist_begin(), document.termlist_end(),
		new_doclen, false);

	// Set the new document length
	doclens.insert(make_pair(did, new_doclen));
	totlen_added += new_doclen;
    } catch (...) {
	// If an error occurs while adding a document, or doing any other
	// transaction, the modifications so far must be cleared before
	// returning control to the user - otherwise partial modifications will
	// persist in memory, and eventually get written to disk.
	buffered_tables->cancel();
	totlen_added = 0;
	totlen_removed = 0;
	freq_deltas.clear();
	doclens.clear();
	mod_plists.clear();
	changes_made = 0;
	throw;
    }

    // FIXME: this should be configurable
    // FIXME: this should be done by checking memory usage, not the number of
    // changes.
    // We could also look at:
    // * mod_plists.size()
    // * doclens.size()
    // * freq_deltas.size()
    //
    // cout << "+++ mod_plists.size() " << mod_plists.size() <<
    //     ", doclens.size() " << doclens.size() <<
    //	   ", totlen_added + totlen_removed " << totlen_added + totlen_removed
    //	   << ", freq_deltas.size() " << freq_deltas.size() << endl;
    if (++changes_made >= 1000) {
	do_flush();
    }

    RETURN(did);
}

void
QuartzWritableDatabase::do_delete_document(Xapian::docid did)
{
    DEBUGCALL(DB, void, "QuartzWritableDatabase::do_delete_document", did);
    Assert(did != 0);
    Assert(buffered_tables != 0);

    try {
	if (doclens.find(did) != doclens.end()) {
	    // This document was added or modified in the batch currently
	    // being buffered.  This should be unusual, and it's fiddly
	    // to handle, so we just flush and then handle as normal.
	    do_flush_const();
	}

	// Remove the record.
	QuartzRecordManager::delete_record(
		*(buffered_tables->get_record_table()), did);

	// Remove the values
	QuartzValueManager::delete_all_values(
		*(buffered_tables->get_value_table()),
		did);

	// OK, now add entries to remove the postings in the underlying record.
	Xapian::Internal::RefCntPtr<const QuartzWritableDatabase> ptrtothis(this);
	QuartzTermList termlist(ptrtothis,
				database_ro.tables->get_termlist_table(),
				did, database_ro.get_doccount());

	totlen_removed += termlist.get_doclength();

	termlist.next();
	while (!termlist.at_end()) {
	    string tname = termlist.get_termname();
	    QuartzPositionList::delete_positionlist(
		buffered_tables->get_positionlist_table(),
		did, tname);
	    termcount wdf = termlist.get_wdf();

	    map<string, pair<termcount_diff, termcount_diff> >::iterator i;
	    i = freq_deltas.find(tname);
	    if (i == freq_deltas.end()) {
		freq_deltas.insert(make_pair(tname, make_pair(-1, -termcount_diff(wdf))));
	    } else {
		--i->second.first;
		i->second.second -= wdf;
	    }

	    // Remove did from tname's postlist
	    map<string, map<docid, pair<char, termcount> > >::iterator j;
	    j = mod_plists.find(tname);
	    if (j == mod_plists.end()) {
		map<docid, pair<char, termcount> > m;
		j = mod_plists.insert(make_pair(tname, m)).first;
	    }
	    map<docid, pair<char, termcount> >::iterator k;
	    k = j->second.find(did);
	    if (k != j->second.end()) {
		if (k->second.first == 'A') {
		    j->second.erase(k);
		} else {
		    k->second.first = 'D';
		}
	    } else {
		j->second.insert(make_pair(did, make_pair('D', 0u)));
	    }

	    termlist.next();
	}

	// Remove the termlist.
	QuartzTermList::delete_termlist(buffered_tables->get_termlist_table(),
					did);
    } catch (...) {
	// If an error occurs while deleting a document, or doing any other
	// transaction, the modifications so far must be cleared before
	// returning control to the user - otherwise partial modifications will
	// persist in memory, and eventually get written to disk.
	buffered_tables->cancel();
	totlen_added = 0;
	totlen_removed = 0;
	freq_deltas.clear();
	doclens.clear();
	mod_plists.clear();
	changes_made = 0;
	throw;
    }

    // FIXME: this should be configurable and/or different - see above.
    if (++changes_made >= 1000) {
	do_flush();
    }
}

void
QuartzWritableDatabase::do_replace_document(Xapian::docid did,
				    const Xapian::Document & document)
{
    DEBUGCALL(DB, void, "QuartzWritableDatabase::do_replace_document", did << ", " << document);
    Assert(did != 0);
    Assert(buffered_tables != 0);

    try {
	if (doclens.find(did) != doclens.end()) {
	    // This document was added or modified in the batch currently
	    // being buffered.  This should be unusual, and it's fiddly
	    // to handle, so we just flush and then handle as normal.
	    do_flush_const();
	}

	// OK, now add entries to remove the postings in the underlying record.
	Xapian::Internal::RefCntPtr<const QuartzWritableDatabase> ptrtothis(this);
	QuartzTermList termlist(ptrtothis,
				database_ro.tables->get_termlist_table(),
				did, database_ro.get_doccount());

	termlist.next();
	while (!termlist.at_end()) {
	    string tname = termlist.get_termname();
	    termcount wdf = termlist.get_wdf();

	    map<string, pair<termcount_diff, termcount_diff> >::iterator i;
	    i = freq_deltas.find(tname);
	    if (i == freq_deltas.end()) {
		freq_deltas.insert(make_pair(tname, make_pair(-1, -termcount_diff(wdf))));
	    } else {
		--i->second.first;
		i->second.second -= wdf;
	    }

	    // Remove did from tname's postlist
	    map<string, map<docid, pair<char, termcount> > >::iterator j;
	    j = mod_plists.find(tname);
	    if (j == mod_plists.end()) {
		map<docid, pair<char, termcount> > m;
		j = mod_plists.insert(make_pair(tname, m)).first;
	    }
	    map<docid, pair<char, termcount> >::iterator k;
	    k = j->second.find(did);
	    if (k != j->second.end()) {
		if (k->second.first == 'A') {
		    j->second.erase(k);
		} else {
		    k->second.first = 'D';
		}
	    } else {
		j->second.insert(make_pair(did, make_pair('D', 0u)));
	    }

	    termlist.next();
	}

	totlen_removed += termlist.get_doclength();

	// Replace the record
	QuartzRecordManager::replace_record(
		*(buffered_tables->get_record_table()),
		document.get_data(),
		did);

	// FIXME: we read the values delete them and then replace in case
	// they come from where they're going!  Better to ask Document
	// nicely and shortcut in this case!
	{
	    list<pair<string, Xapian::valueno> > tmp;
	    Xapian::ValueIterator value = document.values_begin();
	    Xapian::ValueIterator value_end = document.values_end();
	    for ( ; value != value_end; ++value) {
		tmp.push_back(make_pair(*value, value.get_valueno()));
	    }
	//	QuartzValueManager::add_value(
	//	    *(buffered_tables->get_value_table()),
	//	    *value, did, value.get_valueno());
	
	    // Replace the values.
	    QuartzValueManager::delete_all_values(
		    *(buffered_tables->get_value_table()),
		    did);

	    // Set the values.
	    list<pair<string, Xapian::valueno> >::const_iterator i;
	    for (i = tmp.begin(); i != tmp.end(); ++i) {
		QuartzValueManager::add_value(
		    *(buffered_tables->get_value_table()),
		    i->first, did, i->second);
	    }
	}

	quartz_doclen_t new_doclen = 0;
	{
	    Xapian::TermIterator term = document.termlist_begin();
	    Xapian::TermIterator term_end = document.termlist_end();
	    for ( ; term != term_end; ++term) {
		// Calculate the new document length
		new_doclen += term.get_wdf();
		string tname = *term;
		termcount wdf = term.get_wdf();

		map<string, pair<termcount_diff, termcount_diff> >::iterator i;
		i = freq_deltas.find(tname);
		if (i == freq_deltas.end()) {
		    freq_deltas.insert(make_pair(tname, make_pair(1, termcount_diff(wdf))));
		} else {
		    ++i->second.first;
		    i->second.second += wdf;
		}

		// Add did to tname's postlist
		map<string, map<docid, pair<char, termcount> > >::iterator j;
		j = mod_plists.find(tname);
		if (j == mod_plists.end()) {
		    map<docid, pair<char, termcount> > m;
		    j = mod_plists.insert(make_pair(tname, m)).first;
		}
		map<docid, pair<char, termcount> >::iterator k;
		k = j->second.find(did);
		if (k != j->second.end()) {
		    if (k->second.first == 'D') k->second.first = 'M';
		    k->second.second = wdf;
		} else {
		    j->second.insert(make_pair(did, make_pair('A', wdf)));
		}

		// FIXME : this might not work if we replace a positionlist
		// with itself (e.g. if a document is replaced with itself
		// with just the values changed)
		QuartzPositionList::delete_positionlist(
		    buffered_tables->get_positionlist_table(),
		    did, tname);
		if (term.positionlist_begin() != term.positionlist_end()) {
		    QuartzPositionList::set_positionlist(
			buffered_tables->get_positionlist_table(), did, tname,
			term.positionlist_begin(), term.positionlist_end());
		}
	    }
	}

	// Set the termlist
	QuartzTermList::set_entries(buffered_tables->get_termlist_table(), did,
		document.termlist_begin(), document.termlist_end(),
		new_doclen, false);

	// Set the new document length
	doclens.insert(make_pair(did, new_doclen));
	totlen_added += new_doclen;
    } catch (...) {
	// If an error occurs while replacing a document, or doing any other
	// transaction, the modifications so far must be cleared before
	// returning control to the user - otherwise partial modifications will
	// persist in memory, and eventually get written to disk.
	buffered_tables->cancel();
	totlen_added = 0;
	totlen_removed = 0;
	freq_deltas.clear();
	doclens.clear();
	mod_plists.clear();
	changes_made = 0;
	throw;
    }

    // FIXME: this should be configurable and/or different - see above.
    if (++changes_made >= 1000) {
	do_flush();
    }
}

Xapian::doccount
QuartzWritableDatabase::get_doccount() const
{
    DEBUGCALL(DB, Xapian::doccount, "QuartzWritableDatabase::get_doccount", "");
    RETURN(database_ro.get_doccount());
}

Xapian::doclength
QuartzWritableDatabase::get_avlength() const
{
    DEBUGCALL(DB, Xapian::doclength, "QuartzWritableDatabase::get_avlength", "");
    // Need to flush (or adjust return value, or at least flush the changes to
    // the total length).
    do_flush_const();
    RETURN(database_ro.get_avlength());
}

Xapian::doclength
QuartzWritableDatabase::get_doclength(Xapian::docid did) const
{
    DEBUGCALL(DB, Xapian::doclength, "QuartzWritableDatabase::get_doclength", did);
    map<docid, termcount>::const_iterator i = doclens.find(did);
    if (i != doclens.end()) RETURN(i->second);

    RETURN(database_ro.get_doclength(did));
}

Xapian::doccount
QuartzWritableDatabase::get_termfreq(const string & tname) const
{
    DEBUGCALL(DB, Xapian::doccount, "QuartzWritableDatabase::get_termfreq", tname);
    Xapian::doccount termfreq = database_ro.get_termfreq(tname);
    map<string, pair<termcount_diff, termcount_diff> >::const_iterator i;
    i = freq_deltas.find(tname);
    if (i != freq_deltas.end()) termfreq += i->second.first;
    RETURN(termfreq);
}

Xapian::termcount
QuartzWritableDatabase::get_collection_freq(const string & tname) const
{
    DEBUGCALL(DB, Xapian::termcount, "QuartzWritableDatabase::get_collection_freq", tname);
    Xapian::termcount collfreq = database_ro.get_collection_freq(tname);

    map<string, pair<termcount_diff, termcount_diff> >::const_iterator i;
    i = freq_deltas.find(tname);
    if (i != freq_deltas.end()) collfreq += i->second.second;

    RETURN(collfreq);
}

bool
QuartzWritableDatabase::term_exists(const string & tname) const
{
    DEBUGCALL(DB, bool, "QuartzWritableDatabase::term_exists", tname);
    RETURN(get_termfreq(tname) != 0);
}


LeafPostList *
QuartzWritableDatabase::do_open_post_list(const string& tname) const
{
    DEBUGCALL(DB, LeafPostList *, "QuartzWritableDatabase::do_open_post_list", tname);

    // Need to flush iff we've got buffered changes to this term's postlist.
    map<string, map<docid, pair<char, termcount> > >::const_iterator j;
    j = mod_plists.find(tname);
    if (j != mod_plists.end()) do_flush_const();

    Xapian::Internal::RefCntPtr<const QuartzWritableDatabase> ptrtothis(this);

    RETURN(database_ro.open_post_list_internal(tname, ptrtothis));
}

LeafTermList *
QuartzWritableDatabase::open_term_list(Xapian::docid did) const
{
    DEBUGCALL(DB, LeafTermList *, "QuartzWritableDatabase::open_term_list",
	      did);

    // Only need to flush if document #did has been modified.
    if (doclens.find(did) != doclens.end()) do_flush_const();

    Xapian::Internal::RefCntPtr<const QuartzWritableDatabase> ptrtothis(this);
    RETURN(database_ro.open_term_list_internal(did, ptrtothis));
}

Xapian::Document::Internal *
QuartzWritableDatabase::open_document(Xapian::docid did, bool lazy) const
{
    DEBUGCALL(DB, Xapian::Document::Internal *, "QuartzWritableDatabase::open_document",
	      did << ", " << lazy);
    Assert(did != 0);

    // Only need to flush if document #did has been modified.
    if (doclens.find(did) != doclens.end()) do_flush_const();

    Xapian::Internal::RefCntPtr<const QuartzWritableDatabase> ptrtothis(this);
    RETURN(new QuartzDocument(ptrtothis,
			      buffered_tables->get_value_table(),
			      buffered_tables->get_record_table(),
			      did, lazy));
}

PositionList *
QuartzWritableDatabase::open_position_list(Xapian::docid did,
				   const string & tname) const
{
    Assert(did != 0);

    AutoPtr<QuartzPositionList> poslist(new QuartzPositionList());
    poslist->read_data(buffered_tables->get_positionlist_table(), did, tname);
    if (poslist->get_size() == 0) {
	// Check that term / document combination exists.
	Xapian::Internal::RefCntPtr<const QuartzWritableDatabase> ptrtothis(this);
	// If the doc doesn't exist, this will throw Xapian::DocNotFoundError:
	AutoPtr<LeafTermList> ltl(database_ro.open_term_list_internal(did, ptrtothis));
	ltl->skip_to(tname);
	if (ltl->at_end() || ltl->get_termname() != tname)
	    throw Xapian::RangeError("Can't open position list: requested term is not present in document.");
    }

    return poslist.release();
}

void
QuartzWritableDatabase::do_reopen()
{
    DEBUGCALL(DB, void, "QuartzWritableDatabase::do_reopen", "");
    // Nothing to do - we're the only writer, and so must be up to date.
}

TermList *
QuartzWritableDatabase::open_allterms() const
{
    DEBUGCALL(DB, TermList *, "QuartzWritableDatabase::open_allterms", "");
    // Terms may have been added or removed, so we need to flush.
    do_flush_const();
    QuartzTable *t = buffered_tables->get_postlist_table();
    AutoPtr<QuartzCursor> pl_cursor(t->cursor_get());
    RETURN(new QuartzAllTermsList(Xapian::Internal::RefCntPtr<const QuartzWritableDatabase>(this),
				  pl_cursor, t->get_entry_count()));
}
