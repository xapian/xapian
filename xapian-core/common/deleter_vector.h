/* deleter_vector.h: a simple version of std::vector for pointers which
 *                   deletes the pointers when destroyed.
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
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

#ifndef OM_HGUARD_DELETER_VECTOR_H
#define OM_HGUARD_DELETER_VECTOR_H

#include <config.h>

#include "omdebug.h"
#include <vector>

template <class T>
class deleter_vector : private std::vector<T> {
    public:
	typedef std::vector<T> base;
	using std::vector<T>::iterator;
	using std::vector<T>::const_iterator;
	using std::vector<T>::begin;
	using std::vector<T>::end;
	using std::vector<T>::push_back;
	using std::vector<T>::size;
	using std::vector<T>::operator[];

	void swap(deleter_vector<T>& other) {
	    DEBUGCALL(UNKNOWN, void, "deleter_vector::swap", "<deleter_vector>");
	    base::swap(other);
	}
	typename base::iterator erase(typename base::iterator i) {
	    DEBUGCALL(UNKNOWN, void, "deleter_vector::erase", "<iterator>");
	    if (i != end()) {
		delete *i;
		*i = 0;
		return base::erase(i);
	    }
	    return i;
	}
	void clear() {
	    DEBUGCALL(UNKNOWN, void, "deleter_vector::clear", "");
	    for (typename base::iterator i = base::begin();
		 i != base::end();
		 ++i) {
		delete *i;
	    }
	    base::clear();
	}
	~deleter_vector() {
	    DEBUGCALL(UNKNOWN, void, "deleter_vector::~deleter_vector", "");
	    for (typename base::iterator i = base::begin();
		 i != end();
		 ++i) {
		delete *i;
	    }
	}
};

#endif /* OM_HGUARD_DELETER_VECTOR_H */
