/** @file
 * @brief Extract text and metadata using libmwaw.
 */
/* Copyright (C) 2019 Bruno Baruffaldi
 * Copyright (C) 2020 Parth Kapadia
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
#include "str.h"
#include "stringutils.h"

#include <librevenge-generators/librevenge-generators.h>
#include <librevenge-stream/librevenge-stream.h>
#include <libmwaw/libmwaw.hxx>

#define HANDLE_FIELD(START, END, FIELD, OUT...) \
   handle_field((START), (END), (FIELD), (CONST_STRLEN(FIELD)), OUT)

using namespace librevenge;
using namespace std;

// Handle a field for which we only take a single value - we avoid copying in
// this case.
static void
handle_field(const char* start,
	     const char* end,
	     const char* field,
	     size_t len,
	     const char*& out,
	     size_t& out_len)
{
    if (size_t(end - start) > len && memcmp(start, field, len) == 0) {
	start += len;
	while (start != end && isspace(*start)) start++;
	if (start != end && (end[-1] != '\r' || --end != start)) {
	    out = start;
	    out_len = end - start;
	}
    }
}

// Handle a field for which we process multiple instances.  We just send each
// occurrence as we see it.
static void
handle_field(const char* start,
	     const char* end,
	     const char* field,
	     size_t len,
	     Field code)
{
    if (size_t(end - start) > len && memcmp(start, field, len) == 0) {
	start += len;
	while (start != end && isspace(*start)) start++;
	if (start != end && (end[-1] != '\r' || --end != start)) {
	    send_field(code, start, end - start);
	}
    }
}

static void
parse_metadata(const char* data, size_t len)
{
    const char* author;
    size_t author_len = 0;

    const char* p = data;
    const char* end = p + len;

    while (p != end) {
	const char* start = p;
	p = static_cast<const char*>(memchr(p, '\n', end - start));
	const char* eol;
	if (p)
	    eol = p++;
	else
	    p = eol = end;
	if ((end - start) > 5 && memcmp(start, "meta:", 5) == 0) {
	    start += 5;
	    switch (*start) {
		case 'i': {
		    // Use dc:creator in preference to meta:initial-creator.
		    if (!author_len)
			HANDLE_FIELD(start, eol, "initial-creator",
				     author, author_len);
		    break;
		}
		case 'k': {
		    HANDLE_FIELD(start, eol, "keyword", FIELD_KEYWORDS);
		    break;
		}
	    }
	} else if ((end - start) > 3 && memcmp(start, "dc:", 3) == 0) {
	    start += 3;
	    switch (*start) {
		case 'c': {
		    // Use dc:creator in preference to meta:initial-creator.
		    HANDLE_FIELD(start, eol, "creator", author, author_len);
		    break;
		}
		case 's': {
		    HANDLE_FIELD(start, eol, "subject", FIELD_KEYWORDS);
		    break;
		}
		case 't': {
		    HANDLE_FIELD(start, eol, "title", FIELD_TITLE);
		    break;
		}
	    }
	} else if ((end - start) > 8 && memcmp(start, "dcterms:", 8) == 0) {
	    start += 8;
	    HANDLE_FIELD(start, eol, "available", FIELD_KEYWORDS);
	}
    }

    if (author_len) {
	send_field(FIELD_AUTHOR, author, author_len);
    }
}

static void
parse_content(const RVNGStringVector& pages)
{
    auto page_count = pages.size();
    send_field_page_count(page_count);
    for (unsigned i = 0; i < page_count; ++i) {
	send_field(FIELD_BODY, pages[i].cstr(), pages[i].size());
    }
}

static void
extract_text(RVNGFileStream* input)
{
    RVNGString dump;
    RVNGTextTextGenerator content_gen(dump, false);
    if (MWAWDocument::parse(input, &content_gen) != MWAWDocument::MWAW_R_OK) {
	send_field(FIELD_ERROR, "Failed to extract text");
	return;
    }
    send_field(FIELD_BODY, dump.cstr(), dump.size());

    RVNGString metadata;
    RVNGTextTextGenerator metadata_gen(metadata, true);
    if (MWAWDocument::parse(input, &metadata_gen) != MWAWDocument::MWAW_R_OK) {
	send_field(FIELD_ERROR, "Failed to extract metadata");
	return;
    }

    parse_metadata(metadata.cstr(), metadata.size());
}

static void
extract_spreadsheet(RVNGFileStream* input)
{
    RVNGStringVector pages_metadata;
    RVNGTextSpreadsheetGenerator metadata(pages_metadata, true);
    MWAWDocument::Result result = MWAWDocument::parse(input, &metadata);
    if (result != MWAWDocument::MWAW_R_OK) {
	send_field(FIELD_ERROR, "Failed to extract metadata");
	return;
    }

    for (unsigned i = 0; i < pages_metadata.size(); ++i) {
	parse_metadata(pages_metadata[i].cstr(),
		       pages_metadata[i].size());
    }

    RVNGStringVector pages;
    RVNGTextSpreadsheetGenerator content(pages, false);
    result = MWAWDocument::parse(input, &content);
    if (result != MWAWDocument::MWAW_R_OK) {
	send_field(FIELD_ERROR, "Failed to extract text");
	return;
    }
    parse_content(pages);
}

static void
extract_presentation(RVNGFileStream* input)
{
    RVNGStringVector pages;
    RVNGTextPresentationGenerator content(pages);
    MWAWDocument::Result result = MWAWDocument::parse(input, &content);
    if (result != MWAWDocument::MWAW_R_OK) {
	send_field(FIELD_ERROR, "Failed to extract text");
	return;
    }
    parse_content(pages);
}

static void
extract_drawing(RVNGFileStream* input)
{
    RVNGStringVector pages;
    RVNGTextDrawingGenerator content(pages);
    MWAWDocument::Result result = MWAWDocument::parse(input, &content);
    if (result != MWAWDocument::MWAW_R_OK) {
	send_field(FIELD_ERROR, "Failed to extract text");
	return;
    }
    parse_content(pages);
}

void
extract(const string& filename, const string& mimetype)
{
    // To store the kind and type of document
    MWAWDocument::Kind kind;
    MWAWDocument::Type type;
    MWAWDocument::Confidence confidence;
    RVNGFileStream input(filename.c_str());

    confidence = MWAWDocument::isFileFormatSupported(&input, type, kind);
    if (confidence != MWAWDocument::MWAW_C_EXCELLENT) {
	send_field(FIELD_ERROR, "File format not supported");
	return;
    }

    switch (kind) {
	case MWAWDocument::MWAW_K_TEXT:
	    extract_text(&input);
	    break;
	case MWAWDocument::MWAW_K_SPREADSHEET:
	case MWAWDocument::MWAW_K_DATABASE:
	    extract_spreadsheet(&input);
	    break;
	case MWAWDocument::MWAW_K_PRESENTATION:
	    extract_presentation(&input);
	    break;
	default:
	    extract_drawing(&input);
	    break;
    }
}
