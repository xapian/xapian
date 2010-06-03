/** \file  stem.h
 *  \brief stemming algorithms
 */
/* Copyright (C) 2005,2007 Olly Betts
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

#ifndef XAPIAN_INCLUDED_STEM_H
#define XAPIAN_INCLUDED_STEM_H

#include <xapian/base.h>
#include <xapian/visibility.h>

#include <string>

namespace Xapian {

/// Class representing a stemming algorithm.
class XAPIAN_VISIBILITY_DEFAULT Stem {
  public:
    /// @private @internal Class representing the stemmer internals.
    class Internal;
    /// @private @internal Reference counted internals.
    Xapian::Internal::RefCntPtr<Internal> internal;

    /// Copy constructor.
    Stem(const Stem & o);

    /// Assignment.
    void operator=(const Stem & o);

    /** Construct a Xapian::Stem object which doesn't change terms.
     *
     *  Equivalent to Stem("none").
     */
    Stem();

    /** Construct a Xapian::Stem object for a particular language.
     *
     *  @param language	Either the English name for the language
     *			or the two letter ISO639 code.
     *
     *  The following language names are understood (aliases follow the
     *  name):
     *
     *  - none - don't stem terms
     *  - danish (da)
     *  - dutch (nl)
     *  - english (en) - Martin Porter's 2002 revision of his stemmer
     *  - english_lovins (lovins) - Lovin's stemmer
     *  - english_porter (porter) - Porter's stemmer as described in
     *			his 1980 paper
     *  - finnish (fi)
     *  - french (fr)
     *  - german (de)
     *  - german2 - Normalises umlauts and &szlig;
     *  - hungarian (hu)
     *  - italian (it)
     *  - kraaij_pohlmann - A different Dutch stemmer
     *  - norwegian (nb, nn, no)
     *  - portuguese (pt)
     *  - romanian (ro)
     *  - russian (ru)
     *  - spanish (es)
     *  - swedish (sv)
     *  - turkish (tr)
     *
     *  @exception		Xapian::InvalidArgumentError is thrown if
     *			language isn't recognised.
     */
    explicit Stem(const std::string &language);

    /// Destructor.
    ~Stem();

    /** Stem a word.
     *
     *  @param word		a word to stem.
     *  @return		the stem
     */
    std::string operator()(const std::string &word) const;

    /// Return a string describing this object.
    std::string get_description() const;

    /** Return a list of available languages.
     *
     *  Each stemmer is only included once in the list (not once for
     *  each alias).  The name included is the English name of the
     *  language.
     *
     *  The list is returned as a string, with language names separated by
     *  spaces.  This is a static method, so a Xapian::Stem object is not
     *  required for this operation.
     */
    static std::string get_available_languages();
};

}

#endif // XAPIAN_INCLUDED_STEM_H
