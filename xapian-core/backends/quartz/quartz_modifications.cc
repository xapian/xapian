/* quartz_modifications.cc: Management of modifications to a quartz database
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

#include "quartz_modifications.h"
#include "quartz_table.h"

#include "om/omindexdoc.h"
#include "om/omsettings.h"

#include <map>

QuartzModifications::QuartzModifications(QuartzDiskTableManager * table_manager_)
	: table_manager(table_manager_)
{
    open_diffs();
}

QuartzModifications::~QuartzModifications()
{
}

void
QuartzModifications::open_diffs()
{
    postlist_diffs.reset(new QuartzPostListDiffs(&(table_manager->postlist_table)));
    positionlist_diffs.reset(new QuartzPositionListDiffs(&(table_manager->positionlist_table)));
    termlist_diffs.reset(new QuartzTermListDiffs(&(table_manager->termlist_table)));
    lexicon_diffs.reset(new QuartzLexiconDiffs(&(table_manager->lexicon_table)));
    attribute_diffs.reset(new QuartzAttributeDiffs(&(table_manager->attribute_table)));
    record_diffs.reset(new QuartzRecordDiffs(&(table_manager->record_table)));
}

void
QuartzModifications::close_diffs()
{
    postlist_diffs.reset();
    positionlist_diffs.reset();
    termlist_diffs.reset();
    lexicon_diffs.reset();
    attribute_diffs.reset();
    record_diffs.reset();
}

void
QuartzModifications::apply()
{
    if(!postlist_diffs->is_modified() &&
       !positionlist_diffs->is_modified() &&
       !termlist_diffs->is_modified() &&
       !lexicon_diffs->is_modified() &&
       !attribute_diffs->is_modified() &&
       !record_diffs->is_modified()) {
	table_manager->log->make_entry("No modifications to apply.");
	return;
    }

    bool success;
    QuartzRevisionNumber old_revision(table_manager->get_revision_number());
    QuartzRevisionNumber new_revision(table_manager->get_next_revision_number());

    table_manager->log->make_entry("Applying modifications.  New revision number is " + new_revision.get_description() + ".");

    success = postlist_diffs->apply(new_revision);
    if (success) { success = positionlist_diffs->apply(new_revision); }
    if (success) { success = termlist_diffs->apply(new_revision); }
    if (success) { success = lexicon_diffs->apply(new_revision); }
    if (success) { success = attribute_diffs->apply(new_revision); }
    if (success) { success = record_diffs->apply(new_revision); }

    if (!success) {
	// Modifications failed.  Wipe all the modifications from memory.
	table_manager->log->make_entry("Attempted modifications failed.  Wiping partial modifications.");
	close_diffs();
	
	// Reopen tables with old revision number, 
	table_manager->log->make_entry("Reopening tables without modifications: old revision is " + old_revision.get_description() + ".");
	table_manager->open_tables(old_revision);

	// Increase revision numbers to new revision number plus one,
	// writing increased numbers to all tables.
	new_revision.increment();
	table_manager->log->make_entry("Increasing revision number in all tables to " + new_revision.get_description() + ".");

	std::map<QuartzDbKey, QuartzDbTag *> null_entries;
	table_manager->postlist_table.set_entries(null_entries, new_revision);
	table_manager->positionlist_table.set_entries(null_entries, new_revision);
	table_manager->termlist_table.set_entries(null_entries, new_revision);
	table_manager->lexicon_table.set_entries(null_entries, new_revision);
	table_manager->attribute_table.set_entries(null_entries, new_revision);
	table_manager->record_table.set_entries(null_entries, new_revision);

	// Prepare for further modifications.
	open_diffs();
    }
    table_manager->log->make_entry("Modifications succeeded.");
}

om_docid
QuartzModifications::get_newdocid()
{
    // FIXME: read from record 0.
    return 1;
}

om_docid
QuartzModifications::add_document(const OmDocumentContents & document)
{
    om_docid did = get_newdocid();

    record_diffs->add_record(did, document.data);

    OmDocumentContents::document_terms::const_iterator i;
    for (i = document.terms.begin(); i != document.terms.end(); i++) {
	postlist_diffs->add_posting(i->second.tname, did, i->second.wdf);
	positionlist_diffs->add_positionlist(did,
					     i->second.tname,
					     i->second.positions);
    }

    return did;
}

void
QuartzModifications::delete_document(om_docid did)
{
    throw OmUnimplementedError("QuartzModifications::delete_document() not yet implemented");
}

void
QuartzModifications::replace_document(om_docid did,
				      const OmDocumentContents & document)
{
    throw OmUnimplementedError("QuartzModifications::replace_document() not yet implemented");
}

