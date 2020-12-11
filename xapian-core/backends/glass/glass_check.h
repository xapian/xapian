/** @file
 * @brief Btree checking
 */
/* Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2002,2004,2005,2006,2008,2009,2011,2012,2013,2014 Olly Betts
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

#ifndef OM_HGUARD_GLASS_CHECK_H
#define OM_HGUARD_GLASS_CHECK_H

#include "glass_table.h"
#include "noreturn.h"

#include <iosfwd>
#include <string>

class GlassVersion;

class GlassTableCheck : public GlassTable {
  public:
    static GlassTableCheck * check(
	    const char * tablename, const std::string & path, int fd,
	    off_t offset_,
	    const GlassVersion & version_file,
	    int opts, std::ostream *out);

  private:
    GlassTableCheck(const char * tablename_, const std::string &path_,
		    bool readonly_, std::ostream *out_)
	: GlassTable(tablename_, path_, readonly_), out(out_) { }

    GlassTableCheck(const char * tablename_, int fd, off_t offset_,
		    bool readonly_, std::ostream *out_)
	: GlassTable(tablename_, fd, offset_, readonly_), out(out_) { }

    void block_check(Glass::Cursor * C_, int j, int opts,
		     GlassFreeListChecker &flcheck);
    int block_usage(const uint8_t * p) const;
    void report_block(int m, int n, const uint8_t * p) const;
    void report_block_full(int m, int n, const uint8_t * p) const;
    void report_cursor(int N, const Glass::Cursor *C_) const;

    void print_key(const uint8_t * p, int c, int j) const;
    void print_tag(const uint8_t * p, int c, int j) const;
    void print_spaces(int n) const;
    void print_bytes(int n, const uint8_t * p) const;

    std::ostream *out;
};

#endif /* OM_HGUARD_GLASS_CHECK_H */
