/** @file cjk-tokenizer.h
 * @brief Tokenise CJK text as n-grams
 */
/* Copyright (c) 2007, 2008 Yung-chung Lin (henearkrxern@gmail.com)
 * Copyright (c) 2011 Richard Boulton (richard@tartarus.org)
 * Copyright (c) 2011 Brandon Schaefer (brandontschaefer@gmail.com)
 * Copyright (c) 2011 Olly Betts
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

namespace CJK {

/** Should we use the CJK n-gram code?
 *
 *  The first time this is called it reads the environmental variable
 *  XAPIAN_CJK_NGRAM and returns true if it is set to a non-empty value.
 *  Subsequent calls cache and return the same value.
 */
bool is_cjk_enabled();

bool codepoint_is_cjk(unsigned codepoint);

std::string get_cjk(Xapian::Utf8Iterator &it);

}

class CJKTokenIterator {
    Xapian::Utf8Iterator it;

    mutable Xapian::Utf8Iterator p;

    mutable unsigned len;

    mutable std::string current_token;

  public:
    CJKTokenIterator(const std::string & s)
	: it(s) { }

    CJKTokenIterator(const Xapian::Utf8Iterator & it_)
	: it(it_) { }

    CJKTokenIterator()
	: it() { }

    const std::string & operator*() const;

    CJKTokenIterator & operator++();

    /// Get the length of the current token in Unicode characters.
    unsigned get_length() const { return len; }

    friend bool operator==(const CJKTokenIterator &, const CJKTokenIterator &);
};

inline bool
operator==(const CJKTokenIterator & a, const CJKTokenIterator & b)
{
    // We only really care about comparisons where one or other is an end
    // iterator.
    return a.it == b.it;
}

inline bool
operator!=(const CJKTokenIterator & a, const CJKTokenIterator & b)
{
    return !(a == b);
}

#endif // XAPIAN_INCLUDED_CJK_TOKENIZER_H
