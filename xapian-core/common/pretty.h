/** @file
 * @brief Convert types to pretty representations
 */
/* Copyright (C) 2010,2011,2012,2014,2016,2023 Olly Betts
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

#ifndef XAPIAN_INCLUDED_PRETTY_H
#define XAPIAN_INCLUDED_PRETTY_H

#include <list>
#include <map>
#include <ostream>
#include <string>
#include <vector>

#include "xapian/intrusive_ptr.h"
#include "xapian/types.h"

template<class S>
struct PrettyOStream {
    /// The std::ostream object we're outputting to.
    S & os;

    PrettyOStream(S & os_) : os(os_) { }
    template<typename T> PrettyOStream & operator|(const T & t) {
	os << ", ";
	return *this << t;
    }
};

struct Literal {
    const char * _lit;
    explicit Literal(const char * lit) : _lit(lit) { }
    explicit Literal(const std::string & s) : _lit(s.c_str()) { }
};

/// Default is to output as std::ostream would.
template<class S, class T>
inline PrettyOStream<S> &
operator<<(PrettyOStream<S> &ps, const T & t)
{
    ps.os << t;
    return ps;
}

/** Allow writing literal strings.
 *
 *  For example:
 *
 *  PrettyOStream<std::ostream> ps(std::cout);
 *  ps << Literal("x = ") << x << Literal(", y = ") << y << endl;
 */
template<class S>
inline PrettyOStream<S> &
operator<<(PrettyOStream<S> &ps, const Literal & t)
{
    ps.os << t._lit;
    return ps;
}

template<class S, class T>
inline PrettyOStream<S> &
operator<<(PrettyOStream<S> &ps, const T * t)
{
    if (!t) {
	ps.os << "NULL";
	return ps;
    }
    ps.os << '&';
    return ps << *t;
}

template<class S, class T>
inline PrettyOStream<S> &
operator<<(PrettyOStream<S> &ps, const T ** t)
{
    ps.os << (void*)t;
    return ps;
}

template<class S>
inline PrettyOStream<S> &
operator<<(PrettyOStream<S> &ps, const void * t)
{
    ps.os << "(void*)" << t;
    return ps;
}

// FIXME: We probably don't want to inline this, but need to arrange to
// put it somewhere sane out-of-line.
inline void write_ch(std::ostream & os, unsigned char ch)
{
    if (ch < 32 || ch >= 127) {
	os << '\\';
	if (ch >= 7 && ch <= 13) {
	    os << "abtnvfr"[ch - 7];
	} else {
	    os << char('0' | (ch >> 6));
	    os << char('0' | ((ch >> 3) & 7));
	    os << char('0' | (ch & 7));
	}
    } else if (ch == '\\') {
	os << "\\\\";
    } else if (ch == '"') {
	os << "\\\"";
    } else {
	os << ch;
    }
}

template<class S>
inline PrettyOStream<S> &
operator<<(PrettyOStream<S> &ps, const char * str)
{
    ps.os << '"';
    while (*str) {
	write_ch(ps.os, *str++);
    }
    ps.os << '"';
    return ps;
}

template<class S>
inline PrettyOStream<S> &
operator<<(PrettyOStream<S> &ps, const std::string & str)
{
    ps.os << '"';
    for (std::string::const_iterator i = str.begin(); i != str.end(); ++i) {
	write_ch(ps.os, *i);
    }
    ps.os << '"';
    return ps;
}

template<class S>
inline PrettyOStream<S> &
operator<<(PrettyOStream<S> &ps, std::string &)
{
    ps.os << "std::string&";
    return ps;
}

template<class S>
inline PrettyOStream<S> &
operator<<(PrettyOStream<S> &ps, std::string *)
{
    ps.os << "std::string*";
    return ps;
}

template<class S>
inline PrettyOStream<S> &
operator<<(PrettyOStream<S> &ps, unsigned char ch)
{
    ps.os << '\'';
    if (ch < 32 || ch >= 127) {
	ps.os << '\\';
	if (ch >= 7 && ch <= 13) {
	    ps.os << "abtnvfr"[ch - 7];
	} else if (ch == '\0') {
	    ps.os << "\\0";
	} else {
	    ps.os << "0123456789abcdef"[ch >> 4];
	    ps.os << "0123456789abcdef"[ch & 0x0f];
	}
    } else if (ch == '\\') {
	ps.os << "\\\\";
    } else if (ch == '\'') {
	ps.os << "\\'";
    } else {
	ps.os << ch;
    }
    ps.os << '\'';
    return ps;
}

template<class S>
inline PrettyOStream<S> &
operator<<(PrettyOStream<S> &ps, bool b)
{
    ps.os << (b ? "true" : "false");
    return ps;
}

/*
template<class S>
inline PrettyOStream<S> &
operator<<(PrettyOStream<S> &ps, bool &)
{
    ps.os << "bool&";
    return ps;
}
*/

