/* da_database.h: C++ class definition for DA access routines
 *
 * ----START-LICENCE----
 * Copyright 1999,2000 Dialog Corporation
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

#ifndef _da_record_h_
#define _da_record_h_

#include "irdocument.h"
#include "daread.h"

class DADatabase;

class DADocument : public virtual IRDocument {
    friend class DADatabase;
    private:
	const DADatabase * database;
	docid did;
	mutable struct record * rec;

	DADocument(const DADatabase *, docid);

	// Stop copying
	DADocument(const DADocument &);
	DADocument & operator = (const DADocument &);
    public:
	~DADocument();

	IRKey get_key(keyno) const;
	IRData get_data() const;
};

inline
DADocument::DADocument(const DADatabase * _database, docid _did)
	: database(_database), did(_did), rec(NULL)
{
}


#endif /* _da_record_h_ */
