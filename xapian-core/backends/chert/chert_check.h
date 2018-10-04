/** @file chert_check.h
 * @brief Btree checking
 */
/* Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2002,2004,2005,2006,2008,2011,2012,2013,2014 Olly Betts
 * Copyright 2008 Lemur Consulting Ltd
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

#ifndef OM_HGUARD_CHERT_CHECK_H
#define OM_HGUARD_CHERT_CHECK_H

#include "chert_table.h"
#include "noreturn.h"

#include <iosfwd>
#include <string>

class ChertTableCheck : public ChertTable {
    public:
	static void check(const char * tablename, const std::string & path,
			  chert_revision_number_t * rev_ptr,
			  int opts, std::ostream *out);
    private:
	ChertTableCheck(const char * tablename_, const std::string &path_,
			bool readonly, std::ostream *out_)
	    : ChertTable(tablename_, path_, readonly), out(out_),
	      check_item_count(0), check_sequential(true),
	      last_sequential_block(0) { }

	void block_check(Cursor * C_, int j, int opts);
	int block_usage(const uint8_t * p) const;
	void report_block(int m, int n, const uint8_t * p) const;
	void report_block_full(int m, int n, const uint8_t * p) const;
	void report_cursor(int N, const Cursor *C_) const;

	XAPIAN_NORETURN(void failure(const char * msg) const);
	void print_key(const uint8_t * p, int c, int j) const;
	void print_tag(const uint8_t * p, int c, int j) const;
	void print_spaces(int n) const;
	void print_bytes(int n, const uint8_t * p) const;

	std::ostream *out;

	chert_tablesize_t check_item_count;

	bool check_sequential;

	uint4 last_sequential_block;
};

#endif /* OM_HGUARD_CHERT_CHECK_H */
