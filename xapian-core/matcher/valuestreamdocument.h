/** @file
 * @brief A document which gets its values from a ValueStreamManager.
 */
/* Copyright (C) 2009,2011,2014,2017 Olly Betts
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
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

#ifndef XAPIAN_INCLUDED_VALUESTREAMDOCUMENT_H
#define XAPIAN_INCLUDED_VALUESTREAMDOCUMENT_H

#include "backends/documentinternal.h"
#include "backends/multi.h"
#include "backends/multi/multi_database.h"
#include "backends/valuelist.h"
#include "omassert.h"
#include "xapian/database.h"
#include "xapian/types.h"

#include <map>

/// A document which gets its values from a ValueStreamManager.
class ValueStreamDocument : public Xapian::Document::Internal {
    /// Don't allow assignment.
    void operator=(const ValueStreamDocument &);

    /// Don't allow copying.
    ValueStreamDocument(const ValueStreamDocument &);

    mutable std::map<Xapian::valueno, ValueList *> valuelists;

    Xapian::Database db;

    Xapian::doccount current = 0;

    Xapian::doccount n_shards;

    mutable Xapian::Document::Internal * doc = NULL;

    /** Private constructor.
     *
     *  This is an implementation detail - the public constructor forwards to
     *  this constructor so we can use n_shards_ to init our parent class.
     */
    ValueStreamDocument(const Xapian::Database& db_, Xapian::doccount n_shards_)
	: Internal(n_shards_ == 1 ?
		   db_.internal.get() :
		   static_cast<MultiDatabase*>(db_.internal.get())->shards[0],
		   0),
	  db(db_),
	  n_shards(n_shards_) {}

  public:
    explicit ValueStreamDocument(const Xapian::Database& db_)
	: ValueStreamDocument(db_, db_.internal->size()) {}

    void new_shard(Xapian::doccount n);

    ~ValueStreamDocument();

    void set_shard_document(Xapian::docid shard_did) {
	if (did != shard_did) {
	    did = shard_did;
	    delete doc;
	    doc = NULL;
	}
    }

    void set_document(Xapian::docid did_) {
	AssertEq(current, shard_number(did_, n_shards));
	set_shard_document(shard_docid(did_, n_shards));
    }

    // Optimise away the virtual call when the matcher wants to know a value.
    std::string get_value(Xapian::valueno slot) const {
	return ValueStreamDocument::fetch_value(slot);
    }

  protected:
    /** Implementation of virtual methods @{ */
    std::string fetch_value(Xapian::valueno slot) const;
    void fetch_all_values(std::map<Xapian::valueno, std::string> & values_) const;
    std::string fetch_data() const;
    /** @} */
};

#endif // XAPIAN_INCLUDED_VALUESTREAMDOCUMENT_H
