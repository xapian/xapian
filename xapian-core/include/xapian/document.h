/** @file document.h
 * @brief Class representing a document
 */
/* Copyright (C) 2010,2015,2016,2017,2018,2019 Olly Betts
 * Copyright 2009 Lemur Consulting Ltd
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

#ifndef XAPIAN_INCLUDED_DOCUMENT_H
#define XAPIAN_INCLUDED_DOCUMENT_H

#if !defined XAPIAN_IN_XAPIAN_H && !defined XAPIAN_LIB_BUILD
# error Never use <xapian/document.h> directly; include <xapian.h> instead.
#endif

#include <string>

#include <xapian/attributes.h>
#include <xapian/intrusive_ptr.h>
#include <xapian/termiterator.h>
#include <xapian/types.h>
#include <xapian/valueiterator.h>
#include <xapian/visibility.h>

namespace Xapian {

/** Class representing a document.
 *
 *  The term "document" shouldn't be taken too literally - really it's a "thing
 *  to retrieve", as the list of search results is essentially a list of
 *  documents.
 *
 *  Document objects fetch information from the database lazily.  Usually
 *  this behaviour isn't visible to users (except for the speed benefits), but
 *  if the document in the database is modified or deleted then preexisting
 *  Document objects may return the old or new versions of data (or throw
 *  Xapian::DocNotFoundError in the case of deletion).
 *
 *  Since Database objects work on a snapshot of the database's state, the
 *  situation above can only happen with a WritableDatabase object, or if
 *  you call Database::reopen() on the Database object which you got the
 *  Document from.
 *
 *  We recommend you avoid designs where this behaviour is an issue, but if
 *  you need a way to make a non-lazy version of a Document object, you can do
 *  this like so:
 *
 *      doc = Xapian::Document::unserialise(doc.serialise());
 */
class XAPIAN_VISIBILITY_DEFAULT Document {
  public:
    /// Class representing the Document internals.
    class Internal;
    /// @private @internal Reference counted internals.
    Xapian::Internal::intrusive_ptr_nonnull<Internal> internal;

    /// @private @internal Wrap an existing Internal.
    XAPIAN_VISIBILITY_INTERNAL
    explicit Document(Internal*);

    /** Copy constructor.
     *
     *  The internals are reference counted, so copying is cheap.
     */
    Document(const Document& o);

    /** Assignment operator.
     *
     *  The internals are reference counted, so assignment is cheap.
     */
    Document& operator=(const Document& o);

    /// Move constructor.
    Document(Document&& o);

    /// Move assignment operator.
    Document& operator=(Document&& o);

    /** Default constructor.
     *
     *  Creates an empty Document.
     */
    Document();

    /// Destructor.
    ~Document();

    /** Get the document ID this document came from.
     *
     *  If this document didn't come from a database, this will be 0 (in Xapian
     *  1.0.22/1.2.4 or later; prior to this the returned value was uninitialised
     *  in this case).
     *
     *  Note that if the document came from a sharded database, this is the docid
     *  in the shard it came from, not the docid in the combined database.
     */
    Xapian::docid get_docid() const;

    /// Get the document data.
    std::string get_data() const;

    /// Set the document data.
    void set_data(const std::string& data);

    /// Add a term to this document.
    void add_term(const std::string& term, Xapian::termcount wdf_inc = 1);

    /** Add a boolean filter term to the document.
     *
     *  This method adds @a term to the document with wdf of 0 -
     *  this is generally what you want for a term used for boolean
     *  filtering as the wdf of such terms is ignored, and it doesn't
     *  make sense for them to contribute to the document's length.
     *
     *  If the specified term already indexes this document, this method
     *  has no effect.
     *
     *  It is exactly the same as add_term(term, 0) and is provided as a
     *  way to make a common operation more explicit.
     *
     *  This method was added in Xapian 1.0.18.
     *
     *  @param term		The term to add.
     */
    void add_boolean_term(const std::string& term) { add_term(term, 0); }

    /// Remove a term from this document.
    void remove_term(const std::string& term);

