/* quartz_database.cc: quartz database
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

#include "config.h"

// This is needed so that u_long gets defined, despite our specifying -ansi;
// otherwise db_cxx.h is broken.
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif /* _GNU_SOURCE */
#include <sys/types.h>
#include <db_cxx.h>

#include "quartz_database.h"

class QuartzDatabase::Internal {
};

QuartzDatabase::QuartzDatabase(const DatabaseBuilderParams & params)
{
// initialise environment
// Check version of BerkeleyDB available
// Call DBENV->set_tmp_dir with a safe directory (with known permissions)
// Make sure that environment is not in a network filesystem, eg NFS.
// Specify DBRECOVER flag to DBENV->open.
}

QuartzDatabase::~QuartzDatabase()
{
}


void
QuartzDatabase::do_begin_session(om_timeout timeout)
{
}

void
QuartzDatabase::do_end_session()
{
}

void
QuartzDatabase::do_flush()
{
}


void
QuartzDatabase::do_begin_transaction()
{
    throw OmUnimplementedError("QuartzDatabase::do_begin_transaction() not yet implemented");
}

void
QuartzDatabase::do_commit_transaction()
{
    throw OmUnimplementedError("QuartzDatabase::do_commit_transaction() not yet implemented");
}

void
QuartzDatabase::do_cancel_transaction()
{
    throw OmUnimplementedError("QuartzDatabase::do_cancel_transaction() not yet implemented");
}


om_docid
QuartzDatabase::do_add_document(const OmDocumentContents & document)
{
}

void
QuartzDatabase::do_delete_document(om_docid did)
{
}

void
QuartzDatabase::do_replace_document(om_docid did,
				    const OmDocumentContents & document)
{
}


OmDocumentContents
QuartzDatabase::do_get_document(om_docid did)
{
}



om_doccount 
QuartzDatabase::get_doccount() const
{
}

om_doclength
QuartzDatabase::get_avlength() const
{
}

om_doclength
QuartzDatabase::get_doclength(om_docid did) const
{
}

om_doccount
QuartzDatabase::get_termfreq(const om_termname & tname) const
{
}

bool
QuartzDatabase::term_exists(const om_termname & tname) const
{
}


LeafPostList *
QuartzDatabase::open_post_list(const om_termname& tname) const
{
}

LeafTermList *
QuartzDatabase::open_term_list(om_docid did) const
{
}

LeafDocument *
QuartzDatabase::open_document(om_docid did) const
{
}

