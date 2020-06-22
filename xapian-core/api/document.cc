/** @file document.cc
 * @brief Class representing a document
 */
/* Copyright 2008,2017,2018 Olly Betts
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

#include "xapian/document.h"

#include <string>

#include "backends/documentinternal.h"
#include "net/serialise.h"
#include "str.h"

#include "xapian/error.h"

using namespace std;

[[noreturn]]
static void
throw_invalid_arg_empty_term()
{
    throw Xapian::InvalidArgumentError("Empty termnames are invalid");
}

namespace Xapian {

Document::Document(Document::Internal* internal_)
    : internal(internal_)
{
}

Document::Document(const Document&) = default;

Document&
Document::operator=(const Document&) = default;

Document::Document(Document&&) = default;

Document&
Document::operator=(Document&&) = default;

Document::Document() : internal(new Xapian::Document::Internal)
{
}

Document::~Document()
{
}

Xapian::docid
Document::get_docid() const
{
    return internal->get_docid();
}

string
Document::get_data() const
{
    return internal->get_data();
}

void
Document::set_data(const string& data)
{
    internal->set_data(data);
}

void
Document::add_term(const string& term, Xapian::termcount wdf_inc)
{
    if (term.empty()) {
	throw_invalid_arg_empty_term();
    }
    internal->add_term(term, wdf_inc);
}

void
Document::remove_term(const string& term)
{
    if (term.empty()) {
	throw_invalid_arg_empty_term();
    }
    if (!internal->remove_term(term)) {
	string m;
	m = "Document::remove_term() failed - term '";
	m += term;
	m += "' not present";
	throw Xapian::InvalidArgumentError(m);
    }
}

void
Document::add_posting(const string& term,
		      Xapian::termpos term_pos,
		      Xapian::termcount wdf_inc)
{
    if (term.empty()) {
	throw_invalid_arg_empty_term();
    }
    internal->add_posting(term, term_pos, wdf_inc);
}

void
Document::remove_posting(const string& term,
			 Xapian::termpos term_pos,
			 Xapian::termcount wdf_dec)
{
    if (term.empty()) {
	throw_invalid_arg_empty_term();
    }
    auto res = internal->remove_posting(term, term_pos, wdf_dec);
    if (res != Document::Internal::OK) {
	string m = "Document::remove_posting() failed - term '";
	m += term;
	if (res == Document::Internal::NO_TERM) {
	    m += "' not present";
	} else {
	    m += "' not present at position ";
	    m += str(term_pos);
	}
	throw Xapian::InvalidArgumentError(m);
    }
}

Xapian::termpos
Document::remove_postings(const string& term,
			  Xapian::termpos term_pos_first,
			  Xapian::termpos term_pos_last,
			  Xapian::termcount wdf_dec)
{
    if (term.empty()) {
	throw_invalid_arg_empty_term();
    }
    if (rare(term_pos_first > term_pos_last)) {
	return 0;
    }
    Xapian::termpos n_removed;
    auto res = internal->remove_postings(term, term_pos_first, term_pos_last,
					 wdf_dec, n_removed);
    if (res != Document::Internal::OK) {
	string m = "Document::remove_postings() failed - term '";
	m += term;
	m += "' not present";
	throw Xapian::InvalidArgumentError(m);
    }
    return n_removed;
}

void
Document::clear_terms()
{
    internal->clear_terms();
}

Xapian::termcount
Document::termlist_count() const {
    return internal->termlist_count();
}

TermIterator
Document::termlist_begin() const
{
    return TermIterator(internal->open_term_list());
}

string
Document::get_value(Xapian::valueno slot) const
{
    return internal->get_value(slot);
}

void
Document::add_value(Xapian::valueno slot, const string& value)
{
    internal->add_value(slot, value);
}

void
Document::clear_values()
{
    internal->clear_values();
}

Xapian::termcount
Document::values_count() const {
    return internal->values_count();
}

ValueIterator
Document::values_begin() const
{
    return internal->values_begin();
}

string
Document::serialise() const
{
    return serialise_document(*this);
}

Document
Document::unserialise(const string& serialised)
{
    return unserialise_document(serialised);
}

string
Document::get_description() const
{
    return internal->get_description();
}

}
