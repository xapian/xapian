/** @file database.h
 * @brief An indexed database of documents
 */
/* Copyright 2003,2004,2005,2006,2007,2008,2009,2011,2012,2013,2014,2015,2016,2017,2018,2019 Olly Betts
 * Copyright 2007,2008,2009 Lemur Consulting Ltd
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

#ifndef XAPIAN_INCLUDED_DATABASE_H
#define XAPIAN_INCLUDED_DATABASE_H

#if !defined XAPIAN_IN_XAPIAN_H && !defined XAPIAN_LIB_BUILD
# error Never use <xapian/database.h> directly; include <xapian.h> instead.
#endif

#include <iosfwd>
#include <string>
#include <utility>
#include <vector>

#include <xapian/attributes.h>
#include <xapian/constants.h>
#include <xapian/intrusive_ptr.h>
#include <xapian/positioniterator.h>
#include <xapian/postingiterator.h>
#include <xapian/termiterator.h>
#include <xapian/types.h>
#include <xapian/valueiterator.h>
#include <xapian/visibility.h>

namespace Xapian {

class Compactor;
class Document;
class WritableDatabase;

/** An indexed database of documents.
 *
 *  A Database object contains zero or more shards, and operations are
 *  performed across these shards.
 *
 *  To perform a search on a Database, you need to use an Enquire object.
 *
 *  @since 1.5.0 This class is a reference counted handle like many other
 *	   Xapian API classes.  In earlier versions, it worked like a typedef
 *	   to std::vector<database_shard>.  The key difference is that
 *	   previously copying or assigning a Xapian::Database made a deep copy,
 *	   whereas now it makes a shallow copy.
 *
 *  Most methods can throw:
 *
 *  @exception Xapian::DatabaseCorruptError if database corruption is detected
 *  @exception Xapian::DatabaseError in various situation (for example, if
 *	       there's an I/O error).
 *  @exception Xapian::DatabaseModifiedError if the revision being read has
 *	       been discarded
 *  @exception Xapian::DatabaseClosedError may be thrown by some methods after
 *	       after @a close() has been called
 *  @exception Xapian::NetworkError when remote databases are in use
 */
class XAPIAN_VISIBILITY_DEFAULT Database {
    /// @internal Implementation behind check() static methods.
    static size_t check_(const std::string* path_ptr,
			 int fd,
			 int opts,
			 std::ostream* out);

    /// @internal Implementation behind public compact() methods.
    void compact_(const std::string* output_ptr,
		  int fd,
		  unsigned flags,
		  int block_size,
		  Xapian::Compactor* compactor) const;

  protected:
    /// @private @internal Implementation behind public add_database() methods.
    void add_database_(const Database& other, bool read_only);

  public:
    /// Class representing the Database internals.
    class Internal;
    /// @private @internal Reference counted internals.
    Xapian::Internal::intrusive_ptr_nonnull<Internal> internal;

    /** Add shards from another Database.
     *
     *  Any shards in @a other are appended to the list of shards in this
     *  object.  The shards are reference counted and also remain in @a other.
     *
     *  @param other	Another Database to add shards from
     *
     *  @exception Xapian::InvalidArgumentError if @a other is the same object
     *		   as this.
     */
    void add_database(const Database& other) {
	add_database_(other, true);
    }

    /** Return number of shards in this Database object. */
    size_t size() const;

    /** Construct a Database containing no shards.
     *
     *  You can then add shards by calling add_database().  A Database
     *  containing no shards can also be useful in situations where you need an
     *  empty database.
     */
    Database();

    /** Open a Database.
     *
     *  @param path	Filing system path to open database from
     *  @param flags	Bitwise-or of Xapian::DB_* constants
     *
     *  The @a path can be a file (for a stub database or a single-file glass
     *  database) or a directory (for a standard glass database).  If
     *  @a flags includes @a DB_BACKEND_INMEMORY then @a path is ignored.
     *
     *  @exception Xapian::DatabaseOpeningError if the specified database
     *		   cannot be opened
     *  @exception Xapian::DatabaseVersionError if the specified database has
     *		   a format too old or too new to be supported.
     */
    explicit Database(const std::string& path, int flags = 0);

    /** Open a single-file Database.
     *
     *  This method opens a single-file Database given a file descriptor open
     *  on it.  Xapian looks starting at the current file offset, allowing a
     *  single file database to be easily embedded within another file.
     *
     *  @param fd	File descriptor for the file.  Xapian takes ownership
     *			of this and will close it when the database is closed.
     *  @param flags	Bitwise-or of Xapian::DB_* constants.
     *
     *  @exception Xapian::DatabaseOpeningError if the specified database
     *		   cannot be opened
     *  @exception Xapian::DatabaseVersionError if the specified database has
     *		   a format too old or too new to be supported.
     */
    explicit Database(int fd, int flags = 0);

    /// @private @internal Wrap an existing Internal.
    XAPIAN_VISIBILITY_INTERNAL
    explicit Database(Internal* internal) XAPIAN_NONNULL();

    /// Destructor.
    virtual ~Database();

    /** Copy constructor.
     *
     *  The internals are reference counted, so copying is cheap.
     */
    Database(const Database& o);

    /** Assignment operator.
     *
     *  The internals are reference counted, so assignment is cheap.
     */
    Database& operator=(const Database& o);

    /// Move constructor.
    Database(Database&& o);

    /// Move assignment operator.
    Database& operator=(Database&& o);

    /** Reopen the database at the latest available revision.
     *
     *  Xapian databases (at least with most backends) support versioning
     *  such that a Database object uses a snapshot of the database.
     *  However, write operations may cause this snapshot to be discarded,
     *  which can cause Xapian::DatabaseModifiedError to be thrown.  You
     *  can recover from this situation by calling reopen() and restarting
     *  the search operation.
     *
     *  All shards are updated to the latest available revision.  This should
     *  be a cheap operation if they're already at the latest revision, so
     *  if you're using the same Database object for many searches it's
     *  reasonable to call reopen() before each search.
     *
     *  @return true if one or more shards have moved to a newer revision
     *		(if false is returned then it's definitely the case that no
     *		shards were reopened, which applications may find useful when
     *		caching results, etc).  In Xapian < 1.3.0, this method did not
     *		return a value.
     *
     *  @exception Xapian::DatabaseError is thrown if close() has been called
     *		   on any of the shards.
     */
    bool reopen();

