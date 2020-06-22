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

#include <archive.h>
#include <archive_entry.h>

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
	const char* file = filename.c_str();
	struct archive* archive_obj = archive_read_new();
	archive_read_support_filter_all(archive_obj);
	archive_read_support_format_all(archive_obj);
	int status_code = archive_read_open_filename(archive_obj,
		                                     file, 10240);

	if (status_code != ARCHIVE_OK) {
	    error = "Libarchive failed to open the file " + filename;
	    return false;
	}

	size_t total;
	ssize_t size;
	string s;
	struct archive_entry* entry;

	// extracting data from content.xml and styles.xml
	while (archive_read_next_header(archive_obj, &entry) == ARCHIVE_OK) {
	    string pathname =  archive_entry_pathname(entry);
	    if (pathname == "content.xml") {
		total = archive_entry_size(entry);
		//char buf[total];
		auto i = s.size();
		s.resize(i + total);
		size = archive_read_data(archive_obj,  &s[i], total);

		if (size <= 0) {
		    error = "Libarchive was not able to extract data from "
		            "content.xml";
		    return false;
		}

		s.resize(i + size);
	    } else if (pathname == "styles.xml") {
		total = archive_entry_size(entry);
		auto i = s.size();
		s.resize(i + total);
		size = archive_read_data(archive_obj, &s[i], total);

		if (size <= 0) {
		    error = "Libarchive was not able to extract data from "
		            "styles.xml";
		    return false;
		}

		s.resize(i + size);
	    } else if (pathname == "meta.xml") {
		total = archive_entry_size(entry);
		string s_meta;
		auto i = s_meta.size();
		s_meta.resize(i + total);
		size = archive_read_data(archive_obj, &s_meta[i], total);

		if (size <= 0) {
		    // indexing file even if this fails
		}

		s_meta.resize(i + size);
		MetaXmlParser metaxmlparser;
		metaxmlparser.parse(s_meta);
		title = metaxmlparser.title;
		keywords = metaxmlparser.keywords;
		author = metaxmlparser.author;
	    }
	}
	status_code = archive_read_free(archive_obj);
	if (status_code != ARCHIVE_OK) {
	    error += archive_error_string(archive_obj);
	    return false;
	}

	OpenDocParser parser;
	parser.parse(s);
	dump = parser.dump;
	} catch (...) {
	    error = "Libarchive threw an exception";
	    return false;
	}

	return true;
}
