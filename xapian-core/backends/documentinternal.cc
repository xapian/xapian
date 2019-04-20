/** @file documentinternal.cc
 * @brief Abstract base class for a document
 */
/* Copyright 2017 Olly Betts
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

#include "backends/documentinternal.h"

#include "api/documenttermlist.h"
#include "api/documentvaluelist.h"
#include "str.h"
#include "unicode/description_append.h"

#include "xapian/valueiterator.h"

#include <memory>

using namespace std;

namespace Xapian {

void
Document::Internal::ensure_terms_fetched() const
{
    if (terms)
	return;

    terms.reset(new map<string, TermInfo>());
    termlist_size = 0;
    if (!database.get())
	return;

    unique_ptr<TermList> t(database->open_term_list(did));
    while (t->next(), !t->at_end()) {
	++termlist_size;
	auto&& r = terms->emplace_hint(terms->end(),
				       t->get_termname(),
				       TermInfo(t->get_wdf()));
	TermInfo& term = r->second;
	unique_ptr<PositionList> p(t->positionlist_begin());
	while (p->next()) {
	    term.append_position(p->get_position());
	}
    }
}

void
Document::Internal::ensure_values_fetched() const
{
    if (values)
	return;

    values.reset(new map<Xapian::valueno, string>());
    if (database.get()) {
	fetch_all_values(*values);
    }
}

string
Document::Internal::fetch_data() const
{
    return string();
}

void
Document::Internal::fetch_all_values(map<Xapian::valueno,
				     string>& values_) const
{
    values_.clear();
}

string
Document::Internal::fetch_value(Xapian::valueno) const
{
    return string();
}

Document::Internal::~Internal()
{
    if (database.get())
	database->invalidate_doc_object(this);
}

TermList*
Document::Internal::open_term_list() const
{
    if (terms)
	return new DocumentTermList(this);

    if (!database.get())
	return NULL;

    return database->open_term_list(did);
}

Xapian::ValueIterator
Document::Internal::values_begin() const
{
    if (!values && database.get()) {
	values.reset(new map<Xapian::valueno, string>());
	fetch_all_values(*values);
    }

    if (!values || values->empty())
	return Xapian::ValueIterator();

    return Xapian::ValueIterator(new DocumentValueList(this));
}

string
Document::Internal::get_description() const
{
    string desc = "Document(docid=";
    desc += str(did);

    if (data) {
	desc += ", data=";
	description_append(desc, *data);
    }

    if (terms) {
	desc += ", terms[";
	desc += str(terms->size());
	desc += ']';
    }

    if (values) {
	desc += ", values[";
	desc += str(values->size());
	desc += ']';
    }

    if (database.get()) {
	desc += ", db=";
	desc += database->get_description();
    }

    desc += ')';

    return desc;
}

}
