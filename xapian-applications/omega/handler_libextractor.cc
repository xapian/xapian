/** @file
 * @brief Extract metadata using libextractor.
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
#include "parseint.h"

#include <extractor.h>
#include <cstring>
#include <sysexits.h>

using namespace std;

struct metadata {
    string dump;
    string title;
    string author;
    string keywords;
    int pages = -1;
};

/** Store metadata in its corresponding variable.
 *
 *  @param cls  		passed as last parameter from EXTRACTOR_extract
 *  @param plugin_name  	name of the plugin
 *  @param type  		mime-type of file according to libextractor
 *  @param format		format information about data
 *  @param data_mime_type	mimetype of data according to libextractor
 *  @param data 		actual meta-data found
 *  @param data_len		number of bytes in data
 */
static int
process_metadata(void* cls,
		 const char* plugin_name,
		 enum EXTRACTOR_MetaType type,
		 enum EXTRACTOR_MetaFormat format,
		 const char* data_mime_type,
		 const char* data,
		 size_t data_len)
{
    struct metadata* md = static_cast<struct metadata*>(cls);

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
	    if (!md->title.empty())
		md->title += ' ';
	    md->title.append(data, data_len);
	    break;

	case EXTRACTOR_METATYPE_PAGE_COUNT: {
	    unsigned p;
	    if (parse_unsigned(data, p)) {
		md->pages = p;
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
	    if (!md->author.empty())
		md->author += ' ';
	    md->author.append(data, data_len);
	    break;

	case EXTRACTOR_METATYPE_KEYWORDS:
	    if (!md->keywords.empty())
		md->keywords += ' ';
	    md->keywords.append(data, data_len);
	    break;

	case EXTRACTOR_METATYPE_ABSTRACT:
	case EXTRACTOR_METATYPE_COMMENT:
	case EXTRACTOR_METATYPE_DESCRIPTION:
	case EXTRACTOR_METATYPE_LYRICS:
	case EXTRACTOR_METATYPE_SUMMARY:
	    if (!md->dump.empty())
		md->dump += ' ';
	    md->dump.append(data, data_len);
	    break;

	default:
	    // Ignore other metadata.
	    break;
    }
    return 0;
}

static auto initialise() {
    // Add all default plugins.
    auto plugins =
	EXTRACTOR_plugin_add_defaults(EXTRACTOR_OPTION_DEFAULT_POLICY);
    if (!plugins)
	exit(EX_UNAVAILABLE);
    return plugins;
}

void
extract(const string& filename, const string& mimetype)
try {
    // Initialise  on first run.
    static auto plugins = initialise();

    // If plugin not found/ File format not recognised/ corrupt file
    // no data is extracted, rather than reporting an error.
    struct metadata md;
    EXTRACTOR_extract(plugins, filename.c_str(),
		      NULL, 0,
		      &process_metadata, &md);
    response(md.dump, md.title, md.keywords, md.author, md.pages);
} catch (...) {
    fail("Libextractor threw an exception");
}
