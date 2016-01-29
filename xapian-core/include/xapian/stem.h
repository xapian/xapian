/** @file stem.h
 * @brief stemming algorithms
 */
/* Copyright (C) 2005,2007,2010,2011,2013,2014,2015 Olly Betts
 * Copyright (C) 2010 Evgeny Sizikov
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

#if !defined XAPIAN_IN_XAPIAN_H && !defined XAPIAN_LIB_BUILD
# error "Never use <xapian/stem.h> directly; include <xapian.h> instead."
#endif

#include <xapian/constinfo.h>
#include <xapian/intrusive_ptr.h>
#include <xapian/visibility.h>

#include <string>

namespace Xapian {

/// Class representing a stemming algorithm implementation.
class XAPIAN_VISIBILITY_DEFAULT StemImplementation
    : public Xapian::Internal::intrusive_base
{
    /// Don't allow assignment.
    void operator=(const StemImplementation &);

    /// Don't allow copying.
    StemImplementation(const StemImplementation &);

  public:
    /// Default constructor.
    StemImplementation() { }

    /// Virtual destructor.
    virtual ~StemImplementation();

    /// Stem the specified word.
    virtual std::string operator()(const std::string & word) = 0;

    /// Return a string describing this object.
    virtual std::string get_description() const = 0;
};

/// Class representing a stemming algorithm.
class XAPIAN_VISIBILITY_DEFAULT Stem {
  public:
    /// @private @internal Reference counted internals.
    Xapian::Internal::intrusive_ptr<StemImplementation> internal;

    /// Copy constructor.
    Stem(const Stem & o);

    /// Assignment.
    Stem & operator=(const Stem & o);

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
     *  - armenian (hy)
     *  - basque (eu)
     *  - catalan (ca)
     *  - danish (da)
     *  - dutch (nl)
     *  - english (en) - Martin Porter's 2002 revision of his stemmer
     *  - earlyenglish - Early English (e.g. Shakespeare, Dickens) stemmer
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
     *  @exception	Xapian::InvalidArgumentError is thrown if
     *			language isn't recognised.
     */
    explicit Stem(const std::string &language);

    /** Construct a Xapian::Stem object with a user-provided stemming algorithm.
     *
     *  You can subclass Xapian::StemImplementation to implement your own
     *  stemming algorithm (or to wrap a third-party algorithm) and then wrap
     *  your implementation in a Xapian::Stem object to pass to the Xapian API.
     *
     *  @param p	The user-subclassed StemImplementation object.  This
     *			is reference counted, and so will be automatically
     *			deleted by the Xapian::Stem wrapper when no longer
     *			required.
     */
    explicit Stem(StemImplementation * p);

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
    static std::string get_available_languages() {
	const struct Xapian::Internal::constinfo * info =
	    Xapian::Internal::get_constinfo_();
	return std::string(info->stemmer_data, info->stemmer_name_len);
    }
};

}

#endif // XAPIAN_INCLUDED_STEM_H
