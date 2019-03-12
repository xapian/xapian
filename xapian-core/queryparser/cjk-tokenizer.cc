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

#include <algorithm>
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

bool
CJK::codepoint_is_cjk(unsigned p)
{
    // Array containing the last value in each range of codepoints which
    // are either all CJK or all non-CJK.
    static const unsigned splits[] = {
	// 1100..11FF; Hangul Jamo
	0x1100 - 1, 0x11FF,
	// 2E80..2EFF; CJK Radicals Supplement
	// 2F00..2FDF; Kangxi Radicals
	0x2E80 - 1, 0x2FDF,
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
	0x3000 - 1, 0x9FFF,
	// A700..A71F; Modifier Tone Letters
	0xA700 - 1, 0xA71F,
	// A960..A97F; Hangul Jamo Extended-A
	0xA960 - 1, 0xA97F,
	// AC00..D7AF; Hangul Syllables
	// D7B0..D7FF; Hangul Jamo Extended-B
	0xAC00 - 1, 0xD7FF,
	// F900..FAFF; CJK Compatibility Ideographs
	0xF900 - 1, 0xFAFF,
	// FE30..FE4F; CJK Compatibility Forms
	0xFE30 - 1, 0xFE4F,
	// FF00..FFEF; Halfwidth and Fullwidth Forms
	0xFF00 - 1, 0xFFEF,
	// 1B000..1B0FF; Kana Supplement
	// 1B100..1B12F; Kana Extended-A
	0x1B000 - 1, 0x1B12F,
	// 1F200..1F2FF; Enclosed Ideographic Supplement
	0x1F200 - 1, 0x1F2FF,
	// 20000..2A6DF; CJK Unified Ideographs Extension B
	0x20000 - 1, 0x2A6DF,
	// 2A700..2B73F; CJK Unified Ideographs Extension C
	// 2B740..2B81F; CJK Unified Ideographs Extension D
	// 2B820..2CEAF; CJK Unified Ideographs Extension E
	// 2CEB0..2EBEF; CJK Unified Ideographs Extension F
	0x2A700 - 1, 0x2EBEF,
	// 2F800..2FA1F; CJK Compatibility Ideographs Supplement
	0x2F800 - 1, 0x2FA1F,
    };
    // Binary chop to find the first entry which is >= p.  If it's an odd
    // offset then the codepoint is CJK; if it's an even offset then it's not.
    auto it = lower_bound(splits, splits + sizeof(splits) / sizeof(*splits), p);
    return ((it - splits) & 1);
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
CJKWordIterator::CJKWordIterator(const char* ptr, size_t len)
{
    UErrorCode err = U_ZERO_ERROR;
    UText utext = UTEXT_INITIALIZER;
    brk = icu::BreakIterator::createWordInstance(0/*unknown locale*/, err);
    if (usual(U_SUCCESS(err))) {
	utext_openUTF8(&utext, ptr, len, &err);
	if (usual(U_SUCCESS(err)))
	    brk->setText(&utext, err);
	utext_close(&utext);
    }
    if (rare(U_FAILURE(err)))
	throw Xapian::InternalError(string("ICU error: ") + u_errorName(err));
    int32_t first = brk->first();
    p = brk->next();
    utf8_ptr = ptr;
    current_token.assign(utf8_ptr + first, p - first);
}

CJKWordIterator &
CJKWordIterator::operator++()
{
    int32_t first = p;
    p = brk->next();
    if (usual(p != done)) {
	current_token.assign(utf8_ptr + first, p - first);
    }
    return *this;
}
#endif