    /** Close the database.
     *
     *  This closes the database and closes all its file handles.
     *
     *  For a WritableDatabase, if a transaction is active it will be aborted,
     *  while if no transaction is active commit() will be implicitly called.
     *  Also the write lock is released.
     *
     *  Calling close() on an object cannot be undone - in particular, a
     *  subsequent call to reopen() on the same object will not reopen it, but
     *  will instead throw a Xapian::DatabaseClosedError exception.
     *
     *  Calling close() again on an object which has already been closed has
     *  no effect (and doesn't raise an exception).
     *
     *  After close() has been called, calls to other methods of the database,
     *  and to methods of other objects associated with the database, will
     *  either:
     *
     *   - behave exactly as they would have done if the database had not been
     *     closed (this can only happen if all the required data is cached)
     *
     *   - raise a Xapian::DatabaseClosedError exception.
     *
     *  The reason for this behaviour is that otherwise we'd have to check that
     *  the database is still open on every method call on every object
     *  associated with a Database, when in many cases they are working on data
     *  which has already been loaded and so they are able to just behave
     *  correctly.
     *
     *  @since This method was added in Xapian 1.1.0.
     */
    virtual void close();

    /// Return a string describing this object.
    virtual std::string get_description() const;

    /** Start iterating the postings of a term.
     *
     *  @param term	The term to iterate the postings of.  An empty string
     *			acts as a special pseudo-term which indexes all the
     *			documents in the database with a wdf of 1.
     */
    PostingIterator postlist_begin(const std::string& term) const;

    /** End iterator corresponding to postlist_begin(). */
    PostingIterator XAPIAN_NOTHROW(postlist_end(const std::string&) const) {
	return PostingIterator();
    }

    /** Start iterating the terms in a document.
     *
     *  @param did	The document id to iterate terms from
     *
     *  The terms are returned ascending string order (by byte value).
     */
    TermIterator termlist_begin(Xapian::docid did) const;

    /** End iterator corresponding to termlist_begin(). */
    TermIterator XAPIAN_NOTHROW(termlist_end(Xapian::docid) const) {
	return TermIterator();
    }

    /** Does this database have any positional information? */
    bool has_positions() const;

    /** Start iterating positions for a term in a document.
     *
     *  @param did	The document id of the document
     *  @param term	The term
     *
     *  @since 1.1.0	If the specified document doesn't exist or the
     *			specified term doesn't exist in the specified document,
     *			then a valid iterator is still returned, but it will be
     *			equal to positionlist_end().
     */
    PositionIterator positionlist_begin(Xapian::docid did, const std::string& term) const;

    /** End iterator corresponding to positionlist_begin(). */
    PositionIterator XAPIAN_NOTHROW(positionlist_end(Xapian::docid, const std::string&) const) {
	return PositionIterator();
    }

    /** Start iterating all terms in the database with a given prefix.
     *
     *  @param prefix	The prefix to restrict the returned terms to (default:
     *			iterate all terms)
     */
    TermIterator allterms_begin(const std::string& prefix = std::string()) const;

    /** End iterator corresponding to allterms_begin(prefix). */
    TermIterator XAPIAN_NOTHROW(allterms_end(const std::string& = std::string()) const) {
	return TermIterator();
    }

    /// Get the number of documents in the database.
    Xapian::doccount get_doccount() const;

    /// Get the highest document id which has been used in the database.
    Xapian::docid get_lastdocid() const;

    /// Get the mean document length in the database.
    double get_average_length() const;

    /// Old name for get_average_length() for backward compatibility.
    double get_avlength() const { return get_average_length(); }

    /** Get the total length of all the documents in the database.
     *
     *  @since Added in Xapian 1.4.5.
     */
    Xapian::totallength get_total_length() const;

    /** Get the number of documents indexed by a specified term.
     *
     *  @param term	The term to get the frequency of.  An empty string
     *			acts as a special pseudo-term which indexes all the
     *			documents in the database, so returns get_doccount().
     *			If the term isn't present in the database, 0 is
     *			returned.
     */
    Xapian::doccount get_termfreq(const std::string& term) const;

    /** Test is a particular term is present in any document.
     *
     *  @param term	The term to test for.  An empty string acts as a
     *			special pseudo-term which indexes all the documents in
     *			the database, so returns true if the database contains
     *			any documents.
     *
     *	db.term_exists(t) gives the same answer as db.get_termfreq(t) != 0, but
     *	is typically more efficient.
     */
    bool term_exists(const std::string& term) const;

    /** Get the total number of occurrences of a specified term.
     *
     *  The collection frequency of a term is defined as the total number of
     *  times it occurs in the database, which is the sum of its wdf in all the
     *  documents it indexes.
     *
     *  @param term	The term to get the collection frequency of.  An empty
     *			string acts as a special pseudo-term which indexes all
     *			the documents in the database, so returns
     *			get_doccount().  If the term isn't present in the
     *			database, 0 is returned.
     */
    Xapian::termcount get_collection_freq(const std::string& term) const;

    /** Return the frequency of a given value slot.
     *
     *  This is the number of documents which have a (non-empty) value stored
     *  in the slot.
     *
     *  @param slot The value slot to examine.
     */
    Xapian::doccount get_value_freq(Xapian::valueno slot) const;

    /** Get a lower bound on the values stored in the given value slot.
     *
     *  If there are no values stored in the given value slot, this will return
     *  an empty string.
     *
     *  @param slot The value slot to examine.
     */
    std::string get_value_lower_bound(Xapian::valueno slot) const;

    /** Get an upper bound on the values stored in the given value slot.
     *
     *  If there are no values stored in the given value slot, this will return
     *  an empty string.
     *
     *  @param slot The value slot to examine.
     */
    std::string get_value_upper_bound(Xapian::valueno slot) const;

