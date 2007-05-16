/** @file termgenerator.h
 * @brief parse free text and generate terms
 */
/* Copyright (C) 2007 Olly Betts
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

/// Parse free text and generate terms.
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

    /// Set the stemmer.
    void set_stemmer(const Xapian::Stem & stemmer);

    /// Set the stopper.
    void set_stopper(const Xapian::Stopper *stop = NULL);

    /// Set the current document.
    void set_document(const Xapian::Document & doc);

    /// Get the current document.
    const Xapian::Document & get_document() const;

    /// Index some text.
    void index_text(const Xapian::Utf8Iterator & itor,
		    Xapian::termcount weight = 1,
		    const std::string & prefix = "");

    /// Index some text in a std::string.
    void index_text(const std::string & text,
		    Xapian::termcount weight = 1,
		    const std::string & prefix = "") {
	return index_text(Utf8Iterator(text), weight, prefix);
    }

    /// Index some text without positional information.
    void index_text_without_positions(const Xapian::Utf8Iterator & itor,
				      Xapian::termcount weight = 1,
				      const std::string & prefix = "");

    /// Index some text in a std::string without positional information.
    void index_text_without_positions(const std::string & text,
				      Xapian::termcount weight = 1,
				      const std::string & prefix = "") {
	return index_text_without_positions(Utf8Iterator(text), weight, prefix);
    }

    /** Increase the termpos used by index_text by @a delta.
     *
     *  This can be used to prevent phrase searches from spanning two
     *  unconnected blocks of text (e.g. the title and body text).
     */
    void increase_termpos(Xapian::termcount delta = 100);

    /// Get the current term position.
    Xapian::termcount get_termpos() const;

    /// Set the current term position.
    void set_termpos(Xapian::termcount termpos);

    /// Return a string describing this object.
    std::string get_description() const;
};

}

#endif // XAPIAN_INCLUDED_TERMGENERATOR_H
