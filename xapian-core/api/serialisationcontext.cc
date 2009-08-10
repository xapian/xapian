/** @file serialisationcontext.cc
 * @brief Context for looking up objects during unserialisation.
 */
/* Copyright (C) 2006,2007,2008,2009 Olly Betts
 * Copyright (C) 2006,2007,2009 Lemur Consulting Ltd
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

#include <config.h>

#include "xapian/serialisationcontext.h"

#include "xapian/error.h"
#include "xapian/matchspy.h"
#include "xapian/postingsource.h"
#include "xapian/weight.h"

#include "serialisationcontextinternal.h"
#include "omdebug.h"

#include <algorithm>
#include <map>

using namespace std;

template<class T>
static inline void
register_object(map<string, T*> & registry, const T & obj)
{
    string name = obj.name();
    if (rare(name.empty())) {
	throw Xapian::InvalidOperationError("Unable to register object - name() method returned empty string");
    }

    pair<typename map<string, T *>::iterator, bool> r;
    r = registry.insert(make_pair(name, static_cast<T*>(NULL)));
    if (!r.second) {
	// Existing element with this key, so replace the pointer with NULL
	// and delete the existing pointer.
	//
	// If the delete throws, this will leave a NULL entry in the map, but
	// that won't affect behaviour as we return NULL for "not found"
	// anyway.  The memory used will be leaked if the dtor throws, but
	// throwing exceptions from the dtor is bad form, so that's not a big
	// problem.
	T * p = NULL;
	swap(p, r.first->second);
	delete p;
    }

    T * clone = obj.clone();
    if (rare(!clone)) {
	throw Xapian::InvalidOperationError("Unable to register object - clone() method returned NULL");
    }

    r.first->second = clone;
}
 
template<class T>
static inline const T *
lookup_object(map<string, T*> registry, const string & name)
{
    typename map<string, T*>::const_iterator i = registry.find(name);
    if (i == registry.end()) {
	return NULL;
    }
    return i->second;
}

namespace Xapian {

SerialisationContext::SerialisationContext(const SerialisationContext & other)
	: internal(other.internal)
{
    LOGCALL_CTOR(API, "Xapian::SerialisationContext::SerialisationContext", "other");
}

SerialisationContext &
SerialisationContext::operator=(const SerialisationContext & other)
{
    LOGCALL(API, Xapian::SerialisationContext &, "Xapian::SerialisationContext::operator=", "other");
    internal = other.internal;
    return(*this);
}

SerialisationContext::SerialisationContext()
	: internal(new SerialisationContext::Internal())
{
    LOGCALL_CTOR(API, "Xapian::SerialisationContext::SerialisationContext", "");
}

SerialisationContext::~SerialisationContext()
{
    LOGCALL_DTOR(API, "Xapian::SerialisationContext::~SerialisationContext");

    // Note - we don't need to do anything special in this destructor, but it
    // does need to be explicitly defined because the definition of the
    // internals is not visible externally, which results in an error if the
    // compiler tries to generate a default destructor.
}

void
SerialisationContext::register_weighting_scheme(const Xapian::Weight &wt)
{
    LOGCALL_VOID(API, "Xapian::SerialisationContext::register_weighting_scheme", wt.name());
    register_object(internal->wtschemes, wt);
}

const Xapian::Weight *
SerialisationContext::get_weighting_scheme(const string & name) const
{
    LOGCALL(API, const Xapian::Weight *, "Xapian::SerialisationContext::get_weighting_scheme", name);
    RETURN(lookup_object(internal->wtschemes, name));
}

void
SerialisationContext::register_posting_source(const Xapian::PostingSource &source)
{
    LOGCALL_VOID(API, "Xapian::SerialisationContext::register_posting_source", source.name());
    register_object(internal->postingsources, source);
}

const Xapian::PostingSource *
SerialisationContext::get_posting_source(const string & name) const
{
    LOGCALL(API, const Xapian::PostingSource *, "Xapian::SerialisationContext::get_posting_source", name);
    RETURN(lookup_object(internal->postingsources, name));
}

void
SerialisationContext::register_match_spy(const Xapian::MatchSpy &spy)
{
    LOGCALL_VOID(API, "Xapian::SerialisationContext::register_match_spy", spy.name());
    register_object(internal->matchspies, spy);
}

const Xapian::MatchSpy *
SerialisationContext::get_match_spy(const string & name) const
{
    LOGCALL(API, const Xapian::MatchSpy *, "Xapian::SerialisationContext::get_match_spy", name);
    RETURN(lookup_object(internal->matchspies, name));
}


SerialisationContext::Internal::Internal()
	: Xapian::Internal::RefCntBase(),
          wtschemes(),
	  postingsources()
{
    add_defaults();
}

SerialisationContext::Internal::~Internal()
{
    clear_weighting_schemes();
    clear_posting_sources();
    clear_match_spies();
}

void
SerialisationContext::Internal::add_defaults()
{
    Xapian::Weight * weight;
    weight = new Xapian::BM25Weight;
    wtschemes[weight->name()] = weight;
    weight = new Xapian::BoolWeight;
    wtschemes[weight->name()] = weight;
    weight = new Xapian::TradWeight;
    wtschemes[weight->name()] = weight;

    Xapian::PostingSource * source;
    source = new Xapian::ValueWeightPostingSource(0);
    postingsources[source->name()] = source;
    source = new Xapian::DecreasingValueWeightPostingSource(0);
    postingsources[source->name()] = source;
    source = new Xapian::ValueMapPostingSource(0);
    postingsources[source->name()] = source;
    source = new Xapian::FixedWeightPostingSource(0.0);
    postingsources[source->name()] = source;

    Xapian::MatchSpy * spy;
    spy = new Xapian::ValueCountMatchSpy();
    matchspies[spy->name()] = spy;
}

void
SerialisationContext::Internal::clear_weighting_schemes()
{
    map<string, Xapian::Weight*>::const_iterator i;
    for (i = wtschemes.begin(); i != wtschemes.end(); ++i) {
	delete i->second;
    }
}

void
SerialisationContext::Internal::clear_posting_sources()
{
    map<string, Xapian::PostingSource *>::const_iterator i;
    for (i = postingsources.begin(); i != postingsources.end(); ++i) {
	delete i->second;
    }
}

void
SerialisationContext::Internal::clear_match_spies()
{
    map<string, Xapian::MatchSpy *>::const_iterator i;
    for (i = matchspies.begin(); i != matchspies.end(); ++i) {
	delete i->second;
    }
}

}
