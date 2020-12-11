/** @file
 * @brief Class for looking up user subclasses during unserialisation.
 */
/* Copyright (C) 2006,2007,2008,2009,2010,2016 Olly Betts
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

#include "xapian/registry.h"

#include "xapian/error.h"
#include "xapian/geospatial.h"
#include "xapian/intrusive_ptr.h"
#include "xapian/matchspy.h"
#include "xapian/postingsource.h"
#include "xapian/weight.h"

#include "debuglog.h"

#include <algorithm>
#include <map>
#include <string>

using namespace std;

class Xapian::Registry::Internal : public Xapian::Internal::intrusive_base {
    friend class Xapian::Registry;

    /// Registered weighting schemes.
    std::map<std::string, Xapian::Weight *> wtschemes;

    /// Registered external posting sources.
    std::map<std::string, Xapian::PostingSource *> postingsources;

    /// Registered match spies.
    std::map<std::string, Xapian::MatchSpy *> matchspies;

    /// Registered lat-long metrics.
    std::map<std::string, Xapian::LatLongMetric *> lat_long_metrics;

    /// Add the standard subclasses provided in the API.
    void add_defaults();

    /// Clear all registered weighting schemes.
    void clear_weighting_schemes();

    /// Clear all registered posting sources.
    void clear_posting_sources();

    /// Clear all registered match spies.
    void clear_match_spies();

    /// Clear all registered lat-long metrics.
    void clear_lat_long_metrics();

  public:
    Internal();
    ~Internal();
};

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

Registry::Internal::Internal()
{
    add_defaults();
}

Registry::Internal::~Internal()
{
    clear_weighting_schemes();
    clear_posting_sources();
    clear_match_spies();
    clear_lat_long_metrics();
}

void
Registry::Internal::add_defaults()
{
    Xapian::Weight * weighting_scheme;
    weighting_scheme = new Xapian::BB2Weight;
    wtschemes[weighting_scheme->name()] = weighting_scheme;
    weighting_scheme = new Xapian::BM25Weight;
    wtschemes[weighting_scheme->name()] = weighting_scheme;
    weighting_scheme = new Xapian::BM25PlusWeight;
    wtschemes[weighting_scheme->name()] = weighting_scheme;
    weighting_scheme = new Xapian::BoolWeight;
    wtschemes[weighting_scheme->name()] = weighting_scheme;
    weighting_scheme = new Xapian::CoordWeight;
    wtschemes[weighting_scheme->name()] = weighting_scheme;
    weighting_scheme = new Xapian::TradWeight;
    wtschemes[weighting_scheme->name()] = weighting_scheme;
    weighting_scheme = new Xapian::TfIdfWeight;
    wtschemes[weighting_scheme->name()] = weighting_scheme;
    weighting_scheme = new Xapian::InL2Weight;
    wtschemes[weighting_scheme->name()] = weighting_scheme;
    weighting_scheme = new Xapian::IfB2Weight;
    wtschemes[weighting_scheme->name()] = weighting_scheme;
    weighting_scheme = new Xapian::IneB2Weight;
    wtschemes[weighting_scheme->name()] = weighting_scheme;
    weighting_scheme = new Xapian::DLHWeight;
    wtschemes[weighting_scheme->name()] = weighting_scheme;
    weighting_scheme = new Xapian::PL2PlusWeight;
    wtschemes[weighting_scheme->name()] = weighting_scheme;
    weighting_scheme = new Xapian::PL2Weight;
    wtschemes[weighting_scheme->name()] = weighting_scheme;
    weighting_scheme = new Xapian::DPHWeight;
    wtschemes[weighting_scheme->name()] = weighting_scheme;
    weighting_scheme = new Xapian::LMWeight;
    wtschemes[weighting_scheme->name()] = weighting_scheme;

    Xapian::PostingSource * source;
    source = new Xapian::ValueWeightPostingSource(0);
    postingsources[source->name()] = source;
    source = new Xapian::DecreasingValueWeightPostingSource(0);
    postingsources[source->name()] = source;
    source = new Xapian::ValueMapPostingSource(0);
    postingsources[source->name()] = source;
    source = new Xapian::FixedWeightPostingSource(0.0);
    postingsources[source->name()] = source;
    source = new Xapian::LatLongDistancePostingSource(0,
	Xapian::LatLongCoords(),
	Xapian::GreatCircleMetric());
    postingsources[source->name()] = source;

    Xapian::MatchSpy * spy;
    spy = new Xapian::ValueCountMatchSpy();
    matchspies[spy->name()] = spy;

    Xapian::LatLongMetric * metric;
    metric = new Xapian::GreatCircleMetric();
    lat_long_metrics[metric->name()] = metric;
}

void
Registry::Internal::clear_weighting_schemes()
{
    map<string, Xapian::Weight*>::const_iterator i;
    for (i = wtschemes.begin(); i != wtschemes.end(); ++i) {
	delete i->second;
    }
}

void
Registry::Internal::clear_posting_sources()
{
    map<string, Xapian::PostingSource *>::const_iterator i;
    for (i = postingsources.begin(); i != postingsources.end(); ++i) {
	delete i->second;
    }
}

void
Registry::Internal::clear_match_spies()
{
    map<string, Xapian::MatchSpy *>::const_iterator i;
    for (i = matchspies.begin(); i != matchspies.end(); ++i) {
	delete i->second;
    }
}

void
Registry::Internal::clear_lat_long_metrics()
{
    map<string, Xapian::LatLongMetric *>::const_iterator i;
    for (i = lat_long_metrics.begin(); i != lat_long_metrics.end(); ++i) {
	delete i->second;
    }
}

Registry::Registry(const Registry & other)
	: internal(other.internal)
{
    LOGCALL_CTOR(API, "Registry", other);
}

Registry &
Registry::operator=(const Registry & other)
{
    LOGCALL(API, Xapian::Registry &, "Xapian::Registry::operator=", other);
    internal = other.internal;
    RETURN(*this);
}

Registry::Registry(Registry &&) = default;

Registry &
Registry::operator=(Registry &&) = default;

Registry::Registry()
	: internal(new Registry::Internal())
{
    LOGCALL_CTOR(API, "Registry", NO_ARGS);
}

Registry::~Registry()
{
    LOGCALL_DTOR(API, "Registry");

    // Note - we don't need to do anything special in this destructor, but it
    // does need to be explicitly defined because the definition of the
    // internals is not visible externally, which results in an error if the
    // compiler tries to generate a default destructor.
}

void
Registry::register_weighting_scheme(const Xapian::Weight &wt)
{
    LOGCALL_VOID(API, "Xapian::Registry::register_weighting_scheme", wt.name());
    register_object(internal->wtschemes, wt);
}

const Xapian::Weight *
Registry::get_weighting_scheme(const string & name) const
{
    LOGCALL(API, const Xapian::Weight *, "Xapian::Registry::get_weighting_scheme", name);
    RETURN(lookup_object(internal->wtschemes, name));
}

void
Registry::register_posting_source(const Xapian::PostingSource &source)
{
    LOGCALL_VOID(API, "Xapian::Registry::register_posting_source", source.name());
    register_object(internal->postingsources, source);
}

const Xapian::PostingSource *
Registry::get_posting_source(const string & name) const
{
    LOGCALL(API, const Xapian::PostingSource *, "Xapian::Registry::get_posting_source", name);
    RETURN(lookup_object(internal->postingsources, name));
}

void
Registry::register_match_spy(const Xapian::MatchSpy &spy)
{
    LOGCALL_VOID(API, "Xapian::Registry::register_match_spy", spy.name());
    register_object(internal->matchspies, spy);
}

const Xapian::MatchSpy *
Registry::get_match_spy(const string & name) const
{
    LOGCALL(API, const Xapian::MatchSpy *, "Xapian::Registry::get_match_spy", name);
    RETURN(lookup_object(internal->matchspies, name));
}

void
Registry::register_lat_long_metric(const Xapian::LatLongMetric &metric)
{
    LOGCALL_VOID(API, "Xapian::Registry::register_lat_long_metric", metric.name());
    register_object(internal->lat_long_metrics, metric);
}

const Xapian::LatLongMetric *
Registry::get_lat_long_metric(const string & name) const
{
    LOGCALL(API, const Xapian::LatLongMetric *, "Xapian::Registry::get_lat_long_metric", name);
    RETURN(lookup_object(internal->lat_long_metrics, name));
}

}
