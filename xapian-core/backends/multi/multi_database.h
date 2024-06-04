/** @file
 *  @brief Sharded database backend
 */
/* Copyright 2017,2019,2024 Olly Betts
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

#ifndef XAPIAN_INCLUDED_MULTI_DATABASE_H
#define XAPIAN_INCLUDED_MULTI_DATABASE_H

#include "api/termlist.h"
#include "backends/databaseinternal.h"
#include "backends/valuelist.h"

#include <string_view>

class LeafPostList;
class Matcher;
class ValueStreamDocument;

namespace Xapian {
struct ReplicationInfo;
namespace Internal {
class PostList;
}
}

using Xapian::Internal::PostList;

/// Sharded database backend.
class MultiDatabase : public Xapian::Database::Internal {
    friend class Matcher;
    friend class PostListTree;
    friend class ValueStreamDocument;
    friend class Xapian::Database;

    Xapian::SmallVectorI<Xapian::Database::Internal> shards;

  public:
    explicit MultiDatabase(size_type reserve_size, bool read_only)
	: Xapian::Database::Internal(read_only ?
				     TRANSACTION_READONLY :
				     TRANSACTION_NONE),
	  shards(reserve_size) {}

    size_type size() const;

    void reserve(size_type new_size) { shards.reserve(new_size); }

    void push_back(Xapian::Database::Internal* shard) {
	shards.push_back(shard);
    }

    bool reopen();

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

    void keep_alive();

    TermList* open_spelling_termlist(std::string_view word) const;

    TermList* open_spelling_wordlist() const;

    Xapian::doccount get_spelling_frequency(std::string_view word) const;

    TermList* open_synonym_termlist(std::string_view term) const;

    TermList* open_synonym_keylist(std::string_view prefix) const;

    std::string get_metadata(std::string_view key) const;

    TermList* open_metadata_keylist(std::string_view prefix) const;

    std::string get_uuid() const;

    bool locked() const;

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

    void end_transaction(bool do_commit);

    Xapian::docid add_document(const Xapian::Document& doc);

    void delete_document(Xapian::docid did);

    void delete_document(std::string_view term);

    void replace_document(Xapian::docid did, const Xapian::Document& doc);

    Xapian::docid replace_document(std::string_view term,
				   const Xapian::Document& doc);

    void request_document(Xapian::docid did) const;

    void add_spelling(std::string_view word, Xapian::termcount freqinc) const;

    Xapian::termcount remove_spelling(std::string_view word,
				      Xapian::termcount freqdec) const;

    void add_synonym(std::string_view term, std::string_view synonym) const;

    void remove_synonym(std::string_view term,
			std::string_view synonym) const;

    void clear_synonyms(std::string_view term) const;

    void set_metadata(std::string_view key, std::string_view value);

    std::string reconstruct_text(Xapian::docid did,
				 size_t length,
				 std::string_view prefix,
				 Xapian::termpos start_pos,
				 Xapian::termpos end_pos) const;

    std::string get_description() const;
};

#endif // XAPIAN_INCLUDED_MULTI_DATABASE_H
