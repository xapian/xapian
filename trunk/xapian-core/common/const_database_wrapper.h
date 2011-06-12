/** @file const_database_wrapper.h
 * @brief Wrapper which exposes only the const methods of database internals.
 */
/* Copyright (C) 2009 Lemur Consulting Ltd
 * Copyright (C) 2009 Olly Betts
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

#ifndef XAPIAN_INCLUDED_CONST_DATABASE_WRAPPER_H
#define XAPIAN_INCLUDED_CONST_DATABASE_WRAPPER_H

#include "xapian/base.h"
#include "database.h"
#include "noreturn.h"

using namespace std;

/** Base class for databases.
 */
class ConstDatabaseWrapper : public Xapian::Database::Internal {
    /// Copies are not allowed.
    ConstDatabaseWrapper(const ConstDatabaseWrapper &);

    /// Assignment is not allowed.
    void operator=(const ConstDatabaseWrapper &);

    /// The const database which this wrapper wraps.
    Xapian::Internal::RefCntPtr<const Xapian::Database::Internal> realdb;

    /// Raise an exception complaining about access to a non-const method.
    XAPIAN_NORETURN(void nonconst_access() const);

  public:
    ConstDatabaseWrapper(Xapian::Internal::RefCntPtr<const Xapian::Database::Internal> realdb_);

    // Const methods: these are proxied to realdb.
    Xapian::doccount get_doccount() const;
    Xapian::docid get_lastdocid() const;
    totlen_t get_total_length() const;
    Xapian::doclength get_avlength() const;
    Xapian::termcount get_doclength(Xapian::docid did) const;
    Xapian::doccount get_termfreq(const string & tname) const;
    Xapian::termcount get_collection_freq(const string & tname) const;
    Xapian::doccount get_value_freq(Xapian::valueno slot) const;
    std::string get_value_lower_bound(Xapian::valueno slot) const;
    std::string get_value_upper_bound(Xapian::valueno slot) const;
    bool term_exists(const string & tname) const;
    bool has_positions() const;
    LeafPostList * open_post_list(const string & tname) const;
    ValueList * open_value_list(Xapian::valueno slot) const;
    TermList * open_term_list(Xapian::docid did) const;
    TermList * open_allterms(const string & prefix) const;
    PositionList * open_position_list(Xapian::docid did,
				      const string & tname) const;
    Xapian::Document::Internal *
	open_document(Xapian::docid did, bool lazy) const;
    TermList * open_spelling_termlist(const string & word) const;
    TermList * open_spelling_wordlist() const;
    Xapian::doccount get_spelling_frequency(const string & word) const;
    TermList * open_synonym_termlist(const string & term) const;
    TermList * open_synonym_keylist(const string & prefix) const;
    string get_metadata(const string & key) const;
    TermList * open_metadata_keylist(const std::string &prefix) const;
    void request_document(Xapian::docid did) const;
    Xapian::Document::Internal * collect_document(Xapian::docid did) const;
    string get_revision_info() const;
    string get_uuid() const;
    void invalidate_doc_object(Xapian::Document::Internal * obj) const;

    // Non-const methods: these raise Xapian::InvalidOperationError
    void add_spelling(const string & word, Xapian::termcount freqinc) const;
    void remove_spelling(const string & word, Xapian::termcount freqdec) const;
    void add_synonym(const string & term, const string & synonym) const;
    void remove_synonym(const string & term, const string & synonym) const;
    void clear_synonyms(const string & term) const;
    void set_metadata(const string &, const string &);
    void reopen();
    void close();
    void commit();
    void cancel();
    void begin_transaction(bool);
    void commit_transaction();
    void cancel_transaction();
    Xapian::docid add_document(const Xapian::Document &);
    void delete_document(Xapian::docid);
    void delete_document(const string &);
    void replace_document(Xapian::docid, const Xapian::Document &);
    Xapian::docid replace_document(const string &, const Xapian::Document &);
    void write_changesets_to_fd(int, const std::string &, bool,
				Xapian::ReplicationInfo *);
    RemoteDatabase * as_remotedatabase();
};

#endif /* XAPIAN_INCLUDED_CONST_DATABASE_WRAPPER_H */
