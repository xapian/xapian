/* omstringstream.h: A replacement for stringstream.
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2002,2003,2006 Olly Betts
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301
 * USA
 */

#ifndef OM_HGUARD_OMSTRINGSTREAM_H
#define OM_HGUARD_OMSTRINGSTREAM_H

// So that we can use the output functions declared here.
#include <xapian/output.h>

#include "omtime.h"

#include <iomanip>

#ifdef HAVE_SSTREAM
#include <sstream>
typedef std::ostringstream om_ostringstream;

inline std::ostream &
operator<<(std::ostream & os, const OmTime & om_time) {
    return os << om_time.sec << '.' << std::setw(6) << std::setfill('0') << om_time.usec;
}

#else // HAVE_SSTREAM

#include <string>
#include <fstream>

class om_ostringstream {
    public:
	/// default constructor
	om_ostringstream();

	/// virtual destructor
	virtual ~om_ostringstream();

	/// Get the string
	std::string str() const;

	/// Set the string
	void str(const std::string &s);

	/// Add a string to the stringstream
	om_ostringstream & operator << (const std::string &);
	om_ostringstream & operator << (const char *);
	om_ostringstream & operator << (const void *);
	om_ostringstream & operator << (char);
	om_ostringstream & operator << (unsigned char);
	om_ostringstream & operator << (int);
	om_ostringstream & operator << (unsigned int);
	om_ostringstream & operator << (long);
	om_ostringstream & operator << (unsigned long);
	om_ostringstream & operator << (double);
	om_ostringstream & operator << (bool);

#if 0
	/// Hack to recognise endl and flush
	om_ostringstream & operator << (ostream& (*)(ostream&));
#endif
    private:
	/// Copies are not allowed.
	om_ostringstream(const om_ostringstream &);

	/// Assignment is not allowed.
	void operator=(const om_ostringstream &);

	/// The string so far
	std::string mystring;
};

inline om_ostringstream &
operator << (om_ostringstream &os, const OmTime &om_time) {
    char buf[256];
#ifdef SNPRINTF
    int len = SNPRINTF(buf, sizeof(buf), "%l.%06l", om_time.sec, om_time.usec);
    if (len == -1 || len > sizeof(buf)) len = sizeof(buf);
    return os << string(buf, len);
#else
    buf[sizeof(buf) - 1] = '\0';
    sprintf(buf, "%l.%06l", om_time.sec, om_time.usec);
    if (buf[sizeof(buf) - 1]) abort(); /* Uh-oh, buffer overrun */
    return os << buf;
#endif
}

#define OSTRINGSTREAMFUNC(X) \
    inline om_ostringstream & \
    operator << (om_ostringstream &os, const X & obj) { \
	return os << (obj.get_description()); \
    } \
    inline om_ostringstream & \
    operator << (om_ostringstream &os, const X * obj) { \
	if (obj == 0) return os << "<"#X" - NULL>"; \
	return os << (obj->get_description()); \
    }

OSTRINGSTREAMFUNC(Xapian::Database)
OSTRINGSTREAMFUNC(Xapian::WritableDatabase)
OSTRINGSTREAMFUNC(Xapian::Document)
OSTRINGSTREAMFUNC(Xapian::Query)
OSTRINGSTREAMFUNC(Xapian::RSet)
OSTRINGSTREAMFUNC(Xapian::MSet)
OSTRINGSTREAMFUNC(Xapian::MSetIterator)
OSTRINGSTREAMFUNC(Xapian::ESet)
OSTRINGSTREAMFUNC(Xapian::ESetIterator)
OSTRINGSTREAMFUNC(Xapian::Enquire)
OSTRINGSTREAMFUNC(Xapian::Stem)
OSTRINGSTREAMFUNC(Xapian::PostingIterator)
OSTRINGSTREAMFUNC(Xapian::PositionIterator)
OSTRINGSTREAMFUNC(Xapian::TermIterator)
OSTRINGSTREAMFUNC(Xapian::ValueIterator)

#undef OSTRINGSTREAMFUNC

#endif // HAVE_SSTREAM

#endif /* OM_HGUARD_STRINGSTREAM_H */
