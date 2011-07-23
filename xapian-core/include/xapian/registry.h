/** @file registry.h
 * @brief Class for looking up user subclasses during unserialisation.
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301
 * USA
 */

#ifndef XAPIAN_INCLUDED_REGISTRY_H
#define XAPIAN_INCLUDED_REGISTRY_H

#include <xapian/intrusive_ptr.h>
#include <xapian/visibility.h>
#include <string>

namespace Xapian {

// Forward declarations.
class MatchSpy;
class PostingSource;
class Weight;

/** Registry for user subclasses.
 *
 *  This class provides a way for the remote server to look up user subclasses
 *  when unserialising.
 */
class XAPIAN_VISIBILITY_DEFAULT Registry {
  public:
    /// Class holding details of the registry.
    class Internal;

  private:
    /// @internal Reference counted internals.
    Xapian::Internal::intrusive_ptr<Internal> internal;

  public:
    /** Copy constructor.
     *
     *  The internals are reference counted, so copying is cheap.
     */
    Registry(const Registry & other);

    /** Assignment operator.
     *
     *  The internals are reference counted, so assignment is cheap.
     */
    Registry & operator=(const Registry & other);

    /** Default constructor.
     *
     *  The registry will contain all standard subclasses of user-subclassable
     *  classes.
     */
    Registry();

    ~Registry();

    /// Register a weighting scheme.
    void register_weighting_scheme(const Xapian::Weight &wt);

    /** Get the weighting scheme given a name.
     *
     *  The returned weighting scheme is owned by the registry object.
     *
     *  Returns NULL if the weighting scheme could not be found.
     */
    const Xapian::Weight *
	    get_weighting_scheme(const std::string & name) const;

    /// Register a user-defined posting source class.
    void register_posting_source(const Xapian::PostingSource &source);

    /** Get a posting source given a name.
     *
     *  The returned posting source is owned by the registry object.
     *
     *  Returns NULL if the posting source could not be found.
     */
    const Xapian::PostingSource *
	    get_posting_source(const std::string & name) const;

    /// Register a user-defined match spy class.
    void register_match_spy(const Xapian::MatchSpy &spy);

    /** Get a match spy given a name.
     *
     *  The returned match spy is owned by the registry object.
     *
     *  Returns NULL if the match spy could not be found.
     */
    const Xapian::MatchSpy *
	    get_match_spy(const std::string & name) const;
};

}

#endif /* XAPIAN_INCLUDED_REGISTRY_H */
