/** @file brass_compact.h
 * @brief Compact a brass database, or merge and compact several.
 */
/* Copyright (C) 2004,2005,2006,2007,2008,2009,2010 Olly Betts
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

#ifndef XAPIAN_INCLUDED_BRASS_COMPACT_H
#define XAPIAN_INCLUDED_BRASS_COMPACT_H

#include <vector>
#include <string>

#include "xapian/compactor.h"
#include "xapian/types.h"

void
compact_brass(Xapian::Compactor & compactor,
	      const char * destdir, const std::vector<std::string> & sources,
	      const std::vector<Xapian::docid> & offset, size_t block_size,
	      Xapian::Compactor::compaction_level compaction, bool multipass,
	      Xapian::docid last_docid);

#endif
