/** @file stats.cc
 * @brief Implementation of methods from stats.h
 */
/* Copyright (C) 2007 Lemur Consulting Ltd
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
#include "stats.h"

#include "utils.h"

using namespace std;

std::string
Stats::get_description() const
{
    string result("Stats(");
    result += "collection_size=" + om_tostring(collection_size);
    result += ", rset_size=" + om_tostring(rset_size);
    result += ", average_length=" + om_tostring(average_length);
    std::map<string, Xapian::doccount>::const_iterator i;
    for (i = termfreq.begin(); i != termfreq.end(); ++i) {
	result += ", termfreq[" + i->first + "]=" + om_tostring(i->second);
    }
    for (i = reltermfreq.begin(); i != reltermfreq.end(); ++i) {
	result += ", reltermfreq[" + i->first + "]=" + om_tostring(i->second);
    }
    result += ")";
    return result;
}
