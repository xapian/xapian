/** \file output.h
 * \brief Functions for output of strings describing Xapian objects.
 */
/*
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2002,2003 Olly Betts
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

#ifndef XAPIAN_INCLUDED_OUTPUT_H
#define XAPIAN_INCLUDED_OUTPUT_H

#include <iosfwd>

#define XAPIAN_OUTPUT_FUNCTION(CLASS) \
inline std::ostream & \
operator<<(std::ostream & os, const CLASS & object) { \
    return os << object.get_description(); \
}

#include "om/omdatabase.h"
XAPIAN_OUTPUT_FUNCTION(OmDatabase)
XAPIAN_OUTPUT_FUNCTION(OmWritableDatabase)

#include "om/omdocument.h"
XAPIAN_OUTPUT_FUNCTION(OmDocument)

#include "om/omquery.h"
XAPIAN_OUTPUT_FUNCTION(OmQuery)

#include "om/omenquire.h"
XAPIAN_OUTPUT_FUNCTION(OmRSet)
XAPIAN_OUTPUT_FUNCTION(OmMSetIterator)
XAPIAN_OUTPUT_FUNCTION(OmMSet)
XAPIAN_OUTPUT_FUNCTION(OmESetIterator)
XAPIAN_OUTPUT_FUNCTION(OmESet)
XAPIAN_OUTPUT_FUNCTION(OmEnquire)

#include <xapian/stem.h>
XAPIAN_OUTPUT_FUNCTION(Xapian::Stem)

#include "om/ompostlistiterator.h"
XAPIAN_OUTPUT_FUNCTION(OmPostListIterator)

#include <xapian/positionlistiterator.h>
XAPIAN_OUTPUT_FUNCTION(Xapian::PositionListIterator)

#include <xapian/termiterator.h>
XAPIAN_OUTPUT_FUNCTION(Xapian::TermIterator)

#include "om/omvalueiterator.h"
XAPIAN_OUTPUT_FUNCTION(OmValueIterator)

#endif /* XAPIAN_INCLUDED_OUTPUT_H */
