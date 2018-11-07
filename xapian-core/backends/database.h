/** @file database.h
 * @brief database class declarations
 */
/* Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2002,2003,2004,2005,2006,2007,2008,2009,2011,2013,2014,2015,2016 Olly Betts
 * Copyright 2006,2008 Lemur Consulting Ltd
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

#ifndef OM_HGUARD_DATABASE_H
#define OM_HGUARD_DATABASE_H

#include <string>

#include "internaltypes.h"

#include "xapian/intrusive_ptr.h"
#include <xapian/types.h>
#include <xapian/database.h>
#include <xapian/document.h>
#include <xapian/positioniterator.h>
#include <xapian/termiterator.h>
#include <xapian/valueiterator.h>

using namespace std;

class LeafPostList;
class RemoteDatabase;

typedef Xapian::TermIterator::Internal TermList;
typedef Xapian::PositionIterator::Internal PositionList;
typedef Xapian::ValueIterator::Internal ValueList;

namespace Xapian {

class Query;
struct ReplicationInfo;

/** Base class for databases.
 */
class Database::Internal : public Xapian::Internal::intrusive_base {
    private:
	/// Copies are not allowed.
	Internal(const Internal &);

	/// Assignment is not allowed.
	void operator=(const Internal &);

    protected:
	/// Transaction state.
	enum {
	    TRANSACTION_UNIMPLEMENTED = -1, // Used by InMemory.
	    TRANSACTION_NONE = 0,
	    TRANSACTION_UNFLUSHED = 1,
	    TRANSACTION_FLUSHED = 2
	} transaction_state;

	bool transaction_active() const { return int(transaction_state) > 0; }

	/** Create a database - called only by derived classes. */
	Internal() : transaction_state(TRANSACTION_NONE) { }

	/** Internal method to perform cleanup when a writable database is
	 *  destroyed with uncommitted changes.
	 *
	 *  A derived class' destructor should call this method before
	 *  destroying the database to ensure that no sessions or
	 *  transactions are in progress at destruction time.
	 *
	 *  Note that it is not safe to throw exceptions from destructors,
	 *  so this method will catch and discard any exceptions.
	 */
	void dtor_called();

    public:
	/** Destroy the database.
	 *
	 *  This method should not be called until all objects using the
	 *  database have been cleaned up.
	 *
	 *  If any transactions are in progress, they should
	 *  be finished by cancel_transaction() or
	 *  commit_transaction() - if this is not done, the destructor
	 *  will attempt to clean things up by cancelling the transaction,
	 *  but any errors produced by these operations will not be reported.
	 */
	virtual ~Internal();

	/** Send a keep-alive signal to a remote database, to stop
	 *  it from timing out.
	 */
	virtual void keep_alive();

	virtual void readahead_for_query(const Xapian::Query & query);

	//////////////////////////////////////////////////////////////////
	// Database statistics:
	// ====================

	/** Return the number of docs in this (sub) database.
	 */
	virtual Xapian::doccount get_doccount() const = 0;

	/** Return the last used document id of this (sub) database.
	 */
	virtual Xapian::docid get_lastdocid() const = 0;

	/** Return the total length of all documents in this database. */
	virtual Xapian::totallength get_total_length() const = 0;

	/** Get the length of a given document.
	 *
	 *  Document length, for the purposes of Xapian, is defined to be
	 *  the number of instances of terms within a document.  Expressed
	 *  differently, the sum of the within document frequencies over
	 *  all the terms in the document.
	 *
	 *  @param did  The document id of the document whose length is
	 *              being requested.
	 */
	virtual Xapian::termcount get_doclength(Xapian::docid did) const = 0;

	/** Get the number of unique term in document.
	 *
	 *  @param did  The document id of the document whose number of terms is
	 *		being requested.
	 */
	virtual	Xapian::termcount get_unique_terms(Xapian::docid did) const = 0;