    /** Get a lower bound on the length of a document in this DB.
     *
     *  This bound does not include any zero-length documents.
     */
    Xapian::termcount get_doclength_lower_bound() const;

    /// Get an upper bound on the length of a document in this DB.
    Xapian::termcount get_doclength_upper_bound() const;

    /// Get an upper bound on the wdf of term @a term.
    Xapian::termcount get_wdf_upper_bound(const std::string& term) const;

    /// Get a lower bound on the unique terms size of a document in this DB.
    Xapian::termcount get_unique_terms_lower_bound() const;

    /// Get an upper bound on the unique terms size of a document in this DB.
    Xapian::termcount get_unique_terms_upper_bound() const;

    /// Return an iterator over the value in slot @a slot for each document.
    ValueIterator valuestream_begin(Xapian::valueno slot) const;

    /// Return end iterator corresponding to valuestream_begin().
    ValueIterator XAPIAN_NOTHROW(valuestream_end(Xapian::valueno) const) {
	return ValueIterator();
    }

    /** Get the length of a document.
     *
     *  @param did   The document id of the document
     *
     *  Xapian defines a document's length as the sum of the wdf of all the
     *  terms which index it.
     */
    Xapian::termcount get_doclength(Xapian::docid did) const;

    /** Get the number of unique terms in a document.
     *
     *  @param did   The document id of the document
     *
     *  This is the number of different terms which index the given document.
     */
    Xapian::termcount get_unique_terms(Xapian::docid did) const;

    /** Send a keep-alive message.
     *
     *  For remote databases, this method sends a message to the server to
     *  reset the timeout timer.  As well as preventing timeouts at the Xapian
     *  remote protocol level, this message will also avoid timeouts at lower
     *  levels.
     *
     *  For local databases, this method does nothing.
     */
    void keep_alive();

    /** Get a document from the database.
     *
     *  The returned object acts as a handle which lazily fetches information
     *  about the specified document from the database.
     *
     *  @param did	The document ID of the document to be get
     *  @param flags	Zero or more flags bitwise-or-ed together (currently
     *			only Xapian::DOC_ASSUME_VALID is supported).
     *			(default: 0)
     *
     *  @exception Xapian::InvalidArgumentError is thrown if @a did is 0.
     *
     *  @exception Xapian::DocNotFoundError is thrown if the specified docid
     *		   is not present in this database.
     */
    Xapian::Document get_document(Xapian::docid did,
				  unsigned flags = 0) const;

    /** Suggest a spelling correction.
     *
     *  @param word			The potentially misspelled word.
     *  @param max_edit_distance	Only consider words which are at most
     *					@a max_edit_distance edits from @a
     *					word.  An edit is a character
     *					insertion, deletion, or the
     *					transposition of two adjacent
     *					characters (default is 2).
     */
    std::string get_spelling_suggestion(const std::string& word,
					unsigned max_edit_distance = 2) const;

    /** An iterator which returns all the spelling correction targets.
     *
     *  This returns all the words which are considered as targets for the
     *  spelling correction algorithm.  The frequency of each word is available
     *  as the term frequency of each entry in the returned iterator.
     */
    Xapian::TermIterator spellings_begin() const;

    /// End iterator corresponding to spellings_begin().
    Xapian::TermIterator XAPIAN_NOTHROW(spellings_end() const) {
	return Xapian::TermIterator();
    }

    /** An iterator which returns all the synonyms for a given term.
     *
     *  @param term	The term to return synonyms for.
     */
    Xapian::TermIterator synonyms_begin(const std::string& term) const;

    /// End iterator corresponding to synonyms_begin(term).
    Xapian::TermIterator XAPIAN_NOTHROW(synonyms_end(const std::string&) const) {
	return Xapian::TermIterator();
    }

    /** An iterator which returns all terms which have synonyms.
     *
     *  @param prefix	If non-empty, only terms with this prefix are returned.
     */
    Xapian::TermIterator synonym_keys_begin(const std::string& prefix = std::string()) const;

    /// End iterator corresponding to synonym_keys_begin(prefix).
    Xapian::TermIterator XAPIAN_NOTHROW(synonym_keys_end(const std::string& = std::string()) const) {
	return Xapian::TermIterator();
    }

    /** Get the user-specified metadata associated with a given key.
     *
     *  User-specified metadata allows you to store arbitrary information in
     *  the form of (key, value) pairs.  See @a
     *  WritableDatabase::set_metadata() for more information.
     *
     *  When invoked on a Xapian::Database object representing multiple
     *  databases, currently only the metadata for the first is considered but
     *  this behaviour may change in the future.
     *
     *  If there is no piece of metadata associated with the specified key, an
     *  empty string is returned (this applies even for backends which don't
     *  support metadata).
     *
     *  Empty keys are not valid, and specifying one will cause an exception.
     *
     *  @param key The key of the metadata item to access.
     *
     *  @return    The retrieved metadata item's value.
     *
     *  @exception Xapian::InvalidArgumentError will be thrown if the key
     *		   supplied is empty.
     */
    std::string get_metadata(const std::string& key) const;

    /** An iterator which returns all user-specified metadata keys.
     *
     *  When invoked on a Xapian::Database object representing multiple
     *  databases, currently only the metadata for the first is considered but
     *  this behaviour may change in the future.
     *
     *  If the backend doesn't support metadata, then this method returns an
     *  iterator which compares equal to that returned by metadata_keys_end().
     *
     *  @param prefix   If non-empty, only keys with this prefix are returned.
     *
     *  @exception Xapian::UnimplementedError will be thrown if the backend
     *		   implements user-specified metadata, but doesn't implement
     *		   iterating its keys (currently this happens for the InMemory
     *		   backend).
     */
    Xapian::TermIterator metadata_keys_begin(const std::string&prefix = std::string()) const;

    /// End iterator corresponding to metadata_keys_begin().
    Xapian::TermIterator XAPIAN_NOTHROW(metadata_keys_end(const std::string& = std::string()) const) {
	return Xapian::TermIterator();
    }

