/* omdocument.cc: class for performing a match
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2003 Olly Betts
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 * -----END-LICENCE-----
 */

#include <config.h>
#include <xapian/document.h>
#include <xapian/types.h>
#include "document.h"
#include "modifieddocument.h"
#include "maptermlist.h"
#include "omvalueiteratorinternal.h"
#include "xapian/error.h"
#include <algorithm>
#include <string>

using namespace std;

namespace Xapian {

// implementation of Document

Document::Document(Document::Internal *internal_) : internal(internal_)
{
}

Document::Document() : internal(0)
{
}

string
Document::get_value(om_valueno value) const
{
    DEBUGAPICALL(string, "Document::get_value", value);
    if (!internal.get()) RETURN("");
    RETURN(internal->get_value(value));
}

string
Document::get_data() const
{
    DEBUGAPICALL(string, "Document::get_data", "");
    if (!internal.get()) RETURN("");
    RETURN(internal->get_data());
}

void
Document::set_data(const string &data)
{
    DEBUGAPICALL(void, "Document::set_data", data);
    if (!internal.get())
	internal = new ModifiedDocument(0);
    else
	internal = internal->modify();
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
    if (!internal.get()) return "Document()";
    return "Document(" + internal->get_description() + ")";
}

void
Document::add_value(om_valueno valueno, const string &value)
{
    DEBUGAPICALL(void, "Document::add_value", valueno << ", " << value);
    if (!internal.get())
	internal = new ModifiedDocument(0);
    else
	internal = internal->modify();
    internal->add_value(valueno, value);
}

void
Document::remove_value(om_valueno valueno)
{
    DEBUGAPICALL(void, "Document::remove_value", valueno);
    if (!internal.get())
	internal = new ModifiedDocument(0);
    else
	internal = internal->modify();
    internal->remove_value(valueno);
}

void
Document::clear_values()
{
    DEBUGAPICALL(void, "Document::clear_values", "");
    if (!internal.get())
	internal = new ModifiedDocument(0);
    else
	internal = internal->modify();
    internal->clear_values();
}

void
Document::add_posting(const string & tname,
			om_termpos tpos,
			om_termcount wdfinc)
{
    DEBUGAPICALL(void, "Document::add_posting",
		 tname << ", " << tpos << ", " << wdfinc);
    if (tname.empty()) {
	throw InvalidArgumentError("Empty termnames aren't allowed.");
    }
    if (!internal.get())
	internal = new ModifiedDocument(0);
    else
	internal = internal->modify();
    internal->add_posting(tname, tpos, wdfinc);
}

void
Document::add_term_nopos(const string & tname, om_termcount wdfinc)
{
    DEBUGAPICALL(void, "Document::add_term_nopos", tname << ", " << wdfinc);
    if (tname.empty()) {
	throw InvalidArgumentError("Empty termnames aren't allowed.");
    }
    if (!internal.get())
	internal = new ModifiedDocument(0);
    else
	internal = internal->modify();
    internal->add_term_nopos(tname, wdfinc);
}

void
Document::remove_posting(const string & tname, om_termpos tpos,
			 om_termcount wdfdec)
{
    DEBUGAPICALL(void, "Document::remove_posting",
		 tname << ", " << tpos << ", " << wdfdec);
    if (tname.empty()) {
	throw InvalidArgumentError("Empty termnames aren't allowed.");
    }
    if (!internal.get())
	internal = new ModifiedDocument(0);
    else
	internal = internal->modify();
    internal->remove_posting(tname, tpos, wdfdec);
}

void
Document::remove_term(const string & tname)
{
    DEBUGAPICALL(void, "Document::remove_term", tname);
    if (!internal.get())
	internal = new ModifiedDocument(0);
    else
	internal = internal->modify();
    internal->remove_term(tname);
}

void
Document::clear_terms()
{
    DEBUGAPICALL(void, "Document::clear_terms", "");
    if (!internal.get())
	internal = new ModifiedDocument(0);
    else
	internal = internal->modify();
    internal->clear_terms();
}

om_termcount
Document::termlist_count() {
    DEBUGAPICALL(om_termcount, "Document::termlist_count", "");
    if (!internal.get())
	internal = new ModifiedDocument(0);
    else
	internal = internal->modify();
    RETURN(internal->termlist_count());
}

TermIterator
Document::termlist_begin() const
{
    DEBUGAPICALL(TermIterator, "Document::termlist_begin", "");
    if (!internal.get()) RETURN(NULL);
    RETURN(TermIterator(internal->open_term_list()));
}

TermIterator
Document::termlist_end() const
{
    DEBUGAPICALL(TermIterator, "Document::termlist_end", "");
    RETURN(TermIterator(NULL));
}

om_termcount
Document::values_count() {
    DEBUGAPICALL(om_termcount, "Document::values_count", "");
    if (!internal.get())
	internal = new ModifiedDocument(0);
    else
	internal = internal->modify();
    RETURN(internal->values_count());
}

OmValueIterator
Document::values_begin() const
{
    DEBUGAPICALL(OmValueIterator, "Document::values_begin", "");
    if (!internal.get())
	internal = new ModifiedDocument(0);
    else
	internal = internal->modify();
    RETURN(OmValueIterator(internal->values_begin()));
}

OmValueIterator
Document::values_end() const
{
    DEBUGAPICALL(OmValueIterator, "Document::values_end", "");
    if (!internal.get())
	internal = new ModifiedDocument(0);
    else
	internal = internal->modify();
    RETURN(OmValueIterator(internal->values_end()));
}

}

