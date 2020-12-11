/** @file
 * @brief Field parsing for Omega.
 */
/* Copyright 2018 Olly Betts
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

#ifndef OMEGA_INCLUDED_FIELDS_H
#define OMEGA_INCLUDED_FIELDS_H

#include <unordered_map>
#include <string>
#include <utility>

class Fields {
    std::unordered_map<std::string, std::string> fields;

  public:
    Fields() {}

    /** Parse fields.
     *
     *  @param data	The data to parse.
     *
     *  @param names	If non-NULL and not pointing to an empty string, then
     *			specifies the field names for corresponding lines in
     *			@a data.
     */
    void parse_fields(const std::string& data, const std::string* names);

    /// Lookup field @a name.
    const std::string& get_field(const std::string& name) const {
	auto it = fields.find(name);
	if (it != fields.end()) {
	    return it->second;
	}
	const static std::string empty_string;
	return empty_string;
    }
};

inline void
Fields::parse_fields(const std::string& data,
		     const std::string* names)
{
    fields.clear();

    if (names && !names->empty()) {
	// Each line in data is a field, with field names taken from
	// corresponding entries in the tab-separated list specified by names.
	std::string::size_type v = 0;
	std::string::size_type n = 0;
	do {
	    std::string::size_type n_start = n;
	    n = names->find('\t', n);
	    std::string::size_type v_start = v;
	    v = data.find('\n', v);
	    fields.emplace(names->substr(n_start, n - n_start),
			   data.substr(v_start, v - v_start));
	    // If n or v is std::string::npos then incrementing wraps to 0.
	} while (++n && ++v);
	return;
    }

    // Each line specifies a field in the format NAME=VALUE, where NAME doesn't
    // contain "=" but VALUE may.
    std::string::size_type i = 0;
    do {
	std::string::size_type n_start = i;
	char ch;
	while ((ch = data[i]) != '=') {
	    // Fast test for '\n' or '\0', with false positives for '\x02' and
	    // '\x08' (the latter two are probably unlikely in this context).
	    if (rare((ch &~ '\n') == 0)) {
		// Lines without an '=' should be rare.
		if (ch == '\n') {
		    // No "=" in this line - such lines are ignored.
		    ++i;
		    continue;
		}
		if (i == data.size()) {
		    return;
		}
	    }
	    ++i;
	}

	std::string::size_type eq = i;

	// Scan ahead to the end of the line.
	while ((ch = data[++i]) != '\n') {
	    if (ch == '\0' && i == data.size()) {
		i = std::string::npos;
		break;
	    }
	}

	std::string::size_type v = eq + 1;
	std::string name(data, n_start, eq - n_start);
	auto r = fields.emplace(name, std::string());
	if (r.second) {
	    // New entry - fill in.
	    r.first->second.assign(data, v, i - v);
	} else {
	    // Existing entry, so accumulate values as tab-separated list.
	    std::string& value = r.first->second;
	    value += '\t';
	    value.append(data, v, i - v);
	}
	// If i is std::string::npos then incrementing wraps to 0.
    } while (++i);
}

#endif // OMEGA_INCLUDED_FIELDS_H
