/** @file
 * @brief Extract text and metadata using gmime.
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

#include "htmlparser.h"
#include "utf8convert.h"

#include <glib.h>
#include <gmime/gmime.h>
#include <string.h>

using namespace std;

constexpr unsigned SIZE = 4096;

static void
extract_html(const string& text, string& charset, string& dump)
{
    HtmlParser parser;
    if (charset.empty())
	charset = "UTF-8";
    try {
	parser.ignore_metarobots();
	parser.parse(text, charset, false);
	dump += parser.dump;
    } catch (const string& newcharset) {
	parser.reset();
	parser.ignore_metarobots();
	parser.parse(text, newcharset, true);
	dump += parser.dump;
    }
}

static size_t
decode(unsigned char* text, size_t len, GMimeContentEncoding encoding,
       int* state, guint32* save)
{
    unsigned char buffer[SIZE];
    switch (encoding) {
	case GMIME_CONTENT_ENCODING_BASE64: {
	    len = g_mime_encoding_base64_decode_step(text, len, buffer,
						     state, save);
	    memcpy(text, buffer, len);
	    break;
	}
	case GMIME_CONTENT_ENCODING_UUENCODE: {
	    len = g_mime_encoding_uudecode_step(text, len, buffer,
						state, save);
	    memcpy(text, buffer, len);
	    break;
	}
	case GMIME_CONTENT_ENCODING_QUOTEDPRINTABLE: {
	    len = g_mime_encoding_quoted_decode_step(text, len, buffer,
						     state, save);
	    memcpy(text, buffer, len);
	    break;
	}
	default:
	    break;
    }
    return len;
}

static bool
parser_content(GMimeObject* me, string& dump)
{
    GMimeContentType* ct = g_mime_object_get_content_type(me);
    if (GMIME_IS_MULTIPART(me)) {
	GMimeMultipart* mpart = reinterpret_cast<GMimeMultipart*>(me);
	string subtype = g_mime_content_type_get_media_subtype(ct);
	int count = g_mime_multipart_get_count(mpart);
	if (subtype == "alternative") {
	    for (int i = 0; i < count; ++i) {
		GMimeObject* part = g_mime_multipart_get_part(mpart, i);
		if (parser_content(part, dump))
		    return true;
	    }
	} else {
	    bool ret = false;
	    for (int i = 0; i < count; ++i) {
		GMimeObject* part = g_mime_multipart_get_part(mpart, i);
		ret |= parser_content(part, dump);
	    }
	    return ret;
	}
    } else if (GMIME_IS_PART(me)) {
	GMimePart* part = reinterpret_cast<GMimePart*>(me);
	GMimeDataWrapper* content = g_mime_part_get_content_object(part);
	string type = g_mime_content_type_get_media_type(ct);
	string subtype = g_mime_content_type_get_media_subtype(ct);
	string charset;
	const char* p = g_mime_object_get_content_type_parameter(me, "charset");
	if (p) charset = g_mime_charset_canon_name(p);
	if (type == "text") {
	    string text;
	    char buffer[SIZE];
	    GMimeStream* sr = g_mime_data_wrapper_get_stream(content);
	    auto encoding = g_mime_part_get_content_encoding(part);
	    int state = 0, len = 0;
	    guint32 save = 0;
	    do {
		len = g_mime_stream_read(sr, buffer, SIZE);
		if (0 < len) {
		    auto u_buff = reinterpret_cast<unsigned char*>(buffer);
		    len = decode(u_buff, len, encoding, &state, &save);
		    if (0 < len)
			text.append(buffer, len);
		}
	    } while (0 < len);
	    if (subtype == "plain") {
		convert_to_utf8(text, charset);
		dump.append(text);
	    } else if (subtype == "html") {
		extract_html(text, charset, dump);
	    }
	    return true;
	}
    }
    return false;
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
    static bool first_time = true;
    if (first_time) {
	g_mime_init(0);
	first_time = false;
    }

    FILE* fp = fopen(filename.c_str(), "r");

    if (fp == NULL) {
	error = "Gmime Error: fail open " + filename;
	return false;
    }

    GMimeStream* stream = g_mime_stream_file_new(fp);
    GMimeParser* parser = g_mime_parser_new_with_stream(stream);
    GMimeMessage* message = g_mime_parser_construct_message(parser);
    GMimeObject* body = g_mime_message_get_body(message);
    author = g_mime_message_get_sender(message);
    title = g_mime_message_get_subject(message);

    parser_content(body, dump);

    (void)pages;
    return true;
}