    /** Get a UUID for the database.
     *
     *  The UUID will persist for the lifetime of the database.
     *
     *  Replicas (eg, made with the replication protocol, or by copying all the
     *  database files) will have the same UUID.  However, copies (made with
     *  copydatabase, or xapian-compact) will have different UUIDs.
     *
     *  If the backend does not support UUIDs or this database has no
     *  subdatabases, the UUID will be empty.
     *
     *  If this database has multiple sub-databases, the UUID string will
     *  contain the UUIDs of all the sub-databases separated by colons.
     */
    std::string get_uuid() const;

    /** Test if this database is currently locked for writing.
     *
     *  If the underlying object is actually a WritableDatabase, always returns
     *  true unless close() has been called.
     *
     *  Otherwise tests if there's a writer holding the lock (or if we can't
     *  test for a lock without taking it on the current platform, throw
     *  Xapian::UnimplementedError).  If there's an error while trying to test
     *  the lock, throws Xapian::DatabaseLockError.
     *
     *  For multi-databases, this tests each sub-database and returns true if
     *  any of them are locked.
     */
    bool locked() const;

    /** Lock a read-only database for writing.
     *
     *  If the database is actually already writable (i.e. a WritableDatabase
     *  via a Database reference) then the same database is returned (with
     *  its flags updated, so this provides an efficient way to modify flags
     *  on an open WritableDatabase).
     *
     *  Unlike unlock(), the object this is called on remains open.
     *
     *  @param flags  The flags to use for the writable database.  Flags which
     *		      specify how to open the database are ignored (e.g.
     *		      DB_CREATE_OR_OVERWRITE doesn't result in the database
     *		      being wiped), and flags which specify the backend are
     *		      also ignored as they are only relevant when creating
     *		      a new database.
     *
     *  @return  A WritableDatabase object open on the same database.
     */
    Xapian::WritableDatabase lock(int flags = 0);

    /** Release a database write lock.
     *
     *  If called on a read-only database then the same database is returned.
     *
     *  If called on a writable database, the object this method was called
     *  on is closed.
     *
     *  @return  A Database object open on the same database.
     */
    Xapian::Database unlock();

    /** Get the revision of the database.
     *
     *  The revision is an unsigned integer which increases with each commit.
     *
     *  The database must have exactly one sub-database, which must be of type
     *  glass.  Otherwise an exception will be thrown.
     *
     *  Experimental - see
     *  https://xapian.org/docs/deprecation#experimental-features
     */
    Xapian::rev get_revision() const;

    /** Check the integrity of a database or database table.
     *
     *  @param path	Path to database or table
     *  @param opts	Options to use for check
     *  @param out	std::ostream to write output to (NULL for no output)
     */
    static size_t check(const std::string& path,
			int opts = 0,
			std::ostream* out = NULL) {
	return check_(&path, 0, opts, out);
    }

    /** Check the integrity of a single file database.
     *
     *  @param fd	file descriptor for the database.  The current file
     *			offset is used, allowing checking a single file
     *			database which is embedded within another file.  Xapian
     *			takes ownership of the file descriptor and will close
     *			it before returning.
     *  @param opts	Options to use for check
     *  @param out	std::ostream to write output to (NULL for no output)
     */
    static size_t check(int fd, int opts = 0, std::ostream* out = NULL) {
	return check_(NULL, fd, opts, out);
    }

    /** Produce a compact version of this database.
     *
     *  New 1.3.4.  Various methods of the Compactor class were deprecated in
     *  1.3.4.
     *
     *  @param output	Path to write the compact version to.  This can be the
     *			same as an input if that input is a stub database (in
     *			which case the database(s) listed in the stub will be
     *			compacted to a new database and then the stub will be
     *			atomically updated to point to this new database).
     *
     *  @param flags	Any of the following combined using bitwise-or (| in
     *			C++):
     *   - Xapian::DBCOMPACT_NO_RENUMBER By default the document ids will
     *		be renumbered the output - currently by applying the same
     *		offset to all the document ids in a particular source database.
     *		If this flag is specified, then this renumbering doesn't
     *		happen, but all the document ids must be unique over all source
     *		databases.  Currently the ranges of document ids in each source
     *		must not overlap either, though this restriction may be removed
     *		in the future.
     *   - Xapian::DBCOMPACT_MULTIPASS
     *		If merging more than 3 databases, merge the postlists in
     *		multiple passes, which is generally faster but requires more
     *		disk space for temporary files.
     *   - Xapian::DBCOMPACT_SINGLE_FILE
     *		Produce a single-file database (only supported for glass
     *		currently).
     *   - At most one of:
     *     - Xapian::Compactor::STANDARD - Don't split items unnecessarily.
     *     - Xapian::Compactor::FULL     - Split items whenever it saves space
     *					   (the default).
     *     - Xapian::Compactor::FULLER   - Allow oversize items to save more
     *					   space (not recommended if you ever
     *					   plan to update the compacted
     *					   database).
     *   - At most one of the following to specify the output format (currently
     *     only glass to honey conversion is supported, and all shards of the
     *     input must have the same format):
     *     - Xapian::DB_BACKEND_HONEY
     *
     *  @param block_size	This specifies the block size (in bytes) for to
     *				use for the output.  For glass, the block size
     *				must be a power of 2 between 2048 and 65536
     *				(inclusive), and the default (also used if an
     *				invalid value is passed) is 8192 bytes.
     */
    void compact(const std::string& output,
		 unsigned flags = 0,
		 int block_size = 0) {
	compact_(&output, 0, flags, block_size, NULL);
    }

