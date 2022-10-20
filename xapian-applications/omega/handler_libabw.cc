/** @file
 * @brief Extract text and metadata using libabw.
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
#include "stringutils.h"

#include <librevenge-generators/librevenge-generators.h>
#include <librevenge-stream/librevenge-stream.h>
#include <libabw/libabw.h>

#define PARSE_FIELD(START, END, FIELD, OUT...) \
   parse_metadata_field((START), (END), (FIELD), (CONST_STRLEN(FIELD)), OUT)

using namespace librevenge;
using namespace std;

// Handle a field for which we only take a single value - we avoid copying in
// this case.
static void
parse_metadata_field(const char* start,
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

// Handle a field for which we concatenate multiple instances.  We need to copy
// in this case.
static void
parse_metadata_field(const char* start,
		     const char* end,
		     const char* field,
		     size_t len,
		     string& out)
{
    if (size_t(end - start) > len && memcmp(start, field, len) == 0) {
	start += len;
	while (start != end && isspace(*start)) start++;
	if (start != end && (end[-1] != '\r' || --end != start)) {
	    if (!out.empty())
		out.push_back(' ');
	    out.append(start, end - start);
	}
    }
}

static void
parse_metadata(const char* data, size_t len,
	       const char*& title, size_t& title_len,
	       const char*& author, size_t& author_len,
	       string& keywords)
{
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
		    if (!author)
			PARSE_FIELD(start, eol, "initial-creator",
				    author, author_len);
		    break;
		}
		case 'k': {
		    PARSE_FIELD(start, eol, "keyword", keywords);
		    break;
		}
	    }
	} else if ((end - start) > 3 && memcmp(start, "dc:", 3) == 0) {
	    start += 3;
	    switch (*start) {
		case 'c': {
		    // Use dc:creator in preference to meta:initial-creator.
		    PARSE_FIELD(start, eol, "creator", author, author_len);
		    break;
		}
		case 's': {
		    PARSE_FIELD(start, eol, "subject", keywords);
		    break;
		}
		case 't': {
		    PARSE_FIELD(start, eol, "title", title, title_len);
		    break;
		}
	    }
	} else if ((end - start) > 8 && memcmp(start, "dcterms:", 8) == 0) {
	    start += 8;
	    PARSE_FIELD(start, eol, "available", keywords);
	}
    }
}

void
extract(const string& filename, const string& mimetype)
try {
    RVNGFileStream input(filename.c_str());

    if (!libabw::AbiDocument::isFileFormatSupported(&input)) {
	fail("Libabw Error: The format is not supported");
    }

    RVNGString metadata, dump;

    RVNGTextTextGenerator metadata_gen(metadata, true);
    if (!libabw::AbiDocument::parse(&input, &metadata_gen)) {
	fail("Libabw Error: Fail to extract metadata");
	return;
    }

    const char* title = nullptr;
    const char* author = nullptr;
    size_t title_len = 0, author_len = 0;
    string keywords;
    parse_metadata(metadata.cstr(), metadata.size(),
		   title, title_len,
		   author, author_len,
		   keywords);

    // Extract body text.
    RVNGTextTextGenerator content(dump, false);
    if (!libabw::AbiDocument::parse(&input, &content)) {
	fail("Libabw Error: Fail to extract text");
	return;
    }

    response(dump.cstr(), dump.size(),
	     title, title_len,
	     keywords.data(), keywords.size(),
	     author, author_len,
	     -1);
} catch (...) {
    fail("Libabw threw an exception");
}
