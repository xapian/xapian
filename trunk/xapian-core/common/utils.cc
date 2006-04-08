/* utils.cc: Various useful utilities
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2003,2004,2005 Olly Betts
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 * -----END-LICENCE-----
 */

#include <config.h>

#include "utils.h"

#include <stdio.h>

using namespace std;

#ifdef __WIN32__
#include "safewindows.h"
#endif

// This ought to be enough for any of the conversions below.
#define BUFSIZE 100

#ifdef SNPRINTF
#define CONVERT_TO_STRING(FMT) \
    char buf[BUFSIZE];\
    int len = SNPRINTF(buf, BUFSIZE, (FMT), val);\
    if (len == -1 || len > BUFSIZE) return string(buf, BUFSIZE);\
    return string(buf, len);
#else
#define CONVERT_TO_STRING(FMT) \
    char buf[BUFSIZE];\
    buf[BUFSIZE - 1] = '\0';\
    sprintf(buf, (FMT), val);\
    if (buf[BUFSIZE - 1]) abort(); /* Uh-oh, buffer overrun */ \
    return string(buf);
#endif

// Convert a number to a string
string
om_tostring(int val)
{
    CONVERT_TO_STRING("%d")
}

string
om_tostring(unsigned int val)
{
    CONVERT_TO_STRING("%u")
}

string
om_tostring(long int val)
{
    CONVERT_TO_STRING("%ld")
}

string
om_tostring(unsigned long int val)
{
    CONVERT_TO_STRING("%lu")
}

string
om_tostring(double val)
{
    CONVERT_TO_STRING("%.20g")
}

string
om_tostring(const void * val)
{
    CONVERT_TO_STRING("%p")
}

string
om_tostring(bool val)
{
    return val ? "1" : "0";
}

void
split_words(string text, vector<string> &words, char ws)
{
    if (text.length() > 0 && text[0] == ws) {
	text.erase(0, text.find_first_not_of(ws));
    }
    while (text.length() > 0) {
	words.push_back(text.substr(0, text.find_first_of(ws)));
	text.erase(0, text.find_first_of(ws));
	text.erase(0, text.find_first_not_of(ws));
    }
}

int
map_string_to_value(const StringAndValue * haystack, const string & needle)
{
    while (haystack->name[0] != '\0') {
	if (haystack->name == needle) break;
	haystack++;
    }
    return haystack->value;
}

/** Return true if the file fname exists
 */
bool
file_exists(const string &fname)
{
    struct stat sbuf;
    // exists && is a regular file
    return stat(fname, &sbuf) == 0 && S_ISREG(sbuf.st_mode);
}

/// Remove a directory and contents.
void
rmdir(const string &filename)
{
    // Check filename exists and is actually a directory
    struct stat sb;
    if (stat(filename, &sb) != 0 || !S_ISDIR(sb.st_mode)) return;

    string safefile = filename;
#ifdef __WIN32__
# if 1
    string::iterator i;
    for (i = safefile.begin(); i != safefile.end(); ++i) {
	if (*i == '/') {
	    // Convert Unix path separators to backslashes.  C library
	    // functions understand "/" in paths, but Win32 API functions
	    // don't.
	    *i = '\\';
	} else if (*i < 32 || strchr("<>\"|*?", *i)) {
	    // Check for illegal characters in filename.
	    return;
	}
    }

    static int win95 = -1;
    if (win95 == -1) {
	OSVERSIONINFO info;
	memset(&info, 0, sizeof(OSVERSIONINFO));
	info.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	if (GetVersionEx(&info)) {
	    win95 = (info.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS);
	}
    }

    if (win95) {
	// for 95 like systems:
	system("deltree /y \"" + safefile + "\"");
    } else {
	// for NT like systems:
	system("rd /s /q \"" + safefile + "\"");
    }
# else
    safefile.append("\0", 2);
    SHFILEOPSTRUCT shfo;
    memset((void*)&shfo, 0, sizeof(shfo));
    shfo.hwnd = 0;
    shfo.wFunc = FO_DELETE;
    shfo.pFrom = safefile.data();
    shfo.fFlags = FOF_NOCONFIRMATION|FOF_NOERRORUI|FOF_SILENT;
    (void)SHFileOperation(&shfo);
# endif
#else
    string::size_type p = 0;
    while (p < safefile.size()) {
	// Don't escape a few safe characters which are common in filenames
	if (!C_isalnum(safefile[p]) && strchr("/._-", safefile[p]) == NULL) {
	    safefile.insert(p, "\\");
	    ++p;
	}
	++p;
    }
    system("rm -rf " + safefile);
#endif
}

