/** @file cjk-tokenizer.cc
 * @brief Tokenise CJK text as n-grams
 */
/* Copyright (c) 2007, 2008 Yung-chung Lin (henearkrxern@gmail.com)
 * Copyright (c) 2011 Richard Boulton (richard@tartarus.org)
 * Copyright (c) 2011 Brandon Schaefer (brandontschaefer@gmail.com)
 * Copyright (c) 2011,2018,2019 Olly Betts
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

bool
CJK::codepoint_is_cjk_wordchar(unsigned p)
{
    return codepoint_is_cjk(p) && Xapian::Unicode::is_wordchar(p);
}

size_t
CJK::get_cjk(Xapian::Utf8Iterator& it)
{
    size_t char_count = 0;
    while (it != Xapian::Utf8Iterator() &&
	   codepoint_is_cjk_wordchar(*it)) {
	++char_count;
	++it;
    }
    return char_count;
}

void
CJKNgramIterator::init() {
    if (it != Xapian::Utf8Iterator()) {
	unsigned ch = *it;
	if (CJK::codepoint_is_cjk_wordchar(ch)) {
	    Xapian::Unicode::append_utf8(current_token, ch);
	    ++it;
	} else {
	    current_token.resize(0);
	}
    }
}

CJKNgramIterator&
CJKNgramIterator::operator++()
{
    if (offset == 0) {
	if (it != Xapian::Utf8Iterator()) {
	    unsigned ch = *it;
	    if (CJK::codepoint_is_cjk_wordchar(ch)) {
		offset = current_token.size();
		Xapian::Unicode::append_utf8(current_token, ch);
		++it;
	    } else {
		current_token.resize(0);
	    }
	} else {
	    current_token.resize(0);
	}
    } else {
	current_token.erase(0, offset);
	offset = 0;
    }
    return *this;
}

#ifdef USE_ICU
CJKWordIterator::CJKWordIterator(const std::string & s)
{
    UErrorCode err = U_ZERO_ERROR;
    UText utext = UTEXT_INITIALIZER;
    brk = icu::BreakIterator::createWordInstance(0/*unknown locale*/, err);
    if (usual(U_SUCCESS(err))) {
	utext_openUTF8(&utext, s.data(), s.size(), &err);
	if (usual(U_SUCCESS(err)))
	    brk->setText(&utext, err);
	utext_close(&utext);
    }
    if (rare(U_FAILURE(err)))
	throw Xapian::InternalError(string("ICU error: ") + u_errorName(err));
    q = brk->first();
    p = brk->next();
    utf8_ptr = s.data();
    current_token.assign(utf8_ptr + q, p - q);
}

CJKWordIterator &
CJKWordIterator::operator++()
{
    q = p;
    p = brk->next();
    if (usual(p != done)) {
	current_token.assign(utf8_ptr + q, p - q);
    }
    return *this;
}
#endif
