/* omdocument.cc: class for performing a match
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2003,2004,2006 Olly Betts
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
#include <xapian/types.h>
#include "document.h"
#include "maptermlist.h"
#include <xapian/error.h>
#include <xapian/valueiterator.h>
#include <algorithm>
#include <string>

using namespace std;

namespace Xapian {

// implementation of Document

Document::Document(Document::Internal *internal_) : internal(internal_)
{
}

Document::Document() : internal(new Xapian::Document::Internal())
{
}

string
Document::get_value(Xapian::valueno value) const
{
    DEBUGAPICALL(string, "Document::get_value", value);
    RETURN(internal->get_value(value));
}

string
Document::get_data() const
{
    DEBUGAPICALL(string, "Document::get_data", "");
    RETURN(internal->get_data());
}

void
Document::set_data(const string &data)
{
    DEBUGAPICALL(void, "Document::set_data", data);
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
    return "Document(" + internal->get_description() + ")";
}

void
Document::add_value(Xapian::valueno valueno, const string &value)
{
    DEBUGAPICALL(void, "Document::add_value", valueno << ", " << value);
    internal->add_value(valueno, value);
}

void
Document::remove_value(Xapian::valueno valueno)
{
    DEBUGAPICALL(void, "Document::remove_value", valueno);
    internal->remove_value(valueno);
}

void
Document::clear_values()
{
    DEBUGAPICALL(void, "Document::clear_values", "");
    internal->clear_values();
}

void
Document::add_posting(const string & tname,
			Xapian::termpos tpos,
			Xapian::termcount wdfinc)
{
    DEBUGAPICALL(void, "Document::add_posting",
		 tname << ", " << tpos << ", " << wdfinc);
    if (tname.empty()) {
	throw InvalidArgumentError("Empty termnames aren't allowed.");
    }
    internal->add_posting(tname, tpos, wdfinc);
}

void
Document::add_term(const string & tname, Xapian::termcount wdfinc)
{
    DEBUGAPICALL(void, "Document::add_term", tname << ", " << wdfinc);
    if (tname.empty()) {
	throw InvalidArgumentError("Empty termnames aren't allowed.");
    }
    internal->add_term(tname, wdfinc);
}

void
Document::add_term_nopos(const std::string & term, Xapian::termcount wdfinc)
{
    Document::add_term(term, wdfinc);
}

void
Document::remove_posting(const string & tname, Xapian::termpos tpos,
			 Xapian::termcount wdfdec)
{
    DEBUGAPICALL(void, "Document::remove_posting",
		 tname << ", " << tpos << ", " << wdfdec);
    if (tname.empty()) {
	throw InvalidArgumentError("Empty termnames aren't allowed.");
    }
    internal->remove_posting(tname, tpos, wdfdec);
}

void
Document::remove_term(const string & tname)
{
    DEBUGAPICALL(void, "Document::remove_term", tname);
    internal->remove_term(tname);
}

void
Document::clear_terms()
{
    DEBUGAPICALL(void, "Document::clear_terms", "");
    internal->clear_terms();
}

Xapian::termcount
Document::termlist_count() const {
    DEBUGAPICALL(Xapian::termcount, "Document::termlist_count", "");
    RETURN(internal->termlist_count());
}

TermIterator
Document::termlist_begin() const
{
    DEBUGAPICALL(TermIterator, "Document::termlist_begin", "");
    RETURN(TermIterator(internal->open_term_list()));
}

Xapian::termcount
Document::values_count() const {
    DEBUGAPICALL(Xapian::termcount, "Document::values_count", "");
    RETURN(internal->values_count());
}

ValueIterator
Document::values_begin() const
{
    DEBUGAPICALL(ValueIterator, "Document::values_begin", "");
    RETURN(ValueIterator(0, *this));
}

ValueIterator
Document::values_end() const
{
    DEBUGAPICALL(ValueIterator, "Document::values_end", "");
    RETURN(ValueIterator(internal->values_count(), *this));
}

}

/////////////////////////////////////////////////////////////////////////////

void
OmDocumentTerm::add_position(Xapian::termpos tpos)
{
    DEBUGAPICALL(void, "OmDocumentTerm::add_position", tpos);
    
    // We generally expect term positions to be added in approximately
    // increasing order, so check the end first
    Xapian::termpos last = positions.empty() ? 0 : positions.back();
    if (tpos > last) {
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
    DEBUGAPICALL(void, "OmDocumentTerm::remove_position", tpos);
    
    // Search for the position the term occurs at.  Use binary chop to
    // search, since this is a sorted list.
    vector<Xapian::termpos>::iterator i;
    i = lower_bound(positions.begin(), positions.end(), tpos);
    if (i == positions.end() || *i != tpos) {
	throw Xapian::InvalidArgumentError("Position `" + om_tostring(tpos) +
				     "' not found in list of positions that `" +
				     tname +
				     "' occurs at,"
				     " when removing position from list");
    }
    positions.erase(i);
}

string
OmDocumentTerm::get_description() const
{
    DEBUGCALL(INTRO, string, "OmDocumentTerm::get_description", "");
    string description;

    description = "OmDocumentTerm(" + tname +
	    ", wdf = " + om_tostring(wdf) +
	    ", positions[" + om_tostring(positions.size()) + "]" +
	    ")";
    RETURN(description);
}

string
Xapian::Document::Internal::get_value(Xapian::valueno valueid) const
{
    if (values_here) {
	map<Xapian::valueno, string>::const_iterator i;
	i = values.find(valueid);
	if (i == values.end()) return "";
	return i->second;
    }
    if (!database) return "";
    return do_get_value(valueid);
}
	
map<Xapian::valueno, string>
Xapian::Document::Internal::get_all_values() const
{
    need_values();
    return values;
}

string
Xapian::Document::Internal::get_data() const
{
    if (data_here) return data;
    if (!database) return "";
    return do_get_data();
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
    DEBUGCALL(MATCH, TermList *, "Document::Internal::open_term_list", "");
    if (terms_here) {
	RETURN(new MapTermList(terms.begin(), terms.end(), terms.size()));
    }
    if (!database) return NULL;
    RETURN(database->open_term_list(did));
}

void
Xapian::Document::Internal::add_value(Xapian::valueno valueno, const string &value)
{
    need_values();
    values[valueno] = value;
    value_nos.clear();
}

void
Xapian::Document::Internal::remove_value(Xapian::valueno valueno)
{
    need_values();
    map<Xapian::valueno, string>::iterator i = values.find(valueno);
    if (i == values.end()) {
	throw Xapian::InvalidArgumentError("Value #" + om_tostring(valueno) +
		" is not present in document, in "
		"Xapian::Document::Internal::remove_value()");
    }
    values.erase(i);
    value_nos.clear();
}

void
Xapian::Document::Internal::clear_values()
{
    values.clear();
    value_nos.clear();
    values_here = true;
}

void
Xapian::Document::Internal::add_posting(const string & tname, Xapian::termpos tpos,
			      Xapian::termcount wdfinc)
{
    need_terms();

    map<string, OmDocumentTerm>::iterator i;
    i = terms.find(tname);
    if (i == terms.end()) {
	OmDocumentTerm newterm(tname, wdfinc);
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
	OmDocumentTerm newterm(tname, wdfinc);
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
	throw Xapian::InvalidArgumentError("Term `" + tname +
		"' is not present in document, in "
		"Xapian::Document::Internal::remove_posting()");
    }
    i->second.remove_position(tpos);
    if (wdfdec) i->second.dec_wdf(wdfdec);
}

void
Xapian::Document::Internal::remove_term(const string & tname)
{
    need_terms();
    map<string, OmDocumentTerm>::iterator i;
    i = terms.find(tname);
    if (i == terms.end()) {
	throw Xapian::InvalidArgumentError("Term `" + tname +
		"' is not present in document, in "
		"Xapian::Document::Internal::remove_term()");
    }
    terms.erase(i);
}
	
void
Xapian::Document::Internal::clear_terms()
{
    terms.clear();
    terms_here = true;
}

Xapian::termcount
Xapian::Document::Internal::termlist_count() const
{
    if (!terms_here) {
	// How equivalent is this line to the rest?
	// return database ? database->open_term_list(did)->get_approx_size() : 0;
	need_terms();
    }
    Assert(terms_here);
    return terms.size();
}

void
Xapian::Document::Internal::need_terms() const
{
    if (terms_here) return;
    if (database) {
	Xapian::TermIterator t(database->open_term_list(did));
	Xapian::TermIterator tend(NULL);
	for ( ; t != tend; ++t) {
	    Xapian::PositionIterator p = t.positionlist_begin();
	    Xapian::PositionIterator pend = t.positionlist_end();
	    OmDocumentTerm term(*t, t.get_wdf());
	    for ( ; p != pend; ++p) {
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
    DEBUGLINE(UNKNOWN, "Xapian::Document::Internal::values_count() called");
    need_values();
    Assert(values_here);
    return values.size();
}

string
Xapian::Document::Internal::get_description() const
{
    string description = "Xapian::Document::Internal(";

    if (data_here) description += "data=`" + data + "'";

    if (values_here) {
	if (data_here) description += ", ";
	description += "values[" + om_tostring(values.size()) + "]";
    }

    if (terms_here) {
	if (data_here || values_here) description += ", ";
	description += "terms[" + om_tostring(terms.size()) + "]";
    }

    if (database) {
	if (data_here || values_here || terms_here) description += ", ";
	description += "doc=";
	description += "?"; // do_get_description(); ?
    }

    description += ')';

    return description;
}

void
Xapian::Document::Internal::need_values() const
{
    if (!values_here) {
	if (database) {
	    values = do_get_all_values();
	    value_nos.clear();
	}
	values_here = true;
    }
}
