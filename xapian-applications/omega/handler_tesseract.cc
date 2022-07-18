/** @file
 * @brief Extract text from Images using tesseract.
 */
/* Copyright (C) 2019 Bruno Baruffaldi
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

// Workaround stupidity in tesseract headers.
#undef HAVE_CONFIG_H
#include <tesseract/baseapi.h>
#include <leptonica/allheaders.h>

#include <sysexits.h>

using namespace std;
using namespace tesseract;

static TessBaseAPI* ocr = NULL;

static void
clear_text(string& dump, const char* text)
{
    if (text) {
	for (int i = 0; text[i] != '\0'; ++i) {
	    if (!isspace(text[i]) || (i && !isspace(text[i - 1])))
		dump.push_back(text[i]);
	}
	delete [] text;
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
    // Create the ocr if necessary
    if (!ocr) {
	ocr = new TessBaseAPI();
	ocr->SetPageSegMode(PSM_AUTO_OSD);
    }

    // Call Init() for each document so any adaptive state is reset as
    // we don't want the order of indexing documents to affect the text
    // indexed for each document.
    //
    // Tesseract documents that passing nullptr for the second parameter
    // here is the same as "eng", but that fails to work on macos (tested
    // with the homebrew tesseract v5.1.0).
    //
    // FIXME: We ought to provide a way to allow the language to use here
    // to be specified.
    if (ocr->Init(nullptr, "eng"))
	_Exit(EX_UNAVAILABLE);

    // Open Image
    Pix* image = pixRead(filename.c_str());

    if (!image) {
	error = "Tesseract Error: Error while opening the image";
	return false;
    }

    ocr->SetImage(image);

    // Get OCR result
    clear_text(dump, ocr->GetUTF8Text());

    // Release memory.
    ocr->Clear();
    pixDestroy(&image);

    (void)title;
    (void)keywords;
    (void)author;
    (void)pages;

    return true;
}
