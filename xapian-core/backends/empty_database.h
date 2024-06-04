/** @file
 * @brief Empty database internals
 */
/* Copyright 2017,2024 Olly Betts
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#ifndef XAPIAN_INCLUDED_EMPTY_DATABASE_H
#define XAPIAN_INCLUDED_EMPTY_DATABASE_H

#include "backends/databaseinternal.h"

#include <string_view>

/// Empty database internals.
class EmptyDatabase : public Xapian::Database::Internal {
  public:
    EmptyDatabase() : Xapian::Database::Internal(TRANSACTION_NONE) {}

    size_type size() const;

    void close();

    PostList* open_post_list(std::string_view term) const;

    LeafPostList* open_leaf_post_list(std::string_view term,
				      bool need_read_pos) const;

    TermList* open_term_list(Xapian::docid did) const;

    TermList* open_term_list_direct(Xapian::docid did) const;

    TermList* open_allterms(std::string_view prefix) const;

    bool has_positions() const;

    PositionList* open_position_list(Xapian::docid did,
				     std::string_view term) const;
    Xapian::doccount get_doccount() const;

    Xapian::docid get_lastdocid() const;

    Xapian::totallength get_total_length() const;

    void get_freqs(std::string_view term,
		   Xapian::doccount* tf_ptr,
		   Xapian::termcount* cf_ptr) const;

    Xapian::doccount get_value_freq(Xapian::valueno slot) const;

    std::string get_value_lower_bound(Xapian::valueno slot) const;

    std::string get_value_upper_bound(Xapian::valueno slot) const;

    Xapian::termcount get_doclength_lower_bound() const;

    Xapian::termcount get_doclength_upper_bound() const;

    Xapian::termcount get_wdf_upper_bound(std::string_view term) const;

    ValueList* open_value_list(Xapian::valueno slot) const;

    Xapian::termcount get_doclength(Xapian::docid did) const;

    Xapian::termcount get_unique_terms(Xapian::docid did) const;

    Xapian::termcount get_wdfdocmax(Xapian::docid did) const;

    Xapian::Document::Internal* open_document(Xapian::docid did,
					      bool lazy) const;

    bool term_exists(std::string_view term) const;

    TermList* open_spelling_termlist(std::string_view word) const;

    TermList* open_spelling_wordlist() const;

    Xapian::doccount get_spelling_frequency(std::string_view word) const;

    TermList* open_synonym_termlist(std::string_view term) const;

    TermList* open_synonym_keylist(std::string_view prefix) const;

    std::string get_metadata(std::string_view key) const;

    TermList* open_metadata_keylist(std::string_view prefix) const;

    void write_changesets_to_fd(int fd,
				std::string_view start_revision,
				bool need_whole_db,
				Xapian::ReplicationInfo* info);

    void invalidate_doc_object(Xapian::Document::Internal* obj) const;

    Xapian::rev get_revision() const;

    int get_backend_info(std::string* path) const;

    void commit();

    void cancel();

    void begin_transaction(bool flushed);

    // No need to overload end_transaction() - the base class implementation
    // will fail with InvalidOperationError "not in a transaction" which seems
    // totally appropriate.  We overload begin_transaction() because otherwise
    // the transaction would start successfully whereas it's better to fail
    // early.

    Xapian::docid add_document(const Xapian::Document& doc);

    void delete_document(Xapian::docid did);

    void delete_document(std::string_view term);

    void replace_document(Xapian::docid did, const Xapian::Document& doc);

    Xapian::docid replace_document(std::string_view term,
				   const Xapian::Document& doc);

    void add_spelling(std::string_view word, Xapian::termcount freqinc) const;

    Xapian::termcount remove_spelling(std::string_view word,
				      Xapian::termcount freqdec) const;

    void add_synonym(std::string_view term, std::string_view synonym) const;

    void remove_synonym(std::string_view term,
			std::string_view synonym) const;

    void clear_synonyms(std::string_view term) const;

    void set_metadata(std::string_view key, std::string_view value);

    std::string get_description() const;
};

#endif // XAPIAN_INCLUDED_EMPTY_DATABASE_H
