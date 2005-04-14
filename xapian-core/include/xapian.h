// xapian.h: Public interfaces for core Xapian library
//
// Copyright (C) 2003,2004,2005 Olly Betts
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 2 of the
// License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA

#ifndef XAPIAN_INCLUDED_XAPIAN_H
#define XAPIAN_INCLUDED_XAPIAN_H

// Set defines for library version and check C++ ABI versions match
#include <xapian/version.h>

// Types
#include <xapian/types.h>

// Exceptions
#include <xapian/error.h>
#include <xapian/errorhandler.h>

// Access to databases, documents, etc
#include <xapian/database.h>
#include <xapian/document.h>
#include <xapian/positioniterator.h>
#include <xapian/postingiterator.h>
#include <xapian/termiterator.h>
#include <xapian/valueiterator.h>

// Searching
#include <xapian/enquire.h>
#include <xapian/expanddecider.h>
#include <xapian/query.h>
#include <xapian/queryparser.h>

// Stemming
#include <xapian/stem.h>

#endif /* XAPIAN_INCLUDED_XAPIAN_H */
