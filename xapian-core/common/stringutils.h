/** @file stringutils.h
 * @brief Various handy helpers which std::string really should provide.
 */
/* Copyright (C) 2004,2005,2006,2007,2008,2009,2010 Olly Betts
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

#include <xapian/visibility.h>

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

// Like C's isXXXXX() but:
//  (a) always work in the C locale
//  (b) handle signed char as well as unsigned char
//  (c) have a suitable signature for use as predicates with find_if()
//  (d) add negated versions isnotXXXXX() which are useful as predicates
//  (e) add some extra categories we find useful

namespace Xapian {
    namespace Internal {
	const unsigned char IS_DIGIT = 0x01;
	const unsigned char IS_LOWER = 0x02;
	const unsigned char IS_UPPER = 0x04;
	const unsigned char IS_HEX   = 0x08;
	const unsigned char IS_SIGN  = 0x10;
	const unsigned char IS_SPACE = 0x20;
	XAPIAN_VISIBILITY_DEFAULT
	extern const unsigned char is_tab[];
	XAPIAN_VISIBILITY_DEFAULT
	extern const unsigned char lo_tab[];
	XAPIAN_VISIBILITY_DEFAULT
	extern const unsigned char up_tab[];
    }
}

// Add explicit conversion to bool to prevent compiler warning from "aCC +w":
// Warning (suggestion) 818: [...] # Type `int' is larger than type `bool',
// truncation in value may result.

inline unsigned char C_tab_(char ch) {
    using Xapian::Internal::is_tab;
    return is_tab[static_cast<unsigned char>(ch)];
}

inline bool C_isdigit(char ch) {
    using namespace Xapian::Internal;
    return bool(C_tab_(ch) & IS_DIGIT);
}

inline bool C_isxdigit(char ch) {
    using namespace Xapian::Internal;
    return bool(C_tab_(ch) & IS_HEX);
}

inline bool C_islcxdigit(char ch) {
    using namespace Xapian::Internal;
    return (C_tab_(ch) & (IS_UPPER|IS_HEX)) == IS_HEX;
}

inline bool C_isupper(char ch) {
    using namespace Xapian::Internal;
    return bool(C_tab_(ch) & IS_UPPER);
}

inline bool C_islower(char ch) {
    using namespace Xapian::Internal;
    return bool(C_tab_(ch) & IS_LOWER);
}

inline bool C_isalpha(char ch) {
    using namespace Xapian::Internal;
    return bool(C_tab_(ch) & (IS_UPPER|IS_LOWER));
}

inline bool C_isalnum(char ch) {
    using namespace Xapian::Internal;
    return bool(C_tab_(ch) & (IS_UPPER|IS_LOWER|IS_DIGIT));
}

inline bool C_isspace(char ch) {
    using namespace Xapian::Internal;
    return bool(C_tab_(ch) & IS_SPACE);
}

inline bool C_issign(char ch) {
    using namespace Xapian::Internal;
    return bool(C_tab_(ch) & IS_SIGN);
}

inline bool C_isupdig(char ch) {
    using namespace Xapian::Internal;
    return bool(C_tab_(ch) & (IS_UPPER|IS_DIGIT));
}

inline bool C_isnotdigit(char ch) { return !C_isdigit(ch); }
inline bool C_isnotxdigit(char ch) { return !C_isxdigit(ch); }
inline bool C_isnotupper(char ch) { return !C_isupper(ch); }
inline bool C_isnotlower(char ch) { return !C_islower(ch); }
inline bool C_isnotalpha(char ch) { return !C_isalpha(ch); }
inline bool C_isnotalnum(char ch) { return !C_isalnum(ch); }
inline bool C_isnotspace(char ch) { return !C_isspace(ch); }
inline bool C_isnotsign(char ch) { return !C_issign(ch); }

inline char C_tolower(char ch) {
    using namespace Xapian::Internal;
    return lo_tab[static_cast<unsigned char>(ch)];
}

inline char C_toupper(char ch) {
    using namespace Xapian::Internal;
    return up_tab[static_cast<unsigned char>(ch)];
}

#endif // XAPIAN_INCLUDED_STRINGUTILS_H
