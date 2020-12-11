/** @file
 * @brief Extract metadata using libextractor.
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

#include <extractor.h>
#include <cstring>

using namespace std;

struct metadata {
    string& title;
    string& author;
    string& keywords;
    string& pages;

    metadata(string& t, string& a, string& k, string& p)
	    : title(t), author(a), keywords(k), pages(p) {}
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

    switch (type) {
	case EXTRACTOR_METATYPE_BOOK_TITLE:
	case EXTRACTOR_METATYPE_JOURNAL_NAME:
	case EXTRACTOR_METATYPE_ORIGINAL_TITLE:
	case EXTRACTOR_METATYPE_SUBJECT:
	case EXTRACTOR_METATYPE_TITLE:
	    if (!md->title.empty())
		md->title.append(" ");
	    md->title.append(data, data_len);
	    break;

	case EXTRACTOR_METATYPE_PAGE_COUNT:
	    md->pages.assign(data, data_len);
	    break;

	case EXTRACTOR_METATYPE_ARTIST:
	case EXTRACTOR_METATYPE_AUTHOR_NAME:
	case EXTRACTOR_METATYPE_CREATOR:
	case EXTRACTOR_METATYPE_MOVIE_DIRECTOR:
	case EXTRACTOR_METATYPE_ORIGINAL_ARTIST:
	case EXTRACTOR_METATYPE_ORIGINAL_WRITER:
	case EXTRACTOR_METATYPE_PUBLISHER:
	    if (!md->author.empty())
		md->author.append(" ");
	    md->author.append(data, data_len);
	    break;

	default:
	    if (!md->keywords.empty())
		md->keywords.append(" ");
	    md->keywords.append(data, data_len);
    }
    return 0;
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
    try {
	struct metadata md(title, author, keywords, pages);

	// Add all default plugins
	auto plugins =
	 EXTRACTOR_plugin_add_defaults(EXTRACTOR_OPTION_DEFAULT_POLICY);

	// If plugin not found/ File format not recognised/ corrupt file
	// returns null and not an error.
	EXTRACTOR_extract(plugins, filename.c_str(),
			  NULL, 0,
			  &process_metadata, &md);
	EXTRACTOR_plugin_remove_all(plugins);
    } catch (...) {
	error = "Libextractor threw an exception";
	return false;
    }

    return true;
}