template<class S>
inline PrettyOStream<S> &
operator<<(PrettyOStream<S> &ps, Xapian::termcount * p)
{
    ps.os << "(Xapian::termcount*)" << (void*)p;
    return ps;
}

template<class S, typename T>
inline PrettyOStream<S> &
operator<<(PrettyOStream<S> &ps, std::list<T> &) {
    ps.os << "std::list&";
    return ps;
}

template<class S, typename T>
inline PrettyOStream<S> &
operator<<(PrettyOStream<S> &ps, const std::list<T> &) {
    ps.os << "std::list";
    // FIXME: could show first up to N elements.
    return ps;
}

template<class S, typename K, typename V>
inline PrettyOStream<S> &
operator<<(PrettyOStream<S> &ps, std::map<K, V> *) {
    ps.os << "std::map*";
    return ps;
}

template<class S, typename K, typename V>
inline PrettyOStream<S> &
operator<<(PrettyOStream<S> &ps, std::map<K, V> &) {
    ps.os << "std::map&";
    return ps;
}

template<class S, typename K, typename V>
inline PrettyOStream<S> &
operator<<(PrettyOStream<S> &ps, const std::map<K, V> & m) {
    ps.os << "std::map(" << m.size() << ')';
    // FIXME: could show first up to N elements.
    return ps;
}

template<class S, typename T>
inline PrettyOStream<S> &
operator<<(PrettyOStream<S> &ps, const std::vector<T> & v) {
    ps.os << "std::vector(" << v.size() << ')';
    // FIXME: could show first up to N elements.
    return ps;
}

template<class S, typename T, typename U>
inline PrettyOStream<S> &
operator<<(PrettyOStream<S> &ps, const std::pair<T, U>& v) {
    ps.os << "std::pair(" << v.first << ", " << v.second << ')';
    return ps;
}

namespace Xapian {
    class ExpandDecider;
    class LatLongMetric;
    class MatchDecider;
    class Registry;
    class Weight;
    namespace Internal {
	class AndContext;
	class ExpandStats;
	class ExpandWeight;
	class OrContext;
    }
}

namespace Glass {
    class RootInfo;
}

class ChertCursor;
class ChertDatabase;
class ChertTable;
class GlassCursor;
class GlassDatabase;
class GlassFreeListChecker;
class GlassTable;

#define XAPIAN_PRETTY_AS_CLASSNAME(C)\
template<class S>\
inline PrettyOStream<S> &\
operator<<(PrettyOStream<S> &ps, const C &) {\
    ps.os << #C;\
    return ps;\
}

XAPIAN_PRETTY_AS_CLASSNAME(Xapian::ExpandDecider)
XAPIAN_PRETTY_AS_CLASSNAME(Xapian::LatLongMetric)
XAPIAN_PRETTY_AS_CLASSNAME(Xapian::MatchDecider)
XAPIAN_PRETTY_AS_CLASSNAME(Xapian::Registry)
XAPIAN_PRETTY_AS_CLASSNAME(Xapian::Weight)
XAPIAN_PRETTY_AS_CLASSNAME(Xapian::Internal::AndContext)
XAPIAN_PRETTY_AS_CLASSNAME(Xapian::Internal::ExpandStats)
XAPIAN_PRETTY_AS_CLASSNAME(Xapian::Internal::ExpandWeight)
XAPIAN_PRETTY_AS_CLASSNAME(Xapian::Internal::OrContext)
XAPIAN_PRETTY_AS_CLASSNAME(ChertCursor)
XAPIAN_PRETTY_AS_CLASSNAME(ChertDatabase)
XAPIAN_PRETTY_AS_CLASSNAME(ChertTable)
XAPIAN_PRETTY_AS_CLASSNAME(Glass::RootInfo)
XAPIAN_PRETTY_AS_CLASSNAME(GlassCursor)
XAPIAN_PRETTY_AS_CLASSNAME(GlassFreeListChecker)
XAPIAN_PRETTY_AS_CLASSNAME(GlassDatabase)
XAPIAN_PRETTY_AS_CLASSNAME(GlassTable)

template<class S>
inline PrettyOStream<S> &
operator<<(PrettyOStream<S> &ps, const Xapian::Weight *p) {
    ps.os << "(Xapian:Weight*)" << (const void*)p;
    return ps;
}

class RemoteConnection;

template<class S>
inline PrettyOStream<S> &
operator<<(PrettyOStream<S> &ps, const RemoteConnection &) {
    ps.os << "RemoteConnection";
    return ps;
}

#include "backends/database.h"

template<class S>
inline PrettyOStream<S> &
operator<<(PrettyOStream<S> &ps, const Xapian::Database::Internal *p) {
    ps.os << "(Database::Internal*)" << (const void*)p;
    return ps;
}

template<class S, class T>
inline PrettyOStream<S> &
operator<<(PrettyOStream<S> &ps, Xapian::Internal::intrusive_ptr<const T> t) {
    ps.os << "intrusive_ptr->";
    return ps << t.get();
}

#endif // XAPIAN_INCLUDED_PRETTY_H
