/* deleter_map.h: a simple version of std::map for pointers which deletes 
 *                the pointers when destroyed.
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

#ifndef OM_HGUARD_DELETER_MAP_H
#define OM_HGUARD_DELETER_MAP_H

#include "config.h"

#include <map>

template <class T, class U>
class deleter_map : private std::map<T, U> {
    public:
	typedef std::map<T, U> base;
	using std::map<T, U>::iterator;
	using std::map<T, U>::const_iterator;
	using std::map<T, U>::begin;
	using std::map<T, U>::end;
	using std::map<T, U>::find;

	U &operator[](const T &key) {
	    typename base::const_iterator i = base::find(key);
	    if (i == base::end()) {
		// preset to 0 if not present
		base::operator[](key) = 0;
	    }
	    return base::operator[](key);
	}
	void erase(typename base::iterator i) {
	    if (i != end()) {
		delete i->second;
		i->second = 0;
		base::erase(i);
	    }
	}
	void clear() {
	    for (typename base::iterator i = base::begin();
		 i != base::end();
		 ++i) {
		delete i->second;
		i->second = 0;
	    }
	    base::clear();
	}
	~deleter_map() {
	    for (typename std::map<T,U>::iterator i = begin();
		 i != end();
		 ++i) {
		delete i->second;
	    }
	}
};

#endif /* OM_HGUARD_DELETER_MAP_H */
