/** @file sorter.h
 * @brief Build sort keys for MSet ordering
 */
/* Copyright (C) 2007,2009 Olly Betts
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#ifndef XAPIAN_INCLUDED_SORTER_H
#define XAPIAN_INCLUDED_SORTER_H

#include <string>
#include <vector>

#include <xapian/document.h>
#include <xapian/visibility.h>

namespace Xapian {

/** Virtual base class for sorter functor. */
class XAPIAN_VISIBILITY_DEFAULT Sorter {
  public:
    /** This method takes a Document object and builds a sort key from it.
     *
     *  Documents are then ordered by a string compare on the sort keys.
     */
    virtual std::string operator()(const Xapian::Document & doc) const = 0;

    /** Virtual destructor, because we have virtual methods. */
    virtual ~Sorter();
};

/** Sorter subclass which sorts by a several values.
 *
 *  Results are ordered by the first value.  In the event of a tie, the
 *  second is used.  If this is the same for both, the third is used, and
 *  so on.
 */
class XAPIAN_VISIBILITY_DEFAULT MultiValueSorter : public Sorter {
    std::vector<std::pair<Xapian::valueno, bool> > valnos;

  public:
    MultiValueSorter() { }

    template <class Iterator>
    MultiValueSorter(Iterator begin, Iterator end) {
	while (begin != end) add(*begin++);
    }

    virtual std::string operator()(const Xapian::Document & doc) const;

    void add(Xapian::valueno valno, bool forward = true) {
	valnos.push_back(std::make_pair(valno, forward));
    }
};

}

#endif // XAPIAN_INCLUDED_SORTER_H
