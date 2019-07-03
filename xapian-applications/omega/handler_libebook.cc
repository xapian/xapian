/** @file handler_libebook.cc
 * @brief @brief Extract text and metadata using libe-book.
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

#include <xapian.h>
#include <string.h>
#include <unordered_map>
#include <memory>

#include <librevenge-0.0/librevenge-generators/librevenge-generators.h>
#include <librevenge-0.0/librevenge-stream/librevenge-stream.h>
#include <iostream>
#include <libe-book/libe-book.h>

using libebook::EBOOKDocument;
using namespace std;
using namespace librevenge;

static
char* str_copy(const char* text) {
    int len = strlen(text);
    char* line = new char[len];
    strcpy(line, text);
    return line;
}

static
void clear_text(string& str,const char* text) {
    int len = strlen(text);
    for (int i = 0; i < len; ++i)
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
	    (EBOOKDocument::CONFIDENCE_WEAK != confidence))
	    return false;

	RVNGTextTextGenerator metadata(metadata_dump, true);

	if (EBOOKDocument::RESULT_OK !=
	    EBOOKDocument::parse(input.get(), &metadata, type))
	    return false;

	unordered_map<string,char*> hash;
	char* str = str_copy(metadata_dump.cstr());
	char* pch = strtok(str, "\n");
	while (pch != NULL) {
	    char* sep = strpbrk(pch, " ");
	    *sep = '\0';
	    hash[pch] = sep + 1;
	    pch = strtok(NULL, "\n");
	}
	delete[] str;
	unordered_map<string,char*>::const_iterator it;

	// Extract Author if it possible
	if ((it = hash.find("dc:creator")) != hash.end())
	    clear_text(author, it->second);
	else if ((it = hash.find("meta:initial-creator")) != hash.end())
	    clear_text(author, it->second);

	// Extract Title if it possible
	if ((it = hash.find("dc:title")) != hash.end())
	    clear_text(title, it->second);

	// Extract Keywords if it possible
	if ((it = hash.find("dc:subject")) != hash.end())
	    clear_text(keywords, it->second);
	if ((it = hash.find("dcterms:available")) != hash.end())
	    clear_text(keywords, it->second);
	pages = "";
	hash.clear();

	// Extract Dump if it possible
	RVNGTextTextGenerator content(content_dump, false);

	if (EBOOKDocument::RESULT_OK !=
	    EBOOKDocument::parse(input.get(), &content, type))
	    return false;
	clear_text(dump, content_dump.cstr());
    } catch (...) {
	cerr << "Libe-book threw an exception" << endl;
	return false;
    }
    return true;
}