	/** Returns frequencies for a term.
	 *
	 *  @param term		The term to get frequencies for
	 *  @param termfreq_ptr	Point to return number of docs indexed by @a
	 *			term (or NULL not to return)
	 *  @param collfreq_ptr	Point to return number of occurrences of @a
	 *			term in the database (or NULL not to return)
	 */
	virtual void get_freqs(const string & term,
			       Xapian::doccount * termfreq_ptr,
			       Xapian::termcount * collfreq_ptr) const = 0;

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
	virtual Xapian::doccount get_value_freq(Xapian::valueno slot) const;

	/** Get a lower bound on the values stored in the given value slot.
	 *
	 *  If the lower bound isn't available for the given database type,
	 *  this will return the lowest possible bound - the empty string.
	 *
	 *  @param slot The value slot to examine.
	 */
	virtual std::string get_value_lower_bound(Xapian::valueno slot) const;

	/** Get an upper bound on the values stored in the given value slot.
	 *
	 *  @param slot The value slot to examine.
	 *
	 *  @exception UnimplementedError The upper bound of the values isn't
	 *  available for this database type.
	 */
	virtual std::string get_value_upper_bound(Xapian::valueno slot) const;

	/// Get a lower bound on the length of a document in this DB.
	virtual Xapian::termcount get_doclength_lower_bound() const;

	/// Get an upper bound on the length of a document in this DB.
	virtual Xapian::termcount get_doclength_upper_bound() const;

	/// Get an upper bound on the wdf of term @a term.
	virtual Xapian::termcount get_wdf_upper_bound(const std::string & term) const;

	/** Check whether a given term is in the database.
	 *
	 *  @param tname  The term whose presence is being checked.
	 */
	virtual bool term_exists(const string & tname) const = 0;

	/** Check whether this database contains any positional information.
	 */
	virtual bool has_positions() const = 0;

	//////////////////////////////////////////////////////////////////
	// Data item access methods:
	// =========================

	/** Open a posting list.
	 *
	 *  Method defined by subclass to open a posting list.
	 *  This is a list of all the documents which contain a given term.
	 *
	 *  @param tname  The term whose posting list is being requested.
	 *
	 *  @return       A pointer to the newly created posting list.
	 *		  If the term doesn't exist, a LeafPostList object
	 *		  returning no documents is returned, which makes it
	 *		  easier to implement a search over multiple databases.
	 *		  This object must be deleted by the caller after
	 *                use.
	 */
	virtual LeafPostList * open_post_list(const string & tname) const = 0;

	/** Open a value stream.
	 *
	 *  This returns the value in a particular slot for each document.
	 *
	 *  @param slot	The value slot.
	 *
	 *  @return	Pointer to a new ValueList object which should be
	 *		deleted by the caller once it is no longer needed.
	 */
	virtual ValueList * open_value_list(Xapian::valueno slot) const;

	/** Open a term list.
	 *
	 *  This is a list of all the terms contained by a given document.
	 *
	 *  @param did    The document id whose term list is being requested.
	 *
	 *  @return       A pointer to the newly created term list.
	 *                This object must be deleted by the caller after
	 *                use.
	 */
	virtual TermList * open_term_list(Xapian::docid did) const = 0;

	/** Open an allterms list.
	 *
	 *  This is a list of all the terms in the database
	 *
	 *  @param prefix The prefix to restrict the terms to.
	 *  @return       A pointer to the newly created allterms list.
	 *                This object must be deleted by the caller after
	 *                use.
	 */
	virtual TermList * open_allterms(const string & prefix) const = 0;

	/** Open a position list for the given term in the given document.
	 *
	 *  @param did    The document id for which a position list is being
	 *                requested.
	 *  @param tname  The term for which a position list is being
	 *                requested.
	 *
	 *  @return       A pointer to the newly created position list.
	 *                This object must be deleted by the caller after
	 *                use.
	 */
	virtual PositionList * open_position_list(Xapian::docid did,
					const string & tname) const = 0;

