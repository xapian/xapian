/* sleepy_database.h: C++ class definition for sleepycat access routines
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

#ifndef OM_HGUARD_SLEEPY_DATABASE_H
#define OM_HGUARD_SLEEPY_DATABASE_H

#include <stdlib.h>
#include <memory>
#include "database.h"
#include "om/omerror.h"
#include "om/omdocument.h"

class SleepyDatabaseTermCache;
class SleepyDatabaseInternals;

/** A database using the sleepycat database library.
 *  This currently uses the C++ interface, version 2.2.6, but may work
 *  with other versions.
 *
 *  Sleepycat is available from http://www.sleepycat.com/
 */
class SleepyDatabase : public IRDatabase {
    friend class DatabaseBuilder;
    private:
	std::auto_ptr<SleepyDatabaseInternals> internals;

	std::auto_ptr<SleepyDatabaseTermCache> termcache;

	/** Create and open a sleepycat database.
	 *
	 *  @exception OmOpeningError thrown if database can't be opened.
	 *
	 *  @param params Parameters supplied by the user to specify the
	 *                location of the database to open.
	 */
	SleepyDatabase(const OmSettings &params, bool readonly);

	/** Make a new entry in a postlist.
	 *
	 *  This opens the specified postlist, and adds the information
	 *  specified.  If the document ID supplied is already in the
	 *  postlist, its entry is overwritten.
	 *
	 *  @param tid        The term ID of the postlist to be modified.
	 *  @param did        The document ID for the entry.
	 *  @param wdf        The WDF for the entry (number of occurrences
	 *                    in the given document).
	 *  @param positions  A list of positions at which the term occurs.
	 *                    This list must be strictly increasing (ie, no
	 *                    duplicates).
	 *  @param doclength  The length of the document (ie, sum of wdfs).
	 *
	 *  @return The new size of the postlist - ie, the new termfrequency.
	 */
	om_doccount add_entry_to_postlist(om_termid tid,
					  om_docid did,
					  om_termcount wdf,
					  const std::vector<om_termpos> & positions,
					  om_doclength doclength);

	/** Make a new document, and return the new document ID.
	 *
	 *  @param document The document to store in the database.
	 *
	 *  @return The newly allocated document ID.
	 */
	om_docid make_new_document(const struct OmDocumentContents & document);

	/** Make a new termlist.
	 *
	 *  A termlist for this document ID must not already exist.
	 *
	 *  @param did    The document ID
	 *  @param terms  The terms to make the termlist out of.
	 */
	void make_new_termlist(om_docid did,
			       const std::map<om_termid, OmDocumentTerm> & terms);

	//@{
	/** Implementation of virtual methods: see IRDatabase for details.
	 */
	void do_begin_session(om_timeout timeout);
	void do_end_session();
	void do_flush();

	void do_begin_transaction();
	void do_commit_transaction();
	void do_cancel_transaction();

	om_docid do_add_document(const OmDocumentContents & document);
	void do_delete_document(om_docid did);
	void do_replace_document(om_docid did,
				 const OmDocumentContents & document);
	OmDocumentContents do_get_document(om_docid did);
	//@}

    public:
	~SleepyDatabase();

	// Virtual methods of IRDatabase
	om_doccount  get_doccount() const;
	om_doclength get_avlength() const;
	om_doclength get_doclength(om_docid did) const;
	om_doccount get_termfreq(const om_termname & tname) const;
	bool term_exists(const om_termname & tname) const;

	LeafPostList * do_open_post_list(const om_termname & tname) const;
	LeafTermList * open_term_list(om_docid did) const;
	LeafDocument * open_document(om_docid did) const;
};

#endif /* OM_HGUARD_SLEEPY_DATABASE_H */
