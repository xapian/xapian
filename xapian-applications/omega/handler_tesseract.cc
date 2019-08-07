/** @file handler_tesseract.cc
 * @brief Extract text from Images using tesseract.
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

#include <tesseract/baseapi.h>
#include <leptonica/allheaders.h>

using namespace std;
using namespace tesseract;

TessBaseAPI* ocr = NULL;

static bool
clear_text(string& dump, const char* text)
{
    bool k = false;
    if (text) {
	for (int i = 0; text[i] != '\0'; ++i) {
	    if (!isspace(text[i]) || (i && !isspace(text[i - 1]))) {
		dump.push_back(text[i]);
		k |= isalnum(text[i]);
	    }
	}
	delete [] text;
    }
    return k;
}

bool
extract(const string& filename,
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
	if (ocr->Init(NULL, NULL))
	    _Exit(1);
	ocr->SetPageSegMode(PSM_AUTO_OSD);
    }
    // Open Image
    Pix* image = pixRead(filename.c_str());
    ocr->SetImage(image);

    // Get OCR result
    bool has_text = clear_text(dump, ocr->GetUTF8Text());

    // Destroy used object and release memory
    pixDestroy(&image);

    if (!has_text) {
	error = "Tesseract Error: The image does not content text\n";
	return false;
    }

    (void)title;
    (void)keywords;
    (void)author;
    (void)pages;

    return true;
}