/////////////////////////////////////////////////////////////////////////////

void
OmDocumentTerm::add_position(om_termpos tpos)
{
    DEBUGAPICALL(void, "OmDocumentTerm::add_position", tpos);
    
    // We generally expect term positions to be added in approximately
    // increasing order, so check the end first
    om_termpos last = positions.empty() ? 0 : positions.back();
    if (tpos > last) {
	positions.push_back(tpos);
	return;
    }

    // Search for the position the term occurs at.  Use binary chop to
    // search, since this is a sorted list.
    vector<om_termpos>::iterator i;
    i = lower_bound(positions.begin(), positions.end(), tpos);
    if (i == positions.end() || *i != tpos) {
	positions.insert(i, tpos);
    }
}

void
OmDocumentTerm::remove_position(om_termpos tpos)
{
    DEBUGAPICALL(void, "OmDocumentTerm::remove_position", tpos);
    
    // Search for the position the term occurs at.  Use binary chop to
    // search, since this is a sorted list.
    vector<om_termpos>::iterator i;
    i = lower_bound(positions.begin(), positions.end(), tpos);
    if (i == positions.end() || *i != tpos) {
	throw Xapian::InvalidArgumentError("Position `" + om_tostring(tpos) +
				     "' not found in list of positions that `" +
				     tname +
				     "' occurs at,"
				     " when removing position from list");
	positions.insert(i, tpos);
    } else {
	positions.erase(i);
    }
}

string
OmDocumentTerm::get_description() const
{
    DEBUGCALL(INTRO, string, "OmDocumentTerm::get_description", "");
    string description;

    description = "OmDocumentTerm(" + tname +
	    ", wdf = " + om_tostring(wdf) +
	    ", termfreq = " + om_tostring(termfreq) +
	    ", positions[" + om_tostring(positions.size()) + "]" +
	    ")";
    RETURN(description);
}

string
ModifiedDocument::get_value(om_valueno valueid) const
{
    if (values_here) {
	map<om_valueno, string>::const_iterator i;
	i = values.find(valueid);
	if (i == values.end()) return "";
	return i->second;
    }
    if (!ptr.get()) return "";
    return ptr->get_value(valueid);
}
	
map<om_valueno, string>
ModifiedDocument::get_all_values() const
{
    if (values_here) return values;
    if (!ptr.get()) {
	map<om_valueno, string> nowt;
	return nowt;
    }
    return ptr->get_all_values();
}

string
ModifiedDocument::get_data() const
{
    if (data_here) return data;
    if (!ptr.get()) return "";
    return ptr->get_data();
}

void
ModifiedDocument::set_data(const string &data_)
{
    data = data_;
    data_here = true;
}

TermList *
ModifiedDocument::open_term_list() const
{
    if (terms_here) {
	return new MapTermList(terms.begin(), terms.end(), terms.size());
    }
    if (!ptr.get()) return NULL;
    return ptr->open_term_list();
}

void
ModifiedDocument::add_value(om_valueno valueno, const string &value)
{
    need_values();
    values.insert(make_pair(valueno, value));	
}

void
ModifiedDocument::remove_value(om_valueno valueno)
{
    need_values();
    values.erase(valueno);
}

void
ModifiedDocument::clear_values()
{
    values.clear();
    values_here = true;
}

