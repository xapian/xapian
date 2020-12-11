/** @file
 * @brief Btree cursor implementation
 */
/* Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002,2003,2004,2005,2006,2007,2008,2009,2010,2012,2013,2015,2016 Olly Betts
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301
 * USA
 */

#include <config.h>

#include "glass_cursor.h"

#include <xapian/error.h>

#include "glass_table.h"
#include "debuglog.h"
#include "omassert.h"

using namespace Glass;

#ifdef XAPIAN_DEBUG_LOG
static string
hex_display_encode(const string & input)
{
    const char * table = "0123456789abcdef";
    string result;
    for (string::const_iterator i = input.begin(); i != input.end(); ++i) {
	unsigned char val = *i;
	result += "\\x";
	result += table[val / 16];
	result += table[val % 16];
    }

    return result;
}
#endif

#define DIR_START        11

GlassCursor::GlassCursor(const GlassTable *B_, const Glass::Cursor * C_)
	: is_positioned(false),
	  is_after_end(false),
	  tag_status(UNREAD),
	  B(B_),
	  version(B_->cursor_version),
	  level(B_->level)
{
    B->cursor_created_since_last_modification = true;
    C = new Glass::Cursor[level + 1];
    if (!C_) C_ = B->C;
    for (int j = 0; j <= level; ++j) {
	C[j].clone(C_[j]);
    }
}

void
GlassCursor::rebuild()
{
    int new_level = B->level;
    if (new_level <= level) {
	for (int j = 0; j < new_level; ++j) {
	    C[j].clone(B->C[j]);
	}
	for (int j = new_level; j <= level; ++j) {
	    C[j].destroy();
	}
    } else {
	Cursor * old_C = C;
	C = new Cursor[new_level + 1];
	for (int i = 0; i < level; ++i) {
	    C[i].swap(old_C[i]);
	    C[i].clone(B->C[i]);
	}
	delete [] old_C;
	for (int j = level; j < new_level; ++j) {
	    C[j].init(B->block_size);
	}
    }
    level = new_level;
    C[level].clone(B->C[level]);
    version = B->cursor_version;
    B->cursor_created_since_last_modification = true;
}

GlassCursor::~GlassCursor()
{
    // We must not use B here, as it may have already been destroyed.
    delete [] C;
}

bool
GlassCursor::next()
{
    LOGCALL(DB, bool, "GlassCursor::next", NO_ARGS);
    Assert(!is_after_end);
    if (B->cursor_version != version) {
	// find_entry() will call rebuild().
	(void)find_entry(current_key);
	// If the key was found, we're now pointing to it, otherwise we're
	// pointing to the entry before.  Either way, we now want to move to
	// the next key.
    }
    if (tag_status == UNREAD || tag_status == UNREAD_ON_LAST_CHUNK) {
	while (true) {
	    if (!B->next(C, 0)) {
		is_positioned = false;
		break;
	    }
	    if (tag_status == UNREAD_ON_LAST_CHUNK ||
		LeafItem(C[0].get_p(), C[0].c).first_component()) {
		is_positioned = true;
		break;
	    }
	}
    }

    if (!is_positioned) {
	is_after_end = true;
	RETURN(false);
    }

    get_key(&current_key);
    tag_status = UNREAD;

    LOGLINE(DB, "Moved to entry: key=" << hex_display_encode(current_key));
    RETURN(true);
}

bool
GlassCursor::find_entry(const string &key)
{
    LOGCALL(DB, bool, "GlassCursor::find_entry", key);
    if (B->cursor_version != version) {
	rebuild();
    }

    is_after_end = false;

    bool found;

    is_positioned = true;
    if (key.size() > GLASS_BTREE_MAX_KEY_LEN) {
	// Can't find key - too long to possibly be present, so find the
	// truncated form but ignore "found".
	B->form_key(key.substr(0, GLASS_BTREE_MAX_KEY_LEN));
	(void)(B->find(C));
	found = false;
    } else {
	B->form_key(key);
	found = B->find(C);
    }

    if (found) {
	tag_status = UNREAD;
	current_key = key;
    } else {
	// Be lazy about stepping back to the first chunk, as we may never be
	// asked to read the tag.
	tag_status = UNREAD_ON_LAST_CHUNK;
	if (C[0].c < DIR_START) {
	    // It would be nice to be lazy about this too, but we need to
	    // be on an actual entry in order to read the key.
	    C[0].c = DIR_START;
	    if (!B->prev(C, 0)) {
		tag_status = UNREAD;
	    }
	}
	get_key(&current_key);
    }

    LOGLINE(DB, "Found entry: key=" << hex_display_encode(current_key));
    RETURN(found);
}

