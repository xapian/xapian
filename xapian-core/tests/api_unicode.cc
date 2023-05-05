/** @file
 * @brief Test the Unicode and UTF-8 classes and functions.
 */
/* Copyright (C) 2006,2007,2008,2009,2010,2011,2012,2013,2014,2015,2016 Olly Betts
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
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

#include <config.h>

#include "api_unicode.h"

#include <xapian.h>

#include "apitest.h"
#include "testutils.h"

#include <cctype>

using namespace std;

struct testcase {
    const char* a;
    const char* b;
};

static const testcase testcases[] = {
    { "abcd", "abcd" }, // Sanity check!
    { "a\x80""bcd", "a\xc2\x80""bcd" },
    { "a\xa0", "a\xc2\xa0" },
    { "a\xa0z", "a\xc2\xa0z" },
    { "x\xc1yz", "x\xc3\x81yz" },
    { "\xc2z", "\xc3\x82z" },
    { "\xc2", "\xc3\x82" },
    { "xy\xc3z", "xy\xc3\x83z" },
    { "xy\xc3\xc3z", "xy\xc3\x83\xc3\x83z" },
    { "xy\xc3\xc3", "xy\xc3\x83\xc3\x83" },
    { "\xe0", "\xc3\xa0" },
    { "\xe0\x80", "\xc3\xa0\xc2\x80" },
    { "\xe0\xc0", "\xc3\xa0\xc3\x80" },
    { "\xe0\xc0z", "\xc3\xa0\xc3\x80z" },
    { "\xe0\xc0zz", "\xc3\xa0\xc3\x80zz" },
    { "\xe0\xc0\x81", "\xc3\xa0\xc3\x80\xc2\x81" },
    { "\xe0\x82\xc1", "\xc3\xa0\xc2\x82\xc3\x81" },
    { "\xe0\xc5\xc7", "\xc3\xa0\xc3\x85\xc3\x87" },
    { "\xf0", "\xc3\xb0" },
    { "\xf0\x80", "\xc3\xb0\xc2\x80" },
    { "\xf0\xc0", "\xc3\xb0\xc3\x80" },
    { "\xf0\xc0z", "\xc3\xb0\xc3\x80z" },
    { "\xf0\xc0zz", "\xc3\xb0\xc3\x80zz" },
    { "\xf0\xc0\x81", "\xc3\xb0\xc3\x80\xc2\x81" },
    { "\xf0\x82\xc1", "\xc3\xb0\xc2\x82\xc3\x81" },
    { "\xf0\xc5\xc7", "\xc3\xb0\xc3\x85\xc3\x87" },
    { "\xf0\xc0\x81\xc9", "\xc3\xb0\xc3\x80\xc2\x81\xc3\x89" },
    { "\xf0\x82\xc1\xc8", "\xc3\xb0\xc2\x82\xc3\x81\xc3\x88" },
    { "\xf0\xc5\xc7\xc6", "\xc3\xb0\xc3\x85\xc3\x87\xc3\x86" },
    { "\xf0\xc0\x81\x89", "\xc3\xb0\xc3\x80\xc2\x81\xc2\x89" },
    { "\xf0\x82\xc1\x88", "\xc3\xb0\xc2\x82\xc3\x81\xc2\x88" },
    { "\xf0\xc5\xc7\xc6", "\xc3\xb0\xc3\x85\xc3\x87\xc3\x86" },
    { "\xf4P\x80\x80", "\xc3\xb4P\xc2\x80\xc2\x80" },
    { "\xf4\x80P\x80", "\xc3\xb4\xc2\x80P\xc2\x80" },
    { "\xf4\x80\x80P", "\xc3\xb4\xc2\x80\xc2\x80P" },
    { "\xfe\xffxyzzy", "\xc3\xbe\xc3\xbfxyzzy" },
    // Overlong encodings:
    { "\xc0\x80", "\xc3\x80\xc2\x80" },
    { "\xc0\xbf", "\xc3\x80\xc2\xbf" },
    { "\xc1\x80", "\xc3\x81\xc2\x80" },
    { "\xc1\xbf", "\xc3\x81\xc2\xbf" },
    { "\xe0\x80\x80", "\xc3\xa0\xc2\x80\xc2\x80" },
    { "\xe0\x9f\xbf", "\xc3\xa0\xc2\x9f\xc2\xbf" },
    { "\xf0\x80\x80\x80", "\xc3\xb0\xc2\x80\xc2\x80\xc2\x80" },
    { "\xf0\x8f\xbf\xbf", "\xc3\xb0\xc2\x8f\xc2\xbf\xc2\xbf" },
    // Above Unicode:
    { "\xf4\x90\x80\x80", "\xc3\xb4\xc2\x90\xc2\x80\xc2\x80" },
    { 0, 0 }
};

// Test handling of invalid UTF-8 is as desired.
DEFINE_TESTCASE(utf8iterator1, !backend) {
    const testcase* p;
    for (p = testcases; p->a; ++p) {
	tout.str(string());
	tout << '"' << p->a << "\" and \"" << p->b << "\"\n";
	size_t a_len = strlen(p->a);
	Xapian::Utf8Iterator a(p->a, a_len);

	size_t b_len = strlen(p->b);
	Xapian::Utf8Iterator b(p->b, b_len);

	while (a != Xapian::Utf8Iterator() && b != Xapian::Utf8Iterator()) {
	    TEST_EQUAL(*a, *b);
	    ++a;
	    ++b;
	}

	// Test that we don't reach the end of one before the other.
	TEST(a == Xapian::Utf8Iterator());
	TEST(b == Xapian::Utf8Iterator());
    }
}

struct testcase2 {
    const char* a;
    unsigned long n;
};

static const testcase2 testcases2[] = {
    { "a", 97 },
    { "\x80", 128 },
    { "\xa0", 160 },
    { "\xc2\x80", 128 },
    { "\xc2\xa0", 160 },
    { "\xe0\xa0\x80", 0x0800 },
    { "\xe1\x80\x80", 0x1000 },
    { "\xf0\xa8\xa8\x8f", 166415 },
    { "\xf3\x80\x80\x80", 0x0c0000 },
    { "\xf4\x80\x80\x80", 0x100000 },
    { 0, 0 }
};

// Test decoding of UTF-8.
DEFINE_TESTCASE(utf8iterator2, !backend) {
    const testcase2* p;
    for (p = testcases2; p->a; ++p) {
	Xapian::Utf8Iterator a(p->a);

	TEST(a != Xapian::Utf8Iterator());
	TEST_EQUAL(*a, p->n);
	TEST(++a == Xapian::Utf8Iterator());
    }
}

// Test Unicode categorisation.
DEFINE_TESTCASE(unicode1, !backend) {
    using namespace Xapian;
    TEST_EQUAL(Unicode::get_category('a'), Unicode::LOWERCASE_LETTER);
    TEST_EQUAL(Unicode::get_category('0'), Unicode::DECIMAL_DIGIT_NUMBER);
    TEST_EQUAL(Unicode::get_category('$'), Unicode::CURRENCY_SYMBOL);
    TEST_EQUAL(Unicode::get_category(0xa3), Unicode::CURRENCY_SYMBOL);
    // U+0242 was added in Unicode 5.0.0.
    TEST_EQUAL(Unicode::get_category(0x242), Unicode::LOWERCASE_LETTER);
    // U+0526 was added in Unicode 6.0.0.
    TEST_EQUAL(Unicode::get_category(0x0526), Unicode::UPPERCASE_LETTER);
    // U+0527 was added in Unicode 6.0.0.
    TEST_EQUAL(Unicode::get_category(0x0527), Unicode::LOWERCASE_LETTER);
    // U+0620 was added in Unicode 6.0.0.
    TEST_EQUAL(Unicode::get_category(0x0620), Unicode::OTHER_LETTER);
    // U+065F was added in Unicode 6.0.0.
    TEST_EQUAL(Unicode::get_category(0x065F), Unicode::NON_SPACING_MARK);
    // U+06DE changed category in Unicode 6.0.0.
    TEST_EQUAL(Unicode::get_category(0x06DE), Unicode::OTHER_SYMBOL);
    // U+0840 was added in Unicode 6.0.0.
    TEST_EQUAL(Unicode::get_category(0x0840), Unicode::OTHER_LETTER);
    // U+093A was added in Unicode 6.0.0.
    TEST_EQUAL(Unicode::get_category(0x093A), Unicode::NON_SPACING_MARK);
    // U+093B was added in Unicode 6.0.0.
    TEST_EQUAL(Unicode::get_category(0x093B), Unicode::COMBINING_SPACING_MARK);
    // U+0CF1 changed category in Unicode 6.0.0.
    TEST_EQUAL(Unicode::get_category(0x0CF1), Unicode::OTHER_LETTER);
    // U+0CF2 changed category in Unicode 6.0.0.
    TEST_EQUAL(Unicode::get_category(0x0CF2), Unicode::OTHER_LETTER);
    // U+11A7 was added in Unicode 5.2.0.
    TEST_EQUAL(Unicode::get_category(0x11A7), Unicode::OTHER_LETTER);
    // U+9FCB was added in Unicode 5.2.0.
    TEST_EQUAL(Unicode::get_category(0x9FCB), Unicode::OTHER_LETTER);
    // U+FA6C was added in Unicode 5.2.0.
    TEST_EQUAL(Unicode::get_category(0xFA6C), Unicode::OTHER_LETTER);
    TEST_EQUAL(Unicode::get_category(0xFFFF), Unicode::UNASSIGNED);
    // Test characters outside BMP.
    TEST_EQUAL(Unicode::get_category(0x10345), Unicode::OTHER_LETTER);
    TEST_EQUAL(Unicode::get_category(0x10FFFD), Unicode::PRIVATE_USE);
    TEST_EQUAL(Unicode::get_category(0x10FFFF), Unicode::UNASSIGNED);
    // U+1109A was added in Unicode 5.2.0.
    TEST_EQUAL(Unicode::get_category(0x1109a), Unicode::OTHER_LETTER);
    // U+1F773 was added in Unicode 6.0.0.
    TEST_EQUAL(Unicode::get_category(0x1F773), Unicode::OTHER_SYMBOL);
    // U+2B740 was added in Unicode 6.0.0.
    TEST_EQUAL(Unicode::get_category(0x2B740), Unicode::OTHER_LETTER);
    // U+2B81D was added in Unicode 6.0.0.
    TEST_EQUAL(Unicode::get_category(0x2B81D), Unicode::OTHER_LETTER);
    // U+00A7 changed category in Unicode 6.1.0 (was OTHER_SYMBOL).
    TEST_EQUAL(Unicode::get_category(0xA7), Unicode::OTHER_PUNCTUATION);
    // U+00AA changed category in Unicode 6.1.0 (was LOWERCASE_LETTER).
    TEST_EQUAL(Unicode::get_category(0xAA), Unicode::OTHER_LETTER);
    // U+00B6 changed category in Unicode 6.1.0 (was OTHER_SYMBOL).
    TEST_EQUAL(Unicode::get_category(0xB6), Unicode::OTHER_PUNCTUATION);
    // U+00BA changed category in Unicode 6.1.0 (was LOWERCASE_LETTER).
    TEST_EQUAL(Unicode::get_category(0xBA), Unicode::OTHER_LETTER);
    // U+058F was added in Unicode 6.1.0.
    TEST_EQUAL(Unicode::get_category(0x058F), Unicode::CURRENCY_SYMBOL);
    // U+0604 was added in Unicode 6.1.0.
    TEST_EQUAL(Unicode::get_category(0x0604), Unicode::FORMAT);
    // U+08A0 was added in Unicode 6.1.0.
    TEST_EQUAL(Unicode::get_category(0x08A0), Unicode::OTHER_LETTER);
    // U+08E4 was added in Unicode 6.1.0.
    TEST_EQUAL(Unicode::get_category(0x08E4), Unicode::NON_SPACING_MARK);
    // U+0AF0 was added in Unicode 6.1.0.
    TEST_EQUAL(Unicode::get_category(0x0AF0), Unicode::OTHER_PUNCTUATION);
    // U+9FCC was added in Unicode 6.1.0.
    TEST_EQUAL(Unicode::get_category(0x9FCC), Unicode::OTHER_LETTER);
    // U+A7F9 was added in Unicode 6.1.0.
    TEST_EQUAL(Unicode::get_category(0xA7F9), Unicode::MODIFIER_LETTER);
    // U+110F0 was added in Unicode 6.1.0.
    TEST_EQUAL(Unicode::get_category(0x110F0), Unicode::DECIMAL_DIGIT_NUMBER);
    // U+11100 was added in Unicode 6.1.0.
    TEST_EQUAL(Unicode::get_category(0x11100), Unicode::NON_SPACING_MARK);
    // U+1EEF0 was added in Unicode 6.1.0.
    TEST_EQUAL(Unicode::get_category(0x1EEF0), Unicode::MATH_SYMBOL);
    // U+1F634 was added in Unicode 6.1.0.
    TEST_EQUAL(Unicode::get_category(0x1F634), Unicode::OTHER_SYMBOL);
    // U+20BA was added in Unicode 6.2.0.
    TEST_EQUAL(Unicode::get_category(0x20BA), Unicode::CURRENCY_SYMBOL);
    // U+061C was added in Unicode 6.3.0.
    TEST_EQUAL(Unicode::get_category(0x61C), Unicode::FORMAT);
    // U+037F "GREEK CAPITAL LETTER YOT" was added in Unicode 7.0.0.
    TEST_EQUAL(Unicode::get_category(0x37F), Unicode::UPPERCASE_LETTER);

    // Added or changed in Unicode 8.0.0:
    // U+08B3 "ARABIC LETTER AIN WITH THREE DOTS BELOW".
    TEST_EQUAL(Unicode::get_category(0x8B3), Unicode::OTHER_LETTER);
    // U+0AF9 "GUJARATI LETTER ZHA".
    TEST_EQUAL(Unicode::get_category(0xAF9), Unicode::OTHER_LETTER);
    // U+0C5A "TELUGU LETTER RRRA".
    TEST_EQUAL(Unicode::get_category(0xC5A), Unicode::OTHER_LETTER);
    // U+0D5F "MALAYALAM LETTER ARCHAIC II".
    TEST_EQUAL(Unicode::get_category(0xD5F), Unicode::OTHER_LETTER);
    // U+13F5 "CHEROKEE LETTER MV".
    TEST_EQUAL(Unicode::get_category(0x13F5), Unicode::UPPERCASE_LETTER);
    // U+13F8 "CHEROKEE SMALL LETTER YE".
    TEST_EQUAL(Unicode::get_category(0x13F8), Unicode::LOWERCASE_LETTER);
    // U+19B7 "NEW TAI LUE VOWEL SIGN O" changed to be OTHER_LETTER in 8.0.0.
    TEST_EQUAL(Unicode::get_category(0x19B7), Unicode::OTHER_LETTER);
    // U+20BE "LARI SIGN".
    TEST_EQUAL(Unicode::get_category(0x20BE), Unicode::CURRENCY_SYMBOL);
    // U+218A "TURNED DIGIT TWO".
    TEST_EQUAL(Unicode::get_category(0x218A), Unicode::OTHER_SYMBOL);
    // U+10C9C "OLD HUNGARIAN CAPITAL LETTER OO".
    TEST_EQUAL(Unicode::get_category(0x10C9C), Unicode::UPPERCASE_LETTER);
    // U+12399 "CUNEIFORM SIGN U U".
    TEST_EQUAL(Unicode::get_category(0x12399), Unicode::OTHER_LETTER);
    // U+1D800 "SIGNWRITING HAND-FIST INDEX".
    TEST_EQUAL(Unicode::get_category(0x1D800), Unicode::OTHER_SYMBOL);

    // Added or changed in Unicode 9.0.0:
    // U+08B6 "ARABIC LETTER BEH WITH SMALL MEEM ABOVE"
    TEST_EQUAL(Unicode::get_category(0x8B6), Unicode::OTHER_LETTER);
    // U+08E2 "ARABIC DISPUTED END OF AYAH"
    TEST_EQUAL(Unicode::get_category(0x8E2), Unicode::FORMAT);
    // U+0C80 "KANNADA SIGN SPACING CANDRABINDU"
    TEST_EQUAL(Unicode::get_category(0xC80), Unicode::OTHER_LETTER);
    // U+0D56 "MALAYALAM LETTER CHILLU LLL"
    TEST_EQUAL(Unicode::get_category(0xD56), Unicode::OTHER_LETTER);
    // U+0D58 "MALAYALAM FRACTION ONE ONE-HUNDRED-AND-SIXTIETH"
    TEST_EQUAL(Unicode::get_category(0xD58), Unicode::OTHER_NUMBER);
    // U+1885 "MONGOLIAN LETTER ALI GALI BALUDA"
    TEST_EQUAL(Unicode::get_category(0x1885), Unicode::NON_SPACING_MARK);
    // U+1886 "MONGOLIAN LETTER ALI GALI THREE BALUDA"
    TEST_EQUAL(Unicode::get_category(0x1886), Unicode::NON_SPACING_MARK);
    // U+104FB "OSAGE SMALL LETTER ZHA"
    TEST_EQUAL(Unicode::get_category(0x104FB), Unicode::LOWERCASE_LETTER);
    // U+1141F "NEWA LETTER TA"
    TEST_EQUAL(Unicode::get_category(0x1141F), Unicode::OTHER_LETTER);
    // U+1F989 "OWL"
    TEST_EQUAL(Unicode::get_category(0x1F989), Unicode::OTHER_SYMBOL);

    // Test some invalid Unicode values.
    TEST_EQUAL(Unicode::get_category(0x110000), Unicode::UNASSIGNED);
    TEST_EQUAL(Unicode::get_category(0xFFFFFFFF), Unicode::UNASSIGNED);
}

DEFINE_TESTCASE(caseconvert1, !backend) {
    using namespace Xapian;
    for (unsigned ch = 0; ch < 128; ++ch) {
	TEST_EQUAL(Unicode::tolower(ch), unsigned(tolower(ch)));
	TEST_EQUAL(Unicode::toupper(ch), unsigned(toupper(ch)));
    }

    // U+0242 was added in Unicode 5.0.0 as a lowercase form of U+0241.
    TEST_EQUAL(Unicode::tolower(0x242), 0x242);
    TEST_EQUAL(Unicode::toupper(0x242), 0x241);
    TEST_EQUAL(Unicode::toupper(0x241), 0x241);
    TEST_EQUAL(Unicode::tolower(0x241), 0x242);

    // Regression test for bug fixed in 1.2.17.
    TEST_EQUAL(Unicode::tolower(0x1c5), 0x1c6);
    TEST_EQUAL(Unicode::tolower(0x1c8), 0x1c9);
    TEST_EQUAL(Unicode::tolower(0x1cb), 0x1cc);
    TEST_EQUAL(Unicode::tolower(0x1f2), 0x1f3);

    // Pound currency symbol:
    TEST_EQUAL(Unicode::tolower(0xa3), 0xa3);
    TEST_EQUAL(Unicode::toupper(0xa3), 0xa3);
    // Unassigned:
    TEST_EQUAL(Unicode::tolower(0xFFFF), 0xFFFF);
    TEST_EQUAL(Unicode::toupper(0xFFFF), 0xFFFF);
    // Test characters outside BMP.
    TEST_EQUAL(Unicode::tolower(0x10345), 0x10345);
    TEST_EQUAL(Unicode::toupper(0x10345), 0x10345);
    TEST_EQUAL(Unicode::tolower(0x10FFFD), 0x10FFFD);
    TEST_EQUAL(Unicode::toupper(0x10FFFD), 0x10FFFD);
    TEST_EQUAL(Unicode::tolower(0x10FFFF), 0x10FFFF);
    TEST_EQUAL(Unicode::toupper(0x10FFFF), 0x10FFFF);
    // Test some invalid Unicode values.
    TEST_EQUAL(Unicode::tolower(0x110000), 0x110000);
    TEST_EQUAL(Unicode::toupper(0x110000), 0x110000);
    TEST_EQUAL(Unicode::tolower(0xFFFFFFFF), 0xFFFFFFFF);
    TEST_EQUAL(Unicode::toupper(0xFFFFFFFF), 0xFFFFFFFF);
}

/// Test Unicode 5.1 and later support.
DEFINE_TESTCASE(caseconvert2, !backend) {
    using namespace Xapian;

    TEST_EQUAL(Unicode::toupper(0x250), 0x2c6f);
    TEST_EQUAL(Unicode::toupper(0x251), 0x2c6d);
    TEST_EQUAL(Unicode::toupper(0x271), 0x2c6e);

    TEST_EQUAL(Unicode::get_category(0x2ec), Unicode::MODIFIER_LETTER);
    TEST_EQUAL(Unicode::get_category(0x374), Unicode::MODIFIER_LETTER);
    TEST_EQUAL(Unicode::get_category(0x487), Unicode::NON_SPACING_MARK);
    TEST_EQUAL(Unicode::get_category(0x5be), Unicode::DASH_PUNCTUATION);
    TEST_EQUAL(Unicode::get_category(0x1f093), Unicode::OTHER_SYMBOL);

    // U+0526, U+0527 and U+A78D were added in Unicode 6.0.0:
    TEST_EQUAL(Unicode::toupper(0x265), 0xa78d);
    TEST_EQUAL(Unicode::tolower(0xa78d), 0x265);
    TEST_EQUAL(Unicode::tolower(0x526), 0x527);
    TEST_EQUAL(Unicode::toupper(0x527), 0x526);

    // U+A7AA was added in Unicode 6.1.0:
    TEST_EQUAL(Unicode::toupper(0x266), 0xa7aa);
    TEST_EQUAL(Unicode::tolower(0xa7aa), 0x266);
    TEST_EQUAL(Unicode::tolower(0x526), 0x527);
    TEST_EQUAL(Unicode::toupper(0x527), 0x526);

    TEST_EQUAL(Unicode::tolower(0x370), 0x371);
    TEST_EQUAL(Unicode::toupper(0x371), 0x370);
    TEST_EQUAL(Unicode::tolower(0x372), 0x373);
    TEST_EQUAL(Unicode::toupper(0x373), 0x372);
    TEST_EQUAL(Unicode::tolower(0x376), 0x377);
    TEST_EQUAL(Unicode::toupper(0x377), 0x376);
    TEST_EQUAL(Unicode::tolower(0x3cf), 0x3d7);
    TEST_EQUAL(Unicode::toupper(0x3d7), 0x3cf);

    // U+20BA was added in Unicode 6.2.0:
    TEST_EQUAL(Unicode::toupper(0x20ba), 0x20ba);
    TEST_EQUAL(Unicode::tolower(0x20ba), 0x20ba);

    // U+061C was added in Unicode 6.3.0:
    TEST_EQUAL(Unicode::toupper(0x61c), 0x61c);
    TEST_EQUAL(Unicode::tolower(0x61c), 0x61c);

    unsigned u;
    for (u = 0x514; u < 0x524; u += 2) {
	TEST_EQUAL(Unicode::get_category(u), Unicode::UPPERCASE_LETTER);
	TEST_EQUAL(Unicode::get_category(u + 1), Unicode::LOWERCASE_LETTER);
	TEST_EQUAL(Unicode::tolower(u), u + 1);
	TEST_EQUAL(Unicode::toupper(u + 1), u);
    }

    // U+A7B1 was added in Unicode 8.0.0 as an uppercase form of U+0287.
    TEST_EQUAL(Unicode::tolower(0xA7B1), 0x0287);
    TEST_EQUAL(Unicode::toupper(0xA7B1), 0xA7B1);
    TEST_EQUAL(Unicode::tolower(0x0287), 0x0287);
    TEST_EQUAL(Unicode::toupper(0x0287), 0xA7B1);

    // U+A7B4 (capital) and U+A7B5 (small) added in Unicode 8.0.0
    TEST_EQUAL(Unicode::tolower(0xA7B4), 0xA7B5);
    TEST_EQUAL(Unicode::toupper(0xA7B4), 0xA7B4);
    TEST_EQUAL(Unicode::tolower(0xA7B5), 0xA7B5);
    TEST_EQUAL(Unicode::toupper(0xA7B5), 0xA7B4);

    // U+A7AE was added in Unicode 9.0.0 as an uppercase form of U+026A.
    TEST_EQUAL(Unicode::tolower(0xA7AE), 0x026A);
    TEST_EQUAL(Unicode::toupper(0xA7AE), 0xA7AE);
    TEST_EQUAL(Unicode::tolower(0x026A), 0x026A);
    TEST_EQUAL(Unicode::toupper(0x026A), 0xA7AE);
}

DEFINE_TESTCASE(utf8convert1, !backend) {
    string s;
    Xapian::Unicode::append_utf8(s, 'a');
    Xapian::Unicode::append_utf8(s, 128);
    Xapian::Unicode::append_utf8(s, 160);
    Xapian::Unicode::append_utf8(s, 0xFFFF);
    Xapian::Unicode::append_utf8(s, 166415);
    Xapian::Unicode::append_utf8(s, 0x10345);
    Xapian::Unicode::append_utf8(s, 0x10FFFD);
    Xapian::Unicode::append_utf8(s, 0xFFFFFFFF);
    Xapian::Unicode::append_utf8(s, 'z');
    TEST_STRINGS_EQUAL(s, "a"
			  "\xc2\x80"
			  "\xc2\xa0"
			  "\xef\xbf\xbf"
			  "\xf0\xa8\xa8\x8f"
			  "\xf0\x90\x8d\x85"
			  "\xf4\x8f\xbf\xbd"
			  ""
			  "z"
			  );
}

DEFINE_TESTCASE(unicodepredicates1, !backend) {
    static const unsigned wordchars[] = {
	// DECIMAL_DIGIT_NUMBER
	'0', '7', '9',
	// LOWERCASE_LETTER
	'a', 'z', 0x250, 0x251, 0x271, 0x3d7,
	0x242, // (added in Unicode 5.0.0)
	// LOWERCASE_LETTER (added in Unicode 5.1.0)
	0x371, 0x373, 0x377, 0x514, 0x516, 0x518, 0x51a, 0x51c, 0x51e,
	0x520, 0x522,
	// UPPERCASE_LETTER
	'A', 'Z', 0x241,
	// UPPERCASE_LETTER (added in Unicode 5.1.0)
	0x370, 0x372, 0x376, 0x3cf, 0x515, 0x517, 0x519, 0x51b, 0x51d, 0x51f,
	0x521, 0x523, 0x2c6d, 0x2c6e, 0x2c6f,
	// OTHER_LETTER
	0x8bb, // Added in Unicode 9.0.0
	0xc80, // Added in Unicode 9.0.0
	0x10345,
	// MODIFIER_LETTER (added in Unicode 5.1.0)
	0x2ec, 0x374,
	// NON_SPACING_MARK (added to is_wordchar() in 1.1.0)
	0x651,
	0x487, // Added in Unicode 5.1.0
	0x8db, // Added in Unicode 9.0.0
	0
    };
    static const unsigned currency[] = {
	// CURRENCY_SYMBOL
	'$', 0xa3,
	// CURRENCY_SYMBOL (added in Unicode 6.2.0)
	0x20ba,
	// CURRENCY_SYMBOL (added in Unicode 8.0.0)
	0x20be,
	0
    };
    static const unsigned whitespace[] = {
	// CONTROL
	'\t', '\n', '\f', '\r',
	// SPACE_SEPARATOR
	' ',
	0
    };
    static const unsigned other[] = {
	// DASH_PUNCTUATION (added in Unicode 5.1.0)
	0x5be,
	// OTHER_SYMBOL
	0xd4f, // Added in Unicode 9.0.0
	0x1f093, // Added in Unicode 5.1.0
	// FORMAT
	0x61c, // Added in Unicode 6.3.0
	0x8e2, // Added in Unicode 9.0.0
	// UNASSIGNED
	0xffff, 0x10ffff, 0x110000, 0xFFFFFFFF,
	// PRIVATE_USE
	0x10fffd,
	0
    };

    for (const unsigned* p = wordchars; *p; ++p) {
	TEST(Xapian::Unicode::is_wordchar(*p));
	TEST(!Xapian::Unicode::is_currency(*p));
	TEST(!Xapian::Unicode::is_whitespace(*p));
    }

    for (const unsigned* p = currency; *p; ++p) {
	TEST(!Xapian::Unicode::is_wordchar(*p));
	TEST(Xapian::Unicode::is_currency(*p));
	TEST(!Xapian::Unicode::is_whitespace(*p));
    }

    for (const unsigned* p = whitespace; *p; ++p) {
	TEST(!Xapian::Unicode::is_wordchar(*p));
	TEST(!Xapian::Unicode::is_currency(*p));
	TEST(Xapian::Unicode::is_whitespace(*p));
    }

    for (const unsigned* p = other; *p; ++p) {
	TEST(!Xapian::Unicode::is_wordchar(*p));
	TEST(!Xapian::Unicode::is_currency(*p));
	TEST(!Xapian::Unicode::is_whitespace(*p));
    }
}
