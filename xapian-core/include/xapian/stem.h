/** \file  stem.h
 *  \brief stemming algorithms
 */
/* Copyright (C) 2005,2007 Olly Betts
 * Copyright (C) 2008 Lemur Consulting Ltd
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

/// Base representing a stemming algorithm.
class XAPIAN_VISIBILITY_DEFAULT StemBase : public Xapian::Internal::RefCntBase {
    /// No copying allowed.
    StemBase(const StemBase & o);

    /// No assignment allowed.
    void operator=(const StemBase & o);

  protected:
    /** Destructor is protected since it should only be called by subclasses
     *  and RefCntPtr.  Subclasses should make their destructors protected,
     *  to force users to use a RefCntPtr to reference them.
     */
    virtual ~StemBase() {}

    friend class Xapian::Internal::RefCntPtr<StemBase>;

  public:
    StemBase() {}

    /** Stem a word.
     *
     *  @param word  a word to stem.
     *  @return      the stemmed form of the word.
     */
    virtual std::string operator()(const std::string &word) const = 0;

    /// Return a string describing this object.
    virtual std::string get_description() const = 0;
};

/// Class representing one of the snowball stemming algorithms.
class XAPIAN_VISIBILITY_DEFAULT StemSnowball : public StemBase {
    /// No copying allowed.
    StemSnowball(const StemSnowball & o);

    /// No assignment allowed.
    void operator=(const StemSnowball & o);

  protected:
    /** Destructor is protected since it should only be called by subclasses
     *  and RefCntPtr.  Subclasses should make their destructors protected,
     *  to force users to use a RefCntPtr to reference them.
     */
    virtual ~StemSnowball();

  public:
    /// @private @internal Class representing the snowball stemmer internals.
    class Internal;

  private:
    /// @private @internal Snowball stemmer internals.
    Internal * internal;

  public:
    /** Construct a Xapian::StemSnowball object for a particular language.
     *
     *  @param language	Either the English name for the language
     *			or the two letter ISO639 code.
     *
     *  The following language names are understood (aliases follow the
     *  name):
     *
     *  - danish (da)
     *  - dutch (nl)
     *  - english (en) - Martin Porter's 2002 revision of his stemmer
     *  - english_lovins (lovins) - Lovin's stemmer
     *  - english_porter (porter) - Porter's stemmer as described in
     *			his 1980 paper
     *  - finnish (fi)
     *  - french (fr)
     *  - german (de)
     *  - italian (it)
     *  - norwegian (no)
     *  - portuguese (pt)
     *  - russian (ru)
     *  - spanish (es)
     *  - swedish (sv)
     *
     *  @exception		Xapian::InvalidArgumentError is thrown if
     *			language isn't recognised.
     */
    explicit StemSnowball(const std::string &language);

    /** Stem a word.
     *
     *  @param word  a word to stem.
     *  @return      the stemmed form of the word.
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

/// Class wrapping a reference counted stemming algorithm.
class XAPIAN_VISIBILITY_DEFAULT Stem {
  public:
    /// @private @internal Reference counted internals.
    Xapian::Internal::RefCntPtr<Xapian::StemBase> internal;

    /// Copy constructor.
    Stem(const Stem & o) : internal(o.internal) { }

    /// Assignment.
    void operator=(const Stem & o) { internal = o.internal; }

    /** Construct a Xapian::Stem object from a pointer to a StemBase.
     */
    Stem(Xapian::Internal::RefCntPtr<Xapian::StemBase> internal_)
	    : internal(internal_) {}

    /** Construct a Xapian::Stem object which doesn't change terms.
     */
    Stem() : internal(0) {}

    /** Construct a Xapian::Stem object for a particular language.
     *
     *  This constructor is included for convenience, and is equivalent to
     *  Stem(new StemSnowball(language)) - except that a language parameter of
     *  "none" will produce a stemmer which doesn't remove any stems.
     *
     *  See Xapian::StemSnowball for details.
     */
    explicit Stem(const std::string &language)
    {
	if (language == "none") 
	    internal = 0;
	else
	    internal = new Xapian::StemSnowball(language);
    }

    /** Stem a word.
     *
     *  @param word  a word to stem.
     *  @return      the stemmed form of the word.
     */
    std::string operator()(const std::string &word) const
    {
	if (!internal.get()) return word;
	return internal->operator()(word);
    }

    /// Return a string describing this object.
    std::string get_description() const
    {
	if (!internal.get()) return "Xapian::Stem()";
	return "Xapian::Stem(" + internal->get_description() + ")";
    }

    /** Return a list of available languages.
     *
     *  This is included for convenience, and is equivalent to
     *  StemSnowball.get_available_languages().
     *
     *  See Xapian::StemSnowball for details.
     */
    static std::string get_available_languages()
    {
	return StemSnowball::get_available_languages();
    }
};

}

#endif // XAPIAN_INCLUDED_STEM_H
