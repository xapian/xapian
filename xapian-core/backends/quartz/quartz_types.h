/* quartz_types.h: Types used by quartz backend
 *
 * ----START-LICENCE----
 * Copyright 1999,2000 BrightStation PLC
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

#ifndef OM_HGUARD_QUARTZ_TYPES_H
#define OM_HGUARD_QUARTZ_TYPES_H

#include "config.h"

typedef unsigned int quartz_blocksize_t;

/** A type used to store a revision number for a database.
 *
 *  FIXME: ensure that this is of a suitable minimum size - 32 bits is
 *  satisfactory in most cases, but in extreme cases could cause the revision
 *  number to roll over after a few years.  It would be better to use 64 bits
 *  (and / or to ensure that rolling over causes no problem).
 */
typedef unsigned long int quartz_revision_number_t;

#endif /* OM_HGUARD_QUARTZ_TYPES_H */
