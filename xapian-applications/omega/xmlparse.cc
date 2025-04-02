/** @file
 * @brief Extract text from Abiword documents.
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

#include "xmlparse.h"

#include <ctime>

#include "strptime.h"
#include "timegm.h"

using namespace std;

// Dummy std::string to point target at, but we actually parse the value into
// the time_t created member variable.
static string created_dummy_string;

bool
XmlParser::opening_tag(const string& tag)
{
    if (target != nullptr ) return true;

    if (tag == "m") {
	string value;
	if (!get_parameter("key", value)) return true;

	if (value == "dc.subject") {
	    // Abiword uses abiword.keywords for keywords - dc.subject
	    // comes from a text field labelled "Subject".  Let's just treat
	    // it as more keywords.
	    target = &keywords;
	} else if (value == "dc.title") {
	    target = &title;
	} else if (value == "dc.description") {
	    target = &sample;
	} else if (value == "dc.creator" || value == "dc.contributor") {
	    target = &author;
	} else if (value == "dc.date") {
	    // Seems to be used as created date (with abiword.date_last_changed
	    // used for the last modified date).  A quick peek at the Abiword
	    // code suggests it preserves "dc.date" on "Save" but updates it on
	    // "Save As".
	    target = &created_dummy_string;
	} else if (value == "abiword.keywords") {
	    target = &keywords;
	}
    } else if (tag == "section") {
	target = &dump;
    }
    return true;
}

bool
XmlParser::closing_tag(const string& tag)
{
    if (tag == "m" || tag == "section")
	target = nullptr;
    return true;
}

void
XmlParser::process_text(const string& content)
{
    if (!target) return;

    if (target == &created_dummy_string) {
	if (created != time_t(-1)) return;

	// E.g. Sun Jul 26 20:07:54 2020
	struct tm tm = {};
	if (strptime(content.c_str(), "%a %b %d %T %Y", &tm)) {
	    // Testing shows abiword writes the timestamp in the creator's
	    // timezone, but which timezone that is does not seem to be
	    // recorded in the saved file so we convert assuming UTC as that's
	    // at least fairly central in the range of possibilities.
	    created = timegm(&tm);
	}
	return;
    }

    if (!target->empty()) *target += ' ';
    *target += content;
}
