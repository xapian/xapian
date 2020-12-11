/** @file
 * @brief Extract text and metadata from Apple documents using libtonyek.
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
#include <config.h>
#include "handler.h"
#include "stringutils.h"

#include <memory>

#include <librevenge-generators/librevenge-generators.h>
#include <librevenge-stream/librevenge-stream.h>
#include <libetonyek/libetonyek.h>

#define PARSE_FIELD(START, END, FIELD, OUT) \
   parse_metadata_field((START), (END), (FIELD), (CONST_STRLEN(FIELD)), (OUT))

using libetonyek::EtonyekDocument;
using namespace librevenge;
using namespace std;

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
clear_text(string& str, const char* text)
{
    if (text) {
	for (int i = 0; text[i] != '\0'; ++i)
	    if (!isspace(text[i]) || (i && !isspace(text[i - 1])))
		str.push_back(text[i]);
	str.push_back('\n');
    }
}

static bool
extract_key(string& dump, RVNGInputStream* input, string& error)
{
    RVNGStringVector content;
    RVNGTextPresentationGenerator document(content);
    if (!EtonyekDocument::parse(input, &document)) {
	error = "Libetonyek Error: Fail to extract text";
	return false;
    }
    // Extract text if possible
    unsigned size = content.size();
    for (unsigned i = 0; i < size; ++i)
	clear_text(dump, content[i].cstr());
    return true;
}

static bool
extract_numbers(string& dump, RVNGInputStream* input, string& error)
{
    RVNGStringVector content;
    RVNGTextSpreadsheetGenerator document(content);

    if (!EtonyekDocument::parse(input, &document)) {
	error = "Libetonyek Error: Fail to extract text";
	return false;
    }
    // Extract text if possible
    unsigned size = content.size();
    for (unsigned i = 0; i < size; ++i)
	clear_text(dump, content[i].cstr());
    return true;
}

static bool
extract_pages(string& dump,
	      string& title,
	      string& author,
	      string& keywords,
	      string& error,
	      RVNGInputStream* input)
{
    RVNGString text_dump, data_dump;
    bool succeed = false;

    // Extract text if possible
    RVNGTextTextGenerator content(text_dump, false);
    if (EtonyekDocument::parse(input, &content)) {
	clear_text(dump, text_dump.cstr());
	succeed = true;
    } else {
	error = "Libetonyek Error: Fail to extract text";
    }
    // Extract metadata if possible
    RVNGTextTextGenerator data(data_dump, true);
    if (EtonyekDocument::parse(input, &data)) {
	const char* metadata = data_dump.cstr();
	size_t len = data_dump.size();
	parse_metadata(metadata, len, author, title, keywords);
	succeed = true;
    } else {
	if (!error.empty())
	    error += " and metadata";
	else
	    error = "Libetonyek Error: Fail to extract metadata";
    }

    if (succeed)
	error.clear();

    return succeed;
}

bool
extract(const string& filename,
	const string& mimetype,
	string& dump,
	string& title,
	string& keyword,
	string& author,
	string& pages,
	string& error)
{
    try {
	unique_ptr<RVNGInputStream> input;

	if (RVNGDirectoryStream::isDirectory(filename.c_str()))
	    input.reset(new RVNGDirectoryStream(filename.c_str()));
	else
	    input.reset(new RVNGFileStream(filename.c_str()));

	EtonyekDocument::Type type = EtonyekDocument::TYPE_UNKNOWN;
	auto confidence = EtonyekDocument::isSupported(input.get(), &type);

	if (confidence == EtonyekDocument::CONFIDENCE_NONE) {
	    error = "Libetonyek Error: The format couldn't be detected";
	    return false;
	}

	switch (type) {
	    case EtonyekDocument::TYPE_PAGES:
		return extract_pages(dump, title, author,
				     keyword, error, input.get());
	    case EtonyekDocument::TYPE_NUMBERS:
		return extract_numbers(dump, input.get(), error);
	    case EtonyekDocument::TYPE_KEYNOTE:
		return extract_key(dump, input.get(), error);
	    default:
		error = "Libetonyek Error: The format is not supported";
	}
	(void)pages;
    } catch (...) {
	error = "Libetonyek threw an exception";
    }
    return false;
}
