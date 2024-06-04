/** @file
 * @brief Empty database internals
 */
/* Copyright (C) 2017,2024 Olly Betts
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#include <config.h>

#include "empty_database.h"

#include "backends.h"
#include "omassert.h"
#include "xapian/error.h"

#include <string_view>

using namespace std;

[[noreturn]]
static void no_subdatabases()
{
    throw Xapian::InvalidOperationError("No subdatabases");
}

EmptyDatabase::size_type
EmptyDatabase::size() const
{
    return 0;
}

void
EmptyDatabase::close()
{
}

PostList*
EmptyDatabase::open_post_list(string_view) const
{
    return NULL;
}

LeafPostList*
EmptyDatabase::open_leaf_post_list(string_view, bool) const
{
    return NULL;
}

TermList*
EmptyDatabase::open_term_list(Xapian::docid) const
{
    no_subdatabases();
}

TermList*
EmptyDatabase::open_term_list_direct(Xapian::docid) const
{
    no_subdatabases();
}

TermList*
EmptyDatabase::open_allterms(string_view) const
{
    return NULL;
}

bool
EmptyDatabase::has_positions() const
{
    return false;
}

PositionList*
EmptyDatabase::open_position_list(Xapian::docid, string_view) const
{
    no_subdatabases();
}

Xapian::doccount
EmptyDatabase::get_doccount() const
{
    return 0;
}

Xapian::docid
EmptyDatabase::get_lastdocid() const
{
    return 0;
}

Xapian::totallength
EmptyDatabase::get_total_length() const
{
    return 0;
}

void
EmptyDatabase::get_freqs(string_view term,
			 Xapian::doccount* tf_ptr,
			 Xapian::termcount* cf_ptr) const
{
    Assert(!term.empty());
    (void)term;

    if (tf_ptr)
	*tf_ptr = 0;
    if (cf_ptr)
	*cf_ptr = 0;
}

Xapian::doccount
EmptyDatabase::get_value_freq(Xapian::valueno) const
{
    return 0;
}

string
EmptyDatabase::get_value_lower_bound(Xapian::valueno) const
{
    return string();
}

string
EmptyDatabase::get_value_upper_bound(Xapian::valueno) const
{
    return string();
}

Xapian::termcount
EmptyDatabase::get_doclength_lower_bound() const
{
    return 0;
}

Xapian::termcount
EmptyDatabase::get_doclength_upper_bound() const
{
    return 0;
}

Xapian::termcount
EmptyDatabase::get_wdf_upper_bound(string_view term) const
{
    Assert(!term.empty());
    (void)term;
    return 0;
}

ValueList*
EmptyDatabase::open_value_list(Xapian::valueno) const
{
    return NULL;
}

Xapian::termcount
EmptyDatabase::get_doclength(Xapian::docid did) const
{
    Assert(did != 0);
    (void)did;
    no_subdatabases();
}

Xapian::termcount
EmptyDatabase::get_unique_terms(Xapian::docid did) const
{
    Assert(did != 0);
    (void)did;
    no_subdatabases();
}

Xapian::termcount
EmptyDatabase::get_wdfdocmax(Xapian::docid did) const
{
    Assert(did != 0);
    (void)did;
    no_subdatabases();
}

Xapian::Document::Internal*
EmptyDatabase::open_document(Xapian::docid did, bool) const
{
    Assert(did != 0);
    (void)did;
    no_subdatabases();
}

bool
EmptyDatabase::term_exists(string_view) const
{
    return false;
}

TermList*
EmptyDatabase::open_spelling_termlist(string_view) const
{
    return NULL;
}

TermList*
EmptyDatabase::open_spelling_wordlist() const
{
    return NULL;
}

Xapian::doccount
EmptyDatabase::get_spelling_frequency(string_view) const
{
    return 0;
}

TermList*
EmptyDatabase::open_synonym_termlist(string_view) const
{
    return NULL;
}

TermList*
EmptyDatabase::open_synonym_keylist(string_view) const
{
    return NULL;
}

string
EmptyDatabase::get_metadata(string_view) const
{
    return string();
}

TermList*
EmptyDatabase::open_metadata_keylist(string_view) const
{
    return NULL;
}

void
EmptyDatabase::write_changesets_to_fd(int,
				      std::string_view,
				      bool,
				      Xapian::ReplicationInfo*)
{
    throw Xapian::InvalidOperationError("write_changesets_to_fd() with "
					"no subdatabases");
}

Xapian::rev
EmptyDatabase::get_revision() const
{
    return 0;
}

void
EmptyDatabase::invalidate_doc_object(Xapian::Document::Internal*) const
{
    // This method should only be called on a single shard.
    Assert(false);
}

int
EmptyDatabase::get_backend_info(string*) const
{
    // This method should only be called on a single shard.
    Assert(false);
    return BACKEND_UNKNOWN;
}

void
EmptyDatabase::commit()
{
    no_subdatabases();
}

void
EmptyDatabase::cancel()
{
}

void
EmptyDatabase::begin_transaction(bool)
{
    no_subdatabases();
}

Xapian::docid
EmptyDatabase::add_document(const Xapian::Document&)
{
    no_subdatabases();
}

void
EmptyDatabase::delete_document(Xapian::docid)
{
    no_subdatabases();
}

void
EmptyDatabase::delete_document(string_view)
{
    no_subdatabases();
}

void
EmptyDatabase::replace_document(Xapian::docid, const Xapian::Document&)
{
    no_subdatabases();
}

Xapian::docid
EmptyDatabase::replace_document(string_view, const Xapian::Document&)
{
    no_subdatabases();
}

void
EmptyDatabase::add_spelling(string_view, Xapian::termcount) const
{
    no_subdatabases();
}

Xapian::termcount
EmptyDatabase::remove_spelling(string_view, Xapian::termcount) const
{
    no_subdatabases();
}

void
EmptyDatabase::add_synonym(string_view, string_view) const
{
    no_subdatabases();
}

void
EmptyDatabase::remove_synonym(string_view, string_view) const
{
    no_subdatabases();
}

void
EmptyDatabase::clear_synonyms(string_view) const
{
    no_subdatabases();
}

void
EmptyDatabase::set_metadata(string_view, string_view)
{
    no_subdatabases();
}

string
EmptyDatabase::get_description() const
{
    return string();
}
