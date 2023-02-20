/** @file
 * @brief Extract text and metadata using gmime.
 */
/* Copyright (C) 2019 Bruno Baruffaldi
 * Copyright (C) 2022,2023 Olly Betts
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
#include "str.h"
#include "stringutils.h"
#include "utf8convert.h"

#include <glib.h>
#include <gmime/gmime.h>
#include <string.h>

#include "safefcntl.h"

using namespace std;

static string
get_attachment_dir()
{
    string result;
    const char* p = getenv("XAPIAN_OMEGA_ATTACHMENT_DIR");
    if (p && *p) {
	result = p;
	if (result.back() != '/') {
	    result += '/';
	}
    }
    return result;
}

/** Directory to save attachments into.
 *
 *  If empty, attachments are not saved.
 */
static string attachment_dir = get_attachment_dir();

/** Counter used to ensure saved attachments have unique names.
 *
 *  Reset to 0 for each email processed.
 */
static unsigned attachment_counter;

/** To aid debugging attachment filenames from emails are used in many cases.
 *
 *  This constant limits how long an attachment leafname can be.
 */
static constexpr size_t MAX_ATTACHMENT_LEAF_LEN = 64;

/** To aid debugging attachment filenames from emails are used in many cases.
 *
 *  This constant limits how long a file extension can be.
 */
static constexpr size_t MAX_ATTACHMENT_EXT_LEN = 14;

/** To aid debugging attachment filenames from emails are used in many cases.
 *
 *  If there's no supplied filename or it isn't suitable then this basename
 *  is used instead (potentially with the extension if there's a supplied
 *  name).
 */
static constexpr const char* FALLBACK_ATTACHMENT_BASENAME = "attachment";

/** Buffer size used when reading and decoding MIME data. */
static constexpr unsigned SIZE = 4096;

static void
extract_html(const string& text, string& charset)
{
    HtmlParser parser;
    if (charset.empty())
	charset = "UTF-8";
    try {
	parser.ignore_metarobots();
	parser.parse(text, charset, false);
    } catch (const string& newcharset) {
	parser.reset();
	parser.ignore_metarobots();
	parser.parse(text, newcharset, true);
    }
    send_field(FIELD_BODY, parser.dump);
}

static std::string
decode(GMimeDataWrapper* content, GMimeContentEncoding content_encoding)
{
    string result;
    char buffer[SIZE];
    unsigned char data[SIZE];
    auto u_buff = reinterpret_cast<unsigned char*>(buffer);
    GMimeStream* sr = g_mime_data_wrapper_get_stream(content);
    int state = 0;
    guint32 save = 0;
    int len;
    switch (content_encoding) {
	case GMIME_CONTENT_ENCODING_BASE64:
	    while ((len = g_mime_stream_read(sr, buffer, SIZE)) > 0) {
		len = g_mime_encoding_base64_decode_step(u_buff, len, data,
							 &state, &save);
		result.append(reinterpret_cast<char*>(data), len);
	    }
	    break;
	case GMIME_CONTENT_ENCODING_UUENCODE:
	    while ((len = g_mime_stream_read(sr, buffer, SIZE)) > 0) {
		len = g_mime_encoding_uudecode_step(u_buff, len, data,
						    &state, &save);
		result.append(reinterpret_cast<char*>(data), len);
	    }
	    break;
	case GMIME_CONTENT_ENCODING_QUOTEDPRINTABLE:
	    while ((len = g_mime_stream_read(sr, buffer, SIZE)) > 0) {
		len = g_mime_encoding_quoted_decode_step(u_buff, len, data,
							 &state, &save);
		result.append(reinterpret_cast<char*>(data), len);
	    }
	    break;
	default:
	    while ((len = g_mime_stream_read(sr, buffer, SIZE)) > 0) {
		result.append(reinterpret_cast<char*>(buffer), len);
	    }
	    break;
    }
    return result;
}

