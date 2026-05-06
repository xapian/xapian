/** @file
 * @brief Extract text and metadata using libspectre and poppler.
 */
/* Copyright (C) 2019 Bruno Baruffaldi
 * Copyright (C) 2022,2023,2026 Olly Betts
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
 * along with this program; if not, see
 * <https://www.gnu.org/licenses/>.
 */

#include <config.h>
#include "handler.h"
#include "str.h"
#include "tmpdir.h"

#include <poppler-document.h>
#include <poppler-page.h>
#include <libspectre/spectre.h>

using namespace std;

static string tmp_pdf_file;
static gchar* tmp_pdf_uri;

bool
initialise(string& error)
{
    tmp_pdf_file = get_tmpfile("tmp.pdf");
    if (tmp_pdf_file.empty()) {
	error = "Couldn't create temporary directory";
	return false;
    }

    GError* e = nullptr;
    tmp_pdf_uri = g_filename_to_uri(tmp_pdf_file.c_str(), NULL, &e);
    if (!tmp_pdf_uri) {
        error = "g_filename_to_uri() failed: "s + e->message;
        g_error_free(e);
        return false;
    }

    return true;
}

void
extract(const string& filename, const string&)
{
    SpectreDocument* ps_doc = spectre_document_new();
    if (!ps_doc) {
        send_field(FIELD_ERROR, "spectre_document_new() failed");
        return;
    }

    spectre_document_load(ps_doc, filename.c_str());
    SpectreStatus s = spectre_document_status(ps_doc);
    if (s != SPECTRE_STATUS_SUCCESS) {
	spectre_document_free(ps_doc);
        send_field(FIELD_ERROR,
		   "spectre_document_load() failed: "s +
		   spectre_status_to_string(s));
	return;
    }

    spectre_document_save_to_pdf(ps_doc, tmp_pdf_file.c_str());
    s = spectre_document_status(ps_doc);
    if (s != SPECTRE_STATUS_SUCCESS) {
	spectre_document_free(ps_doc);
        send_field(FIELD_ERROR,
		   "spectre_document_save_to_pdf() failed: "s +
		   spectre_status_to_string(s));
	return;
    }

    // Extract metadata.
    //
    // Notes:
    //
    // spectre_document_get_creator(ps_doc)) reports the software which
    // created the PostScript file, which is not helpful to report as
    // FIELD_AUTHOR.
    //
    // poppler_document_get_creation_date(doc) is not useful because it
    // returns the date the temporary PDF was created, not the date the
    // PostScript file was.
    //
    // ps2pdf maps PostScript's "For:" to PDF's author field, so we do the
    // same.
    const char* author = spectre_document_get_for(ps_doc);
    if (author) send_field(FIELD_AUTHOR, author);
    const char* title = spectre_document_get_title(ps_doc);
    if (title) send_field(FIELD_TITLE, title);
    spectre_document_free(ps_doc);

    GError* e = nullptr;
    PopplerDocument* doc =
	poppler_document_new_from_file(tmp_pdf_uri, NULL, &e);
    if (!doc) {
        send_field(FIELD_ERROR,
		   "poppler_document_new_from_file() failed: "s + e->message);
        g_error_free(e);
        return;
    }

    int pages = poppler_document_get_n_pages(doc);
    send_field_page_count(pages);
    // Extracting text from PDF file
    for (int i = 0; i < pages; ++i) {
        PopplerPage* page = poppler_document_get_page(doc, i);
        if (!page) {
            g_object_unref(doc);
            send_field(FIELD_ERROR, "Failed to get page " + str(i));
            return;
        }
        send_field(FIELD_BODY, poppler_page_get_text(page));
        g_object_unref(page);
    }
    g_object_unref(doc);
    unlink(tmp_pdf_file.c_str());
}
