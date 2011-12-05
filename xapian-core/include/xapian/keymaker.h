/** @file keymaker.h
 * @brief Build key strings for MSet ordering or collapsing.
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

#ifndef XAPIAN_INCLUDED_KEYMAKER_H
#define XAPIAN_INCLUDED_KEYMAKER_H

#include <string>
#include <vector>

#include <xapian/deprecated.h>
#include <xapian/types.h>
#include <xapian/visibility.h>

namespace Xapian {

class Document;

/** Virtual base class for key making functors. */
class XAPIAN_VISIBILITY_DEFAULT KeyMaker {
  public:
    /** Build a key string for a Document.
     *
     *  These keys can be used for sorting or collapsing matching documents.
     *
     *  @param doc	Document object to build a key for.
     */
    virtual std::string operator()(const Xapian::Document & doc) const = 0;

    /** Virtual destructor, because we have virtual methods. */
    virtual ~KeyMaker();
};

/** KeyMaker subclass which combines several values.
 *
 *  When the result is used for sorting, results are ordered by the first
 *  value.  In the event of a tie, the second is used.  If this is the same for
 *  both, the third is used, and so on.  If @a reverse is true for a value,
 *  then the sort order for that value is reversed.
 *
 *  When used for collapsing, the documents will only be considered equal if
 *  all the values specified match.  If none of the specified values are set
 *  then the generated key will be empty, so such documents won't be collapsed
 *  (which is consistent with the behaviour in the "collapse on a value" case).
 *  If you'd prefer that documents with none of the keys set are collapsed
 *  together, then you can set @a reverse for at least one of the values.
 *  Other than this, it isn't useful to set @a reverse for collapsing.
 */
class XAPIAN_VISIBILITY_DEFAULT MultiValueKeyMaker : public KeyMaker {
    std::vector<std::pair<Xapian::valueno, bool> > slots;

  public:
    MultiValueKeyMaker() { }

    template <class Iterator>
    MultiValueKeyMaker(Iterator begin, Iterator end) {
	while (begin != end) add_value(*begin++);
    }

    virtual std::string operator()(const Xapian::Document & doc) const;

    void add_value(Xapian::valueno slot, bool reverse = false) {
	slots.push_back(std::make_pair(slot, reverse));
    }
};

/** Virtual base class for sorter functor. */
class XAPIAN_VISIBILITY_DEFAULT XAPIAN_DEPRECATED_CLASS Sorter : public KeyMaker { };

/** Sorter subclass which sorts by a several values.
 *
 *  Results are ordered by the first value.  In the event of a tie, the
 *  second is used.  If this is the same for both, the third is used, and
 *  so on.
 *
 *  @deprecated This class is deprecated - you should migrate to using
 *  MultiValueKeyMaker instead.  Note that MultiValueSorter::add() becomes
 *  MultiValueKeyMaker::add_value(), but the sense of the direction flag
 *  is reversed (to be consistent with Enquire::set_sort_by_value()), so:
 *
 *    MultiValueSorter sorter;
 *    // Primary ordering is forwards on value 4.
 *    sorter.add(4);
 *    // Secondary ordering is reverse on value 5.
 *    sorter.add(5, false);
 *
 *  becomes:
 *
 *    MultiValueKeyMaker sorter;
 *    // Primary ordering is forwards on value 4.
 *    sorter.add_value(4);
 *    // Secondary ordering is reverse on value 5.
 *    sorter.add_value(5, true);
 */
class XAPIAN_VISIBILITY_DEFAULT XAPIAN_DEPRECATED_CLASS MultiValueSorter : public Sorter {
    std::vector<std::pair<Xapian::valueno, bool> > slots;

  public:
    MultiValueSorter() { }

    template <class Iterator>
    MultiValueSorter(Iterator begin, Iterator end) {
	while (begin != end) add(*begin++);
    }

    virtual std::string operator()(const Xapian::Document & doc) const;

    void add(Xapian::valueno slot, bool forward = true) {
	slots.push_back(std::make_pair(slot, forward));
    }
};

}

#endif // XAPIAN_INCLUDED_KEYMAKER_H
