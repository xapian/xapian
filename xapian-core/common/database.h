/* database.h: database class declarations
 *
 * ----START-LICENCE----
 * Copyright 1999,2000 Dialog Corporation
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

#ifndef OM_HGUARD_DATABASE_H
#define OM_HGUARD_DATABASE_H

#include <string>
#include "om/omtypes.h"

#include "database_builder.h"
#include "indexer.h"

class OmDocument;
class PostList;
class DBPostList;
class TermList;
class DBTermList;
class RSet;

/** Base class for databases.
 */
class IRDatabase : public virtual IndexerDestination {
    // Class which can create IRDatabases.
    // All classes derived from IRDatabase must also have DatabaseBuilder as
    // a friend, so that they can be constructed in a unified way.
    friend class DatabaseBuilder;
    private:
    	// Open method - called only by DatabaseBuilder
	virtual void open(const DatabaseBuilderParams &) = 0;

	// Note: No close method needed - just delete the object
    protected:
    	// Constructor - called only by derived classes and by DatabaseBuilder
	IRDatabase() : root(this) { return; }

	// Root database - used to calculate collection statistics
	IRDatabase * root;
    public:
        virtual ~IRDatabase() { return; }

	void set_root(IRDatabase *db) {root = db;}

	// Database statistics:
	// ====================

	// Number of docs in the database
	virtual om_doccount  get_doccount() const = 0;
	// Average length of a document
	virtual om_doclength get_avlength() const = 0;

	// Get the length of a given document
	virtual om_doclength get_doclength(om_docid did) const = 0;

	// Number of docs indexed by a given term
	virtual om_doccount get_termfreq(const om_termname & tname) const = 0;

	// Whether a given term is in the database: functionally equivalent
	// to (get_termfreq() != 0), but can be considerably more efficient.
	virtual bool term_exists(const om_termname & tname) const = 0;

	// Data item access methods:
	// =========================

	virtual DBPostList * open_post_list(const om_termname&, RSet *) const = 0;
	virtual DBTermList * open_term_list(om_docid did) const = 0;
	virtual OmDocument * open_document(om_docid did) const = 0;

#if 0
	// Introspection methods:
	// ======================

	virtual const DatabaseBuilderParams & get_database_params() const = 0;
	virtual const IRDatabase * get_database_of_doc(om_docid) const = 0;
#endif
};

#endif /* OM_HGUARD_DATABASE_H */
