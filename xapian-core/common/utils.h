/* utils.h: Various useful utilities
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

#ifndef OM_HGUARD_UTILS_H
#define OM_HGUARD_UTILS_H

#include <string>

// Convert an integer to a string
string om_inttostring(int a);

// Convert a double to a string
string doubletostring(double a);

///////////////////////////////////////////
// Mapping of types as strings to enums  //
///////////////////////////////////////////

template<class X> struct stringToType {
    string name;
    X type;
};

// Note: this just uses a list of entrys, and searches linearly through
// them.  Could at make this do a binary chop, but probably not worth
// doing so, unless list gets large.
template<class X> class stringToTypeMap {
    public:
	static stringToType<X> types[];
	static X get_type(string needle) {
	    stringToType<X>* haystack = types;
	    while(haystack->name.size() != 0) {
		if(haystack->name == needle) break;
		haystack++;
	    }
	    return haystack->type;
	}
};

#endif /* OM_HGUARD_UTILS_H */
