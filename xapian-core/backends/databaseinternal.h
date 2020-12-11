/** @file
 * @brief Virtual base class for Database internals
 */
/* Copyright 2004,2006,2007,2008,2009,2011,2014,2015,2016,2017,2019 Olly Betts
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

#ifndef XAPIAN_INCLUDED_DATABASEINTERNAL_H
#define XAPIAN_INCLUDED_DATABASEINTERNAL_H

#include "internaltypes.h"

#include <xapian/database.h>
#include <xapian/document.h>
#include <xapian/intrusive_ptr.h>
#include <xapian/positioniterator.h>
#include <xapian/postingiterator.h>
#include <xapian/termiterator.h>
#include <xapian/types.h>
#include <xapian/valueiterator.h>

#include <string>

typedef Xapian::TermIterator::Internal TermList;
typedef Xapian::PositionIterator::Internal PositionList;
typedef Xapian::ValueIterator::Internal ValueList;

class LeafPostList;

namespace Xapian {
namespace Internal {
class PostList;
}
}
using Xapian::Internal::PostList;

namespace Xapian {

class Query;
struct ReplicationInfo;

/// Virtual base class for Database internals
class Database::Internal : public Xapian::Internal::intrusive_base {
    friend class Database;

    /// Don't allow assignment.
    Internal& operator=(const Internal&) = delete;

    /// Don't allow copying.
    Internal(const Internal&) = delete;

    /// The "action required" helper for the dtor_called() helper.
    void dtor_called_();

  protected:
    /// Transaction state enum.
    enum transaction_state {
	TRANSACTION_READONLY = -2, // Not a writable database shard.
	TRANSACTION_UNIMPLEMENTED = -1, // Used by InMemory.
	TRANSACTION_NONE = 0,
	TRANSACTION_UNFLUSHED = 1,
	TRANSACTION_FLUSHED = 2
    };

    /** Only constructable as a base class for derived classes.
     *
     *  @param transaction_support  One of:
     *	* TRANSACTION_READONLY - read-only shard
     *	* TRANSACTION_UNIMPLEMENTED - writable but no transaction support
     *	* TRANSACTION_NONE - writable with transaction support
     */
    Internal(transaction_state transaction_support)
	: state(transaction_support) {}

    /// Current transaction state.
    transaction_state state;

    /// Test if this shard is read-only.
    bool is_read_only() const {
	return state == TRANSACTION_READONLY;
    }

    /// Test if a transaction is currently active.
    bool transaction_active() const { return state > 0; }

    /** Helper to process uncommitted changes when a writable db is destroyed.
     *
     *  The destructor of a derived writable database class needs to call this
     *  method - we can't call it from our own destructor because we need to
     *  be able to call methods in the derived class, but that's no longer
     *  valid by the time our destructor runs, as that happens after the
     *  destructor of the derived class has run.
     *
     *  If a transaction is active, it is cancelled.  Otherwise we attempt to
     *  commit uncommitted changes, but because it is not safe to throw
     *  exceptions from destructors, this method will catch and discard any
     *  exceptions.
     */
    void dtor_called() {
	// Inline the check to exclude no-op cases (read-only and unimplemented).
	if (state >= 0)
	    dtor_called_();
    }

  public:
    /** We have virtual methods and want to be able to delete derived classes
     *  using a pointer to the base class, so we need a virtual destructor.
     */
    virtual ~Internal() {}

    typedef Xapian::doccount size_type;

    virtual size_type size() const;

    virtual void keep_alive();

    virtual void readahead_for_query(const Query& query) const;

    virtual doccount get_doccount() const = 0;

    /** Return the last used document id of this (sub) database. */
    virtual docid get_lastdocid() const = 0;

    /** Return the total length of all documents in this database. */
    virtual totallength get_total_length() const = 0;

    virtual termcount get_doclength(docid did) const = 0;

    /** Get the number of unique terms in document.
     *
     *  @param did  The document id of the document to return this value for.
     */
    virtual termcount get_unique_terms(docid did) const = 0;

    /** Get the max wdf in document.
     *
     *  @param did  The document id of the document to return this value for.
     */
    virtual termcount get_wdfdocmax(docid did) const = 0;

    /** Returns frequencies for a term.
     *
     *  @param term		The term to get frequencies for
     *  @param termfreq_ptr	Point to return number of docs indexed by @a
     *				term (or NULL not to return)
     *  @param collfreq_ptr	Point to return number of occurrences of @a
     *				term in the database (or NULL not to return)
     */
    virtual void get_freqs(const std::string& term,
			   doccount* termfreq_ptr,
			   termcount* collfreq_ptr) const = 0;

    /** Return the frequency of a given value slot.
     *
     *  This is the number of documents which have a (non-empty) value
     *  stored in the slot.
     *
     *  @param slot The value slot to examine.
     */
    virtual doccount get_value_freq(valueno slot) const = 0;

    /** Get a lower bound on the values stored in the given value slot.
     *
     *  If the lower bound isn't available for the given database type,
     *  this will return the lowest possible bound - the empty string.
     *
     *  @param slot The value slot to examine.
     */
    virtual std::string get_value_lower_bound(valueno slot) const = 0;

    /** Get an upper bound on the values stored in the given value slot.
     *
     *  @param slot The value slot to examine.
     */
    virtual std::string get_value_upper_bound(valueno slot) const = 0;

    /// Get a lower bound on the length of a document in this DB.
    virtual termcount get_doclength_lower_bound() const = 0;

    /// Get an upper bound on the length of a document in this DB.
    virtual termcount get_doclength_upper_bound() const = 0;

    /// Get an upper bound on the wdf of term @a term.
    virtual termcount get_wdf_upper_bound(const std::string& term) const = 0;

    /// Get a lower bound on the unique terms size of a document in this DB.
    virtual termcount get_unique_terms_lower_bound() const;

    /// Get an upper bound on the unique terms size of a document in this DB.
    virtual termcount get_unique_terms_upper_bound() const;

    virtual bool term_exists(const std::string& term) const = 0;

    /** Check whether this database contains any positional information. */
    virtual bool has_positions() const = 0;

    /** Return a PostList suitable for use in a PostingIterator. */
    virtual PostList* open_post_list(const std::string& term) const = 0;

    /** Create a LeafPostList for use during a match.
     *
     *  @param term		The term to open a postlist for, or the empty
     *				string to create an all-docs postlist.
     *
     *  @param need_read_pos	Does the postlist need to support
     *				read_position_list()?  Note that
     *				open_position_list() may still be called even
     *				if need_read_pos is false.
     */
    virtual LeafPostList* open_leaf_post_list(const std::string& term,
					      bool need_read_pos) const = 0;

    /** Open a value stream.
     *
     *  This returns the value in a particular slot for each document.
     *
     *  @param slot	The value slot.
     *
     *  @return	Pointer to a new ValueList object which should be deleted by
     *		the caller once it is no longer needed.
     */
    virtual ValueList* open_value_list(valueno slot) const;

    virtual TermList* open_term_list(docid did) const = 0;

    /** Like open_term_list() but without MultiTermList wrapper.
     *
     *  MultiDatabase::open_term_list() wraps the returns TermList in a
     *  MultiTermList, but we don't want that for query expansion.
     */
    virtual TermList* open_term_list_direct(docid did) const = 0;

    virtual TermList* open_allterms(const std::string& prefix) const = 0;

    virtual PositionList* open_position_list(docid did,
					     const std::string& term) const = 0;

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
    virtual Document::Internal* open_document(docid did, bool lazy) const = 0;

    /** Create a termlist tree from trigrams of @a word.
     *
     *  You can assume word.size() > 1.
     *
     *  If there are no trigrams, returns NULL.
     */
    virtual TermList* open_spelling_termlist(const std::string& word) const;

    /** Return a termlist which returns the words which are spelling
     *  correction targets.
     *
     *  If there are no spelling correction targets, returns NULL.
     */
    virtual TermList* open_spelling_wordlist() const;

    /** Return the number of times @a word was added as a spelling. */
    virtual doccount get_spelling_frequency(const std::string& word) const;

    /** Add a word to the spelling dictionary.
     *
     *  If the word is already present, its frequency is increased.
     *
     *  @param word	The word to add.
     *  @param freqinc	How much to increase its frequency by.
     */
    virtual void add_spelling(const std::string& word,
			      termcount freqinc) const;

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
    virtual termcount remove_spelling(const std::string& word,
				      termcount freqdec) const;

    /** Open a termlist returning synonyms for a term.
     *
     *  If @a term has no synonyms, returns NULL.
     */
    virtual TermList* open_synonym_termlist(const std::string& term) const;

    /** Open a termlist returning each term which has synonyms.
     *
     *  @param prefix   If non-empty, only terms with this prefix are
     *		    returned.
     */
    virtual TermList* open_synonym_keylist(const std::string& prefix) const;

    /** Add a synonym for a term.
     *
     *  If @a synonym is already a synonym for @a term, then no action is
     *  taken.
     */
    virtual void add_synonym(const std::string& term,
			     const std::string& synonym) const;

    /** Remove a synonym for a term.
     *
     *  If @a synonym isn't a synonym for @a term, then no action is taken.
     */
    virtual void remove_synonym(const std::string& term,
				const std::string& synonym) const;

    /** Clear all synonyms for a term.
     *
     *  If @a term has no synonyms, no action is taken.
     */
    virtual void clear_synonyms(const std::string& term) const;

    /** Get the metadata associated with a given key.
     *
     *  See Database::get_metadata() for more information.
     */
    virtual std::string get_metadata(const std::string& key) const;

    /** Open a termlist returning each metadata key.
     *
     *  Only metadata keys which are associated with a non-empty value will
     *  be returned.
     *
     *  @param prefix   If non-empty, only keys with this prefix are returned.
     */
    virtual TermList* open_metadata_keylist(const std::string& prefix) const;

    /** Set the metadata associated with a given key.
     *
     *  See WritableDatabase::set_metadata() for more information.
     */
    virtual void set_metadata(const std::string& key, const std::string& value);

    /** Reopen the database to the latest available revision.
     *
     *  Database backends which don't support simultaneous update and
     *  reading probably don't need to do anything here.
     */
    virtual bool reopen();

    /** Close the database */
    virtual void close() = 0;

    /** Commit pending modifications to the database. */
    virtual void commit();

    /** Cancel pending modifications to the database. */
    virtual void cancel();

    /** Begin transaction. */
    virtual void begin_transaction(bool flushed);

    /** End transaction.
     *
     *  @param do_commit	If true, commits the transaction; if false,
     *				cancels the transaction.
     */
    virtual void end_transaction(bool do_commit);

    virtual docid add_document(const Document& document);

    virtual void delete_document(docid did);

    /** Delete any documents indexed by a term from the database. */
    virtual void delete_document(const std::string& unique_term);

    virtual void replace_document(docid did,
				  const Document& document);

    /** Replace any documents matching a term. */
    virtual docid replace_document(const std::string& unique_term,
				   const Document& document);

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
    virtual void request_document(docid did) const;

    /** Write a set of changesets to a file descriptor.
     *
     *  This call may reopen the database, leaving it pointing to a more
     *  recent version of the database.
     */
    virtual void write_changesets_to_fd(int fd,
					const std::string& start_revision,
					bool need_whole_db,
					ReplicationInfo* info);

    /// Get revision number of database (if meaningful).
    virtual Xapian::rev get_revision() const;

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
    virtual std::string get_uuid() const;

    /** Notify the database that document is no longer valid.
     *
     *  This is used to invalidate references to a document kept by a
     *  database for doing lazy updates.  If we moved to using a weak_ptr
     *  instead we wouldn't need a special method for this, but it would
     *  involve a fair bit of reorganising of other parts of the code.
     */
    virtual void invalidate_doc_object(Document::Internal* obj) const;

    /** Get backend information about this database.
     *
     *  @param path	If non-NULL, and set the pointed to string to the file
     *			path of this database (or if to some string describing
     *			the database in a backend-specified format if "path"
     *			isn't a concept which  make sense).
     *
     *  @return	A constant indicating the backend type.
     */
    virtual int get_backend_info(std::string* path) const = 0;

    /** Find lowest and highest docids actually in use.
     *
     *  Only used by compaction, so only needs to be implemented by
     *  backends which support compaction.
     */
    virtual void get_used_docid_range(docid& first,
				      docid& last) const;

    /** Return true if the database is open for writing.
     *
     *  If this is a WritableDatabase, always returns true.
     *
     *  For a Database, test if there's a writer holding the lock (or if
     *  we can't test for a lock without taking it on the current platform,
     *  throw Xapian::UnimplementedError).
     */
    virtual bool locked() const;

    /** Lock a read-only database for writing or unlock a writable database.
     *
     *  This is the internal method behind Database::lock() and
     *  Database::unlock().
     *
     *  In the unlocking case, the writable database is closed.  In the
     *  locking case, the read-only database is left open.
     *
     *  @param flags  Xapian::DB_READONLY_ to unlock, otherwise the flags
     *		      to use when opening from writing.
     *
     *  @return  The new Database::Internal object (or the current object
     *		 if no action is required - e.g. unlock on a read-only
     *		 database).
     */
    virtual Internal* update_lock(int flags);

    virtual std::string reconstruct_text(Xapian::docid did,
					 size_t length,
					 const std::string& prefix,
					 Xapian::termpos start_pos,
					 Xapian::termpos end_pos) const;

    /// Return a string describing this object.
    virtual std::string get_description() const = 0;
};

}

#endif // XAPIAN_INCLUDED_DATABASEINTERNAL_H
