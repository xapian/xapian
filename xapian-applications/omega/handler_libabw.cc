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

#include <cstring>

#include <librevenge-generators/librevenge-generators.h>
#include <librevenge-stream/librevenge-stream.h>
#include <libabw/libabw.h>

using namespace std;
using namespace librevenge;

static void
clear_text(string& str, const char* text)
{
    if (text) {
	for (int i = 0; text[i] != '\0'; ++i)
	    if (!isspace(text[i]) || (i && !isspace(text[i - 1])))
		str.push_back(text[i]);
    }
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
    RVNGFileStream input(filename.c_str());
    RVNGString metadata_dump, content_dump;

    RVNGTextTextGenerator metadata(metadata_dump, true);
    if (libabw::AbiDocument::parse(&input, &metadata))
	cout<<metadata_dump.cstr();

    RVNGTextTextGenerator content(content_dump, false);
    if (libabw::AbiDocument::parse(&input, &content))
	cout<<content_dump.cstr();
}
