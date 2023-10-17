/** @file
 * @brief Extract text and metadata using libgepub
 */
/* Copyright (C) 2023 Olly Betts
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

#include "htmlparser.h"

#include <gepub-doc.h>

#include <string_view>

using namespace std;

static void
send_glib_field(Field field, gchar* data)
{
    if (data) {
	send_field(field, data);
	g_free(data);
    }
}

bool
initialise()
{
    return true;
}

void
extract(const string& filename, const string&)
{
    GError* e = nullptr;
    GepubDoc* doc = gepub_doc_new(filename.c_str(), &e);
    if (!doc) {
	send_field(FIELD_ERROR, "gepub_doc_new() failed: ");
	send_field(FIELD_ERROR, e->message);
	g_error_free(e);
	return;
    }

    int chapters = gepub_doc_get_n_chapters(doc);
    // "Page count" is not a concept which seems to perfectly fit with an EPUB
    // so (at least for now) we report the number of chapters.
    send_field_page_count(chapters);
    for (int i = 0; i < chapters; ++i) {
	gepub_doc_set_chapter(doc, i);
	GBytes* html = gepub_doc_get_current(doc);
	gsize size;
	auto data = static_cast<const char*>(g_bytes_get_data(html, &size));
	HtmlParser parser;
	try {
	    parser.parse(string_view(data, size), "utf-8", false);
	} catch (const string& newcharset) {
	    parser.parse(string_view(data, size), newcharset, true);
	}
	send_field(FIELD_BODY, parser.dump);
	g_bytes_unref(html);
    }

    // Extract metadata.
    send_glib_field(FIELD_AUTHOR,
		    gepub_doc_get_metadata(doc, GEPUB_META_AUTHOR));
    send_glib_field(FIELD_KEYWORDS,
		    gepub_doc_get_metadata(doc, GEPUB_META_DESC));
    send_glib_field(FIELD_TITLE,
		    gepub_doc_get_metadata(doc, GEPUB_META_TITLE));

    g_object_unref(doc);
}
