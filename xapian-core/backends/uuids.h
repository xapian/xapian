/** @file
 * @brief Class for handling UUIDs
 */
/* Copyright 2008 Lemur Consulting Ltd
 * Copyright 2009,2010,2013,2016,2018 Olly Betts
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#ifndef XAPIAN_INCLUDED_UUIDS_H
#define XAPIAN_INCLUDED_UUIDS_H

#include <cstring>
#include <string>

class Uuid {
  public:
    /// The size of a UUID in bytes.
    static constexpr unsigned BINARY_SIZE = 16;

    /// The size of a UUID string in bytes (not including trailing '\0').
    static constexpr unsigned STRING_SIZE = 36;

    Uuid() {}

    void generate();

    void parse(const char* in);

    void parse(const std::string& in) { return parse(in.data()); }

    // Not currently used outside unittest.cc.
    void clear() {
	std::memset(uuid_data, 0, BINARY_SIZE);
    }

    // Not currently used outside unittest.cc.
    bool is_null() const {
	for (auto ch : uuid_data) {
	    if (ch)
		return false;
	}
	return true;
    }

    std::string to_string() const;

    const char* data() const {
	return reinterpret_cast<const char*>(uuid_data);
    }

    void assign(const char* p) { std::memcpy(uuid_data, p, BINARY_SIZE); }

  private:
    /// The UUID data.
    unsigned char uuid_data[BINARY_SIZE];
};

#endif /* XAPIAN_INCLUDED_UUIDS_H */
