/** @file
 * @brief Cast a value to a type, clamping out of range values
 */
/* Copyright (C) 2026 Olly Betts
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

#ifndef XAPIAN_INCLUDED_CLAMP_CAST_H
#define XAPIAN_INCLUDED_CLAMP_CAST_H

#include <algorithm>
#include <limits>

/** Cast a value to a type, clamping out of range values.
 *
 *  Version for const pointers.
 */
template<typename T, typename U>
T
clamp_cast(U value)
{
    // Add a suitably-typed zero to each argument we pass to std::clamp to get
    // everything in an appropriate common type via C++ type promotion rules.
    auto v = value + T{0};
    auto min = std::numeric_limits<T>::min() + U{0};
    auto max = std::numeric_limits<T>::max() + U{0};
    return static_cast<T>(std::clamp(v, min, max));
}

#endif /* XAPIAN_INCLUDED_CLAMP_CAST_H */
