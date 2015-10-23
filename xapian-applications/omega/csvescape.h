/* @file csvescape.h
 * @brief Escape a string for use in a CSV file
 */
/* Copyright (C) 2015 Olly Betts
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#ifndef OMEGA_INCLUDED_CSVESCAPE_H
#define OMEGA_INCLUDED_CSVESCAPE_H

#include <string>

/** Escape @a s for use as a field in a CSV file.
 *
 *  Escaping is done as described in RFC4180, except that we treat any
 *  byte value not otherwise mentioned as being 'TEXTDATA' (so %x00-%x09,
 *  %x0B-%x0C, %x0E-%x1F, %x7F-%xFF are also permitted there).
 */
void csv_escape(std::string &s);

#endif // OMEGA_INCLUDED_CSVESCAPE_H