	/** Open a document.
	 *
	 *  This is used to access the values and data associated with a
	 *  document.  See class Xapian::Document::Internal for further details.
	 *
	 *  @param did    The document id which is being requested.
	 *
	 *  @param lazy   No need to check that this document actually exists.
	 *                Used when we already know that this document exists
	 *                (only a hint - the backend may still check).
	 *
	 *  @return       A pointer to the newly created document object.
	 *                This object must be deleted by the caller after
	 *                use.
	 */
	virtual Xapian::Document::Internal *
	open_document(Xapian::docid did, bool lazy) const = 0;

	/** Create a termlist tree from trigrams of @a word.
	 *
	 *  You can assume word.size() > 1.
	 *
	 *  If there are no trigrams, returns NULL.
	 */
	virtual TermList * open_spelling_termlist(const string & word) const;

	/** Return a termlist which returns the words which are spelling
	 *  correction targets.
	 *
	 *  If there are no spelling correction targets, returns NULL.
	 */
	virtual TermList * open_spelling_wordlist() const;

	/** Return the number of times @a word was added as a spelling. */
	virtual Xapian::doccount get_spelling_frequency(const string & word) const;

	/** Add a word to the spelling dictionary.
	 *
	 *  If the word is already present, its frequency is increased.
	 *
	 *  @param word	    The word to add.
	 *  @param freqinc  How much to increase its frequency by.
	 */
	virtual void add_spelling(const string & word,
				  Xapian::termcount freqinc) const;

	/** Remove a word from the spelling dictionary.
	 *
	 *  The word's frequency is decreased, and if would become zero or less
	 *  then the word is removed completely.
	 *
	 *  @param word	    The word to remove.
	 *  @param freqdec  How much to decrease its frequency by.
	 */
	virtual void remove_spelling(const string & word,
				     Xapian::termcount freqdec) const;

	/** Open a termlist returning synonyms for a term.
	 *
	 *  If @a term has no synonyms, returns NULL.
	 */
	virtual TermList * open_synonym_termlist(const string & term) const;

	/** Open a termlist returning each term which has synonyms.
	 *
	 *  @param prefix   If non-empty, only terms with this prefix are
	 *		    returned.
	 */
	virtual TermList * open_synonym_keylist(const string & prefix) const;

	/** Add a synonym for a term.
	 *
	 *  If @a synonym is already a synonym for @a term, then no action is
	 *  taken.
	 */
	virtual void add_synonym(const string & term, const string & synonym) const;

	/** Remove a synonym for a term.
	 *
	 *  If @a synonym isn't a synonym for @a term, then no action is taken.
	 */
	virtual void remove_synonym(const string & term, const string & synonym) const;

	/** Clear all synonyms for a term.
	 *
	 *  If @a term has no synonyms, no action is taken.
	 */
	virtual void clear_synonyms(const string & term) const;

	/** Get the metadata associated with a given key.
	 *
	 *  See Database::get_metadata() for more information.
	 */
	virtual string get_metadata(const string & key) const;

	/** Open a termlist returning each metadata key.
	 *
	 *  Only metadata keys which are associated with a non-empty value will
	 *  be returned.
	 *
	 *  @param prefix   If non-empty, only keys with this prefix are
	 *		    returned.
	 */
	virtual TermList * open_metadata_keylist(const std::string &prefix) const;

	/** Set the metadata associated with a given key.
	 *
	 *  See WritableDatabase::set_metadata() for more information.
	 */
	virtual void set_metadata(const string & key, const string & value);

	/** Reopen the database to the latest available revision.
	 *
	 *  Database backends which don't support simultaneous update and
	 *  reading probably don't need to do anything here.
	 */
	virtual bool reopen();

	/** Close the database
	 */
	virtual void close() = 0;

	//////////////////////////////////////////////////////////////////
	// Modifying the database:
	// =======================

	/** Commit pending modifications to the database.
	 *
	 *  See WritableDatabase::commit() for more information.
	 */
	virtual void commit();

