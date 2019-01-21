/** @file stem.h
 * @brief stemming algorithms
 */
/* Copyright (C) 2005,2007,2010,2011,2013,2014,2015,2018 Olly Betts
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

#include <iostream>
#include <cstring>
#include <unordered_map>

namespace Xapian {

/// Class representing a stemming algorithm implementation.
class XAPIAN_VISIBILITY_DEFAULT StemImplementation
    : public Xapian::Internal::intrusive_base
{
    /// Don't allow assignment.
    void operator=(const StemImplementation &) = delete;

    /// Don't allow copying.
    StemImplementation(const StemImplementation &) = delete;

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

    /// Move constructor.
    Stem(Stem && o);

    /// Move assignment operator.
    Stem & operator=(Stem && o);

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
     *  - arabic (ar) - Since Xapian 1.3.5
     *  - armenian (hy) - Since Xapian 1.3.0
     *  - basque (eu) - Since Xapian 1.3.0
     *  - catalan (ca) - Since Xapian 1.3.0
     *  - danish (da)
     *  - dutch (nl)
     *  - english (en) - Martin Porter's 2002 revision of his stemmer
     *  - earlyenglish - Early English (e.g. Shakespeare, Dickens) stemmer
     *    (since Xapian 1.3.2)
     *  - english_lovins (lovins) - Lovin's stemmer
     *  - english_porter (porter) - Porter's stemmer as described in
     *			his 1980 paper
     *  - finnish (fi)
     *  - french (fr)
     *  - german (de)
     *  - german2 - Normalises umlauts and &szlig;
     *  - hungarian (hu)
     *  - indonesian (id) - Since Xapian 1.4.6
     *  - irish (ga) - Since Xapian 1.4.7
     *  - italian (it)
     *  - kraaij_pohlmann - A different Dutch stemmer
     *  - lithuanian (lt) - Since Xapian 1.4.7
     *  - nepali (ne) - Since Xapian 1.4.7
     *  - norwegian (nb, nn, no)
     *  - portuguese (pt)
     *  - romanian (ro)
     *  - russian (ru)
     *  - spanish (es)
     *  - swedish (sv)
     *  - tamil (ta) - Since Xapian 1.4.7
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

class XAPIAN_VISIBILITY_DEFAULT KrovetzStemmer : public StemImplementation {
    public:
        KrovetzStemmer();
        ~KrovetzStemmer();
        /// maximum number of characters in a word to be stemmed.
        static const int MAX_WORD_LENGTH=25;
        /*!
            \brief stem a term using the Krovetz algorithm. 
            The stem returned may be longer than the input term.
            May return a pointer
            to the private attribute stem. Performs case normalization on its
            input argument. Return values should be copied before
            calling the method again.
            @param term the term to stem
            @return the stemmed term or the original term if no stemming was 
            performed.
        */
        char * kstem_stemmer(char *term);
        /*!
            \brief stem a term using the Krovetz algorithm into the specified
            buffer.
            The stem returned may be longer than the input term.
            Performs case normalization on its input argument. 
            @param term the term to stem
            @param buffer the buffer to hold the stemmed term. The buffer should
            be at MAX_WORD_LENGTH or larger.
            @return the number of characters written to the buffer, including
            the terminating '\\0'. If 0, the caller should use the value in term.
        */
        int kstem_stem_tobuffer(char *term, char *buffer);
        /*!
            \brief Add an entry to the stemmer's dictionary table.
            @param variant the spelling for the entry.
            @param word the stem to use for the variant. If "", the variant
            stems to itself.
            @param exc Is the word an exception to the spelling rules.
        */
        void kstem_add_table_entry(const char* variant, const char* word, 
                                    bool exc=false);

        std::string operator()(const std::string & word);

        std::string get_description() const;

    private:
        /// Dictionary table entry
        typedef struct dictEntry {
            /// is the word an exception to stemming rules?
            bool exception;      
            /// stem to use for this entry.
            const char *root;
        } dictEntry;
        /// Two term hashtable entry for caching across calls
        typedef struct cacheEntry {
            /// flag for first or second entry most recently used.
            char flag; 
            /// first entry variant
            char word1[MAX_WORD_LENGTH];
            /// first entry stem
            char stem1[MAX_WORD_LENGTH];
            /// second entry variant
            char word2[MAX_WORD_LENGTH];
            /// second entry stem
            char stem2[MAX_WORD_LENGTH];
        } cacheEntry;

        // operates on atribute word.
        bool ends(const char *s, int sufflen);
        void setsuff(const char *str, int length);
        dictEntry *getdep(char *word);
        bool lookup(char *word);
        bool cons(int i);
        bool vowelinstem();
        bool vowel(int i);
        bool doublec(int i);
        void plural();
        void past_tense();
        void aspect();
        void ion_endings();
        void er_and_or_endings ();
        void ly_endings ();
        void al_endings() ;
        void ive_endings() ;
        void ize_endings() ;
        void ment_endings() ;
        void ity_endings() ;
        void ble_endings() ;
        void ness_endings() ;
        void ism_endings();
        void ic_endings();
        void ncy_endings();
        void nce_endings();
        // maint.
        void loadTables();
        struct eqstr {
            bool operator()(const char* s1, const char* s2) const {
                return strcmp(s1, s2) == 0;
            }
        };
        typedef std::unordered_map<const char *, dictEntry, std::hash<std::string>, eqstr> dictTable;
        dictTable dictEntries;
        // this needs to be a bounded size cache.
        // kstem.cpp uses size 30013 entries.
        cacheEntry *stemCache;
        // size
        int stemhtsize;
        // state
        // k = wordlength - 1
        int k;
        // j is stemlength - 1
        int j;
        // pointer to the output buffer
        char *m_word;
        // used by kstem_stemmer to return a safe value.
        char stem[MAX_WORD_LENGTH];
    };
}

#endif // XAPIAN_INCLUDED_STEM_H
