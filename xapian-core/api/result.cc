/** @file result.cc
 * @brief A result inside an MSet
 */
/* Copyright 2017 Olly Betts
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

#include <config.h>

#include "result.h"

#include "str.h"
#include "unicode/description_append.h"

using namespace std;

string
Result::get_description() const
{
    string desc = "Result(";
    desc += str(did);
    desc += ", ";
    desc += str(weight);
    if (!sort_key.empty()) {
	desc += ", sort_key=";
	description_append(desc, sort_key);
    }
    if (!collapse_key.empty()) {
	desc += ", collapse_key=";
	description_append(desc, collapse_key);
    }
    if (collapse_count) {
	desc += ", collapse_count=";
	desc += str(collapse_count);
    }
    desc += ')';
    return desc;
}
