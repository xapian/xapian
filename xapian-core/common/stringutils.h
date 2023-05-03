/** @file
 * @brief Various handy helpers which std::string really should provide.
 */
/* Copyright (C) 2004-2022 Olly Betts
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

#ifndef XAPIAN_INCLUDED_STRINGUTILS_H
#define XAPIAN_INCLUDED_STRINGUTILS_H

#include <xapian/constinfo.h>

#include <algorithm>
#include <string>
#include <cstring>

/** Helper macro for STRINGIZE - the nested call is required because of how
 *  # works in macros.
 */
#define STRINGIZE_(X) #X

/// The STRINGIZE macro converts its parameter into a string constant.
#define STRINGIZE(X) STRINGIZE_(X)

/** Returns the length of a string constant.
 *
 *  We rely on concatenation of string literals to produce an error if this
 *  macro is applied to something other than a string literal.
 */
#define CONST_STRLEN(S) (sizeof(S"") - 1)

/* C++20 added starts_with(), ends_with() and contains() methods to std::string
 * and std::string_view which provide this functionality, but we don't yet
 * require C++20.
 */

inline bool
startswith(const std::string & s, char pfx)
{
    return !s.empty() && s[0] == pfx;
}

inline bool
startswith(const std::string & s, const char * pfx, size_t len)
{
    return s.size() >= len && (std::memcmp(s.data(), pfx, len) == 0);
}

inline bool
startswith(const std::string & s, const char * pfx)
{
    return startswith(s, pfx, std::strlen(pfx));
}

inline bool
startswith(const std::string & s, const std::string & pfx)
{
    return startswith(s, pfx.data(), pfx.size());
}

inline bool
endswith(const std::string & s, char sfx)
{
    return !s.empty() && s[s.size() - 1] == sfx;
}

inline bool
endswith(const std::string & s, const char * sfx, size_t len)
{
    return s.size() >= len && (std::memcmp(s.data() + s.size() - len, sfx, len) == 0);
}

inline bool
endswith(const std::string & s, const char * sfx)
{
    return endswith(s, sfx, std::strlen(sfx));
}

inline bool
endswith(const std::string & s, const std::string & sfx)
{
    return endswith(s, sfx.data(), sfx.size());
}

inline bool
contains(const std::string& s, char substring)
{
    return s.find(substring) != s.npos;
}

inline bool
contains(const std::string& s, const char* substring, size_t len)
{
    return s.find(substring, 0, len) != s.npos;
}

inline bool
contains(const std::string& s, const char* substring)
{
    return s.find(substring) != s.npos;
}

inline bool
contains(const std::string& s, const std::string& substring)
{
    return s.find(substring) != s.npos;
}

inline std::string::size_type
common_prefix_length(const std::string &a, const std::string &b)
{
    std::string::size_type minlen = std::min(a.size(), b.size());
    std::string::size_type common;
    for (common = 0; common < minlen; ++common) {
	if (a[common] != b[common]) break;
    }
    return common;
}

inline std::string::size_type
common_prefix_length(const std::string& a, const std::string& b,
		     std::string::size_type max_prefix_len)
{
    std::string::size_type minlen = std::min({a.size(),
					      b.size(),
					      max_prefix_len});
    std::string::size_type common;
    for (common = 0; common < minlen; ++common) {
	if (a[common] != b[common]) break;
    }
    return common;
}

// Like C's isXXXXX() but:
//  (a) always work in the C locale
//  (b) handle signed char as well as unsigned char
//  (c) have a suitable signature for use as predicates with find_if()
//  (d) add negated versions isnotXXXXX() which are useful as predicates

namespace Xapian {
    namespace Internal {
	const unsigned char HEX_MASK = 0x0f;
	const unsigned char IS_UPPER = 0x10;
	const unsigned char IS_ALPHA = 0x20; // NB Same as ASCII "case bit".
	const unsigned char IS_DIGIT = 0x40;
	const unsigned char IS_SPACE = 0x80;
    }
}

// FIXME: These functions assume ASCII or an ASCII compatible character set
// such as ISO-8859-N or UTF-8.  EBCDIC would need some work (patches
// welcome!)
static_assert('\x20' == ' ', "character set isn't a superset of ASCII");

// Add explicit conversion to bool to prevent compiler warning from "aCC +w":
// Warning (suggestion) 818: [...] # Type `int' is larger than type `bool',
// truncation in value may result.

inline unsigned char C_tab_(char ch) {
    const unsigned char * C_tab = Xapian::Internal::get_constinfo_()->C_tab;
    return C_tab[static_cast<unsigned char>(ch)];
}

inline bool C_isdigit(char ch) {
    using namespace Xapian::Internal;
    return bool(C_tab_(ch) & IS_DIGIT);
}

inline bool C_isxdigit(char ch) {
    using namespace Xapian::Internal;
    // Include IS_DIGIT so '0' gives true.
    return bool(C_tab_(ch) & (HEX_MASK|IS_DIGIT));
}

inline bool C_isupper(char ch) {
    using namespace Xapian::Internal;
    return bool(C_tab_(ch) & IS_UPPER);
}

inline bool C_islower(char ch) {
    using namespace Xapian::Internal;
    return (C_tab_(ch) & (IS_ALPHA|IS_UPPER)) == IS_ALPHA;
}

inline bool C_isalpha(char ch) {
    using namespace Xapian::Internal;
    return bool(C_tab_(ch) & IS_ALPHA);
}

inline bool C_isalnum(char ch) {
    using namespace Xapian::Internal;
    return bool(C_tab_(ch) & (IS_ALPHA|IS_DIGIT));
}

inline bool C_isspace(char ch) {
    using namespace Xapian::Internal;
    return bool(C_tab_(ch) & IS_SPACE);
}

inline bool C_isnotdigit(char ch) { return !C_isdigit(ch); }
inline bool C_isnotxdigit(char ch) { return !C_isxdigit(ch); }
inline bool C_isnotupper(char ch) { return !C_isupper(ch); }
inline bool C_isnotlower(char ch) { return !C_islower(ch); }
inline bool C_isnotalpha(char ch) { return !C_isalpha(ch); }
inline bool C_isnotalnum(char ch) { return !C_isalnum(ch); }
inline bool C_isnotspace(char ch) { return !C_isspace(ch); }

inline char C_tolower(char ch) {
    using namespace Xapian::Internal;
    return ch | (C_tab_(ch) & IS_ALPHA);
}

inline char C_toupper(char ch) {
    using namespace Xapian::Internal;
    return ch &~ (C_tab_(ch) & IS_ALPHA);
}

inline int hex_digit(char ch) {
    using namespace Xapian::Internal;
    return C_tab_(ch) & HEX_MASK;
}

/** Decode a pair of ASCII hex digits.
 *
 *  E.g. hex_decode('4', 'A') gives 'J'.
 *
 *  If C_isxdigit(ch1) isn't true then ch1 is treated as '0', and similarly for
 *  ch2.
 */
inline char hex_decode(char ch1, char ch2) {
    return char(hex_digit(ch1) << 4 | hex_digit(ch2));
}

#endif // XAPIAN_INCLUDED_STRINGUTILS_H
