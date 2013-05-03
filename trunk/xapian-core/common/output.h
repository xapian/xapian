/** @file output.h
 * @brief Functions for output of strings describing Xapian objects.
 */
/*
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2002,2003,2004,2007,2009,2011 Olly Betts
 * Copyright 2007 Lemur Consulting Ltd
 * Copyright 2010 Richard Boulton
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

#ifndef XAPIAN_INCLUDED_OUTPUT_H
#define XAPIAN_INCLUDED_OUTPUT_H

#include <ostream>

/// @Internal Helper macro for defining stream output of Xapian class.
#define XAPIAN_OUTPUT_FUNCTION(CLASS) \
inline std::ostream & \
operator<<(std::ostream & os, const CLASS & object) { \
    return os << object.get_description(); \
}

#include <xapian/database.h>
XAPIAN_OUTPUT_FUNCTION(Xapian::Database)
XAPIAN_OUTPUT_FUNCTION(Xapian::WritableDatabase)

#include <xapian/document.h>
XAPIAN_OUTPUT_FUNCTION(Xapian::Document)

#include <xapian/query.h>
XAPIAN_OUTPUT_FUNCTION(Xapian::Query)
XAPIAN_OUTPUT_FUNCTION(Xapian::Query::Internal)

#include <xapian/enquire.h>
XAPIAN_OUTPUT_FUNCTION(Xapian::RSet)
XAPIAN_OUTPUT_FUNCTION(Xapian::MSetIterator)
XAPIAN_OUTPUT_FUNCTION(Xapian::MSet)
XAPIAN_OUTPUT_FUNCTION(Xapian::ESetIterator)
XAPIAN_OUTPUT_FUNCTION(Xapian::ESet)
XAPIAN_OUTPUT_FUNCTION(Xapian::Enquire)

#include <xapian/geospatial.h>
XAPIAN_OUTPUT_FUNCTION(Xapian::LatLongCoord)
XAPIAN_OUTPUT_FUNCTION(Xapian::LatLongCoords)

#include <xapian/stem.h>
XAPIAN_OUTPUT_FUNCTION(Xapian::Stem)

#include <xapian/postingiterator.h>
XAPIAN_OUTPUT_FUNCTION(Xapian::PostingIterator)

#include <xapian/positioniterator.h>
XAPIAN_OUTPUT_FUNCTION(Xapian::PositionIterator)

#include <xapian/termiterator.h>
XAPIAN_OUTPUT_FUNCTION(Xapian::TermIterator)

#include <xapian/valueiterator.h>
XAPIAN_OUTPUT_FUNCTION(Xapian::ValueIterator)

#include <xapian/matchspy.h>
XAPIAN_OUTPUT_FUNCTION(Xapian::MatchSpy)

#include <xapian/postingsource.h>
XAPIAN_OUTPUT_FUNCTION(Xapian::PostingSource)

#endif /* XAPIAN_INCLUDED_OUTPUT_H */
