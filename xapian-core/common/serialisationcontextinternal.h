/** @file serialisationcontextinternal.h
 * @brief Internals of SerialisationContext object.
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

#ifndef XAPIAN_INCLUDED_SERIALISATIONCONTEXTINTERNAL_H
#define XAPIAN_INCLUDED_SERIALISATIONCONTEXTINTERNAL_H

#include "xapian/base.h"
#include "xapian/serialisationcontext.h"

#include <map>
#include <string>

namespace Xapian {
    class Weight;
    class PostingSource;
    class MatchSpy;
}

class Xapian::SerialisationContext::Internal
	: public Xapian::Internal::RefCntBase {
    /// Registered weighting schemes.
    std::map<std::string, Xapian::Weight *> wtschemes;

    /// Registered external posting sources.
    std::map<std::string, Xapian::PostingSource *> postingsources;

    /// Registered match spies.
    std::map<std::string, Xapian::MatchSpy *> matchspies;

    /// Add the standard default weighting schemes and posting sources.
    void add_defaults();

    /// Clear all registered weighting schemes from the context.
    void clear_weighting_schemes();

    /// Clear all registered posting sources from the context.
    void clear_posting_sources();

    /// Clear all registered match spies from the context.
    void clear_match_spies();

  public:
    Internal();
    ~Internal();

    /// Register a weighting scheme with the context.
    void register_weighting_scheme(const Xapian::Weight &wt);

    /** Get a weighting scheme given a name.
     *
     *  The returned weighting scheme is owned by the context object.
     *
     *  Returns NULL if the weighting scheme could not be found.
     */
    const Xapian::Weight *
	    get_weighting_scheme(const std::string & name) const;

    /// Register a user-defined posting source class.
    void register_posting_source(const Xapian::PostingSource &source);

    /** Get a posting source given a name.
     *
     *  The returned posting source is owned by the context object.
     *
     *  Returns NULL if the posting source could not be found.
     */
    const Xapian::PostingSource *
	    get_posting_source(const std::string & name) const;

    /// Register a user-defined match spy class.
    void register_match_spy(const Xapian::MatchSpy &spy);

    /** Get a match spy given a name.
     *
     *  The returned match spy is owned by the context object.
     *
     *  Returns NULL if the match spy could not be found.
     */
    const Xapian::MatchSpy *
	    get_match_spy(const std::string & name) const;

};

#endif // XAPIAN_INCLUDED_SERIALISATIONCONTEXTINTERNAL_H
