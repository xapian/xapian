/** @file chert_chunkedlisttable.h
 * @brief Subclass of ChertTable which holds chunked lists.
 */
/* Copyright (C) 2007,2008 Olly Betts
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#ifndef XAPIAN_INCLUDED_CHERT_CHUNKEDLISTTABLE_H
#define XAPIAN_INCLUDED_CHERT_CHUNKEDLISTTABLE_H

#include "chert_table.h"

#include <string>

class ChertChunkedListTable : public ChertTable {
  public:
    /** Create a new ChertChunkedListTable object.
     *
     *  This method does not create or open the table on disk - you
     *  must call the create() or open() methods respectively!
     *
     *  @param name_	    The table name (e.g. "values").
     *  @param append	    What to append to dbdir to get (e.g. "/values.").
     *  @param dbdir	    The directory the chert database is stored in.
     *  @param readonly	    true if we're opening read-only, else false.
     *  @param compress_strategy_	DONT_COMPRESS, Z_DEFAULT_STRATEGY,
     *					Z_FILTERED, Z_HUFFMAN_ONLY, or Z_RLE.
     *  @param lazy_	    If true, don't create the table until it's needed.
     */
    ChertChunkedListTable(const char * name_, const char * append,
			  const std::string & dbdir, bool readonly,
			  int compress_strategy_ = DONT_COMPRESS,
			  bool lazy_ = false)
	: ChertTable(name_, dbdir + append, readonly, compress_strategy_, lazy_)
    { }
};

#endif // XAPIAN_INCLUDED_CHERT_CHUNKEDLISTTABLE_H
