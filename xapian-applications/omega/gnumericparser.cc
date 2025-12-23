/** @file
 * @brief Extract text from Gnumeric spreadsheets
 */
/* Copyright (C) 2020,2025 Olly Betts
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

#include <config.h>

#include "gnumericparser.h"

#include "datetime.h"
#include "stringutils.h"

using namespace std;

// Dummy std::string to point target at, but we actually parse the value into
// the time_t created member variable.
static string created_dummy_string;

bool
GnumericParser::opening_tag(const string& tag)
{
    if (target != nullptr) return true;

    if (tag.size() < 8) return true;
    if (tag == "gnm:Cell") {
	string value;
	if (!get_attribute("ValueType", value)) return true;
	target = &dump;
    } else if (startswith(tag, "dc:")) {
	if (tag == "dc:subject") {
	    // dc:subject is "Subject and Keywords":
	    // "Typically, Subject will be expressed as keywords, key phrases
	    // or classification codes that describe a topic of the resource."
	    // Gnumeric uses meta:keyword for keywords - dc:subject
	    // comes from a text field labelled "Subject".  Let's just treat
	    // it as more keywords.
	    target = &keywords;
	} else if (tag == "dc:title") {
	    target = &title;
	} else if (tag == "dc:description") {
	    // Gnumeric's field is labelled "Comments".
	    target = &sample;
	} else if (tag == "dc:initial-creator") {
	    target = &author;
	}
    } else if (tag == "gnm:SheetName") {
	// Treat sheet names as keywords so they don't appear at the start
	// of the sample.
	target = &keywords;
    } else if (tag == "meta:creation-date") {
	target = &created_dummy_string;
    }
    return true;
}

bool
GnumericParser::closing_tag(const string&)
{
    target = nullptr;
    return true;
}

void
GnumericParser::process_content(const string& content)
{
    if (!target) return;

    if (target == &created_dummy_string) {
	if (created != time_t(-1)) return;

	// E.g. 2025-03-31T18:40:06Z
	created = parse_datetime(content);
	return;
    }

    if (!target->empty()) *target += ' ';
    *target += content;
}
