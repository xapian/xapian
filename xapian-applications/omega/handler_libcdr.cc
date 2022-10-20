/** @file
 * @brief Extract text using libcdr.
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

#include <librevenge-generators/librevenge-generators.h>
#include <librevenge-stream/librevenge-stream.h>
#include <libcdr/libcdr.h>

using namespace librevenge;
using namespace std;

void
extract(const string& filename, const string& mimetype)
try {
    RVNGFileStream input(filename.c_str());
    RVNGStringVector cdr_pages;
    RVNGTextDrawingGenerator content(cdr_pages);

    // check if cdr file supported
    if (!libcdr::CDRDocument::isSupported(&input)) {
	fail("Libcdr Error: The format is not supported");
	return;
    }

    if (!libcdr::CDRDocument::parse(&input, &content)) {
	fail("Libcdr Error: Failed to parse the file");
	return;
    }

    string dump;
    int pages = cdr_pages.size();
    for (auto i = 0; i < pages; ++i) {
	dump.append(cdr_pages[i].cstr());
    }

    response(dump.data(), dump.size(),
	     nullptr, 0,
	     nullptr, 0,
	     nullptr, 0,
	     pages);
} catch (...) {
    fail("Libcdr threw an exception");
}
