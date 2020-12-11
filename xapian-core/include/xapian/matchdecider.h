/** @file
 * @brief Abstract base class for match deciders
 */
/* Copyright (C) 2017 Olly Betts
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

#ifndef XAPIAN_INCLUDED_MATCHDECIDER_H
#define XAPIAN_INCLUDED_MATCHDECIDER_H

#if !defined XAPIAN_IN_XAPIAN_H && !defined XAPIAN_LIB_BUILD
# error Never use <xapian/matchdecider.h> directly; include <xapian.h> instead.
#endif

#include <xapian/attributes.h>
#include <xapian/types.h>
#include <xapian/visibility.h>

namespace Xapian {

class Document;

/// Abstract base class for match deciders.
class XAPIAN_VISIBILITY_DEFAULT MatchDecider {
  private:
    /// Don't allow assignment.
    void operator=(const MatchDecider &) = delete;

    /// Don't allow copying.
    MatchDecider(const MatchDecider &) = delete;

  public:
    /// Default constructor, needed by subclass constructors.
    MatchDecider() noexcept {}

    /** Virtual destructor, because we have virtual methods. */
    virtual ~MatchDecider() { }

    /** Decide whether to accept a document.
     *
     *  This is called by the matcher for documents before adding them to the
     *  candidate result set.  Note that documents accepted here may still not
     *  appear in the final MSet (better documents may be found, for example).
     *
     *  @param doc The document to consider.
     *
     *  @return true if the document should be considered further.
     */
    virtual bool operator()(const Xapian::Document &doc) const = 0;

    /** @private @internal Count of documents accepted by this object.
     *
     *  Used to return stats to the matcher.
     */
    mutable Xapian::doccount docs_allowed_;

    /** @private @internal Count of documents rejected by this object.
     *
     *  Used to return stats to the matcher.
     */
    mutable Xapian::doccount docs_denied_;
};

}

#endif // XAPIAN_INCLUDED_MATCHDECIDER_H
