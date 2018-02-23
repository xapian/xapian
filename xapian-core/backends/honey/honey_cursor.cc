/** @file honey_cursor.cc
 * @brief HoneyCursor class
 */
/* Copyright (C) 2017,2018 Olly Betts
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

#include <config.h>

#include "honey_cursor.h"

#include <string>

#define DEBUGGING false

#ifdef DEBUGGING
# include <iostream>
#endif

using namespace std;

bool
HoneyCursor::next()
{
    if (is_at_end) {
	Assert(false);
	return false;
    }

    if (val_size) {
	// Skip val data we've not looked at.
	fh.skip(val_size);
	val_size = 0;
    }

    if (fh.get_pos() >= root) {
	AssertEq(fh.get_pos(), root);
	is_at_end = true;
	return false;
    }

    int ch = fh.read();
    if (ch == EOF) {
	// The root check above should mean this can't legitimately happen.
	throw Xapian::DatabaseCorruptError("EOF reading key");
    }

    size_t reuse = 0;
    if (!last_key.empty()) {
	reuse = ch;
	ch = fh.read();
	if (ch == EOF) {
	    throw Xapian::DatabaseError("EOF/error while reading key length",
					errno);
	}
    }
    size_t key_size = ch;
    char buf[256];
    fh.read(buf, key_size);
    current_key.assign(last_key, 0, reuse);
    current_key.append(buf, key_size);
    last_key = current_key;

    if (DEBUGGING) {
	string esc;
	description_append(esc, current_key);
	cerr << "K:" << esc << endl;
    }

    int r;
    {
	// FIXME: rework to take advantage of buffering that's happening
	// anyway?
	char * p = buf;
	for (int i = 0; i < 8; ++i) {
	    int ch2 = fh.read();
	    if (ch2 == EOF) {
		break;
	    }
	    *p++ = ch2;
	    if (ch2 < 128) break;
	}
	r = p - buf;
    }
    const char* p = buf;
    const char* end = p + r;
    if (!unpack_uint(&p, end, &val_size)) {
	throw Xapian::DatabaseError("val_size unpack_uint invalid");
    }
    if (p != end) abort();
    current_compressed = val_size & 1;
    val_size >>= 1;

    // FIXME: Always resize to 0?  Not doing so avoids always having to clear
    // all the data before reading it.
    if (true && val_size == 0)
	current_tag.resize(0);

    is_at_end = false;
    return true;
}

bool
HoneyCursor::read_tag(bool keep_compressed)
{
    if (val_size) {
	current_tag.resize(val_size);
	fh.read(&(current_tag[0]), val_size);
	if (DEBUGGING) {
	    cerr << "read " << val_size << " bytes of value data ending @"
		 << fh.get_pos() << endl;
	}
	val_size = 0;
	if (DEBUGGING) {
	    string esc;
	    description_append(esc, current_tag);
	    cerr << "V:" << esc << endl;
	}
    }
    if (!keep_compressed && current_compressed) {
	// Need to decompress.
	comp_stream.decompress_start();
	string new_tag;
	if (!comp_stream.decompress_chunk(current_tag.data(),
					  current_tag.size(),
					  new_tag)) {
	    // Decompression didn't complete.
	    abort();
	}
	swap(current_tag, new_tag);
	current_compressed = false;
	if (DEBUGGING) {
	    cerr << "decompressed to " << current_tag.size()
		 << "bytes of value data" << endl;
	}
    }
    return current_compressed;
}

bool
HoneyCursor::find_exact(const string& key)
{
    if (DEBUGGING) {
	string esc;
	description_append(esc, key);
	cerr << "find_exact(" << esc << ") @" << fh.get_pos() << endl;
    }

    if (rare(key.empty()))
	return false;

    bool use_index = true;
    if (!is_at_end && !current_key.empty() && current_key[0] == key[0]) {
	int cmp0 = current_key.compare(key);
	if (cmp0 == 0) return true;
	if (cmp0 < 0) {
	    // We're going forwards to a key with the same first character, so
	    // an array index won't help us.
	    use_index = false;
	}
    }

    if (use_index) {
	fh.rewind(root);
	unsigned index_type = fh.read();
	if (index_type != 0x00)
	    throw Xapian::DatabaseCorruptError("Unknown index type");
	unsigned char first = key[0] - fh.read();
	unsigned char range = fh.read();
	if (first > range)
	    return false;
	fh.skip(first * 4); // FIXME: pointer width
	off_t jump = fh.read() << 24;
	jump |= fh.read() << 16;
	jump |= fh.read() << 8;
	jump |= fh.read();
	fh.rewind(jump);
	// The jump point will be an entirely new key (because it is the first
	// key with that initial character), and we drop in as if this was the
	// first key so set last_key to be empty.
	last_key = string();
	val_size = 0;
    }

    while (next()) {
	int cmp = current_key.compare(key);
	if (cmp == 0) return true;
	if (cmp > 0) break;
    }
    return false;
}

bool
HoneyCursor::find_entry_ge(const string& key)
{
    if (DEBUGGING) {
	string esc;
	description_append(esc, key);
	cerr << "find_entry_ge(" << esc << ") @" << fh.get_pos() << endl;
    }

    if (rare(key.empty())) {
	rewind();
	return false;
    }

    bool use_index = true;
    if (!is_at_end && !last_key.empty() && last_key[0] == key[0]) {
	int cmp0 = last_key.compare(key);
	if (cmp0 == 0) {
	    current_key = last_key;
	    return true;
	}
	if (cmp0 < 0) {
	    // We're going forwards to a key with the same first character, so
	    // an array index won't help us.
	    use_index = false;
	}
    }

    if (use_index) {
	fh.rewind(root);
	unsigned index_type = fh.read();
	if (index_type != 0x00)
	    throw Xapian::DatabaseCorruptError("Unknown index type");
	unsigned char first = key[0] - fh.read();
	unsigned char range = fh.read();
	if (first > range) {
	    is_at_end = true;
	    return false;
	}
	fh.skip(first * 4); // FIXME: pointer width
	off_t jump = fh.read() << 24;
	jump |= fh.read() << 16;
	jump |= fh.read() << 8;
	jump |= fh.read();
	fh.rewind(jump);
	AssertEq(jump, fh.get_pos());
	// The jump point will be an entirely new key (because it is the first
	// key with that initial character), and we drop in as if this was the
	// first key so set last_key to be empty.
	last_key = string();
	is_at_end = false;
	val_size = 0;
    }

    while (next()) {
	int cmp = current_key.compare(key);
	if (cmp == 0) return true;
	if (cmp > 0) return false;
    }
    return false;
}

bool
HoneyCursor::prev()
{
    string key;
    if (is_at_end) {
	// To position on the last key we just do a < search for a key greater
	// than any possible key - one longer than the longest possible length
	// and consisting entirely of the highest sorting byte value.
	key.assign(HONEY_MAX_KEY_LEN + 1, '\xff');
    } else {
	if (current_key.empty())
	    return false;
	key = current_key;
    }

    // FIXME: use index - for an array index we can look at index points for
    // first characters starting with key[0] and working down.
    rewind();

    off_t pos;
    string k;
    size_t vs;
    bool compressed;
    do {
	pos = fh.get_pos();
	k = current_key;
	vs = val_size;
	compressed = current_compressed;
    } while (next() && current_key < key);

    // Back up to previous entry.
    is_at_end = false;
    last_key = current_key = k;
    val_size = vs;
    current_compressed = compressed;
    fh.set_pos(pos);

    return true;
}
