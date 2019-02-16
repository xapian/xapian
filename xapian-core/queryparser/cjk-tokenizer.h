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
    mutable unsigned len;

    mutable std::string current_token;

  public:
    /// Get the length of the current token in Unicode characters.
    unsigned get_length() const { return len; }
};

class CJKNgramIterator : public CJKTokenIterator {
    Xapian::Utf8Iterator it;

    mutable Xapian::Utf8Iterator p;

  public:
    explicit CJKNgramIterator(const std::string & s)
	: it(s) { }

    explicit CJKNgramIterator(const Xapian::Utf8Iterator & it_)
	: it(it_) { }

    CJKNgramIterator()
	: it() { }

    CJKNgramIterator& operator++();

    const std::string & operator*() const;

    bool operator==(const CJKNgramIterator & other) const {
	// We only really care about comparisons where one or other is an end
	// iterator.
	return it == other.it;
    }

    bool operator!=(const CJKNgramIterator & other) const {
	return !(*this == other);
    }
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

  public:
    CJKWordIterator(const std::string & s);

    CJKWordIterator()
	: p(done), q(done), brk(NULL) { }

    ~CJKWordIterator() { delete brk; }

    CJKWordIterator & operator++();

    const std::string & operator*() const;

    bool operator==(const CJKWordIterator & other) const {
	return p == other.p && q == other.q;
    }

    bool operator!=(const CJKWordIterator & other) const {
	return !(*this == other);
    }
};

#endif // XAPIAN_INCLUDED_CJK_TOKENIZER_H
