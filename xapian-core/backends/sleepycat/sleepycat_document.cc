/* sleepy_document.cc: A document in a sleepycat database
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

#include "sleepy_document.h"
#include <om/omdocument.h>

SleepyDocument::SleepyDocument(Db * db_, om_docid did_)
	: db(db_),
	  did(did_)
{
    Assert(false);
}

SleepyDocument::SleepyDocument(Db * db_,
			       const OmData & data_,
			       const map<om_keyno, OmKey> & keys_ )
	: db(db_)
{
    Assert(false);
}

om_docid
SleepyDocument::get_docid() const
{
    Assert(false);
}

OmKey
SleepyDocument::do_get_key(om_keyno keyid) const
{
    Assert(false);
}

OmData
SleepyDocument::do_get_data() const
{
    Assert(false);
}
