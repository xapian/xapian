/** \file output.h
 * \brief Functions for output of strings describing Xapian objects.
 */
/*
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2002,2003,2004 Olly Betts
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

#include <fstream>

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

#include <xapian/enquire.h>
XAPIAN_OUTPUT_FUNCTION(Xapian::RSet)
XAPIAN_OUTPUT_FUNCTION(Xapian::MSetIterator)
XAPIAN_OUTPUT_FUNCTION(Xapian::MSet)
XAPIAN_OUTPUT_FUNCTION(Xapian::ESetIterator)
XAPIAN_OUTPUT_FUNCTION(Xapian::ESet)
XAPIAN_OUTPUT_FUNCTION(Xapian::Enquire)

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

#endif /* XAPIAN_INCLUDED_OUTPUT_H */
