/* multi_database.h: C++ class definition for multiple database access
 *
 * ----START-LICENCE----
 * Copyright 1999,2000 BrightStation PLC
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

#ifndef OM_HGUARD_MULTI_DATABASE_H
#define OM_HGUARD_MULTI_DATABASE_H

#include "database.h"
#include "leafpostlist.h"
#include <om/omenquire.h>
#include <stdlib.h>
#include <set>
#include <vector>

/** A database which contains other databases.
 */
class MultiDatabase : public IRDatabase {
    /** DatabaseBuilder is a friend of this class, so that it can call
     *  the constructor.
     */
    friend class DatabaseBuilder;

    /** OmDatabase::Internal is a friend of this class,
     *  so that it can call the constructor, passing in IRDatabase pointers.
     */
    friend class OmDatabase::Internal;

    /** MultiMatch is a friend of this class so that it can access
     *  `databases'.  FIXME: this isn't very clean.
     */
    friend class MultiMatch;

    private:
	// List of subdatabases
	std::vector<OmRefCntPtr<IRDatabase> > databases;

	mutable bool length_initialised;
	mutable om_doclength avlength;

	om_doccount multiplier;

	/** This constructor isn't supported by the multibackend.
	 *
	 *  @exception OmInvalidOperationError will always be thrown.
	 */
	MultiDatabase(const OmSettings & params);

	/** Create a multi database from a set of existing databases.
	 *
	 *  @param databases_  A vector containing the existing databases.
	 *
	 *  @exception OmInvalidArgumentError if no databases are specified
	 *             in the vector.
	 */
	MultiDatabase(std::vector<OmRefCntPtr<IRDatabase> > databases_);

	//@{
	/** MultiDatabase is a readonly database type, and thus this method
	 *  is not supported: if called an exception will be thrown.
	 */
	void do_begin_session(om_timeout timeout) {
	    throw OmUnimplementedError(
		"MultiDatabase::begin_session() not implemented: readonly database type");
	};

	void do_end_session() {
	    throw OmUnimplementedError(
		"MultiDatabase::do_end_session() not implemented: readonly database type");
	};

	void do_flush() {
	    throw OmUnimplementedError(
		"MultiDatabase::flush() not implemented: readonly database type");
	};

	void do_begin_transaction() {
	    throw OmUnimplementedError(
		"MultiDatabase::begin_transaction() not implemented: readonly database type");
	};

	void do_commit_transaction() {
	    throw OmUnimplementedError(
		"MultiDatabase::commit_transaction() not implemented: readonly database type");
	};

	void do_cancel_transaction() {
	    throw OmUnimplementedError(
		"MultiDatabase::cancel_transaction() not implemented: readonly database type");
	};

	om_docid do_add_document(const struct OmDocumentContents & document) {
	    throw OmUnimplementedError(
		"MultiDatabase::add_document() not implemented: readonly database type");
	}

	void do_delete_document(om_docid did) {
	    throw OmUnimplementedError(
		"MultiDatabase::delete_document() not implemented: readonly database type");
	};

	void do_replace_document(om_docid did,
				 const OmDocumentContents & document) {
	    throw OmUnimplementedError(
		"MultiDatabase::replace_document() not implemented: readonly database type");
	};

	//@}

	/** Get a document from the database.
	 *  FIXME: implement this method.
	 */
	OmDocumentContents do_get_document(om_docid did) {
	    throw OmUnimplementedError(
		"MultiDatabase::get_document() not yet implemented");
	};

    public:
	~MultiDatabase();

	om_doccount  get_doccount() const;
	om_doclength get_avlength() const;
	om_doclength get_doclength(om_docid did) const;

	om_doccount get_termfreq(const om_termname & tname) const;
	bool term_exists(const om_termname & tname) const;

	LeafPostList * do_open_post_list(const om_termname & tname) const;
	LeafTermList * open_term_list(om_docid did) const;
	LeafDocument * open_document(om_docid did) const;
};

#endif /* OM_HGUARD_MULTI_DATABASE_H */
