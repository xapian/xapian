/* omdocument.cc: class for performing a match
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2003,2004,2006,2007,2008,2009,2011,2013,2014 Olly Betts
 * Copyright 2009 Lemur Consulting Ltd
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

#include <config.h>

#include <xapian/document.h>

#include "backends/documentinternal.h"
#include "debuglog.h"
#include "documentvaluelist.h"
#include "net/serialise.h"
#include "str.h"

#include <xapian/error.h>
#include <xapian/types.h>
#include <xapian/valueiterator.h>

#include <algorithm>
#include <string>

using namespace std;

namespace Xapian {

// implementation of Document

Document::Document(Document::Internal *internal_) : internal(internal_)
{
}

Document::Document() : internal(new Xapian::Document::Internal)
{
}

string
Document::get_value(Xapian::valueno slot) const
{
    LOGCALL(API, string, "Document::get_value", slot);
    RETURN(internal->get_value(slot));
}

string
Document::get_data() const
{
    LOGCALL(API, string, "Document::get_data", NO_ARGS);
    RETURN(internal->get_data());
}

void
Document::set_data(const string &data)
{
    LOGCALL_VOID(API, "Document::set_data", data);
    internal->set_data(data);
}

void
Document::operator=(const Document &other)
{
    // pointers are reference counted.
    internal = other.internal;
}

Document::Document(const Document &other)
	: internal(other.internal)
{
}

Document::~Document()
{
}

string
Document::get_description() const
{
    return internal->get_description();
}

void
Document::add_value(Xapian::valueno slot, const string &value)
{
    LOGCALL_VOID(API, "Document::add_value", slot | value);
    internal->add_value(slot, value);
}

void
Document::clear_values()
{
    LOGCALL_VOID(API, "Document::clear_values", NO_ARGS);
    internal->clear_values();
}

void
Document::add_posting(const string & tname,
			Xapian::termpos tpos,
			Xapian::termcount wdfinc)
{
    LOGCALL_VOID(API, "Document::add_posting", tname | tpos | wdfinc);
    if (tname.empty()) {
	throw InvalidArgumentError("Empty termnames aren't allowed.");
    }
    internal->add_posting(tname, tpos, wdfinc);
}

void
Document::add_term(const string & tname, Xapian::termcount wdfinc)
{
    LOGCALL_VOID(API, "Document::add_term", tname | wdfinc);
    if (tname.empty()) {
	throw InvalidArgumentError("Empty termnames aren't allowed.");
    }
    internal->add_term(tname, wdfinc);
}

void
Document::remove_posting(const string & tname, Xapian::termpos tpos,
			 Xapian::termcount wdfdec)
{
    LOGCALL_VOID(API, "Document::remove_posting", tname | tpos | wdfdec);
    if (tname.empty()) {
	throw InvalidArgumentError("Empty termnames aren't allowed.");
    }
    auto res = internal->remove_posting(tname, tpos, wdfdec);
    if (res != Document::Internal::OK) {
	string desc = "Document::remove_posting() failed - term '";
	desc += tname;
	if (res == Document::Internal::NO_TERM) {
	    desc += "' not present";
	} else {
	    desc += "' not present at position ";
	    desc += str(tpos);
	}
	throw Xapian::InvalidArgumentError(desc);
    }
}

void
Document::remove_term(const string & tname)
{
    LOGCALL_VOID(API, "Document::remove_term", tname);
    if (!internal->remove_term(tname)) {
	throw Xapian::InvalidArgumentError("Term '" + tname +
		"' is not present in document, in "
		"Xapian::Document::remove_term()");
    }
}

void
Document::clear_terms()
{
    LOGCALL_VOID(API, "Document::clear_terms", NO_ARGS);
    internal->clear_terms();
}

Xapian::termcount
Document::termlist_count() const {
    LOGCALL(API, Xapian::termcount, "Document::termlist_count", NO_ARGS);
    RETURN(internal->termlist_count());
}

TermIterator
Document::termlist_begin() const
{
    LOGCALL(API, TermIterator, "Document::termlist_begin", NO_ARGS);
    RETURN(TermIterator(internal->open_term_list()));
}

Xapian::termcount
Document::values_count() const {
    LOGCALL(API, Xapian::termcount, "Document::values_count", NO_ARGS);
    RETURN(internal->values_count());
}

ValueIterator
Document::values_begin() const
{
    LOGCALL(API, ValueIterator, "Document::values_begin", NO_ARGS);
    // Calling values_count() has the side effect of making sure that they have
    // been read into the std::map "values" member of internal.
    if (internal->values_count() == 0) RETURN(ValueIterator());
    RETURN(ValueIterator(new DocumentValueList(internal)));
}

docid
Document::get_docid() const
{
    LOGCALL(API, docid, "Document::get_docid", NO_ARGS);
    RETURN(internal->get_docid());
}

std::string
Document::serialise() const
{
    LOGCALL(API, std::string, "Document::serialise", NO_ARGS);
    RETURN(serialise_document(*this));
}

Document
Document::unserialise(const std::string &s)
{
    LOGCALL_STATIC(API, Document, "Document::unserialise", s);
    RETURN(unserialise_document(s));
}

}
