/** @file cjk-tokenizer.cc
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

#include <config.h>

#include "cjk-tokenizer.h"

#include "omassert.h"
#include "xapian/unicode.h"
#include "xapian/error.h"

#include <cstdlib>
#include <string>

using namespace std;

static unsigned NGRAM_SIZE = 2;

bool
CJK::is_cjk_enabled()
{
    const char * p;
    static bool result = ((p = getenv("XAPIAN_CJK_NGRAM")) != NULL && *p);
    return result;
}

// 2E80..2EFF; CJK Radicals Supplement
// 3000..303F; CJK Symbols and Punctuation
// 3040..309F; Hiragana
// 30A0..30FF; Katakana
// 3100..312F; Bopomofo
// 3130..318F; Hangul Compatibility Jamo
// 3190..319F; Kanbun
// 31A0..31BF; Bopomofo Extended
// 31C0..31EF; CJK Strokes
// 31F0..31FF; Katakana Phonetic Extensions
// 3200..32FF; Enclosed CJK Letters and Months
// 3300..33FF; CJK Compatibility
// 3400..4DBF; CJK Unified Ideographs Extension A
// 4DC0..4DFF; Yijing Hexagram Symbols
// 4E00..9FFF; CJK Unified Ideographs
// A700..A71F; Modifier Tone Letters
// AC00..D7AF; Hangul Syllables
// F900..FAFF; CJK Compatibility Ideographs
// FE30..FE4F; CJK Compatibility Forms
// FF00..FFEF; Halfwidth and Fullwidth Forms
// 20000..2A6DF; CJK Unified Ideographs Extension B
// 2F800..2FA1F; CJK Compatibility Ideographs Supplement
bool
CJK::codepoint_is_cjk(unsigned p)
{
    if (p < 0x2E80) return false;
    return ((p >= 0x2E80 && p <= 0x2EFF) ||
	    (p >= 0x3000 && p <= 0x9FFF) ||
	    (p >= 0xA700 && p <= 0xA71F) ||
	    (p >= 0xAC00 && p <= 0xD7AF) ||
	    (p >= 0xF900 && p <= 0xFAFF) ||
	    (p >= 0xFE30 && p <= 0xFE4F) ||
	    (p >= 0xFF00 && p <= 0xFFEF) ||
	    (p >= 0x20000 && p <= 0x2A6DF) ||
	    (p >= 0x2F800 && p <= 0x2FA1F));
}

string
CJK::get_cjk(Xapian::Utf8Iterator &it)
{
    string str;
    while (it != Xapian::Utf8Iterator() &&
	   codepoint_is_cjk(*it) &&
	   Xapian::Unicode::is_wordchar(*it)) {
	Xapian::Unicode::append_utf8(str, *it);
	++it;
    }
    return str;
}

bool
CJKTokenIterator::equal_to(const CJKTokenIterator & other) const
{
    // We only really care about comparisons where one or other is an end
    // iterator.
    return it == other.it;
}

const string &
CJKNgramIterator::operator*() const
{
    if (current_token.empty()) {
	Assert(it != Xapian::Utf8Iterator());
	p = it;
	Xapian::Unicode::append_utf8(current_token, *p);
	++p;
	len = 1;
    }
    return current_token;
}

CJKTokenIterator &
CJKNgramIterator::operator++()
{
    if (len < NGRAM_SIZE && p != Xapian::Utf8Iterator()) {
	Xapian::Unicode::append_utf8(current_token, *p);
	++p;
	++len;
    } else {
	Assert(it != Xapian::Utf8Iterator());
	++it;
	current_token.resize(0);
    }
    return *this;
}

bool
CJKWordIterator::equal_to(const CJKTokenIterator & other) const
{
    if (CJKWordIterator const* o = dynamic_cast<CJKWordIterator const*>(&other)) {
	return p == o->p && q == o->q;
    } else {
	return false;
    }
}

CJKWordIterator::CJKWordIterator(const std::string & s) : CJKTokenIterator(s)
{
    unsigned c;
    while (it != Xapian::Utf8Iterator()) {
	c = *it;
	++it;
	ustr.append(static_cast<UChar32>(c));
    }

    UErrorCode err = U_ZERO_ERROR;
    brk = icu::BreakIterator::createWordInstance(0/*unknown locale*/, err);
    if (U_FAILURE(err))
	throw Xapian::InternalError(string("ICU error: ") + string(u_errorName(err)));
    brk->setText(ustr);
    q = brk->first();
    p = brk->next();
}

const string &
CJKWordIterator::operator*() const
{
    if (current_token.empty()) {
	Assert(p != q);
	len = 0;
	icu::UnicodeString uword = ustr.tempSubString(q, p - q);
	for (int32_t i = 0; i < uword.length(); i = uword.getChar32Limit(++i)) {
		Xapian::Unicode::append_utf8(current_token, uword.char32At(i));
		len++;
	}
    }
    return current_token;
}


CJKTokenIterator &
CJKWordIterator::operator++()
{
    q = p;
    p = brk->next();
    if (p != done) {
	current_token.resize(0);
	if (p != q) {
		// refresh current_token and len
		current_token = (*(*this));
	}
    } else {
	q = done;
    }
    return *this;
}
