/** @file
 * @brief Test the Unicode and UTF-8 classes and functions.
 */
/* Copyright (C) 2006-2026 Olly Betts
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
 * along with this program; if not, see
 * <https://www.gnu.org/licenses/>.
 */

#include <config.h>

#include "api_unicode.h"

#include <xapian.h>

#include "apitest.h"
#include "stringutils.h"
#include "testutils.h"

#include <cctype>
#include <fstream>
#include <limits>
#include <string_view>

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
    // Surrogate pair cases:
    { "\xed\xa0\x80", "\xc3\xad\xc2\xa0\xc2\x80" },
    { "\xed\xbf\xbf", "\xc3\xad\xc2\xbf\xc2\xbf" },
    { "\xed\xa0\x80" "\xed\xbf\xbf",
      "\xc3\xad\xc2\xa0\xc2\x80" "\xc3\xad\xc2\xbf\xc2\xbf" },
    { 0, 0 }
};

// Test handling of invalid UTF-8 is as desired.
DEFINE_TESTCASE(utf8iterator1, !backend) {
    const testcase* p;
    for (p = testcases; p->a; ++p) {
        tout.str(string());
        tout << '"' << p->a << "\" and \"" << p->b << "\"\n";
        // Exercise construction from pointer and length.
        Xapian::Utf8Iterator a(p->a, strlen(p->a));
        // Exercise construction from std::string_view.
        Xapian::Utf8Iterator b(string_view(p->b));

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

// Test we have the Unicode version we expect.
DEFINE_TESTCASE(unicode1, !backend) {
    using namespace Xapian;
    // We currently support Unicode 17.0.0 - check some codepoints which
    // were added or changed category in this version.
    TEST_EQUAL(Unicode::get_category(0x0295), Unicode::OTHER_LETTER);
    TEST_EQUAL(Unicode::get_category(0x088F), Unicode::OTHER_LETTER);
    TEST_EQUAL(Unicode::get_category(0x0C5C), Unicode::OTHER_LETTER);
    TEST_EQUAL(Unicode::get_category(0x0CDC), Unicode::OTHER_LETTER);
    TEST_EQUAL(Unicode::get_category(0xA7D2), Unicode::UPPERCASE_LETTER);
    TEST_EQUAL(Unicode::get_category(0xA7D4), Unicode::UPPERCASE_LETTER);
    TEST_EQUAL(Unicode::get_category(0x10959), Unicode::OTHER_LETTER);
    TEST_EQUAL(Unicode::get_category(0x11DDB), Unicode::OTHER_LETTER);
    TEST_EQUAL(Unicode::get_category(0x16ED3), Unicode::LOWERCASE_LETTER);
    TEST_EQUAL(Unicode::get_category(0x187FF), Unicode::OTHER_LETTER);
    TEST_EQUAL(Unicode::get_category(0x18D1E), Unicode::OTHER_LETTER);
    TEST_EQUAL(Unicode::get_category(0x1CCFB), Unicode::OTHER_SYMBOL);
    TEST_EQUAL(Unicode::get_category(0x1E6FE), Unicode::OTHER_LETTER);
    TEST_EQUAL(Unicode::get_category(0x1FA8E), Unicode::OTHER_SYMBOL);
    TEST_EQUAL(Unicode::get_category(0x1FACD), Unicode::OTHER_SYMBOL);
    TEST_EQUAL(Unicode::get_category(0x1FBFA), Unicode::OTHER_SYMBOL);
    TEST_EQUAL(Unicode::get_category(0x2B73F), Unicode::OTHER_LETTER);
    TEST_EQUAL(Unicode::get_category(0x2CEAD), Unicode::OTHER_LETTER);
    TEST_EQUAL(Unicode::get_category(0x323B0), Unicode::OTHER_LETTER);
    TEST_EQUAL(Unicode::get_category(0x33479), Unicode::OTHER_LETTER);
}

DEFINE_TESTCASE(caseconvert1, !backend) {
    using namespace Xapian;
    // Test Unicode case matches ISO C for ASCII subset.
    for (unsigned ch = 0; ch < 128; ++ch) {
        TEST_EQUAL(Unicode::tolower(ch), unsigned(tolower(ch)));
        TEST_EQUAL(Unicode::toupper(ch), unsigned(toupper(ch)));
        auto category = Unicode::get_category(ch);
        TEST_EQUAL(category == Unicode::LOWERCASE_LETTER, !!islower(ch));
        TEST_EQUAL(category == Unicode::UPPERCASE_LETTER, !!isupper(ch));
    }
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

static unsigned
decode_codepoint(const char** p) {
    unsigned r = 0;
    while (**p != ';') {
        TEST(C_isxdigit(**p));
        r = (r << 4) | hex_digit(**p);
        ++*p;
    }
    return r;
}

static void
test_codepoint(unsigned codepoint,
               unsigned upper,
               unsigned lower,
               Xapian::Unicode::category category)
{
    using namespace Xapian;
    TEST_EQUAL(Unicode::get_category(codepoint), category);
    TEST_EQUAL(Unicode::toupper(codepoint), upper);
    TEST_EQUAL(Unicode::tolower(codepoint), lower);

    switch (category) {
      case Unicode::CURRENCY_SYMBOL:
        TEST(Unicode::is_currency(codepoint));
        TEST(!Unicode::is_whitespace(codepoint));
        TEST(!Unicode::is_wordchar(codepoint));
        break;
      case Unicode::CONTROL:
      case Unicode::LINE_SEPARATOR:
      case Unicode::PARAGRAPH_SEPARATOR:
      case Unicode::SPACE_SEPARATOR:
        TEST(!Unicode::is_currency(codepoint));
        TEST(Unicode::is_whitespace(codepoint));
        TEST(!Unicode::is_wordchar(codepoint));
        break;
      case Unicode::COMBINING_SPACING_MARK:
      case Unicode::CONNECTOR_PUNCTUATION:
      case Unicode::DECIMAL_DIGIT_NUMBER:
      case Unicode::ENCLOSING_MARK:
      case Unicode::LETTER_NUMBER:
      case Unicode::LOWERCASE_LETTER:
      case Unicode::MODIFIER_LETTER:
      case Unicode::NON_SPACING_MARK:
      case Unicode::OTHER_LETTER:
      case Unicode::OTHER_NUMBER:
      case Unicode::TITLECASE_LETTER:
      case Unicode::UPPERCASE_LETTER:
        TEST(!Unicode::is_currency(codepoint));
        TEST(!Unicode::is_whitespace(codepoint));
        TEST(Unicode::is_wordchar(codepoint));
        break;
      default:
        TEST(!Unicode::is_currency(codepoint));
        TEST(!Unicode::is_whitespace(codepoint));
        TEST(!Unicode::is_wordchar(codepoint));
        break;
    }
}

// Exhaustively test all codepoints in Unicode's assignable range, plus a
// subset above that range.
DEFINE_TESTCASE(unicodetables, !backend) {
    using namespace Xapian;
    string unicode_data_path =
        test_driver::get_srcdir() + "/../unicode/UnicodeData.txt";
    ifstream unicode_data(unicode_data_path, fstream::binary);
    string line;
    unsigned next_codepoint = 0;
    while (getline(unicode_data, line), !unicode_data.eof()) {
        tout.str(string());
        const char* p = line.data();
        unsigned codepoint = decode_codepoint(&p);
        const char* desc = p + 1;
        p = strchr(desc, ';');
        TEST(p != nullptr);
        bool end_of_range =
            (*desc == '<' && p - desc > 5 && memcmp(p - 5, "Last>", 5) == 0);
        ++p;
        TEST(C_isupper(p[0]));
        TEST(C_islower(p[1]));
#define ENCODE(C1, C2) ((C1 - 'A') * 26 + (C2 - 'a'))
        Xapian::Unicode::category category;
        switch (ENCODE(p[0], p[1])) {
          case ENCODE('C', 'c'):
            category = Unicode::CONTROL;
            break;
          case ENCODE('C', 'f'):
            category = Unicode::FORMAT;
            break;
          case ENCODE('C', 'n'):
            category = Unicode::UNASSIGNED;
            break;
          case ENCODE('C', 'o'):
            category = Unicode::PRIVATE_USE;
            break;
          case ENCODE('C', 's'):
            category = Unicode::SURROGATE;
            break;
          case ENCODE('L', 'l'):
            category = Unicode::LOWERCASE_LETTER;
            break;
          case ENCODE('L', 'm'):
            category = Unicode::MODIFIER_LETTER;
            break;
          case ENCODE('L', 'o'):
            category = Unicode::OTHER_LETTER;
            break;
          case ENCODE('L', 't'):
            category = Unicode::TITLECASE_LETTER;
            break;
          case ENCODE('L', 'u'):
            category = Unicode::UPPERCASE_LETTER;
            break;
          case ENCODE('M', 'c'):
            category = Unicode::COMBINING_SPACING_MARK;
            break;
          case ENCODE('M', 'e'):
            category = Unicode::ENCLOSING_MARK;
            break;
          case ENCODE('M', 'n'):
            category = Unicode::NON_SPACING_MARK;
            break;
          case ENCODE('N', 'd'):
            category = Unicode::DECIMAL_DIGIT_NUMBER;
            break;
          case ENCODE('N', 'l'):
            category = Unicode::LETTER_NUMBER;
            break;
          case ENCODE('N', 'o'):
            category = Unicode::OTHER_NUMBER;
            break;
          case ENCODE('P', 'c'):
            category = Unicode::CONNECTOR_PUNCTUATION;
            break;
          case ENCODE('P', 'd'):
            category = Unicode::DASH_PUNCTUATION;
            break;
          case ENCODE('P', 'e'):
            category = Unicode::CLOSE_PUNCTUATION;
            break;
          case ENCODE('P', 'f'):
            category = Unicode::FINAL_QUOTE_PUNCTUATION;
            break;
          case ENCODE('P', 'i'):
            category = Unicode::INITIAL_QUOTE_PUNCTUATION;
            break;
          case ENCODE('P', 'o'):
            category = Unicode::OTHER_PUNCTUATION;
            break;
          case ENCODE('P', 's'):
            category = Unicode::OPEN_PUNCTUATION;
            break;
          case ENCODE('S', 'c'):
            category = Unicode::CURRENCY_SYMBOL;
            break;
          case ENCODE('S', 'k'):
            category = Unicode::MODIFIER_SYMBOL;
            break;
          case ENCODE('S', 'm'):
            category = Unicode::MATH_SYMBOL;
            break;
          case ENCODE('S', 'o'):
            category = Unicode::OTHER_SYMBOL;
            break;
          case ENCODE('Z', 'l'):
            category = Unicode::LINE_SEPARATOR;
            break;
          case ENCODE('Z', 'p'):
            category = Unicode::PARAGRAPH_SEPARATOR;
            break;
          case ENCODE('Z', 's'):
            category = Unicode::SPACE_SEPARATOR;
            break;
          default:
            FAIL_TEST("Unexpected Unicode category '" << p[0] << p[1] << "'");
        }

        for (int i = 2; i < 12; ++i) {
            p = strchr(p + 1, ';');
            TEST(p != nullptr);
        }
        ++p;
        unsigned upper = (*p != ';') ? decode_codepoint(&p) : codepoint;
        ++p;
        unsigned lower = (*p != ';') ? decode_codepoint(&p) : codepoint;

        if (end_of_range) {
            tout << "[" << next_codepoint << ".." << codepoint
                 << "] category=" << category << '\n';
        } else {
            if (next_codepoint < codepoint) {
                tout << "[" << next_codepoint << ".." << codepoint - 1 << "] "
                        "unassigned\n";
            }
            tout << codepoint << " category=" << category
                 << " upper=" << upper
                 << " lower=" << lower << '\n';
        }

        while (next_codepoint < codepoint) {
            if (end_of_range) {
                test_codepoint(next_codepoint, next_codepoint, next_codepoint,
                               category);
            } else {
                test_codepoint(next_codepoint, next_codepoint, next_codepoint,
                               Unicode::UNASSIGNED);
            }
            ++next_codepoint;
        }

        test_codepoint(codepoint, upper, lower, category);
        ++next_codepoint;
    }

    // Test some invalid Unicode values.
    TEST_EQUAL(next_codepoint, 0x10FFFE);

    tout.str(string());
    tout << "Testing unassigned codepoints just above U+10FFFD\n";
    unsigned codepoint = next_codepoint - 1;
    while (++codepoint <= 0x110011) {
        // Test some values just above the upper end of the valid range.
        test_codepoint(codepoint, codepoint, codepoint, Unicode::UNASSIGNED);
    }
    tout.str(string());
    tout << "Testing unassigned codepoints up to max unsigned int\n";
    do {
        // Test "all-F" values up to max of type.
        codepoint = (codepoint << 1) | 0x0FFFFF;
        test_codepoint(codepoint, codepoint, codepoint, Unicode::UNASSIGNED);
    } while (codepoint < numeric_limits<decltype(codepoint)>::max());
}
