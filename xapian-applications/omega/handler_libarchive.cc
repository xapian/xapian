/** @file handler_libarchive.cc
 * @brief Extract text and metadata using libarchive.
 */
/* Copyright (C) 2020 Parth Kapadia
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
#include <config.h>
#include "handler.h"

#include "metaxmlparse.h"
#include "opendocparse.h"
#include "str.h"

#include<archive.h>
#include<archive_entry.h>

#include <cstring>

using namespace std;

bool
extract(const string& filename,
	string& dump,
	string& title,
	string& keywords,
	string& author,
	string& pages,
	string& error)
{
    try {
	struct archive* a;
	struct archive_entry* entry;
	int status_code;
	const char* file = &filename[0];
	a = archive_read_new();
	archive_read_support_filter_all(a);
	archive_read_support_format_all(a);
	status_code = archive_read_open_filename(a, file, 10240);

	if (status_code != ARCHIVE_OK) {
	    error = "Libarchive failed to open the file " + filename;
		return false;
	}

	size_t total;
	ssize_t size;
	string s = "";

	// extracting data from content.xml and styles.xml
	while (archive_read_next_header(a, &entry) == ARCHIVE_OK) {
	    if (strcmp(archive_entry_pathname(entry), "content.xml") == 0) {
		total = archive_entry_size(entry);
		char buf1[total];
		size = archive_read_data(a, buf1, total);

		if (size <= 0) {
		    error = "Libarchive was not able to extract data from "
		    "content.xml";
		    return false;
		}

		s += str(buf1);
		} else if (strcmp(
			archive_entry_pathname(entry), "styles.xml") == 0) {
		total = archive_entry_size(entry);
		char buf2[total];
		size = archive_read_data(a, buf2, total);

		if (size <= 0) {
		    error = "Libarchive was not able to extract data from "
		    "styles.xml";
		    return false;
		}

		s += str(buf2);
		OpenDocParser parser;
		parser.parse(s);
		dump = parser.dump;
		} else if (strcmp(
			archive_entry_pathname(entry), "meta.xml") == 0) {
		total = archive_entry_size(entry);
		char buf3[total];
		size = archive_read_data(a, buf3, total);

		if (size <= 0) {
		    // error handling
		}

		s = str(buf3);
		MetaXmlParser metaxmlparser;
		metaxmlparser.parse(s);
		title = metaxmlparser.title;
		keywords = metaxmlparser.keywords;
		author = metaxmlparser.author;
		}
	}
	status_code = archive_read_free(a);
	if (status_code != ARCHIVE_OK) {
	    return false;
	}

	} catch (...) {
	    error = "Libarchive threw an exception";
	    return false;
	}

	return true;
}
