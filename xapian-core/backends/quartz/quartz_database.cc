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

#include "quartz_db_manager.h"

#include "quartz_database.h"
#include "utils.h"
#include <om/omerror.h>
#include <string>


//
// Compulsory parameters.
// quartz_dir    - Directory that the database is stored in.
//
// Optional parameters.
// quartz_tmpdir - Directory in which to store temporary files.  If not
//                 specified, the database directory will be used.
// quartz_envdir - Directory to use to keep the database environment in.
// 		   If not specified, the database directory will be used.
//
QuartzDatabase::QuartzDatabase(const OmSettings & settings, bool readonly)
	: db_manager(new QuartzDBManager(settings, readonly))
{
}

QuartzDatabase::~QuartzDatabase()
{
    // FIXME: could throw an exception
    internal_end_session();
}


void
QuartzDatabase::do_begin_session(om_timeout timeout)
{
    throw OmUnimplementedError("QuartzDatabase::do_begin_session() not yet implemented");
}

void
QuartzDatabase::do_end_session()
{
    throw OmUnimplementedError("QuartzDatabase::do_end_session() not yet implemented");
}

void
QuartzDatabase::do_flush()
{
    throw OmUnimplementedError("QuartzDatabase::do_flush() not yet implemented");
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
    throw OmUnimplementedError("QuartzDatabase::do_add_document() not yet implemented");
}

void
QuartzDatabase::do_delete_document(om_docid did)
{
    throw OmUnimplementedError("QuartzDatabase::do_delete_document() not yet implemented");
}

void
QuartzDatabase::do_replace_document(om_docid did,
				    const OmDocumentContents & document)
{
    throw OmUnimplementedError("QuartzDatabase::do_replace_document() not yet implemented");
}


OmDocumentContents
QuartzDatabase::do_get_document(om_docid did)
{
    throw OmUnimplementedError("QuartzDatabase::do_get_document() not yet implemented");
}



om_doccount 
QuartzDatabase::get_doccount() const
{
    throw OmUnimplementedError("QuartzDatabase::get_doccount() not yet implemented");
}

om_doclength
QuartzDatabase::get_avlength() const
{
    throw OmUnimplementedError("QuartzDatabase::get_avlength() not yet implemented");
}

om_doclength
QuartzDatabase::get_doclength(om_docid did) const
{
    throw OmUnimplementedError("QuartzDatabase::get_doclength() not yet implemented");
}

om_doccount
QuartzDatabase::get_termfreq(const om_termname & tname) const
{
    throw OmUnimplementedError("QuartzDatabase::get_termfreq() not yet implemented");
}

bool
QuartzDatabase::term_exists(const om_termname & tname) const
{
    throw OmUnimplementedError("QuartzDatabase::term_exists() not yet implemented");
}


LeafPostList *
QuartzDatabase::open_post_list(const om_termname& tname) const
{
    throw OmUnimplementedError("QuartzDatabase::open_post_list() not yet implemented");
}

LeafTermList *
QuartzDatabase::open_term_list(om_docid did) const
{
    throw OmUnimplementedError("QuartzDatabase::open_term_list() not yet implemented");
}

LeafDocument *
QuartzDatabase::open_document(om_docid did) const
{
    throw OmUnimplementedError("QuartzDatabase::open_document() not yet implemented");
}

