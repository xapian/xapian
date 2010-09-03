/** @file documentvaluelist.h
 * @brief Iteration over values in a document.
 */
/* Copyright (C) 2007,2008,2009 Olly Betts
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

#ifndef XAPIAN_INCLUDED_DOCUMENTVALUELIST_H
#define XAPIAN_INCLUDED_DOCUMENTVALUELIST_H

#include "valuelist.h"

#include "document.h"

/// Iteration over values in a document.
class DocumentValueList : public ValueList {
    /// Don't allow assignment.
    void operator=(const DocumentValueList &);

    /// Don't allow copying.
    DocumentValueList(const DocumentValueList &);

    /// Document internals we're iterating over.
    Xapian::Internal::RefCntPtr<const Xapian::Document::Internal> doc;

    /** Iterator over the map inside @a doc.
     *
     *  If we haven't started yet, this will be set to: doc->values.end()
     */
    Xapian::Document::Internal::document_values::const_iterator it;

  public:
    DocumentValueList(const Xapian::Internal::RefCntPtr<Xapian::Document::Internal> & doc_)
	: doc(doc_), it(doc->values.end()) { }

    Xapian::docid get_docid() const;

    std::string get_value() const;

    Xapian::valueno get_valueno() const;

    bool at_end() const;

    void next();

    /// The parameter is actually a Xapian::valueno for this subclass.
    void skip_to(Xapian::docid slot);

    std::string get_description() const;
};

#endif // XAPIAN_INCLUDED_DOCUMENTVALUELIST_H