    /** Produce a compact version of this database.
     *
     *  New 1.3.4.  Various methods of the Compactor class were deprecated in
     *  1.3.4.
     *
     *  This variant writes a single-file database to the specified file
     *  descriptor.  Only the glass backend supports such databases, so
     *  this form is only supported for this backend.
     *
     *  @param fd	File descriptor to write the compact version to.  The
     *			descriptor needs to be readable and writable (open with
     *			O_RDWR) and seekable.  The current file offset is used,
     *			allowing compacting to a single file database embedded
     *			within another file.  Xapian takes ownership of the
     *			file descriptor and will close it before returning.
     *
     *  @param flags	Any of the following combined using bitwise-or (| in
     *			C++):
     *   - Xapian::DBCOMPACT_NO_RENUMBER By default the document ids will
     *		be renumbered the output - currently by applying the same
     *		offset to all the document ids in a particular source database.
     *		If this flag is specified, then this renumbering doesn't
     *		happen, but all the document ids must be unique over all source
     *		databases.  Currently the ranges of document ids in each source
     *		must not overlap either, though this restriction may be removed
     *		in the future.
     *   - Xapian::DBCOMPACT_MULTIPASS
     *		If merging more than 3 databases, merge the postlists in
     *		multiple passes, which is generally faster but requires more
     *		disk space for temporary files.
     *   - Xapian::DBCOMPACT_SINGLE_FILE
     *		Produce a single-file database (only supported for glass
     *		currently).
     *   - At most one of:
     *     - Xapian::Compactor::STANDARD - Don't split items unnecessarily.
     *     - Xapian::Compactor::FULL     - Split items whenever it saves space
     *					   (the default).
     *     - Xapian::Compactor::FULLER   - Allow oversize items to save more
     *					   space (not recommended if you ever
     *					   plan to update the compacted
     *					   database).
     *
     *  @param block_size	This specifies the block size (in bytes) for to
     *				use for the output.  For glass, the block size
     *				must be a power of 2 between 2048 and 65536
     *				(inclusive), and the default (also used if an
     *				invalid value is passed) is 8192 bytes.
     */
    void compact(int fd,
		 unsigned flags = 0,
		 int block_size = 0) {
	compact_(NULL, fd, flags, block_size, NULL);
    }

    /** Produce a compact version of this database.
     *
     *  New 1.3.4.  Various methods of the Compactor class were deprecated
     *  in 1.3.4.
     *
     *  The @a compactor functor allows handling progress output and
     *  specifying how user metadata is merged.
     *
     *  @param output Path to write the compact version to.
     *		  This can be the same as an input if that input is a
     *		  stub database (in which case the database(s) listed
     *		  in the stub will be compacted to a new database and
     *		  then the stub will be atomically updated to point to
     *		  this new database).
     *
     *  @param flags	Any of the following combined using bitwise-or (| in
     *			C++):
     *   - Xapian::DBCOMPACT_NO_RENUMBER By default the document ids will
     *		be renumbered the output - currently by applying the same
     *		offset to all the document ids in a particular source database.
     *		If this flag is specified, then this renumbering doesn't
     *		happen, but all the document ids must be unique over all source
     *		databases.  Currently the ranges of document ids in each source
     *		must not overlap either, though this restriction may be removed
     *		in the future.
     *   - Xapian::DBCOMPACT_MULTIPASS
     *		If merging more than 3 databases, merge the postlists in
     *		multiple passes, which is generally faster but requires more
     *		disk space for temporary files.
     *   - Xapian::DBCOMPACT_SINGLE_FILE
     *		Produce a single-file database (only supported for glass
     *		currently).
     *   - At most one of:
     *     - Xapian::Compactor::STANDARD - Don't split items unnecessarily.
     *     - Xapian::Compactor::FULL     - Split items whenever it saves space
     *					   (the default).
     *     - Xapian::Compactor::FULLER   - Allow oversize items to save more
     *					   space (not recommended if you ever
     *					   plan to update the compacted
     *					   database).
     *
     *  @param block_size	This specifies the block size (in bytes) for to
     *				use for the output.  For glass, the block size
     *				must be a power of 2 between 2048 and 65536
     *				(inclusive), and the default (also used if an
     *				invalid value is passed) is 8192 bytes.
     *
     *  @param compactor Functor
     */
    void compact(const std::string& output,
		 unsigned flags,
		 int block_size,
		 Xapian::Compactor& compactor)
    {
	compact_(&output, 0, flags, block_size, &compactor);
    }

    /** Produce a compact version of this database.
     *
     *  New 1.3.4.  Various methods of the Compactor class were deprecated in
     *  1.3.4.
     *
     *  The @a compactor functor allows handling progress output and specifying
     *  how user metadata is merged.
     *
     *  This variant writes a single-file database to the specified file
     *  descriptor.  Only the glass backend supports such databases, so this
     *  form is only supported for this backend.
     *
     *  @param fd	File descriptor to write the compact version to.  The
     *			descriptor needs to be readable and writable (open with
     *			O_RDWR) and seekable.  The current file offset is used,
     *			allowing compacting to a single file database embedded
     *			within another file.  Xapian takes ownership of the
     *			file descriptor and will close it before returning.
     *
     *  @param flags	Any of the following combined using bitwise-or (| in
     *			C++):
     *   - Xapian::DBCOMPACT_NO_RENUMBER By default the document ids will
     *		be renumbered the output - currently by applying the same
     *		offset to all the document ids in a particular source database.
     *		If this flag is specified, then this renumbering doesn't
     *		happen, but all the document ids must be unique over all source
     *		databases.  Currently the ranges of document ids in each source
     *		must not overlap either, though this restriction may be removed
     *		in the future.
     *   - Xapian::DBCOMPACT_MULTIPASS
     *		If merging more than 3 databases, merge the postlists in
     *		multiple passes, which is generally faster but requires more
     *		disk space for temporary files.
     *   - Xapian::DBCOMPACT_SINGLE_FILE
     *		Produce a single-file database (only supported for glass
     *		currently).
     *   - At most one of:
     *     - Xapian::Compactor::STANDARD - Don't split items unnecessarily.
     *     - Xapian::Compactor::FULL     - Split items whenever it saves space
     *					   (the default).
     *     - Xapian::Compactor::FULLER   - Allow oversize items to save more
     *					   space (not recommended if you ever
     *					   plan to update the compacted
     *					   database).
     *
     *  @param block_size	This specifies the block size (in bytes) for to
     *				use for the output.  For glass, the block size
     *				must be a power of 2 between 2048 and 65536
     *				(inclusive), and the default (also used if an
     *				invalid value is passed) is 8192 bytes.
     *
     *  @param compactor Functor
     */
    void compact(int fd,
		 unsigned flags,
		 int block_size,
		 Xapian::Compactor& compactor)
    {
	compact_(NULL, fd, flags, block_size, &compactor);
    }

