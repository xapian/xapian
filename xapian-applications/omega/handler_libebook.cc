/** @file handler_libebook.cc
 * @brief Extract text and metadata using libe-book.
 */
/* Copyright (C) 2019 Bruno Baruffaldi
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
#include "handler.h"
#include "stringutils.h"

#include <memory>
#include <iostream>

#include <librevenge-generators/librevenge-generators.h>
#include <librevenge-stream/librevenge-stream.h>
#include <libe-book/libe-book.h>

#define PARSE_FIELD(START, END, FIELD, OUT) \
   parse_metadata_field((START), (END), (FIELD), (CONST_STRLEN(FIELD)), (OUT))

using libebook::EBOOKDocument;
using namespace std;
using namespace librevenge;

static void
parse_metadata_field(const char* start,
		     const char* end,
		     const char* field,
		     size_t len,
		     string& out)
{
    if (size_t(end - start) > len && memcmp(start,field,len) == 0) {
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
	       string& author,
	       string& title,
	       string& keywords)
{
    const char* p = data;
    const char* end = p + strlen(data);

    while (p != end) {
	const char* start = p;
	p = static_cast<const char*>(memchr(p, '\n', end - start));
	const char* eol;
	if (p)
	    eol = p++;
	else
	    p = eol = end;
	if ((end - start) > 5 && memcmp(start,"meta:",5) == 0) {
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
	} else if ((end - start) > 3 && memcmp(start,"dc:",3) == 0) {
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
clear_text(string& str, const char* text)
{
    for (int i = 0; text[i]!='\0'; ++i)
	if (!isspace(text[i]) || (i && !isspace(text[i - 1])))
	    str.push_back(text[i]);
}

bool
extract(const string& filename,
	string& dump,
	string& title,
	string& keywords,
	string& author,
	string& pages)
{
    try {
	shared_ptr<RVNGInputStream> input;
	RVNGString content_dump, metadata_dump;
	const char* file = filename.c_str();

	if (RVNGDirectoryStream::isDirectory(file))
	    input.reset(new RVNGDirectoryStream(file));
	else
	    input.reset(new RVNGFileStream(file));

	EBOOKDocument::Type type = EBOOKDocument::TYPE_UNKNOWN;
	EBOOKDocument::Confidence confidence =
				EBOOKDocument::isSupported(input.get(), &type);

	if (EBOOKDocument::CONFIDENCE_SUPPORTED_PART == confidence) {
	    input.reset(RVNGDirectoryStream::createForParent(file));
	    confidence = EBOOKDocument::isSupported(input.get(), &type);
	}

	if ((EBOOKDocument::CONFIDENCE_EXCELLENT != confidence) &&
	    (EBOOKDocument::CONFIDENCE_WEAK != confidence)) {
	    return false;
	}

	RVNGTextTextGenerator metadata(metadata_dump, true);

	if (EBOOKDocument::RESULT_OK !=
	    EBOOKDocument::parse(input.get(), &metadata, type)) {
	    return false;
	}
	// Extract metadata if possible
	parse_metadata(metadata_dump.cstr(), author, title, keywords);
	(void)pages;
	// Extract Dump if possible
	RVNGTextTextGenerator content(content_dump, false);

	if (EBOOKDocument::RESULT_OK !=
	    EBOOKDocument::parse(input.get(), &content, type)) {
	    return false;
	}
	clear_text(dump, content_dump.cstr());
    } catch (...) {
	cerr << "Libe-book threw an exception" << endl;
	return false;
    }
    return true;
}
