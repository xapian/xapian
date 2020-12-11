/** @file
 * @brief Track files we failed to index
 */
/* Copyright (C) 2014,2015 Olly Betts
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

#ifndef OMEGA_INCLUDED_FAILED_H
#define OMEGA_INCLUDED_FAILED_H

#include <cstdlib>
#include <sys/types.h>
#include <string>

#include "str.h"

/** Maintain a "database of failure" in the user metadata.
 *
 *  This tracks the last-modified timestamp and size for non-indexed files, so
 *  we can avoid trying to index them on every pass.
 */
class Failed {
    Xapian::WritableDatabase db;

  public:
    Failed() { }

    void init(Xapian::WritableDatabase & db_) {
	db = db_;
    }

    /** When we fail on a file, we add or replace its entry. */
    void add(const std::string & key, time_t last_mod, off_t size) {
	std::string value = str(last_mod);
	value += ',';
	value += str(size);
	db.set_metadata(key, value);
    }

    bool contains(const std::string & key, time_t& last_mod, off_t& size) {
	const std::string value = db.get_metadata(key);
	if (value.empty()) return false;
	const char * p = value.c_str();
	char * end;
	long long v = strtoll(p, &end, 10);
	if (*end != ',') return false;
	// FIXME: check conversions.
	last_mod = v;
	p = end + 1;
	v = strtoll(p, &end, 10);
	size = v;
	return true;
    }

    void del(const std::string & key) {
	db.set_metadata(key, std::string());
    }

    void clear() {
	for (auto i = db.metadata_keys_begin("U");
	     i != db.metadata_keys_end("U");
	     ++i) {
	    db.set_metadata(*i, std::string());
	}
    }

    bool empty() const {
	return db.metadata_keys_begin("U") == db.metadata_keys_end("U");
    }
};

#endif // OMEGA_INCLUDED_FAILED_H