    /** Reconstruct document text.
     *
     *  This uses term positional information to reconstruct the document text
     *  which was indexed.  Reading the required positional information is
     *  potentially quite I/O intensive.
     *
     *  The reconstructed text will be missing punctuation and most
     *  capitalisation.
     *
     *  @param did	  The document id of the document to reconstruct
     *  @param length	  Number of bytes of text to aim for - note that
     *			  slightly more may be returned (default: 0 meaning
     *			  unlimited)
     *  @param prefix	  Term prefix to reconstruct (default: none)
     *  @param start_pos  First position to reconstruct (default: 0)
     *  @param end_pos    Last position to reconstruct (default: 0 meaning all)
     */
    std::string reconstruct_text(Xapian::docid did,
				 size_t length = 0,
				 const std::string& prefix = std::string(),
				 Xapian::termpos start_pos = 0,
				 Xapian::termpos end_pos = 0) const;
};

/** This class provides read/write access to a database.
 *
 *  A WritableDatabase object contains zero or more shards, and operations are
 *  performed across these shards.  Documents added by add_document() are
 *  stored to the shards in a round-robin fashion.
 *
 *  @since 1.5.0 This class is a reference counted handle like many other
 *	   Xapian API classes.  In earlier versions, it worked like a typedef
 *	   to std::vector<database_shard>.  The key difference is that
 *	   previously copying or assigning a Xapian::Database made a deep copy,
 *	   whereas now it makes a shallow copy.
 *
 *  Most methods can throw:
 *
 *  @exception Xapian::DatabaseCorruptError if database corruption is detected
 *  @exception Xapian::DatabaseError in various situation (for example, calling
 *	       methods after @a close() has been called)
 *  @exception Xapian::NetworkError when remote databases are in use
 */
class XAPIAN_VISIBILITY_DEFAULT WritableDatabase : public Database {
    /** @internal @private Helper method which implements cancel_transaction()
     *  and commit_transaction().
     *
     *  @param do_commit If true, then commit, else cancel.
     */
    void end_transaction_(bool do_commit);

  public:
    /** Create a WritableDatabase with no subdatabases.
     *
     *  The created object isn't very useful in this state - it's intended
     *  as a placeholder value.
     */
    WritableDatabase() : Database() {}

    /** Add shards from another WritableDatabase.
     *
     *  Any shards in @a other are added to the list of shards in this object.
     *  The shards are reference counted and also remain in @a other.
     *
     *  @param other	Another WritableDatabase to add shards from
     *
     *  @exception Xapian::InvalidArgumentError if @a other is the same object
     *		   as this.
     */
    void add_database(const WritableDatabase& other) {
	// This method is provided mainly so that adding a Database to a
	// WritableDatabase is a compile-time error - prior to 1.5.0, it
	// would essentially act as a "black-hole" shard which discarded
	// any changes made to it.
	add_database_(other, false);
    }

    /** Create or open a Xapian database for both reading and writing.
     *
     *  @param path  Filing system path for the database.  If creating a
     *		     new database with a backend which uses a directory of
     *		     files (such as glass does by default) then Xapian will
     *		     create a directory for @a path if necessary (but the
     *		     parent directory must already exist).
     *
     *  @param flags  A bitwise-or (| in C++) combination of:
     *
     *  * at most one of the following constants indicating how to handle
     *	  the database already existing or not (the default action is
     *	  Xapian::DB_CREATE_OR_OPEN):
     *
     *	    Constant                     | DB exists | DB doesn't exist
     *	  ------------------------------ | --------- | ------------------
     *	  Xapian::DB_CREATE_OR_OPEN      | open      | create
     *	  Xapian::DB_CREATE              | fail      | create
     *	  Xapian::DB_CREATE_OR_OVERWRITE | overwrite | create
     *	  Xapian::DB_OPEN                | open      | fail
     *
     *  * at most one of the follow constants indicating which backend to
     *    use when creating a new database, ignored when opening or overwriting
     *    an existing database (default: currently Xapian::DB_BACKEND_GLASS):
     *
     *	    Constant                     | Meaning
     *	  ------------------------------ | -----------------------
     *	  Xapian::DB_BACKEND_GLASS	 | Create a glass database
     *	  Xapian::DB_BACKEND_CHERT       | Create a chert database
     *	  Xapian::DB_BACKEND_INMEMORY    | Create inmemory DB (ignores @a path)
     *
     *  * any number of the following flags:
     *
     *   - Xapian::DB_NO_SYNC don't call fsync() or similar
     *   - Xapian::DB_FULL_SYNC try harder to ensure data is safe
     *   - Xapian::DB_DANGEROUS don't be crash-safe, no concurrent readers
     *   - Xapian::DB_RETRY_LOCK to wait to get a write lock
     *
     *  @param block_size  The block size in bytes to use when creating a
     *			   new database.  This is ignored when opening an
     *			   existing database, and by backends which don't
     *			   have the concept of a block size.  The glass
     *			   backend allows block sizes which are a power of
     *			   2 between 2048 and 65536 (inclusive) and its
     *			   default (also used instead of an invalid value)
     *			   is 8192 bytes.
     *
     *  @exception Xapian::DatabaseLockError is thrown if the database's
     *		   write lock could not be acquired.
     *  @exception Xapian::DatabaseOpeningError if the specified database
     *		   cannot be opened
     *  @exception Xapian::DatabaseVersionError if the specified database has
     *		   a format too old or too new to be supported.
     */
    explicit WritableDatabase(const std::string& path,
			      int flags = 0,
			      int block_size = 0);

    /** @private @internal Create a WritableDatabase given its internals. */
    XAPIAN_VISIBILITY_INTERNAL
    explicit WritableDatabase(Database::Internal* internal_)
	: Database(internal_) {}

    /** Copy constructor.
     *
     *  The internals are reference counted, so copying is cheap.
     */
    WritableDatabase(const WritableDatabase& o) : Database(o) {}

