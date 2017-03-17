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

#include "backends/document.h"
#include "documentvaluelist.h"
#include "maptermlist.h"
#include "net/serialise.h"
#include "str.h"
#include "unicode/description_append.h"

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
Document::remove_value(Xapian::valueno slot)
{
    LOGCALL_VOID(API, "Document::remove_value", slot);
    internal->remove_value(slot);
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
    internal->remove_posting(tname, tpos, wdfdec);
}

void
Document::remove_term(const string & tname)
{
    LOGCALL_VOID(API, "Document::remove_term", tname);
    internal->remove_term(tname);
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

/////////////////////////////////////////////////////////////////////////////

void
OmDocumentTerm::add_position(Xapian::termpos tpos)
{
    LOGCALL_VOID(DB, "OmDocumentTerm::add_position", tpos);

    // We generally expect term positions to be added in approximately
    // increasing order, so check the end first
    if (positions.empty() || tpos > positions.back()) {
	positions.push_back(tpos);
	return;
    }

    // Search for the position the term occurs at.  Use binary chop to
    // search, since this is a sorted list.
    vector<Xapian::termpos>::iterator i;
    i = lower_bound(positions.begin(), positions.end(), tpos);
    if (i == positions.end() || *i != tpos) {
	positions.insert(i, tpos);
    }
}

void
OmDocumentTerm::remove_position(Xapian::termpos tpos)
{
    LOGCALL_VOID(DB, "OmDocumentTerm::remove_position", tpos);

    // Search for the position the term occurs at.  Use binary chop to
    // search, since this is a sorted list.
    vector<Xapian::termpos>::iterator i;
    i = lower_bound(positions.begin(), positions.end(), tpos);
    if (i == positions.end() || *i != tpos) {
	throw Xapian::InvalidArgumentError("Position " + str(tpos) +
				     " not in list, can't remove");
    }
    positions.erase(i);
}

string
OmDocumentTerm::get_description() const
{
    string description;
    description = "OmDocumentTerm(wdf = ";
    description += str(wdf);
    description += ", positions[";
    description += str(positions.size());
    description += "])";
    return description;
}

string
Xapian::Document::Internal::get_value(Xapian::valueno slot) const
{
    if (values_here) {
	map<Xapian::valueno, string>::const_iterator i;
	i = values.find(slot);
	if (i == values.end()) return string();
	return i->second;
    }
    if (!database.get()) return string();
    return do_get_value(slot);
}

string
Xapian::Document::Internal::get_data() const
{
    LOGCALL(DB, string, "Xapian::Document::Internal::get_data", NO_ARGS);
    if (data_here) RETURN(data);
    if (!database.get()) RETURN(string());
    RETURN(do_get_data());
}

void
Xapian::Document::Internal::set_data(const string &data_)
{
    data = data_;
    data_here = true;
}

TermList *
Xapian::Document::Internal::open_term_list() const
{
    LOGCALL(DB, TermList *, "Document::Internal::open_term_list", NO_ARGS);
    if (terms_here) {
	RETURN(new MapTermList(terms.begin(), terms.end()));
    }
    if (!database.get()) RETURN(NULL);
    RETURN(database->open_term_list(did));
}

void
Xapian::Document::Internal::add_value(Xapian::valueno slot, const string &value)
{
    need_values();
    if (!value.empty()) {
	values[slot] = value;
    } else {
	// Empty values aren't stored, but replace any existing value by
	// removing it.
	values.erase(slot);
    }
}

void
Xapian::Document::Internal::remove_value(Xapian::valueno slot)
{
    need_values();
    map<Xapian::valueno, string>::iterator i = values.find(slot);
    if (i == values.end()) {
	throw Xapian::InvalidArgumentError("Value #" + str(slot) +
		" is not present in document, in "
		"Xapian::Document::Internal::remove_value()");
    }
    values.erase(i);
}

void
Xapian::Document::Internal::clear_values()
{
    values.clear();
    values_here = true;
}

void
Xapian::Document::Internal::add_posting(const string & tname, Xapian::termpos tpos,
			      Xapian::termcount wdfinc)
{
    need_terms();
    positions_modified = true;

    map<string, OmDocumentTerm>::iterator i;
    i = terms.find(tname);
    if (i == terms.end()) {
	OmDocumentTerm newterm(wdfinc);
	newterm.add_position(tpos);
	terms.insert(make_pair(tname, newterm));
    } else {
	i->second.add_position(tpos);
	if (wdfinc) i->second.inc_wdf(wdfinc);
    }
}

void
Xapian::Document::Internal::add_term(const string & tname, Xapian::termcount wdfinc)
{
    need_terms();

    map<string, OmDocumentTerm>::iterator i;
    i = terms.find(tname);
    if (i == terms.end()) {
	OmDocumentTerm newterm(wdfinc);
	terms.insert(make_pair(tname, newterm));
    } else {
	if (wdfinc) i->second.inc_wdf(wdfinc);
    }
}

void
Xapian::Document::Internal::remove_posting(const string & tname,
					   Xapian::termpos tpos,
					   Xapian::termcount wdfdec)
{
    need_terms();

    map<string, OmDocumentTerm>::iterator i;
    i = terms.find(tname);
    if (i == terms.end()) {
	throw Xapian::InvalidArgumentError("Term '" + tname +
		"' is not present in document, in "
		"Xapian::Document::Internal::remove_posting()");
    }
    i->second.remove_position(tpos);
    if (wdfdec) i->second.dec_wdf(wdfdec);
    positions_modified = true;
}

void
Xapian::Document::Internal::remove_term(const string & tname)
{
    need_terms();
    map<string, OmDocumentTerm>::iterator i;
    i = terms.find(tname);
    if (i == terms.end()) {
	throw Xapian::InvalidArgumentError("Term '" + tname +
		"' is not present in document, in "
		"Xapian::Document::Internal::remove_term()");
    }
    positions_modified = !i->second.positions.empty();
    terms.erase(i);
}

void
Xapian::Document::Internal::clear_terms()
{
    terms.clear();
    terms_here = true;
    // Assume there was a term with positions for now.
    // FIXME: may be worth checking...
    positions_modified = true;
}

Xapian::termcount
Xapian::Document::Internal::termlist_count() const
{
    if (!terms_here) {
	// How equivalent is this line to the rest?
	// return database.get() ? database->open_term_list(did)->get_approx_size() : 0;
	need_terms();
    }
    Assert(terms_here);
    return terms.size();
}

void
Xapian::Document::Internal::need_terms() const
{
    if (terms_here) return;
    if (database.get()) {
	Xapian::TermIterator t(database->open_term_list(did));
	Xapian::TermIterator tend(NULL);
	for ( ; t != tend; ++t) {
	    Xapian::PositionIterator p = t.positionlist_begin();
	    OmDocumentTerm term(t.get_wdf());
	    for ( ; p != t.positionlist_end(); ++p) {
		term.add_position(*p);
	    }
	    terms.insert(make_pair(*t, term));
	}
    }
    terms_here = true;
}

Xapian::valueno
Xapian::Document::Internal::values_count() const
{
    LOGCALL(DB, Xapian::valueno, "Document::Internal::values_count", NO_ARGS);
    need_values();
    Assert(values_here);
    RETURN(values.size());
}

string
Xapian::Document::Internal::get_description() const
{
    string desc = "Document(";

    // description_append ?
    if (data_here) {
	desc += "data='";
	description_append(desc, data);
	desc += "'";
    }

    if (values_here) {
	if (data_here) desc += ", ";
	desc += "values[";
	desc += str(values.size());
	desc += ']';
    }

    if (terms_here) {
	if (data_here || values_here) desc += ", ";
	desc += "terms[";
	desc += str(terms.size());
	desc += ']';
    }

    if (database.get()) {
	if (data_here || values_here || terms_here) desc += ", ";
	// database->get_description() if/when that returns a non-generic
	// result.
	desc += "db:yes";
    }

    desc += ')';

    return desc;
}

void
Xapian::Document::Internal::need_values() const
{
    if (!values_here) {
	if (database.get()) {
	    Assert(values.empty());
	    do_get_all_values(values);
	}
	values_here = true;
    }
}

Xapian::Document::Internal::~Internal()
{
    if (database.get())
	database->invalidate_doc_object(this);
}
