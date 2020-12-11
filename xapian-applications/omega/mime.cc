/** @file
 * @brief Find MIME type for a file
 */
/* Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2001,2005 James Aylett
 * Copyright 2001,2002 Ananova Ltd
 * Copyright 2002,2003,2004,2005,2006,2007,2008,2009,2010,2011,2012,2013,2014,2015 Olly Betts
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

#include "mime.h"

#include <algorithm>

#include "keyword.h"
#include "mimemap.h"
#include "stringutils.h"

using namespace std;

// The longest string after a '.' to treat as an extension.  If there's a
// longer entry in the mime_map, we set this to that length instead.
size_t max_ext_len = max(size_t(7), MAX_BUILTIN_MIMEMAP_EXTENSION_LEN);

const char *
built_in_mime_map(const string & ext)
{
    int k = keyword2(tab, ext.data(), ext.size());
    return k >= 0 ? default_mime_map[k] : NULL;
}

string
mimetype_from_ext(const map<string, string> & mime_map, string ext)
{
    map<string, string>::const_iterator mt = mime_map.find(ext);
    if (mt != mime_map.end())
	return mt->second;

    const char * r = built_in_mime_map(ext);
    if (r) return r;

    // The extension wasn't found, see if the lower-cased version (if
    // different) is found.
    bool changed = false;
    string::iterator i;
    for (i = ext.begin(); i != ext.end(); ++i) {
	if (*i >= 'A' && *i <= 'Z') {
	    *i = C_tolower(*i);
	    changed = true;
	}
    }

    if (changed) {
	mt = mime_map.find(ext);
	if (mt != mime_map.end())
	    return mt->second;

	r = built_in_mime_map(ext);
	if (r) return r;
    }

    return string();
}
