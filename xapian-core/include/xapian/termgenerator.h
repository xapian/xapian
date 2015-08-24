/** @file termgenerator.h
 * @brief parse free text and generate terms
 */
/* Copyright (C) 2007,2009,2011,2012 Olly Betts
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
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

#ifndef XAPIAN_INCLUDED_TERMGENERATOR_H
#define XAPIAN_INCLUDED_TERMGENERATOR_H

#include <xapian/base.h>
#include <xapian/types.h>
#include <xapian/unicode.h>
#include <xapian/visibility.h>

#include <string>

namespace Xapian {

class Document;
class Stem;
class Stopper;
class WritableDatabase;

/** Parses a piece of text and generate terms.
 *
 * This module takes a piece of text and parses it to produce words which are
 * then used to generate suitable terms for indexing.  The terms generated are
 * suitable for use with Query objects produced by the QueryParser class.
 */
class XAPIAN_VISIBILITY_DEFAULT TermGenerator {
  public:
    /// @private @internal Class representing the TermGenerator internals.
    class Internal;
    /// @private @internal Reference counted internals.
    Xapian::Internal::RefCntPtr<Internal> internal;

    /// Copy constructor.
    TermGenerator(const TermGenerator & o);

    /// Assignment.
    TermGenerator & operator=(const TermGenerator & o);

    /// Default constructor.
    TermGenerator();

    /// Destructor.
    ~TermGenerator();

    /// Set the Xapian::Stem object to be used for generating stemmed terms.
    void set_stemmer(const Xapian::Stem & stemmer);

    /** Set the Xapian::Stopper object to be used for identifying stopwords.
     *
     *  Stemmed forms of stopwords aren't indexed, but unstemmed forms still
     *  are so that searches for phrases including stop words still work.
     *
     *  @param stop	The Stopper object to set (default NULL, which means no
     *			stopwords).
     */
    void set_stopper(const Xapian::Stopper *stop = NULL);

    /// Set the current document.
    void set_document(const Xapian::Document & doc);

    /// Get the current document.
    const Xapian::Document & get_document() const;

    /// Set the database to index spelling data to.
    void set_database(const Xapian::WritableDatabase &db);

    /// Flags to OR together and pass to TermGenerator::set_flags().
    enum flags {
	/// Index data required for spelling correction.
	FLAG_SPELLING = 128 // Value matches QueryParser flag.
    };

    /// Stemming strategies, for use with set_stemming_strategy().
    typedef enum { STEM_NONE, STEM_SOME, STEM_ALL, STEM_ALL_Z } stem_strategy;

    /** Set flags.
     *
     *  The new value of flags is: (flags & mask) ^ toggle
     *
     *  To just set the flags, pass the new flags in toggle and the
     *  default value for mask.
     *
     *  @param toggle	Flags to XOR.
     *  @param mask	Flags to AND with first.
     *
     *  @return		The old flags setting.
     */
    flags set_flags(flags toggle, flags mask = flags(0));

    /** Set the stemming strategy.
     *
     *  This method controls how the stemming algorithm is applied.  It was
     *  new in Xapian 1.3.1.
     *
     *  @param strategy	The strategy to use - possible values are:
     *   - STEM_NONE:	Don't perform any stemming - only unstemmed terms
     *			are generated.
     *   - STEM_SOME:	Generate both stemmed (with a "Z" prefix) and unstemmed
     *			terms.  This is the default strategy.
     *   - STEM_ALL:	Generate only stemmed terms (but without a "Z" prefix).
     *   - STEM_ALL_Z:	Generate only stemmed terms (with a "Z" prefix).
     */
    void set_stemming_strategy(stem_strategy strategy);

    /** Set the maximum length word to index.
     *
     *  The limit is on the length of a word prior to stemming and prior to
     *  adding any term prefix.
     *
     *  The backends mostly impose a limit on the length of terms (often of
     *  about 240 bytes), but it's generally useful to have a lower limit to
     *  help prevent the index being bloated by useless junk terms from trying
     *  to indexing things like binary data, uuencoded data, ASCII art, etc.
     *
     *  This method was new in Xapian 1.3.1.
     *
     *  @param max_word_length	The maximum length word to index, in bytes in
     *				UTF-8 representation.  Default is 64.
     */
    void set_max_word_length(unsigned max_word_length);

    /** Index some text.
     *
     * @param itor	Utf8Iterator pointing to the text to index.
     * @param wdf_inc	The wdf increment (default 1).
     * @param prefix	The term prefix to use (default is no prefix).
     */
    void index_text(const Xapian::Utf8Iterator & itor,
		    Xapian::termcount wdf_inc = 1,
		    const std::string & prefix = std::string());

    /** Index some text in a std::string.
     *
     * @param text	The text to index.
     * @param wdf_inc	The wdf increment (default 1).
     * @param prefix	The term prefix to use (default is no prefix).
     */
    void index_text(const std::string & text,
		    Xapian::termcount wdf_inc = 1,
		    const std::string & prefix = std::string()) {
	return index_text(Utf8Iterator(text), wdf_inc, prefix);
    }

    /** Index some text without positional information.
     *
     * Just like index_text, but no positional information is generated.  This
     * means that the database will be significantly smaller, but that phrase
     * searching and NEAR won't be supported.
     *
     * @param itor	Utf8Iterator pointing to the text to index.
     * @param wdf_inc	The wdf increment (default 1).
     * @param prefix	The term prefix to use (default is no prefix).
     */
    void index_text_without_positions(const Xapian::Utf8Iterator & itor,
				      Xapian::termcount wdf_inc = 1,
				      const std::string & prefix = std::string());

    /** Index some text in a std::string without positional information.
     *
     * Just like index_text, but no positional information is generated.  This
     * means that the database will be significantly smaller, but that phrase
     * searching and NEAR won't be supported.
     *
     * @param text	The text to index.
     * @param wdf_inc	The wdf increment (default 1).
     * @param prefix	The term prefix to use (default is no prefix).
     */
    void index_text_without_positions(const std::string & text,
				      Xapian::termcount wdf_inc = 1,
				      const std::string & prefix = std::string()) {
	return index_text_without_positions(Utf8Iterator(text), wdf_inc, prefix);
    }

    /** Increase the term position used by index_text.
     *
     *  This can be used between indexing text from different fields or other
     *  places to prevent phrase searches from spanning between them (e.g.
     *  between the title and body text, or between two chapters in a book).
     *
     *  @param delta	Amount to increase the term position by (default: 100).
     */
    void increase_termpos(Xapian::termcount delta = 100);

    /// Get the current term position.
    Xapian::termcount get_termpos() const;

    /** Set the current term position.
     *
     *  @param termpos	The new term position to set.
     */
    void set_termpos(Xapian::termcount termpos);

    /// Return a string describing this object.
    std::string get_description() const;
};

}

#endif // XAPIAN_INCLUDED_TERMGENERATOR_H
