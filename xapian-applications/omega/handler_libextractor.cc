/** @file
 * @brief Extract metadata using libextractor.
 */
/* Copyright (C) 2020 Parth Kapadia
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
#include "parseint.h"

#include <extractor.h>
#include <sysexits.h>

using namespace std;

/** Store metadata in its corresponding variable.
 *
 *  @param cls  		last parameter from EXTRACTOR_extract (unused)
 *  @param plugin_name  	name of the plugin (unused)
 *  @param type  		mime-type of file according to libextractor
 *  @param format		format information about data
 *  @param data_mime_type	mimetype according to libextractor (unused)
 *  @param data 		actual meta-data found
 *  @param data_len		number of bytes in data
 */
static int
process_metadata(void*,
		 const char*,
		 enum EXTRACTOR_MetaType type,
		 enum EXTRACTOR_MetaFormat format,
		 const char*,
		 const char* data,
		 size_t data_len)
{
    switch (format) {
	case EXTRACTOR_METAFORMAT_UTF8:
	    break;

	default:
	    // specific encoding unknown
	    // EXTRACTOR_METAFORMAT_UNKNOWN
	    // EXTRACTOR_METAFORMAT_BINARY
	    // EXTRACTOR_METAFORMAT_C_STRING
	    return 0;
    }

    // "data_len is strlen (data)+1"!
    --data_len;

    switch (type) {
	case EXTRACTOR_METATYPE_BOOK_TITLE:
	case EXTRACTOR_METATYPE_JOURNAL_NAME:
	case EXTRACTOR_METATYPE_ORIGINAL_TITLE:
	case EXTRACTOR_METATYPE_SUBJECT:
	case EXTRACTOR_METATYPE_SUBTITLE:
	case EXTRACTOR_METATYPE_TITLE:
	    send_field(FIELD_TITLE, data, data_len);
	    break;

	case EXTRACTOR_METATYPE_PAGE_COUNT: {
	    unsigned p;
	    if (parse_unsigned(data, p)) {
		send_field_page_count(int(p));
	    }
	    break;
	}

	case EXTRACTOR_METATYPE_ARTIST:
	case EXTRACTOR_METATYPE_AUTHOR_NAME:
	case EXTRACTOR_METATYPE_COMPOSER:
	case EXTRACTOR_METATYPE_CONDUCTOR:
	case EXTRACTOR_METATYPE_CREATOR:
	case EXTRACTOR_METATYPE_MOVIE_DIRECTOR:
	case EXTRACTOR_METATYPE_ORIGINAL_ARTIST:
	case EXTRACTOR_METATYPE_ORIGINAL_PERFORMER:
	case EXTRACTOR_METATYPE_ORIGINAL_WRITER:
	case EXTRACTOR_METATYPE_PERFORMER:
	case EXTRACTOR_METATYPE_WRITER:
	    send_field(FIELD_AUTHOR, data, data_len);
	    break;

	case EXTRACTOR_METATYPE_KEYWORDS:
	    send_field(FIELD_KEYWORDS, data, data_len);
	    break;

	case EXTRACTOR_METATYPE_ABSTRACT:
	case EXTRACTOR_METATYPE_COMMENT:
	case EXTRACTOR_METATYPE_DESCRIPTION:
	case EXTRACTOR_METATYPE_LYRICS:
	case EXTRACTOR_METATYPE_SUMMARY:
	    send_field(FIELD_BODY, data, data_len);
	    break;

	default:
	    // Ignore other metadata.
	    break;
    }
    return 0;
}

static struct EXTRACTOR_PluginList* plugins;

bool
initialise()
{
    // Add all default plugins.
    plugins = EXTRACTOR_plugin_add_defaults(EXTRACTOR_OPTION_DEFAULT_POLICY);
    return plugins != nullptr;
}

void
extract(const string& filename, const string& mimetype)
{
    // If plugin not found/ File format not recognised/ corrupt file
    // no data is extracted, rather than reporting an error.
    EXTRACTOR_extract(plugins, filename.c_str(),
		      nullptr, 0,
		      &process_metadata, nullptr);
}