namespace Xapian {
namespace Internal {

// FIXME: These tables assume ASCII or an ASCII compatible character set
// such as ISO-8859-N or UTF-8.  EBCDIC would need some work (patches
// welcome!)  For now, use a compile time check - if '\x20' isn't a space
// then the array dimension will be negative.

// FIXME: look at using boost's static_assert for cleaner compile time
// asserts...

const unsigned char is_tab[('\x20' == ' ') ? 256 : -1] = {
 /* \x00     */ 0,
 /* \x01     */ 0,
 /* \x02     */ 0,
 /* \x03     */ 0,
 /* \x04     */ 0,
 /* \x05     */ 0,
 /* \x06     */ 0,
 /* \x07     */ 0,
 /* \x08     */ 0,
 /* \x09     */ IS_SPACE,
 /* \x0a     */ IS_SPACE,
 /* \x0b     */ 0,
 /* \x0c     */ IS_SPACE,
 /* \x0d     */ IS_SPACE,
 /* \x0e     */ 0,
 /* \x0f     */ 0,
 /* \x10     */ 0,
 /* \x11     */ 0,
 /* \x12     */ 0,
 /* \x13     */ 0,
 /* \x14     */ 0,
 /* \x15     */ 0,
 /* \x16     */ 0,
 /* \x17     */ 0,
 /* \x18     */ 0,
 /* \x19     */ 0,
 /* \x1a     */ 0,
 /* \x1b     */ 0,
 /* \x1c     */ 0,
 /* \x1d     */ 0,
 /* \x1e     */ 0,
 /* \x1f     */ 0,
 /* \x20 ( ) */ IS_SPACE,
 /* \x21 (!) */ 0,
 /* \x22 (") */ 0,
 /* \x23 (#) */ 0,
 /* \x24 ($) */ 0,
 /* \x25 (%) */ 0,
 /* \x26 (&) */ 0,
 /* \x27 (') */ 0,
 /* \x28 (() */ 0,
 /* \x29 ()) */ 0,
 /* \x2a (*) */ 0,
 /* \x2b (+) */ IS_SIGN,
 /* \x2c (,) */ 0,
 /* \x2d (-) */ IS_SIGN,
 /* \x2e (.) */ 0,
 /* \x2f (/) */ 0,
 /* \x30 (0) */ IS_DIGIT|IS_HEX,
 /* \x31 (1) */ IS_DIGIT|IS_HEX,
 /* \x32 (2) */ IS_DIGIT|IS_HEX,
 /* \x33 (3) */ IS_DIGIT|IS_HEX,
 /* \x34 (4) */ IS_DIGIT|IS_HEX,
 /* \x35 (5) */ IS_DIGIT|IS_HEX,
 /* \x36 (6) */ IS_DIGIT|IS_HEX,
 /* \x37 (7) */ IS_DIGIT|IS_HEX,
 /* \x38 (8) */ IS_DIGIT|IS_HEX,
 /* \x39 (9) */ IS_DIGIT|IS_HEX,
 /* \x3a (:) */ 0,
 /* \x3b (;) */ 0,
 /* \x3c (<) */ 0,
 /* \x3d (=) */ 0,
 /* \x3e (>) */ 0,
 /* \x3f (?) */ 0,
 /* \x40 (@) */ 0,
 /* \x41 (A) */ IS_UPPER|IS_HEX,
 /* \x42 (B) */ IS_UPPER|IS_HEX,
 /* \x43 (C) */ IS_UPPER|IS_HEX,
 /* \x44 (D) */ IS_UPPER|IS_HEX,
 /* \x45 (E) */ IS_UPPER|IS_HEX,
 /* \x46 (F) */ IS_UPPER|IS_HEX,
 /* \x47 (G) */ IS_UPPER,
 /* \x48 (H) */ IS_UPPER,
 /* \x49 (I) */ IS_UPPER,
 /* \x4a (J) */ IS_UPPER,
 /* \x4b (K) */ IS_UPPER,
 /* \x4c (L) */ IS_UPPER,
 /* \x4d (M) */ IS_UPPER,
 /* \x4e (N) */ IS_UPPER,
 /* \x4f (O) */ IS_UPPER,
 /* \x50 (P) */ IS_UPPER,
 /* \x51 (Q) */ IS_UPPER,
 /* \x52 (R) */ IS_UPPER,
 /* \x53 (S) */ IS_UPPER,
 /* \x54 (T) */ IS_UPPER,
 /* \x55 (U) */ IS_UPPER,
 /* \x56 (V) */ IS_UPPER,
 /* \x57 (W) */ IS_UPPER,
 /* \x58 (X) */ IS_UPPER,
 /* \x59 (Y) */ IS_UPPER,
 /* \x5a (Z) */ IS_UPPER,
 /* \x5b ([) */ 0,
 /* \x5c (\) */ 0,
 /* \x5d (]) */ 0,
 /* \x5e (^) */ 0,
 /* \x5f (_) */ 0,
 /* \x60 (`) */ 0,
 /* \x61 (a) */ IS_LOWER|IS_HEX,
 /* \x62 (b) */ IS_LOWER|IS_HEX,
 /* \x63 (c) */ IS_LOWER|IS_HEX,
 /* \x64 (d) */ IS_LOWER|IS_HEX,
 /* \x65 (e) */ IS_LOWER|IS_HEX,
 /* \x66 (f) */ IS_LOWER|IS_HEX,
 /* \x67 (g) */ IS_LOWER,
 /* \x68 (h) */ IS_LOWER,
 /* \x69 (i) */ IS_LOWER,
 /* \x6a (j) */ IS_LOWER,
 /* \x6b (k) */ IS_LOWER,
 /* \x6c (l) */ IS_LOWER,
 /* \x6d (m) */ IS_LOWER,
 /* \x6e (n) */ IS_LOWER,
 /* \x6f (o) */ IS_LOWER,
 /* \x70 (p) */ IS_LOWER,
 /* \x71 (q) */ IS_LOWER,
 /* \x72 (r) */ IS_LOWER,
 /* \x73 (s) */ IS_LOWER,
 /* \x74 (t) */ IS_LOWER,
 /* \x75 (u) */ IS_LOWER,
 /* \x76 (v) */ IS_LOWER,
 /* \x77 (w) */ IS_LOWER,
 /* \x78 (x) */ IS_LOWER,
 /* \x79 (y) */ IS_LOWER,
 /* \x7a (z) */ IS_LOWER,
 /* \x7b ({) */ 0,
 /* \x7c (|) */ 0,
 /* \x7d (}) */ 0,
 /* \x7e (~) */ 0,
 /* \x7f     */ 0,
 /* \x80     */ 0,
 /* \x81     */ 0,
 /* \x82     */ 0,
 /* \x83     */ 0,
 /* \x84     */ 0,
 /* \x85     */ 0,
 /* \x86     */ 0,
 /* \x87     */ 0,
 /* \x88     */ 0,
 /* \x89     */ 0,
 /* \x8a     */ 0,
 /* \x8b     */ 0,
 /* \x8c     */ 0,
 /* \x8d     */ 0,
 /* \x8e     */ 0,
 /* \x8f     */ 0,
 /* \x90     */ 0,
 /* \x91     */ 0,
 /* \x92     */ 0,
 /* \x93     */ 0,
 /* \x94     */ 0,
 /* \x95     */ 0,
 /* \x96     */ 0,
 /* \x97     */ 0,
 /* \x98     */ 0,
 /* \x99     */ 0,
 /* \x9a     */ 0,
 /* \x9b     */ 0,
 /* \x9c     */ 0,
 /* \x9d     */ 0,
 /* \x9e     */ 0,
 /* \x9f     */ 0,
 /* \xa0     */ 0,
 /* \xa1 (¡) */ 0,
 /* \xa2 (¢) */ 0,
 /* \xa3 (£) */ 0,
 /* \xa4 (¤) */ 0,
 /* \xa5 (¥) */ 0,
 /* \xa6 (¦) */ 0,
 /* \xa7 (§) */ 0,
 /* \xa8 (¨) */ 0,
 /* \xa9 (©) */ 0,
 /* \xaa (ª) */ 0,
 /* \xab («) */ 0,
 /* \xac (¬) */ 0,
 /* \xad (­) */ 0,
 /* \xae (®) */ 0,
 /* \xaf (¯) */ 0,
 /* \xb0 (°) */ 0,
 /* \xb1 (±) */ 0,
 /* \xb2 (²) */ 0,
 /* \xb3 (³) */ 0,
 /* \xb4 (´) */ 0,
 /* \xb5 (µ) */ 0,
 /* \xb6 (¶) */ 0,
 /* \xb7 (·) */ 0,
 /* \xb8 (¸) */ 0,
 /* \xb9 (¹) */ 0,
 /* \xba (º) */ 0,
 /* \xbb (») */ 0,
 /* \xbc (¼) */ 0,
 /* \xbd (½) */ 0,
 /* \xbe (¾) */ 0,
 /* \xbf (¿) */ 0,
 /* \xc0 (À) */ 0,
 /* \xc1 (Á) */ 0,
 /* \xc2 (Â) */ 0,
 /* \xc3 (Ã) */ 0,
 /* \xc4 (Ä) */ 0,
 /* \xc5 (Å) */ 0,
 /* \xc6 (Æ) */ 0,
 /* \xc7 (Ç) */ 0,
 /* \xc8 (È) */ 0,
 /* \xc9 (É) */ 0,
 /* \xca (Ê) */ 0,
 /* \xcb (Ë) */ 0,
 /* \xcc (Ì) */ 0,
 /* \xcd (Í) */ 0,
 /* \xce (Î) */ 0,
 /* \xcf (Ï) */ 0,
 /* \xd0 (Ð) */ 0,
 /* \xd1 (Ñ) */ 0,
 /* \xd2 (Ò) */ 0,
 /* \xd3 (Ó) */ 0,
 /* \xd4 (Ô) */ 0,
 /* \xd5 (Õ) */ 0,
 /* \xd6 (Ö) */ 0,
 /* \xd7 (×) */ 0,
 /* \xd8 (Ø) */ 0,
 /* \xd9 (Ù) */ 0,
 /* \xda (Ú) */ 0,
 /* \xdb (Û) */ 0,
 /* \xdc (Ü) */ 0,
 /* \xdd (Ý) */ 0,
 /* \xde (Þ) */ 0,
 /* \xdf (ß) */ 0,
 /* \xe0 (à) */ 0,
 /* \xe1 (á) */ 0,
 /* \xe2 (â) */ 0,
 /* \xe3 (ã) */ 0,
 /* \xe4 (ä) */ 0,
 /* \xe5 (å) */ 0,
 /* \xe6 (æ) */ 0,
 /* \xe7 (ç) */ 0,
 /* \xe8 (è) */ 0,
 /* \xe9 (é) */ 0,
 /* \xea (ê) */ 0,
 /* \xeb (ë) */ 0,
 /* \xec (ì) */ 0,
 /* \xed (í) */ 0,
 /* \xee (î) */ 0,
 /* \xef (ï) */ 0,
 /* \xf0 (ð) */ 0,
 /* \xf1 (ñ) */ 0,
 /* \xf2 (ò) */ 0,
 /* \xf3 (ó) */ 0,
 /* \xf4 (ô) */ 0,
 /* \xf5 (õ) */ 0,
 /* \xf6 (ö) */ 0,
 /* \xf7 (÷) */ 0,
 /* \xf8 (ø) */ 0,
 /* \xf9 (ù) */ 0,
 /* \xfa (ú) */ 0,
 /* \xfb (û) */ 0,
 /* \xfc (ü) */ 0,
 /* \xfd (ý) */ 0,
 /* \xfe (þ) */ 0,
 /* \xff (ÿ) */ 0,
};

// C++ doesn't allow the handy C feature of initialising a char
// array with a string which is exactly the right length, so the
// trailing '\0' isn't included, so our tables need to be 257
// bytes!
const unsigned char lo_tab[257] =
    "\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f"
    "\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1a\x1b\x1c\x1d\x1e\x1f"
    " !\"#$%&'()*+,-./0123456789:;<=>?"
    "@abcdefghijklmnopqrstuvwxyz[\\]^_"
    "`abcdefghijklmnopqrstuvwxyz{|}~\x7f"
    "\x80\x81\x82\x83\x84\x85\x86\x87\x88\x89\x8a\x8b\x8c\x8d\x8e\x8f"
    "\x90\x91\x92\x93\x94\x95\x96\x97\x98\x99\x9a\x9b\x9c\x9d\x9e\x9f"
    "\xa0\xa1\xa2\xa3\xa4\xa5\xa6\xa7\xa8\xa9\xaa\xab\xac\xad\xae\xaf"
    "\xb0\xb1\xb2\xb3\xb4\xb5\xb6\xb7\xb8\xb9\xba\xbb\xbc\xbd\xbe\xbf"
    "\xc0\xc1\xc2\xc3\xc4\xc5\xc6\xc7\xc8\xc9\xca\xcb\xcc\xcd\xce\xcf"
    "\xd0\xd1\xd2\xd3\xd4\xd5\xd6\xd7\xd8\xd9\xda\xdb\xdc\xdd\xde\xdf"
    "\xe0\xe1\xe2\xe3\xe4\xe5\xe6\xe7\xe8\xe9\xea\xeb\xec\xed\xee\xef"
    "\xf0\xf1\xf2\xf3\xf4\xf5\xf6\xf7\xf8\xf9\xfa\xfb\xfc\xfd\xfe\xff";

const unsigned char up_tab[257] =
    "\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f"
    "\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1a\x1b\x1c\x1d\x1e\x1f"
    " !\"#$%&'()*+,-./0123456789:;<=>?"
    "@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_"
    "`ABCDEFGHIJKLMNOPQRSTUVWXYZ{|}~\x7f"
    "\x80\x81\x82\x83\x84\x85\x86\x87\x88\x89\x8a\x8b\x8c\x8d\x8e\x8f"
    "\x90\x91\x92\x93\x94\x95\x96\x97\x98\x99\x9a\x9b\x9c\x9d\x9e\x9f"
    "\xa0\xa1\xa2\xa3\xa4\xa5\xa6\xa7\xa8\xa9\xaa\xab\xac\xad\xae\xaf"
    "\xb0\xb1\xb2\xb3\xb4\xb5\xb6\xb7\xb8\xb9\xba\xbb\xbc\xbd\xbe\xbf"
    "\xc0\xc1\xc2\xc3\xc4\xc5\xc6\xc7\xc8\xc9\xca\xcb\xcc\xcd\xce\xcf"
    "\xd0\xd1\xd2\xd3\xd4\xd5\xd6\xd7\xd8\xd9\xda\xdb\xdc\xdd\xde\xdf"
    "\xe0\xe1\xe2\xe3\xe4\xe5\xe6\xe7\xe8\xe9\xea\xeb\xec\xed\xee\xef"
    "\xf0\xf1\xf2\xf3\xf4\xf5\xf6\xf7\xf8\xf9\xfa\xfb\xfc\xfd\xfe\xff";

}
}
