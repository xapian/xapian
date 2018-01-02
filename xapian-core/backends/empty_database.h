/** @file emptydatabase.h
 * @brief Empty database internals
 */
/* Copyright 2017 Olly Betts
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

/// Empty database internals.
class EmptyDatabase : public Xapian::Database::Internal {
  public:
    EmptyDatabase() : Xapian::Database::Internal(TRANSACTION_NONE) {}

    size_type size() const;

    void close();

    PostList* open_post_list(const std::string& term) const;

    LeafPostList* open_leaf_post_list(const std::string& term,
				      bool need_pos) const;

    TermList* open_term_list(Xapian::docid did) const;

    TermList* open_term_list_direct(Xapian::docid did) const;

    TermList* open_allterms(const std::string& prefix) const;

    bool has_positions() const;

    PositionList* open_position_list(Xapian::docid did,
				     const std::string& term) const;
    Xapian::doccount get_doccount() const;

    Xapian::docid get_lastdocid() const;

    Xapian::totallength get_total_length() const;

    void get_freqs(const std::string& term,
		   Xapian::doccount* tf_ptr,
		   Xapian::termcount* cf_ptr) const;

    Xapian::doccount get_value_freq(Xapian::valueno slot) const;

    std::string get_value_lower_bound(Xapian::valueno slot) const;

    std::string get_value_upper_bound(Xapian::valueno slot) const;

    Xapian::termcount get_doclength_lower_bound() const;

    Xapian::termcount get_doclength_upper_bound() const;

    Xapian::termcount get_wdf_upper_bound(const std::string& term) const;

    ValueList* open_value_list(Xapian::valueno slot) const;

    Xapian::termcount get_doclength(Xapian::docid did) const;

    Xapian::termcount get_unique_terms(Xapian::docid did) const;

    Xapian::Document::Internal* open_document(Xapian::docid did,
					      bool lazy) const;

    bool term_exists(const std::string& term) const;

    TermList* open_spelling_termlist(const std::string& word) const;

    TermList* open_spelling_wordlist() const;

    Xapian::doccount get_spelling_frequency(const std::string& word) const;

    TermList* open_synonym_termlist(const std::string& term) const;

    TermList* open_synonym_keylist(const std::string& prefix) const;

    std::string get_metadata(const std::string& key) const;

    TermList* open_metadata_keylist(const std::string& prefix) const;

    void write_changesets_to_fd(int fd,
				const std::string& start_revision,
				bool need_whole_db,
				Xapian::ReplicationInfo* info);

    void invalidate_doc_object(Xapian::Document::Internal* obj) const;

    Xapian::rev get_revision() const;

    int get_backend_info(std::string* path) const;

    void commit();

    void cancel();

    void begin_transaction(bool flushed);

    void end_transaction_(bool do_commit);

    Xapian::docid add_document(const Xapian::Document& doc);

    void delete_document(Xapian::docid did);

    void delete_document(const std::string& term);

    void replace_document(Xapian::docid did, const Xapian::Document& doc);

    Xapian::docid replace_document(const std::string& term,
				   const Xapian::Document& doc);

    void add_spelling(const std::string& word, Xapian::termcount freqinc) const;

    Xapian::termcount remove_spelling(const std::string& word,
				      Xapian::termcount freqdec) const;

    void add_synonym(const std::string& term, const std::string& synonym) const;

    void remove_synonym(const std::string& term,
			const std::string& synonym) const;

    void clear_synonyms(const std::string& term) const;

    void set_metadata(const std::string& key, const std::string& value);

    std::string get_description() const;
};

#endif // XAPIAN_INCLUDED_EMPTY_DATABASE_H
