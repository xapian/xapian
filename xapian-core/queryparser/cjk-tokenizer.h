/** @file cjk-tokenizer.h
 * @brief Tokenise CJK text as n-grams
 */
/* Copyright (c) 2007, 2008 Yung-chung Lin (henearkrxern@gmail.com)
 * Copyright (c) 2011 Richard Boulton (richard@tartarus.org)
 * Copyright (c) 2011 Brandon Schaefer (brandontschaefer@gmail.com)
 * Copyright (c) 2011,2018 Olly Betts
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#ifndef XAPIAN_INCLUDED_CJK_TOKENIZER_H
#define XAPIAN_INCLUDED_CJK_TOKENIZER_H

#include "xapian/unicode.h"

#include <string>

#ifdef __GNUC__
// turn off some warnings for libicu headers
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Wundef"
#endif // __GNUC__

#include <unicode/brkiter.h>
#include <unicode/unistr.h>

#ifdef __GNUC__
// turn the warnings back on
#pragma GCC diagnostic pop
#endif // __GNUC__

namespace CJK {

/** Should we use the CJK n-gram code?
 *
 *  The first time this is called it reads the environment variable
 *  XAPIAN_CJK_NGRAM and returns true if it is set to a non-empty value.
 *  Subsequent calls cache and return the same value.
 */
bool is_cjk_enabled();

bool codepoint_is_cjk(unsigned codepoint);

bool codepoint_is_cjk_wordchar(unsigned codepoint);

std::string get_cjk(Xapian::Utf8Iterator &it, size_t& char_count);

static inline std::string
get_cjk(Xapian::Utf8Iterator &it)
{
    size_t dummy;
    return get_cjk(it, dummy);
}

}

class CJKTokenIterator {
  protected:
    Xapian::Utf8Iterator it;

    mutable unsigned len;

    mutable std::string current_token;

    virtual bool equal_to(const CJKTokenIterator & other) const;

  public:
    explicit CJKTokenIterator(const std::string & s)
	: it(s) { }

    explicit CJKTokenIterator(const Xapian::Utf8Iterator & it_)
	: it(it_) { }

    CJKTokenIterator()
	: it() { }

    virtual ~CJKTokenIterator() {};

    virtual const std::string & operator*() const = 0;

    virtual CJKTokenIterator & operator++() = 0;

    /// Get the length of the current token in Unicode characters.
    unsigned get_length() const { return len; }

    friend bool operator==(const CJKTokenIterator &, const CJKTokenIterator &);
};

class CJKNgramIterator : public CJKTokenIterator {
    mutable Xapian::Utf8Iterator p;

  public:
    CJKNgramIterator(const std::string & s) : CJKTokenIterator(s) {}

    CJKNgramIterator() : CJKTokenIterator() {}

    CJKTokenIterator & operator++();

    const std::string & operator*() const;
};

class CJKWordIterator : public CJKTokenIterator {
    mutable int32_t p, q;

    // copy UBRK_DONE to avoid GCC old-style cast error
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
    static const int32_t done = UBRK_DONE;
#pragma GCC diagnostic pop

    icu::UnicodeString ustr;
    icu::BreakIterator *brk;

  protected:
    bool equal_to(const CJKTokenIterator & other) const;

  public:
    CJKWordIterator(const std::string & s);

    CJKWordIterator() : CJKTokenIterator() { p = q = done; brk = NULL; }

    ~CJKWordIterator() { delete brk; }

    CJKTokenIterator & operator++();

    const std::string & operator*() const;
};

inline bool
operator==(const CJKTokenIterator & a, const CJKTokenIterator & b)
{
    return a.equal_to(b);
}

inline bool
operator!=(const CJKTokenIterator & a, const CJKTokenIterator & b)
{
    return !(a == b);
}

#endif // XAPIAN_INCLUDED_CJK_TOKENIZER_H
