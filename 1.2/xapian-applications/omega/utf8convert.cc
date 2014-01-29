/* utf8convert.cc: convert a string to UTF-8 encoding.
 *
 * Copyright (C) 2006,2007,2008,2010 Olly Betts
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#include <config.h>

#include "utf8convert.h"

#include <algorithm>
#include <string>

#include "safeerrno.h"
#ifdef USE_ICONV
# include <iconv.h>
#else
# include <xapian.h>
#endif
#include "strcasecmp.h"
#include "stringutils.h"

using namespace std;

void
convert_to_utf8(string & text, const string & charset)
{
    // Shortcut if it's already in utf8!
    if (charset.size() == 5 && strcasecmp(charset.c_str(), "utf-8") == 0)
	return;
    if (charset.size() == 4 && strcasecmp(charset.c_str(), "utf8") == 0)
	return;

    // Nobody has told us what charset it's in, so do as little work as
    // possible!
    if (charset.empty())
	return;

    char buf[1024];

#ifdef USE_ICONV
    iconv_t conv = iconv_open("UTF-8", charset.c_str());
    if (conv == (iconv_t)-1) {
	if (charset.size() < 4 || charset[3] == '-')
	    return;

	// Try correcting common misspellings of UTF-16 and UCS-2 charsets.
	// In particular, handle ' ' or '_' instead of '-', and a missing '-',
	// so: UCS2 -> UCS-2, UTF_16 -> UTF-16, etc.
	//
	// Note: libiconv on OSX doesn't support these misspellings, though
	// libiconv on Ubuntu does.
	if (strncasecmp(charset.c_str(), "ucs", 3) != 0 &&
	    strncasecmp(charset.c_str(), "utf", 3) != 0) {
	    return;
	}

	string adjusted_charset(charset, 0, 3);
	adjusted_charset += '-';
	if (charset[3] == ' ' || charset[3] == '_') {
	    adjusted_charset.append(charset, 4, string::npos);
	} else {
	    adjusted_charset.append(charset, 3, string::npos);
	}

	conv = iconv_open("UTF-8", adjusted_charset.c_str());
	if (conv == (iconv_t)-1) return;
    }

    string tmp;

    ICONV_INPUT_TYPE in = const_cast<char *>(text.c_str());
    size_t in_len = text.size();
    while (in_len) {
	char * out = buf;
	size_t out_len = sizeof(buf);
	if (iconv(conv, &in, &in_len, &out, &out_len) == size_t(-1) &&
	    errno != E2BIG) {
	    // FIXME: how to handle this?
	    break;
	}
	tmp.append(buf, out - buf);
    }

    (void)iconv_close(conv);
#else
    /* If we don't have iconv, handle iso-8859-1, utf-16/ucs-2,
     * utf-16be/ucs-2be, and utf-16le/ucs-2le. */
    string tmp;
    const char * p = charset.c_str();

    bool utf16 = false;
    if (strncasecmp(p, "utf", 3) == 0) {
	p += 3;
	if (*p == '-' || *p == '_' || *p == ' ') ++p;
	if (*p != '1' || p[1] != '6') return;
	p += 2;
	utf16 = true;
    } else if (strncasecmp(p, "ucs", 3) == 0) {
	p += 3;
	if (*p == '-' || *p == '_' || *p == ' ') ++p;
	if (*p != '2') return;
	++p;
	utf16 = true;
    }

    if (utf16) {
	if (text.size() < 2) return;

	bool big_endian = true;
	string::const_iterator i = text.begin();
	if (*p == '\0') {
	    if (startswith(text, "\xfe\xff")) {
		i += 2;
	    } else if (startswith(text, "\xff\xfe")) {
		big_endian = false;
		i += 2;
	    }
	    // UTF-16 with no BOM is meant to be assumed to be BE.  Strictly
	    // speaking, we're not meant to assume anything for UCS-2 with
	    // no BOM, but we've got to do something, so we might as well
	    // assume it's UTF-16 mislabelled, which is easy and sane.
	} else if (strcasecmp(p, "LE") == 0) {
	    big_endian = false;
	} else if (!(strcasecmp(p, "BE") == 0)) {
	    return;
	}

	tmp.reserve(text.size() / 2);

	size_t start = 0;
	if (text.size() & 1) {
	    // If there's a half-character at the end, nuke it now to make the
	    // conversion loop below simpler.
	    text.resize(text.size() - 1);
	}

	while (i != text.end()) {
	    unsigned ch = static_cast<unsigned char>(*i++);
	    unsigned ch2 = static_cast<unsigned char>(*i++);
	    if (big_endian) {
		ch = (ch << 8) | ch2;
	    } else {
		ch = (ch2 << 8) | ch;
	    }
	    if (ch >> 10 == 0xd800 >> 10) {
		// Surrogate pair.
		if (i == text.end()) break;
		unsigned hi = (ch & 0x3ff);
		ch = static_cast<unsigned char>(*i++);
		ch2 = static_cast<unsigned char>(*i++);
		if (big_endian) {
		    ch = (ch << 8) | ch2;
		} else {
		    ch = (ch2 << 8) | ch;
		}
		if (ch >> 10 == 0xdc00 >> 10) {
		    ch &= 0x3ff;
		    ch |= (hi << 10);
		    ch += 0x10000;
		}
	    }
	    start += Xapian::Unicode::to_utf8(ch, buf + start);
	    if (start >= sizeof(buf) - 4) {
		tmp.append(buf, start);
		start = 0;
	    }
	}
	if (start) tmp.append(buf, start);
    } else {
	if (strncasecmp(p, "iso", 3) == 0) {
	    p += 3;
	    if (*p == '-' || *p == '_' || *p == ' ') ++p;
	}
	if (strncmp(p, "8859", 4) != 0) return;
	p += 4;
	if (*p == '-' || *p == '_' || *p == ' ') ++p;
	if (strcmp(p, "1") != 0) return;

	// FIXME: pull this out as a standard "normalise utf-8" function?
	tmp.reserve(text.size());

	size_t start = 0;
	for (string::const_iterator i = text.begin(); i != text.end(); ++i) {
	    unsigned ch = static_cast<unsigned char>(*i);
	    start += Xapian::Unicode::to_utf8(ch, buf + start);
	    if (start >= sizeof(buf) - 4) {
		tmp.append(buf, start);
		start = 0;
	    }
	}
	if (start) tmp.append(buf, start);
    }
#endif

    swap(text, tmp);
}