static bool
parse_mime_part(GMimePart* part,
		GMimeContentType* ct,
		bool attachments)
{
    const char* type = g_mime_content_type_get_media_type(ct);
    const char* subtype = g_mime_content_type_get_media_subtype(ct);
    enum { OTHER = 0, TEXT_PLAIN, TEXT_HTML } t = OTHER;
    if (g_ascii_strcasecmp(type, "text") == 0) {
	if (g_ascii_strcasecmp(subtype, "plain") == 0) {
	    t = TEXT_PLAIN;
	} else if (g_ascii_strcasecmp(subtype, "html") == 0) {
	    t = TEXT_HTML;
	}
    }

    if (t == OTHER && !attachments) {
	// We're not interested in this MIME part.
	return false;
    }

#if GMIME_MAJOR_VERSION >= 3
    GMimeDataWrapper* content = g_mime_part_get_content(part);
#else
    GMimeDataWrapper* content = g_mime_part_get_content_object(part);
#endif
    if (!content) return false;

    string data = decode(content, g_mime_part_get_content_encoding(part));
    if (t == TEXT_PLAIN) {
	// Always convert text/plain parts to UTF-8 (doesn't matter whether
	// we're sending as a field or saving as an attachment).
	const char* charset = g_mime_content_type_get_parameter(ct, "charset");
	if (charset) {
	    charset = g_mime_charset_canon_name(charset);
	    convert_to_utf8(data, charset);
	}
    }

    bool is_attachment = g_mime_part_is_attachment(part);
    // If we're not saving attachments, treat all text/plain and text/html
    // parts as inline.
    if (t != OTHER && (!attachments || !is_attachment)) {
	if (t == TEXT_PLAIN) {
	    // text/plain
	    send_field(FIELD_BODY, data);
	} else {
	    // text/html
	    string charset;
	    const char* p = g_mime_content_type_get_parameter(ct, "charset");
	    if (p) {
		charset = g_mime_charset_canon_name(p);
	    }
	    extract_html(data, charset);
	}
	return true;
    }

    // Save attachment.
    string filename = attachment_dir;
    // Prefix with `a` for attachment, `i` for inline.
    filename += (is_attachment ? 'a' : 'i');
    filename += str(attachment_counter++);
    filename += '-';

    // It's much easier to debug if the extracted files can be easily
    // matched up with the attachments in the email so using any
    // supplied filename is good, but we need to keep security concerns
    // in mind.
    const char* leaf = g_mime_part_get_filename(part);
    if (!leaf)
	leaf = g_mime_content_type_get_parameter(ct, "filename");
    if (leaf) {
	// Remove any path.
	const char* slash = strrchr(leaf, '/');
	if (slash) leaf = slash + 1;
	slash = strrchr(leaf, '\\');
	if (slash) leaf = slash + 1;

	size_t len = strlen(leaf);
	if (len < 3 || len > MAX_ATTACHMENT_LEAF_LEN || leaf[0] == '.') {
	    // Don't use the leafname as is, but use the extension if it's
	    // sensible.
	    leaf = strrchr(leaf + 1, '.');
	    if (leaf && strlen(leaf + 1) <= MAX_ATTACHMENT_EXT_LEN) {
		// Use the extension.
	    } else {
		// Don't use the leafname at all.
		leaf = nullptr;
	    }
	}
    }

    if (!leaf || leaf[0] == '.') {
	filename += FALLBACK_ATTACHMENT_BASENAME;
    }
    if (leaf) {
	for (size_t i = 0; leaf[i]; ++i) {
	    char ch = leaf[i];
	    // Only allow clearly safe characters.
	    if (C_isalnum(ch) || ch == '.' || ch == '-' || ch == '+')
		filename += ch;
	    else
		filename += '_';
	}
    }

    // This ends up ignoring the supplied Content-Type.  If the extracted
    // attachments are fed back into the indexer then this will end up
    // determining the MIME Content-Type from the extension or file
    // contents instead.
    //
    // It seems odd to ignore the supplied Content-Type, but in practice
    // the supplied MIME type is likely actually determined by the sending
    // MUA from the extension and/or file contents or is just set to
    // `application/octet-stream`, so arguably it's better to determine the
    // type for ourselves in a consistent way.  Otherwise the exact same
    // attachment could be indexed or not depending who sent it.
    int fd = open(filename.c_str(),
		  O_CREAT | O_EXCL | O_WRONLY | O_BINARY,
		  0664);
    const char* p = data.data();
    size_t count = data.size();
    while (count) {
	ssize_t r = write(fd, p, count);
	if (rare(r < 0)) {
	    if (errno == EINTR) continue;
	    close(fd);
	    unlink(filename.c_str());
	    send_field(FIELD_ERROR, "saving attachment failed");
	    return false;
	}
	p += r;
	count -= r;
    }
    close(fd);
    send_field(FIELD_ATTACHMENT, filename);
    return false;
}

static bool
parse_mime_object(GMimeObject* me, bool attachments)
{
    GMimeContentType* ct = g_mime_object_get_content_type(me);
    if (GMIME_IS_PART(me)) {
	return parse_mime_part(reinterpret_cast<GMimePart*>(me),
			       ct,
			       attachments);
    }

    if (!GMIME_IS_MULTIPART(me))
	return false;

    GMimeMultipart* mpart = reinterpret_cast<GMimeMultipart*>(me);
    const char* subtype = g_mime_content_type_get_media_subtype(ct);
    int count = g_mime_multipart_get_count(mpart);
    if (g_ascii_strcasecmp(subtype, "alternative") == 0) {
	// Use the last MIME part which we get text from.
	for (int i = count - 1; i >= 0; --i) {
	    GMimeObject* part = g_mime_multipart_get_part(mpart, i);
	    // Don't consider parts under an alternative as attachments.
	    if (parse_mime_object(part, false))
		return true;
	}
	return false;
    }

    bool ret = false;
    for (int i = 0; i < count; ++i) {
	GMimeObject* part = g_mime_multipart_get_part(mpart, i);
	ret |= parse_mime_object(part, attachments);
    }
    return ret;
}

