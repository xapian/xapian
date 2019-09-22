/** @file documenttermlist.h
 * @brief Iteration over terms in a document
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

#ifndef XAPIAN_INCLUDED_DOCUMENTTERMLIST_H
#define XAPIAN_INCLUDED_DOCUMENTTERMLIST_H

#include "termlist.h"

#include "backends/documentinternal.h"

#include "omassert.h"

/// Iteration over terms in a document.
class DocumentTermList final : public TermList {
    /// Don't allow assignment.
    void operator=(const DocumentTermList&) = delete;

    /// Don't allow copying.
    DocumentTermList(const DocumentTermList&) = delete;

    /// Document internals we're iterating over.
    Xapian::Internal::intrusive_ptr<const Xapian::Document::Internal> doc;

    /** Iterator over the map inside @a doc.
     *
     *  If we haven't started yet, this will be set to: doc->terms.end()
     */
    std::map<std::string, TermInfo>::const_iterator it;

  public:
    explicit
    DocumentTermList(const Xapian::Document::Internal* doc_)
	: doc(doc_), it(doc->terms->end()) {}

    Xapian::termcount get_approx_size() const;

    std::string get_termname() const;

    Xapian::termcount get_wdf() const;

    Xapian::doccount get_termfreq() const;

    const Xapian::VecCOW<Xapian::termpos> * get_vec_termpos() const;

    PositionList* positionlist_begin() const;

    Xapian::termcount positionlist_count() const;

    TermList * next();

    TermList * skip_to(const std::string& term);

    bool at_end() const;
};

#endif // XAPIAN_INCLUDED_DOCUMENTTERMLIST_H
