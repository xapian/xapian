/** @file serialisationcontext.h
 * @brief Context for looking up objects during unserialisation.
 */
/* Copyright 2009 Lemur Consulting Ltd
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

#ifndef XAPIAN_INCLUDED_SERIALISATIONCONTEXT_H
#define XAPIAN_INCLUDED_SERIALISATIONCONTEXT_H

#include <xapian/base.h>
#include <xapian/visibility.h>
#include <string>

namespace Xapian {

// Forward declarations.
class Weight;
class PostingSource;

/** A context for serialisation.
 *
 *  This context is used to look up weighting schemes and posting sources when
 *  unserialising.
 */
class XAPIAN_VISIBILITY_DEFAULT SerialisationContext {
  public:
    /// Class holding details of the context.
    class Internal;

  private:
    /// @internal Reference counted internals.
    Xapian::Internal::RefCntPtr<Internal> internal;

  public:

    /** Copy the context.
     *
     *  The internals are reference counted, so copying is cheap.
     */
    SerialisationContext(const SerialisationContext & other);

    /** Assign to the context - the copy is shallow.
     *
     *  The internals are reference counted, so assignment is cheap.
     */
    SerialisationContext & operator=(const SerialisationContext & other);

    /** Default constructor: makes a context with default settings.
     *
     *  The context will contain all standard weighting schemes and
     *  posting sources.
     */
    SerialisationContext();

    ~SerialisationContext();

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
};

}

#endif /* XAPIAN_INCLUDED_SERIALISATIONCONTEXT_H */
