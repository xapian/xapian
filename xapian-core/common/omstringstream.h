/* omstringstream.h: A replacement for stringstream.
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2002 Olly Betts
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 * -----END-LICENCE-----
 */

#ifndef OM_HGUARD_OMSTRINGSTREAM_H
#define OM_HGUARD_OMSTRINGSTREAM_H


// So that we can use the output functions declared here.
#include "om/omoutput.h"

#ifdef HAVE_SSTREAM
#include <sstream>
typedef std::ostringstream om_ostringstream;

#else // HAVE_SSTREAM

#include <string>
#include <iostream>
#include <vector>

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

OSTRINGSTREAMFUNC(OmDatabase)
OSTRINGSTREAMFUNC(OmWritableDatabase)
OSTRINGSTREAMFUNC(OmDocument)
OSTRINGSTREAMFUNC(OmQuery)
OSTRINGSTREAMFUNC(OmRSet)
OSTRINGSTREAMFUNC(OmMSet)
OSTRINGSTREAMFUNC(OmMSetIterator)
OSTRINGSTREAMFUNC(OmESet)
OSTRINGSTREAMFUNC(OmESetIterator)
OSTRINGSTREAMFUNC(OmEnquire)
OSTRINGSTREAMFUNC(OmStem)
OSTRINGSTREAMFUNC(OmPostListIterator)
OSTRINGSTREAMFUNC(OmPositionListIterator)
OSTRINGSTREAMFUNC(OmTermIterator)
OSTRINGSTREAMFUNC(OmValueIterator)

inline om_ostringstream &
operator<<(om_ostringstream & os, const std::vector<std::string> & v)
{
    os << "vector<string> {";
    std::vector<std::string>::const_iterator i;
    for (i = v.begin(); i != v.end(); i++) {
	if (i != v.end()) os << ", ";
	os << '"' << *i << '"';
    }
    os << "}";
    return os;
}

#undef OSTRINGSTREAMFUNC

#endif // HAVE_SSTREAM

#endif /* OM_HGUARD_STRINGSTREAM_H */