void
ModifiedDocument::add_posting(const string & tname, om_termpos tpos,
			      om_termcount wdfinc)
{
    need_terms();

    map<string, OmDocumentTerm>::iterator i;
    i = terms.find(tname);
    if (i == terms.end()) {
	OmDocumentTerm newterm(tname);
	newterm.add_position(tpos);
	newterm.set_wdf(wdfinc);
	terms.insert(make_pair(tname, newterm));
    } else {
	i->second.add_position(tpos);
	if (wdfinc) i->second.set_wdf(i->second.get_wdf() + wdfinc);
    }
}

void
ModifiedDocument::add_term_nopos(const string & tname, om_termcount wdfinc)
{
    need_terms();

    map<string, OmDocumentTerm>::iterator i;
    i = terms.find(tname);
    if (i == terms.end()) {
	OmDocumentTerm newterm(tname);
	newterm.set_wdf(wdfinc);
	terms.insert(make_pair(tname, newterm));
    } else {
	if (wdfinc) i->second.set_wdf(i->second.get_wdf() + wdfinc);
    }
}

void
ModifiedDocument::remove_posting(const string & tname, om_termpos tpos,
				 om_termcount wdfdec)	
{
    need_terms();

    map<string, OmDocumentTerm>::iterator i;
    i = terms.find(tname);
    if (i == terms.end()) {
	throw Xapian::InvalidArgumentError("Term `" + tname +
		"' is not present in document, in "
		"ModifiedDocument::remove_posting()");
    }
    i->second.remove_position(tpos);
    if (wdfdec) {
	om_termcount currwdf = i->second.get_wdf();
	currwdf = ((currwdf > wdfdec) ? (currwdf - wdfdec) : 0);
	i->second.set_wdf(currwdf);
    }
}

void
ModifiedDocument::remove_term(const string & tname)
{
    need_terms();
    map<string, OmDocumentTerm>::iterator i;
    i = terms.find(tname);
    if (i == terms.end()) {
	throw Xapian::InvalidArgumentError("Term `" + tname +
		"' is not present in document, in "
		"Document::remove_term()");
    }
    terms.erase(i);
}
	
void
ModifiedDocument::clear_terms()
{
    terms.clear();
    terms_here = true;
}

om_termcount
ModifiedDocument::termlist_count()
{
    if (!terms_here) {
	// How equivalent is this line to the rest?
	// return ptr->open_term_list()->get_approx_size();
	need_terms();
    }
    Assert(terms_here);
    return terms.size();
}

void
ModifiedDocument::need_terms()
{
    if (terms_here) return;
    if (ptr.get()) {
	Xapian::TermIterator t(ptr->open_term_list());
	Xapian::TermIterator tend(NULL);
	for ( ; t != tend; ++t) {
	    Xapian::PositionListIterator p = t.positionlist_begin();
	    Xapian::PositionListIterator pend = t.positionlist_end();
	    OmDocumentTerm term(*t);
	    for ( ; p != pend; ++p) {
		term.add_position(*p);
	    }
	    term.set_wdf(t.get_wdf());
	    terms.insert(make_pair(*t, term));
	}
    }
    terms_here = true;
}

om_termcount
ModifiedDocument::values_count()
{
    need_values();
    Assert(values_here);
    return values.size();
}

OmValueIterator::Internal *
ModifiedDocument::values_begin() const
{
    need_values();
    return new OmValueIterator::Internal(values.begin());
}

OmValueIterator::Internal *
ModifiedDocument::values_end() const
{
    need_values();
    return new OmValueIterator::Internal(values.end());
}

string
ModifiedDocument::get_description() const
{
    string description = "ModifiedDocument(";

    if (data_here) description += "data=`" + data + "'";

    if (values_here) {
	if (data_here) description += ", ";
	description += "values[" + om_tostring(values.size()) + "]";
    }

    if (terms_here) {
	if (data_here || values_here) description += ", ";
	description += "terms[" + om_tostring(terms.size()) + "]";
    }

    if (ptr.get()) {
	if (data_here || values_here || terms_here) description += ", ";
	description += "doc=";
	description += ptr->get_description();
    }

    description += ')';

    return description;
}

void
ModifiedDocument::need_values() const//FIXME const is hack
{
    if (!values_here) {
	if (ptr.get()) values = ptr->get_all_values();
	values_here = true;
    }
}

Xapian::Document::Internal *
ModifiedDocument::modify()
{
    return this;
}

Xapian::Document::Internal * Xapian::Document::Internal::modify()
{
    return new ModifiedDocument(this);
}
