/** @file
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

//#define DEBUGGING

#include "honey_cursor.h"

#include <cerrno>
#include <string>

#ifdef DEBUGGING
# include <iostream>
#endif

using namespace std;

bool
HoneyCursor::do_next()
{
    if (is_at_end) {
	Assert(false);
	return false;
    }

    if (val_size) {
	// Skip val data we've not looked at.
	store.skip(val_size);
	val_size = 0;
    }

    if (store.get_pos() >= root) {
	AssertEq(store.get_pos(), root);
	is_at_end = true;
	return false;
    }

    int ch = store.read();
    if (ch == EOF) {
	// The root check above should mean this can't legitimately happen.
	throw Xapian::DatabaseCorruptError("EOF reading key");
    }

    size_t reuse = ch;
    if (reuse > last_key.size()) {
	throw Xapian::DatabaseCorruptError("Reuse > previous key size");
    }
    ch = store.read();
    if (ch == EOF) {
	throw Xapian::DatabaseError("EOF/error while reading key length",
				    errno);
    }
    size_t key_size = ch;
    char buf[256];
    store.read(buf, key_size);
    current_key.assign(last_key, 0, reuse);
    current_key.append(buf, key_size);
    last_key = current_key;

#ifdef DEBUGGING
    {
	string esc;
	description_append(esc, current_key);
	cerr << "K:" << esc << endl;
    }
#endif

    return next_from_index();
}

bool
HoneyCursor::next_from_index()
{
    char buf[8];
    int r;
    {
	// FIXME: rework to take advantage of buffering that's happening
	// anyway?
	char* p = buf;
	for (int i = 0; i < 8; ++i) {
	    int ch2 = store.read();
	    if (ch2 == EOF) {
		break;
	    }
	    *p++ = char(ch2);
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
	if (store.was_forced_closed()) {
	    HoneyTable::throw_database_closed();
	}

	current_tag.resize(val_size);
	store.read(&(current_tag[0]), val_size);
#ifdef DEBUGGING
	{
	    cerr << "read " << val_size << " bytes of value data ending @"
		 << store.get_pos() << endl;
	    string esc;
	    description_append(esc, current_tag);
	    cerr << "V:" << esc << endl;
	}
#endif
	val_size = 0;
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
#ifdef DEBUGGING
	{
	    cerr << "decompressed to " << current_tag.size()
		 << "bytes of value data" << endl;
	}
#endif
    }
    return current_compressed;
}

bool
HoneyCursor::do_find(const string& key, bool greater_than)
{
    // FIXME: Actually use this!
    (void)greater_than;

#ifdef DEBUGGING
    {
	string esc;
	description_append(esc, key);
	cerr << "do_find(" << esc << ", " << greater_than << ") @"
	     << store.get_pos() << endl;
    }
#endif

    if (key.empty()) {
	rewind();
	do_next();
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

    if (store.was_forced_closed()) {
	HoneyTable::throw_database_closed();
    }

    if (use_index) {
	store.rewind(root);
	int index_type = store.read();
	switch (index_type) {
	    case EOF:
		return false;
	    case 0x00: {
		unsigned char first = key[0] - store.read();
		unsigned char range = store.read();
		if (first > range) {
		    is_at_end = true;
		    return false;
		}
		store.skip(first * 4); // FIXME: pointer width
		off_t jump = store.read_uint4_be();
		store.rewind(jump);
		// The jump point will be an entirely new key (because it is
		// the first key with that initial character), and we drop in
		// as if this was the first key so set last_key to be empty.
		last_key = string();
		break;
	    }
	    case 0x01: {
		size_t j = store.read_uint4_be();
		if (j == 0) {
		    is_at_end = true;
		    return false;
		}
		off_t base = store.get_pos();
		char kkey[SSTINDEX_BINARY_CHOP_KEY_SIZE];
		size_t kkey_len = 0;
		size_t i = 0;
		while (j - i > 1) {
		    size_t k = i + (j - i) / 2;
		    store.set_pos(base + k * SSTINDEX_BINARY_CHOP_ENTRY_SIZE);
		    store.read(kkey, SSTINDEX_BINARY_CHOP_KEY_SIZE);
		    kkey_len = 4;
		    while (kkey_len > 0 && kkey[kkey_len - 1] == '\0')
			--kkey_len;
		    int r = key.compare(0, SSTINDEX_BINARY_CHOP_KEY_SIZE,
					kkey, kkey_len);
		    if (r < 0) {
			j = k;
		    } else {
			i = k;
			if (r == 0) {
			    break;
			}
		    }
		}
		store.set_pos(base + i * SSTINDEX_BINARY_CHOP_ENTRY_SIZE);
		store.read(kkey, SSTINDEX_BINARY_CHOP_KEY_SIZE);
		kkey_len = 4;
		while (kkey_len > 0 && kkey[kkey_len - 1] == '\0') --kkey_len;
		off_t jump = store.read_uint4_be();
		store.rewind(jump);
		// The jump point is to the first key with prefix kkey, so will
		// work if we set last key to kkey.  Unless we're jumping to the
		// start of the table, in which case last_key needs to be empty.
		last_key.assign(kkey, jump == 0 ? 0 : kkey_len);
		break;
	    }
	    case 0x02: {
		// FIXME: If "close" just seek forwards?  Or consider seeking
		// from current index pos?
		// off_t pos = store.get_pos();
		string index_key, prev_index_key;
		make_unsigned<off_t>::type ptr = 0;
		int cmp0 = 1;
#ifdef DEBUGGING
		{
		    cerr << "Using skiplist index\n";
		}
#endif
		while (true) {
		    int reuse = store.read();
		    if (reuse == EOF) break;
		    int len = store.read();
		    if (len == EOF) abort(); // FIXME
#ifdef DEBUGGING
		    {
			cerr << "reuse = " << reuse << " len = " << len << endl;
		    }
#endif
		    index_key.resize(reuse + len);
		    store.read(&index_key[reuse], len);

#ifdef DEBUGGING
		    {
			string desc;
			description_append(desc, index_key);
			cerr << "Index key: " << desc << endl;
		    }
#endif

		    cmp0 = index_key.compare(key);
		    if (cmp0 > 0) {
			index_key = prev_index_key;
			break;
		    }
		    char buf[8];
		    char* e = buf;
		    while (true) {
			int b = store.read();
			*e++ = b;
			if ((b & 0x80) == 0) break;
		    }
		    const char* p = buf;
		    if (!unpack_uint(&p, e, &ptr) || p != e) abort(); // FIXME
#ifdef DEBUGGING
		    {
			cerr << " -> " << ptr << endl;
		    }
#endif
		    if (cmp0 == 0)
			break;
		    prev_index_key = index_key;
#ifdef DEBUGGING
		    {
			string desc;
			description_append(desc, prev_index_key);
			cerr << "prev_index_key -> " << desc << endl;
		    }
#endif
		}
#ifdef DEBUGGING
		{
		    string desc;
		    description_append(desc, index_key);
		    cerr << " index_key = " << desc << ", cmp0 = " << cmp0
			 << ", going to " << ptr << endl;
		}
#endif
		store.set_pos(ptr);

		if (ptr != 0) {
		    last_key = current_key = index_key;
		    bool res = next_from_index();
		    (void)res;
		    Assert(res);
		    if (cmp0 == 0) {
			Assert(ptr != 0);
			return true;
		    }
		    store.skip(val_size);
		} else {
		    last_key = current_key = string();
		}

#ifdef DEBUGGING
		{
		    string desc;
		    description_append(desc, current_key);
		    cerr << "cmp0 was " << cmp0
			 << ", Dropped to data layer on key: " << desc << endl;
		}
#endif

		break;
	    }
	    default: {
		string m = "HoneyCursor: Unknown index type ";
		m += str(index_type);
		throw Xapian::DatabaseCorruptError(m);
	    }
	}
	is_at_end = false;
	val_size = 0;
    }

    while (do_next()) {
	int cmp = current_key.compare(key);
	if (cmp == 0) return true;
	if (cmp > 0) break;
    }
    return false;
}

bool
HoneyCursor::prev()
{
    if (store.was_forced_closed()) {
	HoneyTable::throw_database_closed();
    }

    string key;
    if (is_at_end) {
	// To position on the last key we just do a < search for a key greater
	// than any possible key - one longer than the longest possible length
	// and consisting entirely of the highest sorting byte value.
	key.assign(HONEY_MAX_KEY_LENGTH + 1, '\xff');
    } else {
	if (current_key.empty())
	    return false;
	key = current_key;
    }

    // FIXME: use index - for an array index we can look at index points for
    // first characters starting with key[0] and working down; for a binary
    // chop index we can start at the entry including the current key, or the
    // one before if this is the first key for that index entry; for a skiplist
    // index we can find the previous entry at the index level above.
    rewind();

    off_t pos;
    string k;
    size_t vs;
    bool compressed;
    do {
	pos = store.get_pos();
	k = current_key;
	vs = val_size;
	compressed = current_compressed;
    } while (do_next() && current_key < key);

    // Back up to previous entry.
    is_at_end = false;
    last_key = current_key = k;
    val_size = vs;
    current_compressed = compressed;
    store.set_pos(pos);

    return true;
}
