/** @file
 * @brief HoneyTable class
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

#include "honey_table.h"

#include "honey_cursor.h"
#include "stringutils.h"

#include "unicode/description_append.h"

#include <cerrno>

#ifdef DEBUGGING
# include <iostream>
#endif

using Honey::RootInfo;

using namespace std;

void
HoneyTable::create_and_open(int flags_, const RootInfo& root_info)
{
    Assert(!single_file());
    flags = flags_;
    compress_min = root_info.get_compress_min();
    if (read_only) {
	num_entries = root_info.get_num_entries();
	root = root_info.get_root();
	// FIXME: levels
    }
    if (!store.open(path, read_only))
	throw Xapian::DatabaseOpeningError("Failed to open HoneyTable", errno);
}

void
HoneyTable::open(int flags_, const RootInfo& root_info, honey_revision_number_t)
{
    flags = flags_;
    compress_min = root_info.get_compress_min();
    num_entries = root_info.get_num_entries();
    offset = root_info.get_offset();
    root = root_info.get_root();
    if (!single_file() && !store.open(path, read_only)) {
	if (!lazy)
	    throw Xapian::DatabaseOpeningError("Failed to open HoneyTable",
					       errno);
    }
    store.set_pos(offset);
}

void
HoneyTable::add(const std::string& key,
		const char* val,
		size_t val_size,
		bool compressed)
{
    if (rare(val_size == 0))
	throw Xapian::DatabaseError("HoneyTable::add() passed empty value");
    if (store.was_forced_closed())
	throw_database_closed();
    if (!compressed && compress_min > 0 && val_size > compress_min) {
	size_t compressed_size = val_size;
	CompressionStream comp_stream; // FIXME: reuse
	const char* p = comp_stream.compress(val, &compressed_size);
	if (p) {
	    add(key, p, compressed_size, true);
	    return;
	}
    }

    if (read_only)
	throw Xapian::InvalidOperationError("add() on read-only HoneyTable");
    if (key.size() == 0 || key.size() > HONEY_MAX_KEY_LENGTH)
	throw Xapian::InvalidArgumentError("Invalid key size: " +
					   str(key.size()));
    if (key <= last_key)
	throw Xapian::InvalidOperationError("New key <= previous key");
    size_t reuse = common_prefix_length(last_key, key);

#ifdef SSTINDEX_ARRAY
    if (reuse == 0) {
	index.maybe_add_entry(key, store.get_pos());
    }
#elif defined SSTINDEX_BINARY_CHOP
    // For a binary chop index, the index point is before the key info - the
    // index key must have the same N first bytes as the previous key, where
    // N >= the keep length.
    index.maybe_add_entry(key, store.get_pos());
#elif defined SSTINDEX_SKIPLIST
    // Handled below.
#else
# error SSTINDEX type not specified
#endif

    store.write(static_cast<unsigned char>(reuse));
    store.write(static_cast<unsigned char>(key.size() - reuse));
    store.write(key.data() + reuse, key.size() - reuse);
    ++num_entries;

#ifdef SSTINDEX_SKIPLIST
    // For a skiplist index, the index provides the full key, so the index
    // point is after the key at the level below.
    index.maybe_add_entry(key, store.get_pos());
#endif

    // Encode "compressed?" flag in bottom bit.
    // FIXME: Don't do this if a table is uncompressed?  That saves a byte
    // for each item where the extra bit pushes the length up by a byte.
    size_t val_size_enc = (val_size << 1) | compressed;
    std::string val_len;
    pack_uint(val_len, val_size_enc);
    // FIXME: pass together so we can potentially writev() both?
    store.write(val_len.data(), val_len.size());
    store.write(val, val_size);
    last_key = key;
}

void
HoneyTable::commit(honey_revision_number_t, RootInfo* root_info)
{
    if (root < 0)
	throw Xapian::InvalidOperationError("root not set");

    root_info->set_num_entries(num_entries);
    // offset should already be set.
    root_info->set_root(root);
    // Not really meaningful.
    // root_info->set_free_list(std::string());

    read_only = true;
    store.rewind(offset);
    last_key = string();
}

bool
HoneyTable::read_key(std::string& key,
		     size_t& val_size,
		     bool& compressed) const
{
#ifdef DEBUGGING
    {
	string desc;
	description_append(desc, key);
	cerr << "HoneyTable::read_key(" << desc << ", ...) for path=" << path
	     << endl;
    }
#endif
    if (!read_only) {
	return false;
    }

    AssertRel(store.get_pos(), >=, offset);
    if (store.get_pos() >= root) {
	AssertEq(store.get_pos(), root);
	return false;
    }
    int ch = store.read();
    if (ch == EOF) return false;

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
    key.assign(last_key, 0, reuse);
    key.append(buf, key_size);
    last_key = key;

#ifdef DEBUGGING
    if (false) {
	std::string esc;
	description_append(esc, key);
	std::cout << "K:" << esc << std::endl;
    }
#endif

    int r;
    {
	// FIXME: rework to take advantage of buffering that's happening anyway?
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
    compressed = val_size & 1;
    val_size >>= 1;
    Assert(p == end);
    return true;
}

void
HoneyTable::read_val(std::string& val, size_t val_size) const
{
    AssertRel(store.get_pos() + val_size, <=, size_t(root));
    val.resize(val_size);
    store.read(&(val[0]), val_size);
#ifdef DEBUGGING
    if (false) {
	std::string esc;
	description_append(esc, val);
	std::cout << "V:" << esc << std::endl;
    }
#endif
}

bool
HoneyTable::get_exact_entry(const std::string& key, std::string* tag) const
{
    if (!read_only) std::abort();
    if (rare(!store.is_open())) {
	if (store.was_forced_closed())
	    throw_database_closed();
	return false;
    }
    store.rewind(root);
    if (rare(key.empty()))
	return false;
    bool exact_match = false;
    bool compressed = false;
    size_t val_size = 0;
    int index_type = store.read();
    switch (index_type) {
	case EOF:
	    return false;
	case 0x00: {
	    unsigned char first =
		static_cast<unsigned char>(key[0] - store.read());
	    unsigned char range = store.read();
	    if (first > range)
		return false;
	    store.skip(first * 4); // FIXME: pointer width
	    off_t jump = store.read_uint4_be();
	    store.rewind(jump);
	    // The jump point will be an entirely new key (because it is the
	    // first key with that initial character), and we drop in as if
	    // this was the first key so set last_key to be empty.
	    last_key = string();
	    break;
	}
	case 0x01: {
	    size_t j = store.read_uint4_be();
	    if (j == 0)
		return false;
	    off_t base = store.get_pos();
	    char kkey[SSTINDEX_BINARY_CHOP_KEY_SIZE];
	    size_t kkey_len = 0;
	    size_t i = 0;
	    while (j - i > 1) {
		size_t k = i + (j - i) / 2;
		store.set_pos(base + k * SSTINDEX_BINARY_CHOP_ENTRY_SIZE);
		store.read(kkey, SSTINDEX_BINARY_CHOP_KEY_SIZE);
		kkey_len = 4;
		while (kkey_len > 0 && kkey[kkey_len - 1] == '\0') --kkey_len;
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
	    // FIXME: If "close" just seek forwards?  Or consider seeking from
	    // current index pos?
	    // off_t pos = store.get_pos();
	    string index_key, prev_index_key;
	    make_unsigned<off_t>::type ptr = 0;
	    int cmp0 = 1;
	    while (true) {
		int reuse = store.read();
		if (reuse == EOF) break;
		int len = store.read();
		if (len == EOF) abort(); // FIXME
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
	    }
#ifdef DEBUGGING
	    {
		cerr << " cmp0 = " << cmp0 << ", going to " << ptr << endl;
	    }
#endif
	    store.set_pos(ptr);

	    if (ptr != 0) {
		last_key = index_key;
		char buf[8];
		int r;
		{
		    // FIXME: rework to take advantage of buffering that's happening anyway?
		    char* p = buf;
		    for (int i = 0; i < 8; ++i) {
			int ch2 = store.read();
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
		compressed = val_size & 1;
		val_size >>= 1;
		Assert(p == end);
	    } else {
		last_key = string();
	    }

	    if (cmp0 == 0) {
		exact_match = true;
		break;
	    }

#ifdef DEBUGGING
	    {
		string desc;
		description_append(desc, last_key);
		cerr << "Dropped to data layer on key: " << desc << endl;
	    }
#endif

	    break;
	}
	default: {
	    string m = "HoneyTable: Unknown index type ";
	    m += str(index_type);
	    throw Xapian::DatabaseCorruptError(m);
	}
    }

    std::string k;
    int cmp;
    if (!exact_match) {
	do {
	    if (val_size) {
		// Skip val data we've not looked at.
		store.skip(val_size);
		val_size = 0;
	    }
	    if (!read_key(k, val_size, compressed)) return false;
	    cmp = k.compare(key);
	} while (cmp < 0);
	if (cmp > 0) return false;
    }
    if (tag != NULL) {
	if (compressed) {
	    std::string v;
	    read_val(v, val_size);
	    CompressionStream comp_stream;
	    comp_stream.decompress_start();
	    tag->resize(0);
	    if (!comp_stream.decompress_chunk(v.data(), v.size(), *tag)) {
		// Decompression didn't complete.
		abort();
	    }
	} else {
	    read_val(*tag, val_size);
	}
    }
    return true;
}

HoneyCursor*
HoneyTable::cursor_get() const
{
    if (rare(!store.is_open())) {
	if (store.was_forced_closed())
	    throw_database_closed();
	return NULL;
    }
    return new HoneyCursor(this);
}
