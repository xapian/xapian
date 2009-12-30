/** @file  derefwrapper.h
 *  @brief Class for wrapping type returned by an input_iterator.
 */
/* Copyright (C) 2004,2008,2009 Olly Betts
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

#ifndef XAPIAN_INCLUDED_DEREFWRAPPER_H
#define XAPIAN_INCLUDED_DEREFWRAPPER_H

namespace Xapian {

/** @private @internal Class which returns a value when dereferenced with
 *  operator*.
 *
 *  We need this wrapper class to implement input_iterator semantics for the
 *  postfix operator++ methods of some of our iterator classes.
 */
template<typename T>
class DerefWrapper_ {
    /// Don't allow assignment.
    void operator=(const DerefWrapper_ &);

    /// The value.
    T res;

  public:
    explicit DerefWrapper_(const T &res_) : res(res_) { }
    const T & operator*() const { return res; }
};

}

#endif // XAPIAN_INCLUDED_DEREFWRAPPER_H
