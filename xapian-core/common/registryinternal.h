/** @file registryinternal.h
 * @brief Internals of Xapian::Registry object.
 */
/* Copyright 2009 Lemur Consulting Ltd
 * Copyright 2009 Olly Betts
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

#ifndef XAPIAN_INCLUDED_REGISTRYINTERNAL_H
#define XAPIAN_INCLUDED_REGISTRYINTERNAL_H

#include "xapian/base.h"
#include "xapian/registry.h"

#include <map>
#include <string>

namespace Xapian {
    class Weight;
    class PostingSource;
    class MatchSpy;
}

class Xapian::Registry::Internal : public Xapian::Internal::RefCntBase {
    friend class Xapian::Registry;

    /// Registered weighting schemes.
    std::map<std::string, Xapian::Weight *> wtschemes;

    /// Registered external posting sources.
    std::map<std::string, Xapian::PostingSource *> postingsources;

    /// Registered match spies.
    std::map<std::string, Xapian::MatchSpy *> matchspies;

    /// Add the standard subclasses provided in the API.
    void add_defaults();

    /// Clear all registered weighting schemes.
    void clear_weighting_schemes();

    /// Clear all registered posting sources.
    void clear_posting_sources();

    /// Clear all registered match spies.
    void clear_match_spies();

  public:
    Internal();
    ~Internal();
};

#endif // XAPIAN_INCLUDED_REGISTRYINTERNAL_H
