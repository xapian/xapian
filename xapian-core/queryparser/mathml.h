/** @file mathml.h
 * @brief Declaration of Presentation MathML related tags.
 */
/* Copyright (C) 2018 Guruprasad Hegde
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

#ifndef XAPIAN_INCLUDED_MATHML_H
#define XAPIAN_INCLUDED_MATHML_H

#include <string>
#include <unordered_map>

const unsigned MATHML_SYMBOLS_TOTAL = 30;

enum mathml_id {
    MUNK,	// Unknown
    MI,
    MN,
    MO,
    MTEXT,
    MROW,
    MFRAC,
    MSQRT,
    MROOT,
    MSTYLE,
    MERROR,
    MPADDED,
    MPHANTOM,
    MFENCED,
    MENCLOSE,
    MSUB,
    MSUP,
    MSUBSUP,
    MUNDER,
    MOVER,
    MUNDEROVER,
    MMULTISCRIPTS,
    MTABLE,
    MTABLEDTR,
    MTR,
    MTD,
    MACTION,
    MANNOTATION,
    MSEMANTICS,
    MATH
};

// MathML id look-up table.
std::unordered_map<std::string, mathml_id> mathmlid_table {
    { "", MUNK },
    { "mi", MI },
    { "mn", MN },
    { "mo", MO },
    { "mtext", MTEXT },
    { "mrow", MROW },
    { "mfrac", MFRAC },
    { "msqrt", MSQRT },
    { "mroot", MROOT },
    { "mstyle", MSTYLE },
    { "merror", MERROR },
    { "mpadded", MPADDED },
    { "mphantom", MPHANTOM },
    { "mfenced", MFENCED },
    { "menclose", MENCLOSE },
    { "msub", MSUB },
    { "msup", MSUP },
    { "msubsup", MSUBSUP },
    { "munder", MUNDER },
    { "mover", MOVER },
    { "munderover", MUNDEROVER },
    { "mmultiscripts", MMULTISCRIPTS },
    { "mtable", MTABLE },
    { "mtabledtr", MTABLEDTR },
    { "mtr", MTR },
    { "mtd", MTD },
    { "maction", MACTION },
    { "mannotation", MANNOTATION },
    { "msemantics", MSEMANTICS },
    { "math", MATH } };

// Default prefix value for token elements and default value for layout related
// tags like fraction (/) or square root tag.
std::unordered_map<mathml_id, std::string> tag_values {
    { MI, "V!" },
    { MN, "N!" },
    { MO, "O" },
    { MTEXT, "T!" },
    { MFRAC, "F" },
    { MSQRT, "R" },
    { MROOT, "R" },
    { MTABLE, "M!" } };

#endif
