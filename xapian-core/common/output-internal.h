/** @file output-internal.h
 * @brief Functions for output of strings describing internal Xapian objects.
 */
/*
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2002,2003,2004,2007,2009,2011,2012 Olly Betts
 * Copyright 2007 Lemur Consulting Ltd
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

#ifndef XAPIAN_INCLUDED_OUTPUT_INTERNAL_H
#define XAPIAN_INCLUDED_OUTPUT_INTERNAL_H

#include "output.h"

#include "api/replication.h"
XAPIAN_OUTPUT_FUNCTION(Xapian::DatabaseMaster)
XAPIAN_OUTPUT_FUNCTION(Xapian::DatabaseReplica)

#include "weight/weightinternal.h"
XAPIAN_OUTPUT_FUNCTION(TermFreqs)
XAPIAN_OUTPUT_FUNCTION(Xapian::Weight::Internal)

#endif /* XAPIAN_INCLUDED_OUTPUT_INTERNAL_H */