    /** Assignment operator.
     *
     *  The internals are reference counted, so assignment is cheap.
     */
    WritableDatabase& operator=(const WritableDatabase& o) {
	Database::operator=(o);
	return *this;
    }

    /// Move constructor.
    WritableDatabase(WritableDatabase&& o) : Database(std::move(o)) {}

    /// Move assignment operator.
    WritableDatabase& operator=(WritableDatabase&& o) {
	Database::operator=(std::move(o));
	return *this;
    }

    /** Commit pending modifications.
     *
     *  Updates to a Xapian database are more efficient when applied in bulk,
     *  so by default Xapian stores modifications in memory until a threshold
     *  is exceeded and then they are committed to disk.
     *
     *  When the database is closed (by an explicit call to close() or its
     *  destructor being called) then commit() is implicitly called unless
     *  a transaction is active.
     *
     *  You can force any such pending modifications to be committed by calling
     *  this method, but bear in mind that the batching happens for a reason
     *  and calling commit() a lot is likely to slow down indexing.
     *
     *  If the commit operation succeeds then the changes are reliably written
     *  to disk and available to readers.  If the commit operation fails, then
     *  any pending modifications are discarded.
     *
     *  It's not valid to call commit() within a transaction - see
     *  begin_transaction() for more details of how transactions work in
     *  Xapian.
     *
     *  Currently batched modifications are automatically committed every
     *  10000 documents added, deleted, or modified.  This value is rather
     *  conservative, and if you have a machine with plenty of memory,
     *  you can improve indexing throughput dramatically by setting
     *  XAPIAN_FLUSH_THRESHOLD in the environment to a larger value.
     *
     *  @since This method was new in Xapian 1.1.0 - in earlier versions it
     *	       was called flush().
     */
    void commit();

    /** Begin a transaction.
     *
     *  A Xapian transaction is a set of consecutive modifications to be
     *  committed as an atomic unit - in any committed revision of the
     *  database either none are present or they all are.
     *
     *  A transaction is started with begin_transaction() and can either be
     *  completed by calling commit_transaction() or aborted by calling
     *  cancel_transaction().
     *
     *  Closing the database (by an explicit call to close() or by its
     *  destructor being called) when a transaction is active will implicitly
     *  call cancel_transaction() to abort the transaction and discard the
     *  changes in it.
     *
     *  By default, commit() is implicitly called by begin_transaction() and
     *  commit_transaction() so that the changes in the transaction are
     *  committed or not independent of changes before or after it.
     *
     *  The downside of these implicit calls to commit() is that small
     *  transactions can harm indexing performance in the same way that
     *  explicitly calling commit() frequently can.
     *
     *  If you're applying atomic groups of changes and only wish to ensure
     *  that each group is either applied or not applied, then you can prevent
     *  the automatic commit() before and after the transaction by starting the
     *  transaction with begin_transaction(false).  However, if
     *  cancel_transaction() is called (or if commit_transaction() isn't called
     *  before the WritableDatabase object is destroyed) then any changes which
     *  were pending before the transaction began will also be discarded.
     *
     *  @param flushed	Is this a flushed transaction?  By default transactions
     *			are "flushed", which means that committing a
     *			transaction will ensure those changes are permanently
     *			written to the database.  By contrast, unflushed
     *			transactions only ensure that changes within the
     *			transaction are either all applied or all aren't.
     *
     *  @exception Xapian::UnimplementedError is thrown if this is an InMemory
     *		   database, which don't currently support transactions.
     *  @exception Xapian::InvalidOperationError will be thrown if a transaction
     *		   is already active.
     */
    void begin_transaction(bool flushed = true);

    /** Complete the transaction currently in progress.
     *
     *  If the transaction was begun as a flushed transaction then the changes
     *  in it have been committed to the database upon successful completion
     *  of this method.
     *
     *  If an exception is thrown, then the changes in the transaction will be
     *  discarded (if the transaction was not begun as a flushed transaction,
     *  any changes made but not committed before begin_transaction() will also
     *  be discarded).
     *
     *  In all cases the transaction will no longer be in progress.
     *
     *  @exception Xapian::UnimplementedError is thrown if this is an InMemory
     *		   database, which don't currently support transactions.
     *  @exception Xapian::InvalidOperationError is thrown if no transaction
     *             was active.
     */
    void commit_transaction() { end_transaction_(true); }

    /** Abort the transaction currently in progress.
     *
     *  Changes made within the current transaction will be discarded (if the
     *  transaction was not begun as a flushed transaction, any changes made
     *  but not committed before begin_transaction() will also be discarded).
     *
     *  @exception Xapian::UnimplementedError is thrown if this is an InMemory
     *		   database, which don't currently support transactions.
     *  @exception Xapian::InvalidOperationError is thrown if no transaction
     *             was active.
     */
    void cancel_transaction() { end_transaction_(false); }

    /** Add a document to the database.
     *
     *  The document is allocated document ID (get_lastdocid() + 1) - the
     *  next highest document ID which has never previously been used by
     *  this database (so docids from deleted documents won't be reused).
     *
     *  If you want to specify the document ID to be used, you should
     *  call replace_document() instead.
     *
     *  If a transaction is active, the document addition is added to the
     *  transaction; otherwise it is added to the current batch of changes.
     *  Either way, it won't be visible to readers right away (unless we're
     *  not in a transaction and the addition triggers an automatic commit).
     *
     *  @param doc  The Document object to be added.
     *
     *  @return The document ID allocated to the document.
     */
    Xapian::docid add_document(const Xapian::Document& doc);

    /** Delete a document from the database.
     *
     *  This method removes the document with the specified document ID
     *  from the database.
     *
     *  If a transaction is active, the document removal is added to the
     *  transaction; otherwise it is added to the current batch of changes.
     *  Either way, it won't be visible to readers right away (unless we're
     *  not in a transaction and the addition triggers an automatic commit).
     *
     *  @param did     The document ID of the document to be removed.
     */
    void delete_document(Xapian::docid did);