static void
send_glib_field(Field field, gchar* data)
{
    if (data) {
	send_field(field, data);
	g_free(data);
    }
}

static void
extract_addresses(Field field, InternetAddressList* address_list)
{
#if GMIME_MAJOR_VERSION >= 3
    auto value = internet_address_list_to_string(address_list, NULL, false);
#else
    auto value = internet_address_list_to_string(address_list, false);
#endif
    send_glib_field(field, value);
}

static void
extract_message(GMimeMessage* message)
{
    attachment_counter = 0;
    (void)parse_mime_object(g_mime_message_get_mime_part(message),
			    !attachment_dir.empty());
    send_field(FIELD_TITLE, g_mime_message_get_subject(message));

#if GMIME_MAJOR_VERSION >= 3
    extract_addresses(FIELD_AUTHOR, g_mime_message_get_from(message));
#else
    send_field(FIELD_AUTHOR, g_mime_message_get_sender(message));
#endif
#if GMIME_MAJOR_VERSION >= 3
    extract_addresses(FIELD_TO, g_mime_message_get_to(message));
#else
    extract_addresses(FIELD_TO,
		      g_mime_message_get_recipients(message,
						    GMIME_RECIPIENT_TYPE_TO));
#endif
#if GMIME_MAJOR_VERSION >= 3
    extract_addresses(FIELD_CC, g_mime_message_get_cc(message));
#else
    extract_addresses(FIELD_CC,
		      g_mime_message_get_recipients(message,
						    GMIME_RECIPIENT_TYPE_CC));
#endif
#if GMIME_MAJOR_VERSION >= 3
    extract_addresses(FIELD_BCC, g_mime_message_get_bcc(message));
#else
    extract_addresses(FIELD_BCC,
		      g_mime_message_get_recipients(message,
						    GMIME_RECIPIENT_TYPE_BCC));
#endif
#if GMIME_MAJOR_VERSION >= 3
    GDateTime* datetime = g_mime_message_get_date(message);
    if (datetime) {
	GDateTime* utc_datetime = g_date_time_to_utc(datetime);
	if (utc_datetime) {
	    gint64 unix_time = g_date_time_to_unix(utc_datetime);
	    // Check value doesn't overflow time_t.
	    if (gint64(time_t(unix_time)) == unix_time) {
		send_field_created_date(time_t(unix_time));
	    }
	    g_date_time_unref(utc_datetime);
	}
    }
#else
    time_t datetime;
    int tz_offset;
    g_mime_message_get_date(message, &datetime, &tz_offset);
    if (datetime != time_t(-1)) {
	// The documentation doesn't clearly say, but from testing the
	// time_t value is in UTC which is what we want so we don't need
	// tz_offset.
	//
	// (If we did, tz_offset is not actually in hours as the docs say,
	// but actually hours*100+minutes, e.g. +1300 for UTC+13).
	send_field_created_date(datetime);
    }
#endif
    send_field(FIELD_MESSAGE_ID, g_mime_message_get_message_id(message));
    GMimeObject* object = GMIME_OBJECT(message);
    GMimeHeaderList* headers = g_mime_object_get_header_list(object);
#if GMIME_MAJOR_VERSION >= 3
    int count = g_mime_header_list_get_count(headers);
    for (int i = 0; i < count; ++i) {
	GMimeHeader* header = g_mime_header_list_get_header_at(headers, i);
	auto name = g_mime_header_get_name(header);
# define value g_mime_header_get_value(header)
#else
    g_mime_header_list_foreach(headers,
			       [](const char* name,
				  const char* value,
				  gpointer) {
#endif
	if (g_ascii_strcasecmp(name, "Comments") == 0 ||
	    g_ascii_strcasecmp(name, "Keywords") == 0) {
	    send_field(FIELD_KEYWORDS, value);
	}
#if GMIME_MAJOR_VERSION >= 3
    }
# undef value
#else
			       }, nullptr);
#endif
}

bool
initialise()
{
#if GMIME_MAJOR_VERSION >= 3
    g_mime_init();
#else
    g_mime_init(GMIME_ENABLE_RFC2047_WORKAROUNDS);
#endif
    return true;
}

void
extract(const string& filename,
	const string& mimetype)
{
    FILE* fp = fopen(filename.c_str(), "r");

    if (fp == NULL) {
	send_field(FIELD_ERROR, "fopen() failed");
	return;
    }

    GMimeStream* stream = g_mime_stream_file_new(fp);
    GMimeParser* parser = g_mime_parser_new_with_stream(stream);
#if GMIME_MAJOR_VERSION >= 3
    GMimeMessage* message = g_mime_parser_construct_message(parser, NULL);
#else
    GMimeMessage* message = g_mime_parser_construct_message(parser);
#endif
    if (message) {
	extract_message(message);
	g_object_unref(message);
    }
    g_object_unref(parser);
    g_object_unref(stream);
}
