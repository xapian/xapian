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

    if (fh.get_pos() == root) {
	is_at_end = true;
	return false;
    }

    int ch = fh.read();
    if (ch == EOF) {
	is_at_end = true;
	return false;
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
    if (is_at_end) {
	rewind();
    } else {
	// FIXME: use index
	int cmp0 = current_key.compare(key);
	if (cmp0 == 0) return true;
	if (cmp0 > 0) {
	    rewind();
	}
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
    if (is_at_end) {
	rewind();
    } else {
	// FIXME: use index
	int cmp0 = current_key.compare(key);
	if (cmp0 == 0) return true;
	if (cmp0 > 0) {
	    rewind();
	}
    }

    while (next()) {
	int cmp = current_key.compare(key);
	if (cmp == 0) return true;
	if (cmp > 0) return false;
    }
    is_at_end = true;
    return false;
}

bool
HoneyCursor::find_entry(const string& key)
{
    if (DEBUGGING) {
	string desc;
	description_append(desc, key);
	cerr << "find_entry(" << desc << ") [LE]" << endl;
    }
#if 0
    if (is_at_end) {
	rewind();
    } else {
	// FIXME: use index
	int cmp0 = current_key.compare(key);
	if (cmp0 == 0) return true;
	if (cmp0 > 0) {
	    rewind();
	}
    }

    off_t pos;
    string k;
    size_t vs;
    bool compressed;
    while (pos = fh.get_pos(), k = current_key, vs = val_size,
	   compressed = current_compressed, next()) {
	int cmp = current_key.compare(key);
	if (cmp == 0) return true;
	if (cmp > 0) break;
    }
    fh.set_pos(pos);
    current_key = last_key = k;
    val_size = vs;
    current_compressed = compressed;
    return false;
#else
    if (is_at_end) {
	rewind();
    } else {
	int cmp0 = current_key.compare(key);
	if (cmp0 == 0) {
	    if (DEBUGGING) cerr << " already on it" << endl;
	    return true;
	}
#if 1
	if (cmp0 > 0) {
	    rewind();
	}
#else
	// FIXME: If "close" just seek forwards?  Or consider seeking from
	// current index pos?
	// off_t pos = fh.get_pos();
	fh.set_pos(root);
	string index_key, prev_index_key;
	make_unsigned<off_t>::type ptr = 0;
	while (true) {
	    int reuse = fh.read();
	    if (reuse == -1) break;
	    int len = fh.read();
	    if (len == -1) abort(); // FIXME
	    prev_index_key = index_key;
	    index_key.resize(reuse + len);
	    fh.read(&index_key[reuse], len);

	    if (DEBUGGING) {
		string desc;
		description_append(desc, index_key);
		cerr << "Index key: " << desc << endl;
	    }

	    cmp0 = index_key.compare(key);
	    if (cmp0 > 0) break;
	    // For now (a lie, but the reuse part is correct).
	    last_key = ptr ? index_key : string();
	    char buf[8];
	    char* e = buf;
	    while (true) {
		int b = fh.read();
		*e++ = b;
		if ((b & 0x80) == 0) break;
	    }
	    const char* p = buf;
	    if (!unpack_uint(&p, e, &ptr) || p != e) abort(); // FIXME
	    if (DEBUGGING) cerr << " -> " << ptr << endl;
	    if (cmp0 == 0) {
		if (DEBUGGING)
		    cerr << " hit straight from index" << endl;
		fh.set_pos(ptr);
		current_key = index_key;
		int r;
		{
		    // FIXME: rework to take advantage of buffering that's
		    // happening anyway?
		    char * q = buf;
		    for (int i = 0; i < 8; ++i) {
			int ch2 = fh.read();
			if (ch2 == EOF) {
			    break;
			}
			*q++ = ch2;
			if (ch2 < 128) break;
		    }
		    r = q - buf;
		}
		p = buf;
		const char* end = p + r;
		if (!unpack_uint(&p, end, &val_size)) {
		    throw Xapian::DatabaseError("val_size unpack_uint "
						"invalid");
		}
		bool& compressed = current_compressed;
		compressed = val_size & 1;
		val_size >>= 1;
		if (p != end) abort();
		return true;
	    }
	}
	if (DEBUGGING)
	    cerr << " cmp0 = " << cmp0 << ", going to " << ptr << endl;
	fh.set_pos(ptr);

	// FIXME: crude for now
	if (ptr != 0) {
	    current_key = prev_index_key;
	    char buf[4096];
	    int r;
	    {
		// FIXME: rework to take advantage of buffering that's
		// happening anyway?
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
	    bool& compressed = current_compressed;
	    auto& val = current_tag;
	    compressed = val_size & 1;
	    val_size >>= 1;
	    val.assign(p, end);
	    if (p != end) abort();
	    val_size -= (end - p);
	    while (val_size) {
		size_t n = min(val_size, sizeof(buf));
		if (!fh.read(buf, n))
		    throw Xapian::DatabaseError("read of " + str(n) + "/" +
						str(val_size) + " bytes of "
						"value data failed",
						errno);
		val.append(buf, n);
		val_size -= n;
	    }
	} else {
	    if (!next()) {
		abort();
	    }
	}

	if (DEBUGGING) {
	    string desc;
	    description_append(desc, current_key);
	    cerr << "Dropped to data layer on key: " << desc << endl;
	}

	// FIXME: need to put us in the "read key not tag" state but persist
	// that more?
	// if (cmp0 == 0) this is an exact hit from the index...
#endif
    }

    off_t pos;
    string k;
    size_t vs;
    bool compressed;
    while (true) {
	pos = fh.get_pos();
	k = current_key;
	vs = val_size;
	compressed = current_compressed;

	if (DEBUGGING) {
	    string desc;
	    description_append(desc, current_key);
	    cerr << "@ " << pos << " key: " << desc << endl;
	}

	if (!next()) break;

	int cmp = current_key.compare(key);
	if (cmp == 0) {
	    if (DEBUGGING) cerr << " found it" << endl;
	    return true;
	}
	if (cmp > 0) break;
    }

    if (DEBUGGING) {
	string desc;
	description_append(desc, current_key);
	cerr << " NOT found - reached " << desc << endl;
    }
    // No match so back up to previous entry.
    is_at_end = false;
    last_key = current_key = k;
    val_size = vs;
    current_compressed = compressed;
    fh.set_pos(pos);
    if (DEBUGGING) {
	string desc;
	description_append(desc, current_key);
	cerr << " NOT found - leaving us on " << desc << endl;
    }
    return false;
#endif
}

void
HoneyCursor::find_entry_lt(const string& key)
{
    if (DEBUGGING) {
	string esc;
	description_append(esc, key);
	cerr << "find_entry_lt(" << esc << ") @" << fh.get_pos() << endl;
    }
    // FIXME: use index
    int cmp = -1;
    if (is_at_end || (cmp = key.compare(current_key)) <= 0) {
	if (cmp == 0 && rare(&key == &current_key)) {
	    // Avoid bug with this (which should step back one entry):
	    // cursor.find_entry_lt(cursor.current_key);
	    string copy = current_key;
	    find_entry_lt(copy);
	    return;
	}
	rewind();
    }

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
}
