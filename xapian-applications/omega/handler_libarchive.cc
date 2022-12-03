/** @file
 * @brief Extract text and metadata using libarchive.
 */
/* Copyright (C) 2020 Parth Kapadia
 * Copyright (C) 2022 Olly Betts
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

#include "msxmlparser.h"
#include "opendocmetaparser.h"
#include "opendocparser.h"
#include "stringutils.h"
#include "xlsxparser.h"
#include "xpsparser.h"

#include <archive.h>
#include <archive_entry.h>

#define DEFAULT_BLOCK_SIZE 10240

using namespace std;

static void
parse_metadata(const string& metadata)
{
    OpenDocMetaParser metaparser;
    metaparser.parse(metadata);
    send_field(FIELD_TITLE, metaparser.title);
    send_field(FIELD_KEYWORDS, metaparser.keywords);
    send_field(FIELD_AUTHOR, metaparser.author);
    send_field_page_count(metaparser.pages);
}

static bool
extract_opendoc(struct archive* archive_obj)
{
    string styles;
    OpenDocParser parser;

    struct archive_entry* entry;
    while (archive_read_next_header(archive_obj, &entry) == ARCHIVE_OK) {
	size_t total;
	ssize_t size;
	string pathname = archive_entry_pathname(entry);
	if (pathname == "content.xml") {
	    total = archive_entry_size(entry);
	    string content(total, '\0');
	    size = archive_read_data(archive_obj, &content[0], total);

	    if (size <= 0) {
		send_field(FIELD_ERROR, "Failed to read content.xml");
		return false;
	    }
	    content.resize(size);
	    parser.parse(content);
	} else if (pathname == "styles.xml") {
	    total = archive_entry_size(entry);
	    styles.resize(total);
	    size = archive_read_data(archive_obj, &styles[0], total);

	    if (size <= 0) {
		send_field(FIELD_ERROR, "Failed to read styles.xml");
		return false;
	    }
	    styles.resize(size);
	} else if (pathname == "meta.xml") {
	    total = archive_entry_size(entry);
	    string metadata(total, '\0');
	    size = archive_read_data(archive_obj, &metadata[0], total);

	    if (size > 0) {
		// indexing file even if this fails
		metadata.resize(size);
		parse_metadata(metadata);
	    }
	}
    }

    // We want to parse styles.xml after content.xml, but they could be stored
    // in either order in the ZIP container.
    parser.parse(styles);

    send_field(FIELD_BODY, parser.dump);
    return true;
}

static bool
extract_xlsx(struct archive* archive_obj)
{
    int pages = 0;
    string sheets;
    XlsxParser parser;

    struct archive_entry* entry;
    while (archive_read_next_header(archive_obj, &entry) == ARCHIVE_OK) {
	string pathname = archive_entry_pathname(entry);
	if (pathname == "xl/styles.xml" ||
	    pathname == "xl/workbook.xml" ||
	    pathname == "xl/sharedStrings.xml") {
	    size_t total = archive_entry_size(entry);
	    string shared_strings(total, '\0');
	    ssize_t size = archive_read_data(archive_obj, &shared_strings[0],
					     total);

	    if (size > 0) {
		shared_strings.resize(size);
		parser.parse(shared_strings);
	    }
	} else if (startswith(pathname, "xl/worksheets/sheet")) {
	    auto i = sheets.size();
	    size_t total = archive_entry_size(entry);
	    sheets.resize(i + total);
	    ssize_t size = archive_read_data(archive_obj, &sheets[i], total);

	    if (size <= 0) {
		send_field(FIELD_ERROR, "Failed to read " + pathname);
		return false;
	    }
	    sheets.resize(i + size);
	    ++pages;
	} else if (pathname == "docProps/core.xml") {
	    size_t total = archive_entry_size(entry);
	    string metadata(total, '\0');
	    ssize_t size = archive_read_data(archive_obj, &metadata[0], total);
	    if (size > 0) {
		metadata.resize(size);
		parse_metadata(metadata);
	    }
	}
    }
    parser.parse(sheets);
    send_field(FIELD_BODY, parser.dump);
    send_field_page_count(pages);
    return true;
}

static bool
extract_msxml(struct archive* archive_obj,
	      const string& tail)
{
    size_t total;
    ssize_t size;
    struct archive_entry* entry;
    string content;

    if (startswith(tail, "wordprocessingml.")) {
	while (archive_read_next_header(archive_obj, &entry) == ARCHIVE_OK) {
	    string pathname = archive_entry_pathname(entry);
	    if (pathname == "word/document.xml") {
		auto i = content.size();
		total = archive_entry_size(entry);
		content.resize(i + total);
		size = archive_read_data(archive_obj, &content[i], total);

		if (size <= 0) {
		    send_field(FIELD_ERROR, "Failed to read word/document.xml");
		    return false;
		}
		content.resize(i + size);
	    } else if (startswith(pathname, "word/header") ||
		       startswith(pathname, "word/footer")) {
		auto i = content.size();
		total = archive_entry_size(entry);
		content.resize(i + total);
		size = archive_read_data(archive_obj, &content[i], total);

		if (size > 0) {
		    content.resize(i + size);
		} else {
		    // Ignore this as header/footer may not be present
		    content.resize(i);
		}
	    } else if (pathname == "docProps/core.xml") {
		// docProps/core.xml stores meta data
		total = archive_entry_size(entry);
		string metadata(total, '\0');
		size = archive_read_data(archive_obj, &metadata[0], total);
		if (size > 0) {
		    metadata.resize(size);
		    parse_metadata(metadata);
		}
	    }
	}
    } else if (startswith(tail, "presentationml.")) {
	int pages = 0;
	while (archive_read_next_header(archive_obj, &entry) == ARCHIVE_OK) {
	    string pathname = archive_entry_pathname(entry);
	    if (startswith(pathname, "ppt/slides/slide")) {
		++pages;
		goto handle_pptx_content;
	    } else if (startswith(pathname, "ppt/notesSlides/notesSlide") ||
		       startswith(pathname, "ppt/comments/comment")) {
handle_pptx_content:
		auto i = content.size();
		total = archive_entry_size(entry);
		content.resize(i + total);
		size = archive_read_data(archive_obj, &content[i], total);

		if (size <= 0) {
		    send_field(FIELD_ERROR, "Failed to read " + pathname);
		    return false;
		}
		content.resize(i + size);
	    } else if (pathname == "docProps/core.xml") {
		total = archive_entry_size(entry);
		string metadata(total, '\0');
		size = archive_read_data(archive_obj, &metadata[0], total);
		if (size > 0) {
		    metadata.resize(size);
		    parse_metadata(metadata);
		}
	    }
	}
	send_field_page_count(pages);
    }

    MSXmlParser parser;
    parser.parse(content);
    send_field(FIELD_BODY, parser.dump);
    return true;
}

static bool
extract_xps(struct archive* archive_obj)
{
    int pages = 0;
    string content;
    XpsParser parser;

    struct archive_entry* entry;
    while (archive_read_next_header(archive_obj, &entry) == ARCHIVE_OK) {
	string pathname = archive_entry_pathname(entry);
	if (startswith(pathname, "Documents/1/Pages/") &&
	    endswith(pathname, ".fpage")) {
	    size_t total = archive_entry_size(entry);
	    content.resize(total);
	    ssize_t size = archive_read_data(archive_obj, &content[0], total);

	    if (size <= 0) {
		send_field(FIELD_ERROR, "Failed to read " + pathname);
		return false;
	    }
	    content.resize(size);
	    parser.parse(content);
	    ++pages;
	} else if (pathname == "docProps/core.xml") {
	    // If present, docProps/core.xml stores meta data.
	    size_t total = archive_entry_size(entry);
	    content.resize(total);
	    ssize_t size = archive_read_data(archive_obj, &content[0], total);
	    if (size > 0) {
		content.resize(size);
		parse_metadata(content);
	    }
	}
    }

    send_field(FIELD_BODY, parser.dump);
    send_field_page_count(pages);
    return true;
}

void
extract(const string& filename,
	const string& mimetype)
{
    const char* file = filename.c_str();
    struct archive* archive_obj = archive_read_new();
    archive_read_support_format_zip(archive_obj);
    // Block size will be determined by libarchive automatically for
    // regular files. Specified block size will only be used for tape drives
    // 10240 is chosen as default size (20 records - 512 bytes each)
    int status_code = archive_read_open_filename(archive_obj, file,
						 DEFAULT_BLOCK_SIZE);

    if (status_code != ARCHIVE_OK) {
	send_field(FIELD_ERROR, "Failed to open file");
	return;
    }

    if (startswith(mimetype, "application/vnd.sun.xml.") ||
	startswith(mimetype, "application/vnd.oasis.opendocument.")) {
	if (!extract_opendoc(archive_obj))
	    return;
    } else if (startswith(mimetype,
			  "application/vnd.openxmlformats-officedocument."))
    {
	string tail(mimetype, 46);
	if (startswith(tail, "spreadsheetml.")) {
	    if (!extract_xlsx(archive_obj))
		return;
	} else {
	    if (!extract_msxml(archive_obj, tail))
		return;
	}
    } else if (mimetype == "application/oxps" ||
	       mimetype == "application/vnd.ms-xpsdocument") {
	if (!extract_xps(archive_obj))
	    return;
    }

    status_code = archive_read_free(archive_obj);
    if (status_code != ARCHIVE_OK) {
	send_field(FIELD_ERROR, archive_error_string(archive_obj));
	return;
    }
}