    /** Delete any documents indexed by a term from the database.
     *
     *  This method removes any documents indexed by the specified term
     *  from the database.
     *
     *  A major use is for convenience when UIDs from another system are
     *  mapped to terms in Xapian, although this method has other uses
     *  (for example, you could add a "deletion date" term to documents at
     *  index time and use this method to delete all documents due for
     *  deletion on a particular date).
     *
     *  @param unique_term     The term to remove references to.
     *
     *  @since 1.5.0 The changes made by this method are made atomically.
     *		     Previously automatic commits could happen during the
     *		     batch.
     */
    void delete_document(const std::string& unique_term);

    /** Replace a document in the database.
     *
     *  This method replaces the document with the specified document ID.
     *  If document ID @a did isn't currently used, the document will be
     *  added with document ID @a did.
     *
     *  The monotonic counter used for automatically allocating document
     *  IDs is increased so that the next automatically allocated document
     *  ID will be did + 1.  Be aware that if you use this method to
     *  specify a high document ID for a new document, and also use
     *  WritableDatabase::add_document(), Xapian may get to a state where
     *  this counter wraps around and will be unable to automatically
     *  allocate document IDs!
     *
     *  Note that changes to the database won't be immediately committed to
     *  disk; see commit() for more details.
     *
     *  @param did      The document ID of the document to be replaced.
     *  @param document The new document.
     */
    void replace_document(Xapian::docid did, const Xapian::Document& document);

    /** Replace any documents matching a term.
     *
     *  This method replaces any documents indexed by the specified term
     *  with the specified document.  If any documents are indexed by the
     *  term, the lowest document ID will be used for the document,
     *  otherwise a new document ID will be generated as for add_document.
     *
     *  One common use is to allow UIDs from another system to easily be
     *  mapped to terms in Xapian.  Note that this method doesn't
     *  automatically add unique_term as a term, so you'll need to call
     *  document.add_term(unique_term) first when using replace_document()
     *  in this way.
     *
     *  Note that changes to the database won't be immediately committed to
     *  disk; see commit() for more details.
     *
     *  @param unique_term	The "unique" term.
     *  @param document		The new document.
     *
     *  @return The document ID used by the new document.  If term existed
     *		in the database, this will be the first document ID that
     *		was indexed by that term; otherwise the database allocates
     *		(get_lastdocid() + 1) as it does for add_document().
     *
     *  @since 1.5.0 The changes made by this method are made atomically.
     *		     Previously automatic commits could happen during the
     *		     batch.
     */
    Xapian::docid replace_document(const std::string& unique_term,
				   const Xapian::Document& document);

    /** Add a word to the spelling dictionary.
     *
     *  If the word is already present, its frequency is increased.
     *
     *  @param word	The word to add.
     *  @param freqinc	How much to increase its frequency by (default 1).
     */
    void add_spelling(const std::string& word,
		      Xapian::termcount freqinc = 1) const;

    /** Remove a word from the spelling dictionary.
     *
     *  The word's frequency is decreased, and if would become zero or less
     *  then the word is removed completely.
     *
     *  @param word	The word to remove.
     *  @param freqdec	How much to decrease its frequency by (default 1).
     *
     *  @return Any "unused" freqdec (if the word's frequency was less than
     *		freqdec then the difference is returned, else 0 is returned).
     *		Prior to 1.5.0 this method had void return type.
     */
    termcount remove_spelling(const std::string& word,
			      termcount freqdec = 1) const;

    /** Add a synonym for a term.
     *
     *  @param term	The term to add a synonym for.
     *  @param synonym	The synonym to add.  If this is already a synonym for
     *			@a term, then no action is taken.
     */
    void add_synonym(const std::string& term,
		     const std::string& synonym) const;

    /** Remove a synonym for a term.
     *
     *  @param term	The term to remove a synonym for.
     *  @param synonym	The synonym to remove.  If this isn't currently a
     *			synonym for @a term, then no action is taken.
     */
    void remove_synonym(const std::string& term,
			const std::string& synonym) const;

    /** Remove all synonyms for a term.
     *
     *  @param term	The term to remove all synonyms for.  If the term has
     *			no synonyms, no action is taken.
     */
    void clear_synonyms(const std::string& term) const;

    /** Set the user-specified metadata associated with a given key.
     *
     *  This method sets the metadata value associated with a given key.  If
     *  there is already a metadata value stored in the database with the same
     *  key, the old value is replaced.  If you want to delete an existing item
     *  of metadata, just set its value to the empty string.
     *
     *  User-specified metadata allows you to store arbitrary information in
     *  the form of (key, value) pairs.
     *
     *  There's no hard limit on the number of metadata items, or the size of
     *  the metadata values.  Metadata keys have a limited length, which depend
     *  on the backend.  We recommend limiting them to 200 bytes.  Empty keys
     *  are not valid, and specifying one will cause an exception.
     *
     *  Metadata modifications are committed to disk in the same way as
     *  modifications to the documents in the database are: i.e., modifications
     *  are atomic, and won't be committed to disk immediately (see commit()
     *  for more details).  This allows metadata to be used to link databases
     *  with versioned external resources by storing the appropriate version
     *  number in a metadata item.
     *
     *  You can also use the metadata to store arbitrary extra information
     *  associated with terms, documents, or postings by encoding the termname
     *  and/or document id into the metadata key.
     *
     *  @param key       The key of the metadata item to set.
     *
     *  @param metadata  The value of the metadata item to set.
     *
     *  @exception Xapian::DatabaseError will be thrown if a problem occurs
     *             while writing to the database.
     *
     *  @exception Xapian::DatabaseCorruptError will be thrown if the database
     *		   is in a corrupt state.
     *
     *  @exception Xapian::InvalidArgumentError will be thrown if the key
     *		   supplied is empty.
     *
     *  @exception Xapian::UnimplementedError will be thrown if the database
     *		   backend in use doesn't support user-specified metadata.
     */
    void set_metadata(const std::string& key, const std::string& metadata);

    /// Return a string describing this object.
    std::string get_description() const;
};

}

#endif // XAPIAN_INCLUDED_DATABASE_H
