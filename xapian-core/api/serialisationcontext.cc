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

#include "xapian/enquire.h"
#include "xapian/error.h"
#include "xapian/postingsource.h"
#include "serialisationcontextinternal.h"
#include "omdebug.h"

using namespace std;

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
    internal->register_weighting_scheme(wt);
}

const Xapian::Weight *
SerialisationContext::get_weighting_scheme(const string & name) const
{
    LOGCALL(API, const Xapian::Weight *, "Xapian::SerialisationContext::get_weighting_scheme", name);
    RETURN(internal->get_weighting_scheme(name));
}


void
SerialisationContext::register_posting_source(const Xapian::PostingSource &source)
{
    LOGCALL_VOID(API, "Xapian::SerialisationContext::register_posting_source", source.name());
    internal->register_posting_source(source);
}

const Xapian::PostingSource *
SerialisationContext::get_posting_source(const string & name) const
{
    LOGCALL(API, const Xapian::PostingSource *, "Xapian::SerialisationContext::get_posting_source", name);
    RETURN(internal->get_posting_source(name));
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
    source = new Xapian::ValueMapPostingSource(0);
    postingsources[source->name()] = source;
    source = new Xapian::FixedWeightPostingSource(0.0);
    postingsources[source->name()] = source;
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
SerialisationContext::Internal::register_weighting_scheme(const Xapian::Weight &wt)
{
    string wtname = wt.name();

    map<string, Xapian::Weight *>::const_iterator i;
    i = wtschemes.find(wtname);
    if (i != wtschemes.end()) {
	delete i->second;
    }

    Xapian::Weight * wtclone = NULL;
    try {
	wtclone = wt.clone();
	wtschemes[wtname] = wtclone; 
    } catch(...) {
	delete wtclone;
	wtschemes.erase(wtname);
	throw;
    }
}

const Xapian::Weight *
SerialisationContext::Internal::get_weighting_scheme(const string & name) const
{
    map<string, Xapian::Weight *>::const_iterator i;
    i = wtschemes.find(name);
    if (i == wtschemes.end()) {
	return NULL;
    }
    return i->second;
}

void
SerialisationContext::Internal::register_posting_source(const Xapian::PostingSource &source)
{
    string sourcename = source.name();
    if (sourcename.empty()) {
        throw Xapian::InvalidOperationError("Unable to register posting source - name() method returns empty string.");
    }

    map<string, Xapian::PostingSource *>::const_iterator i;
    i = postingsources.find(sourcename);
    if (i != postingsources.end()) {
	delete i->second;
    }

    Xapian::PostingSource * sourceclone = source.clone();
    if (!sourceclone) {
	postingsources.erase(sourcename);
        throw Xapian::InvalidOperationError("Unable to register posting source - clone() method returns NULL.");
    }
    try {
	postingsources[sourcename] = sourceclone;
    } catch(...) {
	delete sourceclone;
	postingsources.erase(sourcename);
	throw;
    }
}

const Xapian::PostingSource *
SerialisationContext::Internal::get_posting_source(const string & name) const
{
    map<string, Xapian::PostingSource *>::const_iterator i;
    i = postingsources.find(name);
    if (i == postingsources.end()) {
	return NULL;
    }
    return i->second;
}

}
