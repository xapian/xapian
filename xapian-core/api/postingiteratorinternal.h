/** @file
 * @brief Xapian::PostingIterator internals
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

#ifndef XAPIAN_INCLUDED_POSTINGITERATORINTERNAL_H
#define XAPIAN_INCLUDED_POSTINGITERATORINTERNAL_H

#include "xapian/postingiterator.h"

#include "backends/postlist.h"

namespace Xapian {

class PostingIterator::Internal {
    friend class PostingIterator;

    PostList* pl;

    Xapian::Database db;

    unsigned _refs = 0;

  public:
    Internal(PostList* pl_, const Xapian::Database& db_)
	: pl(pl_), db(db_) {}

    ~Internal() {
	delete pl;
    }

    Xapian::docid get_docid() const {
	return pl->get_docid();
    }

    Xapian::termcount get_wdf() const {
	return pl->get_wdf();
    }

    Xapian::termcount get_doclength() const {
	return db.get_doclength(pl->get_docid());
    }

    Xapian::termcount get_unique_terms() const {
	return db.get_unique_terms(pl->get_docid());
    }

    Xapian::termcount get_wdfdocmax() const {
	return db.get_wdfdocmax(pl->get_docid());
    }

    PositionList* open_position_list() const {
	return pl->open_position_list();
    }

    bool next() {
	(void)pl->next();
	return !pl->at_end();
    }

    bool skip_to(Xapian::docid did) {
	(void)pl->skip_to(did);
	return !pl->at_end();
    }

    std::string get_description() const {
	return pl->get_description();
    }
};

}

#endif // XAPIAN_INCLUDED_POSTINGITERATORINTERNAL_H
