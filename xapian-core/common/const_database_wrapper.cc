/** @file const_database_wrapper.cc
 * @brief Wrapper which exposes only the const methods of database internals.
 */
/* Copyright 2009 Lemur Consulting Ltd
 * Copyright 2009 Olly Betts
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301
 * USA
 */

#include <config.h>

#include "const_database_wrapper.h"

#include "xapian/error.h"

void
ConstDatabaseWrapper::nonconst_access() const
{
    throw Xapian::UnimplementedError("Access to non-const method of database "
				     "not supported in this context");
}

ConstDatabaseWrapper::ConstDatabaseWrapper(
	Xapian::Internal::RefCntPtr<const Xapian::Database::Internal> realdb_)
	: realdb(realdb_)
{
}

Xapian::doccount
ConstDatabaseWrapper::get_doccount() const
{
    return realdb->get_doccount();
}

Xapian::docid
ConstDatabaseWrapper::get_lastdocid() const
{
    return realdb->get_lastdocid();
}

totlen_t
ConstDatabaseWrapper::get_total_length() const
{
    return realdb->get_total_length();
}

Xapian::doclength
ConstDatabaseWrapper::get_avlength() const
{
    return realdb->get_avlength();
}

Xapian::termcount
ConstDatabaseWrapper::get_doclength(Xapian::docid did) const
{
    return realdb->get_doclength(did);
}

Xapian::doccount
ConstDatabaseWrapper::get_termfreq(const string & tname) const
{
    return realdb->get_termfreq(tname);
}

Xapian::termcount
ConstDatabaseWrapper::get_collection_freq(const string & tname) const
{
    return realdb->get_collection_freq(tname);
}

Xapian::doccount
ConstDatabaseWrapper::get_value_freq(Xapian::valueno slot) const
{
    return realdb->get_value_freq(slot);
}

std::string
ConstDatabaseWrapper::get_value_lower_bound(Xapian::valueno slot) const
{
    return realdb->get_value_lower_bound(slot);
}

std::string
ConstDatabaseWrapper::get_value_upper_bound(Xapian::valueno slot) const
{
    return realdb->get_value_upper_bound(slot);
}

bool
ConstDatabaseWrapper::term_exists(const string & tname) const
{
    return realdb->term_exists(tname);
}

bool
ConstDatabaseWrapper::has_positions() const
{
    return realdb->has_positions();
}

LeafPostList *
ConstDatabaseWrapper::open_post_list(const string & tname) const
{
    return realdb->open_post_list(tname);
}

ValueList *
ConstDatabaseWrapper::open_value_list(Xapian::valueno slot) const
{
    return realdb->open_value_list(slot);
}

TermList *
ConstDatabaseWrapper::open_term_list(Xapian::docid did) const
{
    return realdb->open_term_list(did);
}

TermList *
ConstDatabaseWrapper::open_allterms(const string & prefix) const
{
    return realdb->open_allterms(prefix);
}

PositionList *
ConstDatabaseWrapper::open_position_list(Xapian::docid did,
					 const string & tname) const
{
    return realdb->open_position_list(did, tname);
}

Xapian::Document::Internal *
ConstDatabaseWrapper::open_document(Xapian::docid did, bool lazy) const
{
    return realdb->open_document(did, lazy);
}

TermList *
ConstDatabaseWrapper::open_spelling_termlist(const string & word) const
{
    return realdb->open_spelling_termlist(word);
}

TermList *
ConstDatabaseWrapper::open_spelling_wordlist() const
{
    return realdb->open_spelling_wordlist();
}

Xapian::doccount
ConstDatabaseWrapper::get_spelling_frequency(const string & word) const
{
    return realdb->get_spelling_frequency(word);
}

TermList *
ConstDatabaseWrapper::open_synonym_termlist(const string & term) const
{
    return realdb->open_synonym_termlist(term);
}

TermList *
ConstDatabaseWrapper::open_synonym_keylist(const string & prefix) const
{
    return realdb->open_synonym_keylist(prefix);
}

string
ConstDatabaseWrapper::get_metadata(const string & key) const
{
    return realdb->get_metadata(key);
}

TermList *
ConstDatabaseWrapper::open_metadata_keylist(const std::string &prefix) const
{
    return realdb->open_metadata_keylist(prefix);
}

void
ConstDatabaseWrapper::request_document(Xapian::docid did) const
{
    return realdb->request_document(did);
}

Xapian::Document::Internal *
ConstDatabaseWrapper::collect_document(Xapian::docid did) const
{
    return realdb->collect_document(did);
}

string
ConstDatabaseWrapper::get_revision_info() const
{
    return realdb->get_revision_info();
}

string
ConstDatabaseWrapper::get_uuid() const
{
    return realdb->get_uuid();
}

void
ConstDatabaseWrapper::invalidate_doc_object(Xapian::Document::Internal * obj) const
{
    return realdb->invalidate_doc_object(obj);
}

// Non-const methods: these raise Xapian::InvalidOperationError
void
ConstDatabaseWrapper::add_spelling(const string &, Xapian::termcount) const
{
    nonconst_access();
}

void
ConstDatabaseWrapper::remove_spelling(const string &, Xapian::termcount) const
{
    nonconst_access();
}

void
ConstDatabaseWrapper::add_synonym(const string &, const string &) const
{
    nonconst_access();
}

void
ConstDatabaseWrapper::remove_synonym(const string &, const string &) const
{
    nonconst_access();
}

void
ConstDatabaseWrapper::clear_synonyms(const string &) const
{
    nonconst_access();
}

void
ConstDatabaseWrapper::set_metadata(const string &, const string &)
{
    nonconst_access();
}

void
ConstDatabaseWrapper::reopen()
{
    nonconst_access();
}

void
ConstDatabaseWrapper::close()
{
    nonconst_access();
}

void
ConstDatabaseWrapper::commit()
{
    nonconst_access();
}

void
ConstDatabaseWrapper::cancel()
{
    nonconst_access();
}

void
ConstDatabaseWrapper::begin_transaction(bool)
{
    nonconst_access();
}

void
ConstDatabaseWrapper::commit_transaction()
{
    nonconst_access();
}

void
ConstDatabaseWrapper::cancel_transaction()
{
    nonconst_access();
}

Xapian::docid
ConstDatabaseWrapper::add_document(const Xapian::Document &)
{
    nonconst_access();
    return 0;
}

void
ConstDatabaseWrapper::delete_document(Xapian::docid)
{
    nonconst_access();
}

void
ConstDatabaseWrapper::delete_document(const string &)
{
    nonconst_access();
}

void
ConstDatabaseWrapper::replace_document(Xapian::docid, const Xapian::Document &)
{
    nonconst_access();
}

Xapian::docid
ConstDatabaseWrapper::replace_document(const string &,
				       const Xapian::Document &)
{
    nonconst_access();
    return 0;
}

void
ConstDatabaseWrapper::write_changesets_to_fd(int, const std::string &, bool,
					     Xapian::ReplicationInfo *)
{
    nonconst_access();
}

RemoteDatabase *
ConstDatabaseWrapper::as_remotedatabase()
{
    nonconst_access();
    return NULL;
}