	/** Cancel pending modifications to the database. */
	virtual void cancel();

	/** Begin a transaction.
	 *
	 *  See WritableDatabase::begin_transaction() for more information.
	 */
	void begin_transaction(bool flushed);

	/** Commit a transaction.
	 *
	 *  See WritableDatabase::commit_transaction() for more information.
	 */
	void commit_transaction();

	/** Cancel a transaction.
	 *
	 *  See WritableDatabase::cancel_transaction() for more information.
	 */
	void cancel_transaction();

	/** Add a new document to the database.
	 *
	 *  See WritableDatabase::add_document() for more information.
	 */
	virtual Xapian::docid add_document(const Xapian::Document & document);

	/** Delete a document in the database.
	 *
	 *  See WritableDatabase::delete_document() for more information.
	 */
	virtual void delete_document(Xapian::docid did);

	/** Delete any documents indexed by a term from the database.
	 *
	 *  See WritableDatabase::delete_document() for more information.
	 */
	virtual void delete_document(const string & unique_term);

	/** Replace a given document in the database.
	 *
	 *  See WritableDatabase::replace_document() for more information.
	 */
	virtual void replace_document(Xapian::docid did,
				      const Xapian::Document & document);

	/** Replace any documents matching a term.
	 *
	 *  See WritableDatabase::replace_document() for more information.
	 */
	virtual Xapian::docid replace_document(const string & unique_term,
					       const Xapian::Document & document);

	/** Request and later collect a document from the database.
	 *  Multiple documents can be requested with request_document(),
	 *  and then collected with collect_document().  Allows the backend
	 *  to optimise (e.g. the remote backend can start requests for all
	 *  the documents so they fetch in parallel).
	 *
	 *  If a backend doesn't support this, request_document() can be a
	 *  no-op and collect_document() the same as open_document().
	 */
	//@{
	virtual void request_document(Xapian::docid /*did*/) const;

	virtual Xapian::Document::Internal * collect_document(Xapian::docid did) const;
	//@}

	/** Write a set of changesets to a file descriptor.
	 *
	 *  This call may reopen the database, leaving it pointing to a more
	 *  recent version of the database.
	 */
	virtual void write_changesets_to_fd(int fd,
					    const std::string & start_revision,
					    bool need_whole_db,
					    Xapian::ReplicationInfo * info);

	/// Get a string describing the current revision of the database.
	virtual string get_revision_info() const;

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
	virtual string get_uuid() const;

	/** Notify the database that document is no longer valid.
	 *
	 *  This is used to invalidate references to a document kept by a
	 *  database for doing lazy updates.  If we moved to using a weak_ptr
	 *  instead we wouldn't need a special method for this, but it would
	 *  involve a fair bit of reorganising of other parts of the code.
	 */
	virtual void invalidate_doc_object(Xapian::Document::Internal * obj) const;

	/** Get backend information about this database.
	 *
	 *  @param path  If non-NULL, and set the pointed to string to the file
	 *		 path of this database (or if to some string describing
	 *		 the database in a backend-specified format if "path"
	 *		 isn't a concept which  make sense).
	 *
	 *  @return	A constant indicating the backend type.
	 */
	virtual int get_backend_info(string * path) const = 0;

	/** Find lowest and highest docids actually in use.
	 *
	 *  Only used by compaction, so only needs to be implemented by
	 *  backends which support compaction.
	 */
	virtual void get_used_docid_range(Xapian::docid & first,
					  Xapian::docid & last) const;

	/** Return true if the database is open for writing.
	 *
	 *  If this is a WritableDatabase, always returns true.
	 *
	 *  For a Database, test if there's a writer holding the lock (or if
	 *  we can't test for a lock without taking it on the current platform,
	 *  throw Xapian::UnimplementedError).
	 */
	virtual bool locked() const;
};

}

#endif /* OM_HGUARD_DATABASE_H */
