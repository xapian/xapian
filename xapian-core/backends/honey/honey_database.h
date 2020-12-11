/** @file
 * @brief Database using honey backend
 */
/* Copyright 2004,2006,2007,2008,2009,2011,2014,2015,2016,2017 Olly Betts
 * Copyright 2007,2008 Lemur Consulting Ltd
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

#ifndef XAPIAN_INCLUDED_HONEY_DATABASE_H
#define XAPIAN_INCLUDED_HONEY_DATABASE_H

#include "backends/databaseinternal.h"

#include "honey_alldocspostlist.h"
#include "honey_docdata.h"
#include "honey_postlisttable.h"
#include "honey_positionlist.h"
#include "honey_spelling.h"
#include "honey_synonym.h"
#include "honey_termlisttable.h"
#include "honey_values.h"
#include "honey_version.h"
#include "xapian/compactor.h"

class HoneyAllTermsList;
class HoneyCursor;
class HoneyPostList;
class HoneySynonymTermList;
class HoneySpellingWordsList;
class HoneyTermList;

/// Database using honey backend.
class HoneyDatabase : public Xapian::Database::Internal {
    friend class HoneyAllTermsList;
    friend class HoneyPosPostList;
    friend class HoneyPostList;
    friend class HoneySpellingWordsList;
    friend class HoneySynonymTermList;
    friend class HoneyTermList;

    /// Don't allow assignment.
    HoneyDatabase& operator=(const HoneyDatabase&) = delete;

    /// Don't allow copying.
    HoneyDatabase(const HoneyDatabase&) = delete;

    /// Path of the directory.
    std::string path;

    /// Version file ("iamhoney").
    HoneyVersion version_file;

    HoneyDocDataTable docdata_table;

    HoneyPostListTable postlist_table;

    HoneyPositionTable position_table;

    mutable HoneySpellingTable spelling_table;

    HoneySynonymTable synonym_table;

    HoneyTermListTable termlist_table;

    HoneyValueManager value_manager;

    mutable Honey::DocLenChunkReader doclen_chunk_reader;

    mutable HoneyCursor* doclen_cursor = NULL;

    [[noreturn]]
    void throw_termlist_table_close_exception() const;

  public:
    explicit
    HoneyDatabase(const std::string& path_, int flags = Xapian::DB_READONLY_);

    explicit
    HoneyDatabase(int fd, int flags = Xapian::DB_READONLY_);

    ~HoneyDatabase();

    void readahead_for_query(const Xapian::Query& query) const;

    Xapian::doccount get_doccount() const;

    /** Return the last used document id of this (sub) database. */
    Xapian::docid get_lastdocid() const;

    /** Return the total length of all documents in this database. */
    Xapian::totallength get_total_length() const;

    Xapian::termcount get_doclength(Xapian::docid did) const;

    /** Get the number of unique terms in document.
     *
     *  @param did  The document id of the document to return this value for.
     */
    Xapian::termcount get_unique_terms(Xapian::docid did) const;

    // Return the max_wdf in the document
    Xapian::termcount get_wdfdocmax(Xapian::docid did) const;

    /** Returns frequencies for a term.
     *
     *  @param term		The term to get frequencies for
     *  @param termfreq_ptr	Point to return number of docs indexed by @a
     *				term (or NULL not to return)
     *  @param collfreq_ptr	Point to return number of occurrences of @a
     *				term in the database (or NULL not to return)
     */
    void get_freqs(const std::string& term,
		   Xapian::doccount* termfreq_ptr,
		   Xapian::termcount* collfreq_ptr) const;

    /** Return the frequency of a given value slot.
     *
     *  This is the number of documents which have a (non-empty) value
     *  stored in the slot.
     *
     *  @param slot The value slot to examine.
     *
     *  @exception UnimplementedError The frequency of the value isn't
     *  available for this database type.
     */
    Xapian::doccount get_value_freq(Xapian::valueno slot) const;

    /** Get a lower bound on the values stored in the given value slot.
     *
     *  If the lower bound isn't available for the given database type,
     *  this will return the lowest possible bound - the empty string.
     *
     *  @param slot The value slot to examine.
     */
    std::string get_value_lower_bound(Xapian::valueno slot) const;

    /** Get an upper bound on the values stored in the given value slot.
     *
     *  @param slot The value slot to examine.
     *
     *  @exception UnimplementedError The upper bound of the values isn't
     *  available for this database type.
     */
    std::string get_value_upper_bound(Xapian::valueno slot) const;

    /// Get a lower bound on the length of a document in this DB.
    Xapian::termcount get_doclength_lower_bound() const;

    /// Get an upper bound on the length of a document in this DB.
    Xapian::termcount get_doclength_upper_bound() const;

    /// Get an upper bound on the wdf of term @a term.
    Xapian::termcount get_wdf_upper_bound(const std::string& term) const;

    /// Get a lower bound on the unique terms size of a document in this DB.
    Xapian::termcount get_unique_terms_lower_bound() const;

    /// Get an upper bound on the unique terms size of a document in this DB.
    Xapian::termcount get_unique_terms_upper_bound() const;

    bool term_exists(const std::string& term) const;

    /** Check whether this database contains any positional information. */
    bool has_positions() const;

    PostList* open_post_list(const std::string& term) const;

    LeafPostList* open_leaf_post_list(const std::string& term,
				      bool need_read_pos) const;

    /** Open a value stream.
     *
     *  This returns the value in a particular slot for each document.
     *
     *  @param slot	The value slot.
     *
     *  @return	Pointer to a new ValueList object which should be deleted by
     *		the caller once it is no longer needed.
     */
    ValueList* open_value_list(Xapian::valueno slot) const;

    TermList* open_term_list(Xapian::docid did) const;

    /** Like open_term_list() but without MultiTermList wrapper.
     *
     *  MultiDatabase::open_term_list() wraps the returns TermList in a
     *  MultiTermList, but we don't want that for query expansion.
     */
    TermList* open_term_list_direct(Xapian::docid did) const;

    TermList* open_allterms(const std::string& prefix) const;

    PositionList* open_position_list(Xapian::docid did,
				     const std::string& term) const;

    /** Open a handle on a document.
     *
     *  The returned handle provides access to document data and document
     *  values.
     *
     *  @param did	The document id to open.
     *
     *  @param lazy	If true, there's no need to check that this document
     *			actually exists (only a hint - the backend may still
     *			check).  Used to avoid unnecessary work when we already
     *			know that the requested document exists.
     *
     *  @return		A new document object, owned by the caller.
     */
    Xapian::Document::Internal* open_document(Xapian::docid did,
					      bool lazy) const;

    /** Create a termlist tree from trigrams of @a word.
     *
     *  You can assume word.size() > 1.
     *
     *  If there are no trigrams, returns NULL.
     */
    TermList* open_spelling_termlist(const std::string& word) const;

    /** Return a termlist which returns the words which are spelling
     *  correction targets.
     *
     *  If there are no spelling correction targets, returns NULL.
     */
    TermList* open_spelling_wordlist() const;

    /** Return the number of times @a word was added as a spelling. */
    Xapian::doccount get_spelling_frequency(const std::string& word) const;

    /** Add a word to the spelling dictionary.
     *
     *  If the word is already present, its frequency is increased.
     *
     *  @param word	The word to add.
     *  @param freqinc	How much to increase its frequency by.
     */
    void add_spelling(const std::string& word,
		      Xapian::termcount freqinc) const;

    /** Remove a word from the spelling dictionary.
     *
     *  The word's frequency is decreased, and if would become zero or less
     *  then the word is removed completely.
     *
     *  @param word	The word to remove.
     *  @param freqdec	How much to decrease its frequency by.
     *
     *  @return Any freqdec not "used up".
     */
    Xapian::termcount remove_spelling(const std::string& word,
				      Xapian::termcount freqdec) const;

    /** Open a termlist returning synonyms for a term.
     *
     *  If @a term has no synonyms, returns NULL.
     */
    TermList* open_synonym_termlist(const std::string& term) const;

    /** Open a termlist returning each term which has synonyms.
     *
     *  @param prefix   If non-empty, only terms with this prefix are
     *		    returned.
     */
    TermList* open_synonym_keylist(const std::string& prefix) const;

    /** Add a synonym for a term.
     *
     *  If @a synonym is already a synonym for @a term, then no action is
     *  taken.
     */
    void add_synonym(const std::string& term,
		     const std::string& synonym) const;

    /** Remove a synonym for a term.
     *
     *  If @a synonym isn't a synonym for @a term, then no action is taken.
     */
    void remove_synonym(const std::string& term,
			const std::string& synonym) const;

    /** Clear all synonyms for a term.
     *
     *  If @a term has no synonyms, no action is taken.
     */
    void clear_synonyms(const std::string& term) const;

    /** Get the metadata associated with a given key.
     *
     *  See Database::get_metadata() for more information.
     */
    std::string get_metadata(const std::string& key) const;

    /** Open a termlist returning each metadata key.
     *
     *  Only metadata keys which are associated with a non-empty value will
     *  be returned.
     *
     *  @param prefix   If non-empty, only keys with this prefix are returned.
     */
    TermList* open_metadata_keylist(const std::string& prefix) const;

    /** Set the metadata associated with a given key.
     *
     *  See WritableDatabase::set_metadata() for more information.
     */
    void set_metadata(const std::string& key, const std::string& value);

    /** Reopen the database to the latest available revision.
     *
     *  Database backends which don't support simultaneous update and
     *  reading probably don't need to do anything here.
     */
    bool reopen();

    /** Close the database */
    void close();

    /** Request a document.
     *
     *  This tells the database that we're going to want a particular
     *  document soon.  It's just a hint which the backend may ignore,
     *  but for glass it issues a preread hint on the file with the
     *  document data in, and for the remote backend it might cause
     *  the document to be fetched asynchronously (this isn't currently
     *  implemented though).
     *
     *  It can be called for multiple documents in turn, and a common usage
     *  pattern would be to iterate over an MSet and request the documents,
     *  then iterate over it again to actually get and display them.
     *
     *  The default implementation is a no-op.
     */
    void request_document(Xapian::docid did) const;

    /// Get the current revision of the database.
    Xapian::rev get_revision() const;

    /** Get a UUID for the database.
     *
     *  The UUID will persist for the lifetime of the database.
     *
     *  Replicas (eg, made with the replication protocol, or by copying all
     *  the database files) will have the same UUID.  However, copies (made
     *  with copydatabase, or xapian-compact) will have different UUIDs.
     *
     *  If the backend does not support UUIDs the empty string is returned.
     */
    std::string get_uuid() const;

    /** Get backend information about this database.
     *
     *  @param path	If non-NULL, and set the pointed to string to the file
     *			path of this database (or if to some string describing
     *			the database in a backend-specified format if "path"
     *			isn't a concept which  make sense).
     *
     *  @return	A constant indicating the backend type.
     */
    int get_backend_info(std::string* path) const;

    /** Find lowest and highest docids actually in use.
     *
     *  Only used by compaction, so only needs to be implemented by
     *  backends which support compaction.
     */
    void get_used_docid_range(Xapian::docid& first, Xapian::docid& last) const;

    static
    void compact(Xapian::Compactor* compactor,
		 const char* destdir,
		 int fd,
		 int source_backend,
		 const std::vector<const Xapian::Database::Internal*>& sources,
		 const std::vector<Xapian::docid>& offset,
		 Xapian::Compactor::compaction_level compaction,
		 unsigned flags,
		 Xapian::docid last_docid);

    bool has_uncommitted_changes() const {
	return false;
    }

    bool single_file() const {
	return false;
    }

    HoneyCursor* get_postlist_cursor() const {
	return postlist_table.cursor_get();
    }

    /// Return a string describing this object.
    std::string get_description() const;
};

#endif // XAPIAN_INCLUDED_HONEY_DATABASE_H
