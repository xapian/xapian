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
#include "msxmlparse.h"
#include "opendocparse.h"

#include <archive.h>
#include <archive_entry.h>

#define DEFAULT_BLOCK_SIZE 10240

using namespace std;

bool
extract(const string& filename,
	const string& mimetype,
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
	archive_read_support_format_zip(archive_obj);
	// Block size will be determined by libarchive automatically for
	// regular files. Specified block size will only be used for tape drives
	// 10240 is chosen as default size (20 records - 512 bytes each)
	int status_code = archive_read_open_filename(archive_obj, file,
						     DEFAULT_BLOCK_SIZE);

	if (status_code != ARCHIVE_OK) {
	    error = "Libarchive failed to open the file " + filename;
	    return false;
	}

	size_t total;
	ssize_t size;
	bool metaxml = false;
	string s, s_meta;
	struct archive_entry* entry;

	if (startswith(mimetype, "application/vnd.sun.xml.") ||
	    startswith(mimetype, "application/vnd.oasis.opendocument.")) {
	    string s_content, s_styles;
	    while (archive_read_next_header(archive_obj, &entry) == ARCHIVE_OK) {
		string pathname = archive_entry_pathname(entry);
		if (pathname == "content.xml") {
		    total = archive_entry_size(entry);
		    s_content.resize(total);
		    size = archive_read_data(archive_obj, &s_content[0], total);

		    if (size <= 0) {
			error = "Libarchive was not able to extract data from "
				"content.xml";
			return false;
		    }
		    s_content.resize(size);
		} else if (pathname == "styles.xml") {
		    total = archive_entry_size(entry);
		    s_styles.resize(total);
		    size = archive_read_data(archive_obj, &s_styles[0], total);

		    if (size <= 0) {
			error = "Libarchive was not able to extract data from "
				"styles.xml";
			return false;
		    }

		    s_styles.resize(size);
		} else if (pathname == "meta.xml") {
			total = archive_entry_size(entry);
			s_meta.resize(total);
			size = archive_read_data(archive_obj, &s_meta[0],
						 total);

			if (size > 0) {
			    // indexing file even if this fails
			    metaxml = true;
			}
		    s_meta.resize(size);
		    }
	    }
	    s = s_content + s_styles;
		OpenDocParser parser;
		parser.parse(s);
		dump = parser.dump;
	} else if (startswith(mimetype, "application/"
					"vnd.openxmlformats-officedocument.")) {
	    string tail(mimetype, 46);
	    // True for wordprocessingml and presentationml
	    // Run MSXmlParser if true
	    bool msxml = false;
	    if (startswith(tail, "wordprocessingml.")) {
		msxml = true;
		while (archive_read_next_header(archive_obj, &entry) == ARCHIVE_OK) {
		    string pathname = archive_entry_pathname(entry);
		    if (pathname == "word/document.xml") {
			auto i = s.size();
			total = archive_entry_size(entry);
			s.resize(i + total);
			size = archive_read_data(archive_obj, &s[i], total);

			if (size <= 0) {
			    error = "Libarchive was not able to extract data "
				    "from word/document.xml";
			    return false;
			}
			s.resize(i + size);
		    } else if (startswith(pathname, "word/header") ||
			     startswith(pathname, "word/footer")) {
			auto i = s.size();
			total = archive_entry_size(entry);
			s.resize(i + total);
			size = archive_read_data(archive_obj, &s[0], total);

			if (size <= 0) {
			    // Ignore this as header/footer may not be present
			}
			s.resize(i + size);
		    } else if (pathname == "docProps/core.xml") {
			total = archive_entry_size(entry);
			    s_meta.resize(total);
			    size = archive_read_data(archive_obj, &s_meta[0],
						     total);
			if (size > 0)
			    metaxml = true;
			s_meta.resize(size);
		    }
		}
	    } else if (startswith(tail, "spreadsheetml.")) {
		string shared_strings, sheets;
		while (archive_read_next_header(archive_obj, &entry) == ARCHIVE_OK) {
		    string pathname = archive_entry_pathname(entry);
		    if (pathname == "xl/styles.xml" ||
			pathname == "xl/workbook.xml" ||
			pathname == "xl/sharedStrings.xml") {
			auto i = shared_strings.size();
			total = archive_entry_size(entry);
			      shared_strings.resize(i + total);
			      size = archive_read_data(archive_obj,
						       &shared_strings[i],
						       total);

			      if (size <= 0) {
				  // Ignore this as header/footer
 				  // may not be present
			      }
			      shared_strings.resize(i + size);
		    } else if (startswith(pathname, "xl/worksheets/sheet")) {
			auto i = sheets.size();
			total = archive_entry_size(entry);
			sheets.resize(i + total);
			size = archive_read_data(archive_obj, &sheets[i],
						 total);

			if (size <= 0) {
			    // Ignore this as header/footer may not be present
			}
			sheets.resize(i + size);
		    } else if (pathname == "docProps/core.xml") {
			total = archive_entry_size(entry);
			    s_meta.resize(total);
			    size = archive_read_data(archive_obj, &s_meta[0],
						     total);
			if (size > 0)
			    metaxml = true;
			s_meta.resize(size);
		    }
		}
		s = shared_strings + sheets;
	    } else if (startswith(tail, "presentationml.")) {
		msxml = true;
		while (archive_read_next_header(archive_obj, &entry) == ARCHIVE_OK) {
		    string pathname = archive_entry_pathname(entry);
		    if (startswith(pathname, "ppt/slides/slide") ||
			startswith(pathname, "ppt/notesSlides/notesSlide") ||
			startswith(pathname, "ppt/comments/comment")) {
			auto i = s.size();
			total = archive_entry_size(entry);
			s.resize(i + total);
			size = archive_read_data(archive_obj, &s[i], total);

			if (size <= 0) {
			    error = "Libarchive was not able to extract data "
				    "from word/document.xml";
			    return false;
			}
			s.resize(i + size);
		    } else if (pathname == "docProps/core.xml") {
			total = archive_entry_size(entry);
			    s_meta.resize(total);
			    size = archive_read_data(archive_obj, &s_meta[0],
						     total);
			if (size > 0)
			    metaxml = true;
			s_meta.resize(size);
		    }
		}
	    }
	    if (msxml) {
		MSXmlParser xmlparser;
		xmlparser.parse_xml(s);
		dump = xmlparser.dump;
	    } else {
		XlsxParser parser;
		    parser.parse(s);
		dump = parser.dump;
	    }
	}
	if (metaxml) {
	    MetaXmlParser metaxmlparser;
	    metaxmlparser.parse(s_meta);
	    title = metaxmlparser.title;
	    keywords = metaxmlparser.keywords;
	    author = metaxmlparser.author;
	}

	status_code = archive_read_free(archive_obj);
	if (status_code != ARCHIVE_OK) {
	    error = archive_error_string(archive_obj);
	    return false;
	}
    } catch (...) {
	error = "Libarchive threw an exception";
	return false;
    }
    return true;
}