    /// Add a posting for a term.
    void add_posting(const std::string& term,
		     Xapian::termpos term_pos,
		     Xapian::termcount wdf_inc = 1);

    /** Remove posting for a term.
     *
     *  The instance of the specified term at position term_pos will be
     *  removed, and the @a wdf reduced by @a wdf_dec (the wdf will not
     *  ever go below zero though - the resultant wdf is clamped to zero
     *  if it would).
     *
     *  If the term doesn't occur at position term_pos then
     *  Xapian::InvalidArgumentError is thrown.  If you want to remove a single
     *  position which may not be present without triggering an exception you
     *  can call <code>remove_postings(term, pos, pos)</code> instead.
     *
     *  Since 1.5.0, if the final position is removed and the wdf becomes zero
     *  then the term will be removed from the document.
     */
    void remove_posting(const std::string& term,
			Xapian::termpos term_pos,
			Xapian::termcount wdf_dec = 1);

    /** Remove a range of postings for a term.
     *
     *  Any instances of the term at positions >= @a term_pos_first and
     *  <= @a term_pos_last will be removed, and the wdf reduced by
     *  @a wdf_dec for each instance removed (the wdf will not ever go
     *  below zero though - the resultant wdf is clamped to zero if it would).
     *
     *  If the term doesn't occur in the range of positions specified (including
     *  if term_pos_first > term_pos_last) then this method does nothing (unlike
     *  @a remove_posting() which throws an exception if the specified position
     *  is not present).
     *
     *  Since 1.5.0, if all remaining positions are removed and the wdf becomes
     *  zero then the term will be removed from the document.  Note that this
     *  only happens if some positions are removed though - calling this method
     *  on a term which has no positions and zero wdf won't remove that term.
     *
     *  @return The number of postings removed.
     *
     *  @since Added in Xapian 1.4.8.
     */
    Xapian::termpos remove_postings(const std::string& term,
				    Xapian::termpos term_pos_first,
				    Xapian::termpos term_pos_last,
				    Xapian::termcount wdf_dec = 1);

    /// Clear all terms from the document.
    void clear_terms();

    /// Return the number of distinct terms in this document.
    Xapian::termcount termlist_count() const;

    /** Start iterating the terms in this document.
     *
     *  The terms are returned ascending string order (by byte value).
     */
    TermIterator termlist_begin() const;

    /// End iterator corresponding to @a termlist_begin().
    TermIterator XAPIAN_NOTHROW(termlist_end() const) {
	return TermIterator();
    }

    /** Read a value slot in this document.
     *
     *  @param slot	The slot to read the value from
     *
     *  @return The value in slot @a slot, or an empty string if not set.
     */
    std::string get_value(Xapian::valueno slot) const;

    /** Add a value to a slot in this document.
     *
     *  @param slot	The slot to set
     *  @param value	The new value
     */
    void add_value(Xapian::valueno slot, const std::string& value);

    /** Remove any value from the specified slot.
     *
     *  @param slot	The slot to remove any value from.
     */
    void remove_value(Xapian::valueno slot) {
	add_value(slot, std::string());
    }

    /// Clear all value slots in this document.
    void clear_values();

    /// Count the value slots used in this document.
    Xapian::valueno values_count() const;

    /** Start iterating the values in this document.
     *
     *  The values are returned in ascending numerical slot order.
     */
    ValueIterator values_begin() const;

    /// End iterator corresponding to @a values_begin().
    ValueIterator XAPIAN_NOTHROW(values_end() const) {
	return ValueIterator();
    }

    /** Efficiently swap this Document object with another. */
    void swap(Document& o) { internal.swap(o.internal); }

    /** Serialise document into a string.
     *
     *  The document representation may change between Xapian releases: even
     *  between minor versions.  However, it is guaranteed not to change if the
     *  remote database protocol has not changed between releases.
     */
    std::string serialise() const;

    /** Unserialise a document from a string produced by serialise(). */
    static Document unserialise(const std::string& serialised);

    /// Return a string describing this object.
    std::string get_description() const;
};

}

#endif // XAPIAN_INCLUDED_DOCUMENT_H
