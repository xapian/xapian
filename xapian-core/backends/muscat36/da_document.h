/* da_document.h: C++ class definition for DA access routines
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

#ifndef OM_HGUARD_DA_DOCUMENT_H
#define OM_HGUARD_DA_DOCUMENT_H

#include "om/omdocument.h"
#include "daread.h"

class DADatabase;

class DADocument : public virtual OmDocument {
    friend class DADatabase;
    private:
	const DADatabase * database;
	om_docid did;
	mutable struct record * rec;
	int heavy_duty;

	DADocument(const DADatabase * database_,
		   om_docid did_,
		   int heavy_duty_);

	// Stop copying
	DADocument(const DADocument &);
	DADocument & operator = (const DADocument &);
    public:
	~DADocument();

	OmKey get_key(om_keyno keyid) const;
	OmData get_data() const;
};

inline
DADocument::DADocument(const DADatabase * database_, om_docid did_,
		       int heavy_duty_)
	: database(database_), did(did_), rec(NULL), heavy_duty(heavy_duty_)
{
}


#endif /* OM_HGUARD_DA_DOCUMENT_H */
