/** @file handler_libmwaw.cc
 * @brief Extract text and metadata using libmwaw.
 */
/* Copyright (C) 2019 Bruno Baruffaldi
 * Copyright (C) 2020 Parth Kapadia
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

#define PARSE_FIELD(START, END, FIELD, OUT) \
   parse_metadata_field((START), (END), (FIELD), (CONST_STRLEN(FIELD)), (OUT))

using namespace std;
using namespace librevenge;

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
parse_metadata(const char* data,
	       size_t len,
	       string& author,
	       string& title,
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
		    if (author.empty())
			PARSE_FIELD(start, eol, "initial-creator", author);
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
		    if (!author.empty())
			author.clear();
		    PARSE_FIELD(start, eol, "creator", author);
		    break;
		}
		case 's': {
		    PARSE_FIELD(start, eol, "subject", keywords);
		    break;
		}
		case 't': {
		    PARSE_FIELD(start, eol, "title", title);
		    break;
		}
	    }
	} else if ((end - start) > 8 && memcmp(start, "dcterms:", 8) == 0) {
	    start += 8;
	    PARSE_FIELD(start, eol, "available", keywords);
	}
    }
}

static void
parse_content(string &dump,
	      string &pages,
	      RVNGStringVector &pages_content)
{
    for (unsigned i = 0; i < pages_content.size(); ++i) {
	dump.append(pages_content[i].cstr());
    }
    pages = str(pages_content.size());
}

static bool
extract_word(string& dump,
	     string& title,
	     string& keywords,
	     string& author,
	     string& error,
	     RVNGFileStream* input)
{
    RVNGString content_dump;
    RVNGTextTextGenerator content(content_dump, false);
    if (MWAWDocument::parse(input, &content) == MWAWDocument::MWAW_R_OK) {
	dump = content_dump.cstr();
    } else {
	error = "Libmwaw Error: Failed to extract text";
	return false;
    }

    RVNGString metadata_dump;
    RVNGTextTextGenerator metadata(metadata_dump, true);
    if (MWAWDocument::parse(input, &metadata) == MWAWDocument::MWAW_R_OK) {
	const char* data = metadata_dump.cstr();
	size_t len = metadata_dump.size();
	parse_metadata(data, len, author, title, keywords);
    } else {
	error = "Libmwaw Error: Failed to extract metadata";
	return false;
    }
    return true;
}

static bool
extract_spreadsheet(string& dump,
		    string& title,
		    string& keywords,
		    string& author,
		    string& error,
		    string& pages,
		    RVNGFileStream* input)
{
    RVNGStringVector pages_metadata;
    RVNGTextSpreadsheetGenerator metadata(pages_metadata, true);
    MWAWDocument::Result result = MWAWDocument::parse(input, &metadata);
    if (result != MWAWDocument::MWAW_R_OK) {
	error = "Libmwaw Error: Failed to extract metadata";
	return false;
    }
    string meta_temp;
    for (unsigned i = 0; i < pages_metadata.size(); ++i) {
	meta_temp.append(pages_metadata[i].cstr());
    }
    const char* data = meta_temp.c_str();
    size_t len = meta_temp.size();
    parse_metadata(data, len, author, title, keywords);

    RVNGStringVector pages_content;
    RVNGTextSpreadsheetGenerator content(pages_content, false);
    result = MWAWDocument::parse(input, &content);
    if (result != MWAWDocument::MWAW_R_OK) {
	error = "Libmwaw Error: Failed to extract text";
	return false;
    }
    parse_content(dump, pages, pages_content);
    return true;
}

static bool
extract_presentation(string& dump,
		     string& pages,
		     string& error,
		     RVNGFileStream* input)
{
    RVNGStringVector pages_content;
    RVNGTextPresentationGenerator content(pages_content);
    MWAWDocument::Result result = MWAWDocument::parse(input, &content);
    if (result != MWAWDocument::MWAW_R_OK) {
	error = "Libmwaw Error: Failed to extract text";
	return false;
    }
    parse_content(dump, pages, pages_content);
    return true;
}

static bool
extract_drawing(string& dump,
		string& pages,
		string& error,
		RVNGFileStream* input)
{
    RVNGStringVector pages_content;
    RVNGTextDrawingGenerator content(pages_content);
    MWAWDocument::Result result = MWAWDocument::parse(input, &content);
    if (result != MWAWDocument::MWAW_R_OK) {
	error = "Libmwaw Error: Failed to extract text";
	return false;
    }
    parse_content(dump, pages, pages_content);
    return true;
}

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
	// To store the kind and type of document
	MWAWDocument::Kind kind;
	MWAWDocument::Type type;
	MWAWDocument::Confidence confidence;
	RVNGFileStream input(filename.c_str());

	confidence = MWAWDocument::isFileFormatSupported(&input, type, kind);
	if (confidence != MWAWDocument::MWAW_C_EXCELLENT) {
	    error = "Libmwaw Error: File format not supported";
	    return false;
	}

	switch (kind) {
	    case MWAWDocument::MWAW_K_TEXT:
		return extract_word(dump, title, keywords, author, error,
				    &input);
	    case MWAWDocument::MWAW_K_SPREADSHEET:
	    case MWAWDocument::MWAW_K_DATABASE:
		return extract_spreadsheet(dump, title, keywords, author, error,
					   pages, &input);
	    case MWAWDocument::MWAW_K_PRESENTATION:
		return extract_presentation(dump, pages, error, &input);
	    default:
		return extract_drawing(dump, pages, error, &input);
	}
    } catch (...) {
	error = "Libmwaw threw an exception";
	return false;
    }
    return false;
}
