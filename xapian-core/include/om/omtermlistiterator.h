/* omtermlistiterator.h
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

#ifndef OM_HGUARD_OMTERMLISTITERATOR_H
#define OM_HGUARD_OMTERMLISTITERATOR_H

//#include <iterator>
#include "omtypes.h"

class OmDatabase;

class OmTermListIterator {
    //    : public iterator<input_iterator_tag, om_termpos, om_termpos, const om_termpos *, om_termpos> {
    private:
	friend class OmDatabase; // So OmDatabase can construct us

	class Internal;

	Internal *internal; // reference counted internals

        friend bool operator==(const OmTermListIterator &a, const OmTermListIterator &b);

	OmTermListIterator(Internal *internal_) {
	    internal = internal_;
	}

    public:
        ~OmTermListIterator();

	OmTermListIterator operator=(OmTermListIterator &o);
    
	const om_termname operator *();

	OmTermListIterator & operator++();

	OmTermListIterator operator++(int);

	// extra method, not required for an input_iterator
	OmTermListIterator skip_to(const om_termname & tname);

	/** Returns a string describing this object.
	 *  Introspection method.
	 */
	std::string get_description() const;
};

#endif /* OM_HGUARD_OMTERMLISTITERATOR_H */
