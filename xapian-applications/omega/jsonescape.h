/** @file
 * @brief JSON escaping
 */
/* Copyright (C) 2013,2018,2019,2021 Olly Betts
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

#ifndef OMEGA_INCLUDED_JSONESCAPE_H
#define OMEGA_INCLUDED_JSONESCAPE_H

#include <string>

void json_escape(std::string &s);

/// Convert a C++ std::map to JSON.
template<typename C, typename F1, typename F2>
static inline std::string
to_json(const C& container, F1 func1, F2 func2)
{
    std::string result = "{\"";
    bool first = true;
    for (auto entry : container) {
	if (first) {
	    first = false;
	} else {
	    result += ",\"";
	}
	std::string key = func1(entry.first);
	json_escape(key);
	result += key;
	result += "\":";
	result += func2(entry.second);
    }
    if (first) {
	// Special case for an empty object.
	result = "{}";
    } else {
	result += "}";
    }
    return result;
}

#endif // OMEGA_INCLUDED_JSONESCAPE_H
