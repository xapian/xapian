/** @file brass_types.h
 * @brief Types used by brass backend and the Btree manager
 */
/* Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002,2003,2004,2008,2009 Olly Betts
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

#ifndef OM_HGUARD_BRASS_TYPES_H
#define OM_HGUARD_BRASS_TYPES_H

#include "brass_defs.h"

/** An integer type for storing the length of a document - ie, the sum of the
 *  wdfs of the terms in the document.
 */
typedef unsigned int brass_doclen_t;

#endif /* OM_HGUARD_BRASS_TYPES_H */
