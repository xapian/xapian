/** \file omoutput.h
 * \brief Functions for outputting strings describing OM objects.
 */
/*
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

#ifndef OM_HGUARD_OMOUTPUT_H
#define OM_HGUARD_OMOUTPUT_H

#include <iostream>

#define OUTPUT_FUNCTION(a) \
inline std::ostream & \
operator<<(std::ostream & os, const a & obj) { \
    return os << obj.get_description(); \
}

#include "om/omdatabase.h"
OUTPUT_FUNCTION(OmDatabase)
OUTPUT_FUNCTION(OmWritableDatabase)

#include "om/omdocument.h"
OUTPUT_FUNCTION(OmDocument)

#include "om/omquery.h"
OUTPUT_FUNCTION(OmQuery)

#include "om/omenquire.h"
OUTPUT_FUNCTION(OmRSet)
OUTPUT_FUNCTION(OmMSetIterator)
OUTPUT_FUNCTION(OmMSet)
OUTPUT_FUNCTION(OmESetIterator)
OUTPUT_FUNCTION(OmESet)
OUTPUT_FUNCTION(OmEnquire)

#include "xapian/stem.h"
OUTPUT_FUNCTION(Xapian::Stem)

#include "om/ompostlistiterator.h"
OUTPUT_FUNCTION(OmPostListIterator)

#include "xapian/positionlistiterator.h"
OUTPUT_FUNCTION(Xapian::PositionListIterator)

#include "xapian/termiterator.h"
OUTPUT_FUNCTION(Xapian::TermIterator)

#include "om/omvalueiterator.h"
OUTPUT_FUNCTION(OmValueIterator)

#endif /* OM_HGUARD_OMOUTPUT_H */
