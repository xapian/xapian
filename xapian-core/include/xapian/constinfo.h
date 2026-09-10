/** @file
 *  @brief Mechanism for accessing a struct of constant information
 */
// Copyright (C) 2003-2026 Olly Betts
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, see
// <https://www.gnu.org/licenses/>.

#ifndef XAPIAN_INCLUDED_XAPIAN_CONSTINFO_H
#define XAPIAN_INCLUDED_XAPIAN_CONSTINFO_H

#include <xapian/attributes.h>
#include <xapian/visibility.h>

namespace Xapian {
namespace Internal {

/** @private @internal */
struct constinfo {
    unsigned char C_tab[256];
    int major, minor, revision;
    char str[8];
    unsigned stemmer_name_len;
    // FIXME: We don't want to fix the size of this in the API headers.
    char stemmer_data[512];
};

/** @private @internal
 *
 *  Rather than having a separate function to access each piece of information,
 *  we put it all into a structure and have a single function which returns a
 *  pointer to this (and we mark that function with attribute const, so the
 *  compiler should be able to CSE calls to it.  This means that when Xapian is
 *  loaded as a shared library we save N-1 relocations (where N is the
 *  number of pieces of information), which reduces the library load time.
 */
XAPIAN_VISIBILITY_DEFAULT
const struct constinfo* get_constinfo_() noexcept XAPIAN_CONST_FUNCTION;

// @private @internal
const unsigned char C_HEX_MASK = 0x0f;
// @private @internal
const unsigned char C_IS_UPPER = 0x10;
// @private @internal
const unsigned char C_IS_ALPHA = 0x20; // NB Same as ASCII "case bit".
// @private @internal
const unsigned char C_IS_DIGIT = 0x40;
// @private @internal
const unsigned char C_IS_SPACE = 0x80;

// @private @internal
inline unsigned char c_tab_(char ch) {
    const unsigned char * C_tab = Xapian::Internal::get_constinfo_()->C_tab;
    return C_tab[static_cast<unsigned char>(ch)];
}

}

/** Functions associated with handling single-byte characters.
 *
 *  Most of the functions in this namespace work like the standard C library
 *  function with the same name, except they aren't affected by the current
 *  locale - they work as if the locale is set to "C", so e.g. isalpha() only
 *  returns true for ASCII letters.  They work for characters in ASCII and
 *  character sets where ASCII is a subset (such as iso-8859-1 and UTF-8).
 *
 *  Other differences to the standard C library versions are that they handle
 *  signed char values as well as unsigned, and have a suitable signature for
 *  using as a predicate with C++ standard library functions such as
 *  `find_if()`.
 *
 *  @since Xapian 2.2.0.
 */
namespace C {

// These functions assume an ASCII-compatible encoding.  Supporting EBCDIC
// would need significant work.
static_assert('\x20' == ' ', "character set isn't a superset of ASCII");

/// Like std::isdigit() but always uses the C locale.
inline bool isdigit(char ch) {
    using namespace Xapian::Internal;
    return bool(c_tab_(ch) & C_IS_DIGIT);
}

/// Like std::isxdigit() but always uses the C locale.
inline bool isxdigit(char ch) {
    using namespace Xapian::Internal;
    // Include C_IS_DIGIT so '0' gives true.
    return bool(c_tab_(ch) & (C_HEX_MASK|C_IS_DIGIT));
}

/// Like std::isupper() but always uses the C locale.
inline bool isupper(char ch) {
    using namespace Xapian::Internal;
    return bool(c_tab_(ch) & C_IS_UPPER);
}

/// Like std::islower() but always uses the C locale.
inline bool islower(char ch) {
    using namespace Xapian::Internal;
    return (c_tab_(ch) & (C_IS_ALPHA|C_IS_UPPER)) == C_IS_ALPHA;
}

/// Like std::isalpha() but always uses the C locale.
inline bool isalpha(char ch) {
    using namespace Xapian::Internal;
    return bool(c_tab_(ch) & C_IS_ALPHA);
}

/// Like std::isalnum() but always uses the C locale.
inline bool isalnum(char ch) {
    using namespace Xapian::Internal;
    return bool(c_tab_(ch) & (C_IS_ALPHA|C_IS_DIGIT));
}

/// Like std::isspace() but always uses the C locale.
inline bool isspace(char ch) {
    using namespace Xapian::Internal;
    return bool(c_tab_(ch) & C_IS_SPACE);
}

/// Like std::tolower() but always uses the C locale.
inline char tolower(char ch) {
    using namespace Xapian::Internal;
    return ch | (c_tab_(ch) & C_IS_ALPHA);
}

/// Like std::toupper() but always uses the C locale.
inline char toupper(char ch) {
    using namespace Xapian::Internal;
    return ch &~ (c_tab_(ch) & C_IS_ALPHA);
}

/** Convert an ASCII hex digit to its numeric value.
 *
 *  E.g. hex_decode('A', 'A') gives 10.
 *
 *  If Xapian::C::isxdigit(ch) isn't true then ch is treated as '0'.
 */
inline int hex_digit(char ch) {
    using namespace Xapian::Internal;
    return c_tab_(ch) & C_HEX_MASK;
}

/** Decode a pair of ASCII hex digits to an ASCII character.
 *
 *  E.g. hex_decode('4', 'A') gives '\x4A' which is 'J'.
 *
 *  If Xapian::C::isxdigit(ch1) isn't true then ch1 is treated as '0', and
 *  similarly for ch2.
 */
inline unsigned char hex_decode(char ch1, char ch2) {
    return static_cast<unsigned char>(hex_digit(ch1) << 4 | hex_digit(ch2));
}

}
}

#endif /* XAPIAN_INCLUDED_XAPIAN_CONSTINFO_H */
