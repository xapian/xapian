/* omstringstream.h : A replacement for stringstream.
 *
 * ----START-LICENCE----
 * Copyright 1999,2000 BrightStation PLC
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

#include <string>
#include <iostream>
#include <streambuf.h>
#include <memory>

template <class Ch, class Tr = string_char_traits<Ch> >
class om_stringbuf : public streambuf {
    friend class om_ostringstream;
    public:
	om_stringbuf();
    protected:
	int overflow(int c = EOF);

    private:
	std::string buffer;
};

class om_ostringstream : public ostream {
    public:
	/// default constructor
	om_ostringstream();

	/// virtual destructor
	virtual ~om_ostringstream();

	std::string str() const;

    private:
	/** Pointer to the streambuf we use.
	 *  Used to delete it at the end, and to access the
	 *  internal string.
	 */
	auto_ptr<om_stringbuf<char> > ombuf;
};

#endif /* OM_HGUARD_STRINGSTREAM_H */
