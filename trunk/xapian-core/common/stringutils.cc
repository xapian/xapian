/** @file stringutils.cc
 * @brief Various handy helpers which std::string really should provide.
 */
/* Copyright (C) 2007 Olly Betts
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

#include "stringutils.h"

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