void
GlassCursor::find_entry_lt(const string &key)
{
    LOGCALL_VOID(DB, "GlassCursor::find_entry_lt", key);
    if (!find_entry(key)) {
	// The entry wasn't found, so find_entry() left us on the entry before
	// the one we asked for and we're done.
	return;
    }

    Assert(!is_after_end);
    Assert(is_positioned);

    if (!B->prev(C, 0)) {
	is_positioned = false;
	return;
    }
    tag_status = UNREAD_ON_LAST_CHUNK;
    get_key(&current_key);

    LOGLINE(DB, "Found entry: key=" << hex_display_encode(current_key));
}

bool
GlassCursor::find_exact(const string &key)
{
    LOGCALL(DB, bool, "GlassCursor::find_exact", key);
    is_after_end = false;
    is_positioned = false;
    if (rare(key.size() > GLASS_BTREE_MAX_KEY_LEN)) {
	// There can't be a match
	RETURN(false);
    }

    if (B->cursor_version != version) {
	rebuild();
    }

    B->form_key(key);
    if (!B->find(C)) {
	RETURN(false);
    }
    current_key = key;
    B->read_tag(C, &current_tag, false);

    RETURN(true);
}

bool
GlassCursor::find_entry_ge(const string &key)
{
    LOGCALL(DB, bool, "GlassCursor::find_entry_ge", key);
    if (B->cursor_version != version) {
	rebuild();
    }

    is_after_end = false;

    bool found;

    is_positioned = true;
    if (key.size() > GLASS_BTREE_MAX_KEY_LEN) {
	// Can't find key - too long to possibly be present, so find the
	// truncated form but ignore "found".
	B->form_key(key.substr(0, GLASS_BTREE_MAX_KEY_LEN));
	(void)(B->find(C));
	found = false;
    } else {
	B->form_key(key);
	found = B->find(C);
    }

    if (found) {
	current_key = key;
    } else {
	if (!B->next(C, 0)) {
	    is_after_end = true;
	    is_positioned = false;
	    RETURN(false);
	}
	Assert(LeafItem(C[0].get_p(), C[0].c).first_component());
	get_key(&current_key);
    }
    tag_status = UNREAD;

    LOGLINE(DB, "Found entry: key=" << hex_display_encode(current_key));
    RETURN(found);
}

void
GlassCursor::get_key(string * key) const
{
    Assert(B->level <= level);
    Assert(is_positioned);

    (void)LeafItem(C[0].get_p(), C[0].c).key().read(key);
}

bool
GlassCursor::read_tag(bool keep_compressed)
{
    LOGCALL(DB, bool, "GlassCursor::read_tag", keep_compressed);
    if (tag_status == UNREAD_ON_LAST_CHUNK) {
	// Back up to first chunk of this tag.
	while (!LeafItem(C[0].get_p(), C[0].c).first_component()) {
	    if (!B->prev(C, 0)) {
		is_positioned = false;
		throw Xapian::DatabaseCorruptError("find_entry failed to find any entry at all!");
	    }
	}
	tag_status = UNREAD;
    }
    if (tag_status == UNREAD) {
	Assert(B->level <= level);
	Assert(is_positioned);

	if (B->read_tag(C, &current_tag, keep_compressed)) {
	    tag_status = COMPRESSED;
	} else {
	    tag_status = UNCOMPRESSED;
	}

	// We need to call B->next(...) after B->read_tag(...) so that the
	// cursor ends up on the next key.
	is_positioned = B->next(C, 0);

	LOGLINE(DB, "tag=" << hex_display_encode(current_tag));
    }
    RETURN(tag_status == COMPRESSED);
}

bool
MutableGlassCursor::del()
{
    Assert(!is_after_end);

    // MutableGlassCursor is only constructable from a non-const GlassTable*
    // but we store that in the const GlassTable* "B" member of the GlassCursor
    // class to avoid duplicating storage.  So we know it is safe to cast away
    // that const again here.
    (const_cast<GlassTable*>(B))->del(current_key);

    // If we're iterating an older revision of the tree, then the deletion
    // happens in a new (uncommitted) revision and the cursor still sees
    // the deleted key.  But if we're iterating the new uncommitted revision
    // then the deleted key is no longer visible.  We need to handle both
    // cases - either find_entry_ge() finds the deleted key or not.
    if (!find_entry_ge(current_key)) return is_positioned;
    return next();
}
