/** @file postingsource.h
 *  @brief PostingSource class
 */
/* Copyright (C) 2008 Olly Betts
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

#ifndef XAPIAN_INCLUDED_POSTINGSOURCE_H
#define XAPIAN_INCLUDED_POSTINGSOURCE_H

#include <xapian/types.h>
#include <xapian/visibility.h>

#include <string>

namespace Xapian {

/// Base class which provides an "external" source of postings.
class XAPIAN_VISIBILITY_DEFAULT PostingSource {
  public:
    virtual ~PostingSource();

    virtual Xapian::doccount get_termfreq_min() const = 0;

    virtual Xapian::doccount get_termfreq_est() const = 0;

    virtual Xapian::doccount get_termfreq_max() const = 0;

    /**
     *  This default implementation always returns 0, for convenience when
     *  implementing "weight-less" PostingSource subclasses.
     */
    virtual Xapian::weight get_maxweight() const;

    /**
     *  This default implementation always returns 0, for convenience when
     *  implementing "weight-less" PostingSource subclasses.
     */
    virtual Xapian::weight get_weight() const;

    virtual void next(Xapian::weight) = 0;

    /**
     *  This default implementation calls next() repeatedly.
     */
    virtual void skip_to(Xapian::docid, Xapian::weight);

    /**
     *  This default implementation calls skip_to() and always sets valid to
     *  true.
     */
    virtual void check(Xapian::docid, Xapian::weight, bool&);

    virtual bool at_end() const = 0;

    virtual Xapian::docid get_docid() const = 0;

    virtual std::string get_description() const = 0;
};

}

#endif // XAPIAN_INCLUDED_POSTINGSOURCE_H
