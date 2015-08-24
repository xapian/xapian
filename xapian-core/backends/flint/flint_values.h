/* flint_values.h: Values in flint databases
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2002,2003,2004,2008 Olly Betts
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

#ifndef OM_HGUARD_FLINT_VALUES_H
#define OM_HGUARD_FLINT_VALUES_H

#include <map>
#include <string>

#include <xapian/types.h>
#include <xapian/valueiterator.h>
#include "flint_table.h"

using namespace std;

class FlintValueTable : public FlintTable {
    private:
	/** Read an entry from position.  Throw appropriate exceptions if
	 *  data runs out.
	 */
	static void unpack_entry(const char ** pos,
				 const char * end,
				 Xapian::valueno * this_value_no,
				 string & this_value);

    public:
	/** Create a new table object.
	 *
	 *  This does not create the table on disk - the create() method must
	 *  be called before the table is created on disk
	 *
	 *  This also does not open the table - the open() method must be
	 *  called before use is made of the table.
	 *
	 *  @param path_          - Path at which the table is stored.
	 *  @param readonly_      - whether to open the table for read only
	 *                          access.
	 */
	FlintValueTable(const string & path_, bool readonly_)
	    : FlintTable("value", path_ + "/value.", readonly_, DONT_COMPRESS, true) { }

	/** Encode values as a string ready to add to the table.
	 */
	void encode_values(string & s,
			   Xapian::ValueIterator it,
			   const Xapian::ValueIterator & end);

	/** Set values for document @a did encoded as a string. */
	void set_encoded_values(Xapian::docid did, const string & enc);

	/** Get a value.
	 *
	 *  @return The value if found, a null value otherwise.
	 */
	void get_value(string & value, Xapian::docid did,
		       Xapian::valueno slot) const;

	/** Get all values.
	 *
	 *  @param values  A map to be filled with all the values
	 *                     for the specified document.
	 *
	 */
	void get_all_values(map<Xapian::valueno, string> & values,
			    Xapian::docid did) const;

	/** Remove all values.
	 *
	 *  @param did	The document id for which to remove the values.
	 *
	 */
	void delete_all_values(Xapian::docid did);
};

#endif /* OM_HGUARD_FLINT_VALUES_H */
