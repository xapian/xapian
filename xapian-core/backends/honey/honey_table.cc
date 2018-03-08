/** @file honey_table.cc
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

#include "honey_table.h"

#include "honey_cursor.h"
#include "stringutils.h"

using Honey::RootInfo;

using namespace std;

size_t HoneyTable::total_index_size = 0;

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
    if (!fh.open(path, read_only))
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
    if (!single_file() && !fh.open(path, read_only)) {
	if (!lazy)
	    throw Xapian::DatabaseOpeningError("Failed to open HoneyTable",
					       errno);
    }
    fh.set_pos(offset);
}

void
HoneyTable::add(const std::string& key,
		const char* val,
		size_t val_size,
		bool compressed)
{
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
    if (key.size() == 0 || key.size() > HONEY_MAX_KEY_LEN)
	throw Xapian::InvalidArgumentError("Invalid key size: " +
					   str(key.size()));
    if (key <= last_key)
	throw Xapian::InvalidOperationError("New key <= previous key");
    off_t index_pos = fh.get_pos();
    if (!last_key.empty()) {
	size_t reuse = common_prefix_length(last_key, key);
	fh.write(static_cast<unsigned char>(reuse));
	fh.write(static_cast<unsigned char>(key.size() - reuse));
	fh.write(key.data() + reuse, key.size() - reuse);
    } else {
	fh.write(static_cast<unsigned char>(key.size()));
	fh.write(key.data(), key.size());
    }
    ++num_entries;
#if 1 // Array
    // For an array index, the index point is right before the complete key.
    if (!last_key.empty()) ++index_pos;
#elif 0 // Binary chop
    // FIXME implement
#else
    // Skiplist
    index_pos = fh.get_pos();
#endif
    index.maybe_add_entry(key, index_pos);

    // Encode "compressed?" flag in bottom bit.
    // FIXME: Don't do this if a table is uncompressed?  That saves a byte
    // for each item where the extra bit pushes the length up by a byte.
    size_t val_size_enc = (val_size << 1) | compressed;
    std::string val_len;
    pack_uint(val_len, val_size_enc);
    // FIXME: pass together so we can potentially writev() both?
    fh.write(val_len.data(), val_len.size());
    fh.write(val, val_size);
    last_key = key;
}

void
HoneyTable::commit(honey_revision_number_t, RootInfo* root_info)
{
    if (root < 0)
	throw Xapian::InvalidOperationError("root not set");

    root_info->set_level(1); // FIXME: number of index levels
    root_info->set_num_entries(num_entries);
    root_info->set_root_is_fake(false);
    // Not really meaningful.
    root_info->set_sequential(true);
    // offset should already be set.
    root_info->set_root(root);
    // Not really meaningful.
    root_info->set_blocksize(2048);
    // Not really meaningful.
    // root_info->set_free_list(std::string());

    read_only = true;
    fh.rewind(offset);
    last_key = string();
}

bool
HoneyTable::read_key(std::string& key,
		     size_t& val_size,
		     bool& compressed) const
{
    if (!read_only) {
	return false;
    }

    AssertRel(fh.get_pos(), >=, offset);
    if (fh.get_pos() >= root) {
	AssertEq(fh.get_pos(), root);
	return false;
    }
    int ch = fh.read();
    if (ch == EOF) return false;

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
    key.assign(last_key, 0, reuse);
    key.append(buf, key_size);
    last_key = key;

    if (false) {
	std::string esc;
	description_append(esc, key);
	std::cout << "K:" << esc << std::endl;
    }

    int r;
    {
	// FIXME: rework to take advantage of buffering that's happening anyway?
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
    compressed = val_size & 1;
    val_size >>= 1;
    Assert(p == end);
    return true;
}

void
HoneyTable::read_val(std::string& val, size_t val_size) const
{
    AssertRel(fh.get_pos() + val_size, <=, size_t(root));
    val.resize(val_size);
    fh.read(&(val[0]), val_size);
    if (false) {
	std::string esc;
	description_append(esc, val);
	std::cout << "V:" << esc << std::endl;
    }
}

bool
HoneyTable::get_exact_entry(const std::string& key, std::string* tag) const
{
    if (!read_only) std::abort();
    if (!fh.is_open()) {
	if (fh.was_forced_closed())
	    throw_database_closed();
	return false;
    }
    fh.rewind(root);
    if (rare(key.empty()))
	return false;
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
    // The jump point will be an entirely new key (because it is the first key
    // with that initial character), and we drop in as if this was the first
    // key so set last_key to be empty.
    last_key = string();

    std::string k;
    bool compressed;
    int cmp;
    size_t val_size = 0;
    do {
	if (val_size) {
	    // Skip val data we've not looked at.
	    fh.skip(val_size);
	    val_size = 0;
	}
	if (!read_key(k, val_size, compressed)) return false;
	cmp = k.compare(key);
    } while (cmp < 0);
    if (cmp > 0) return false;
    if (tag != NULL) {
	if (compressed) {
	    std::string v;
	    read_val(v, val_size);
	    CompressionStream comp_stream;
	    comp_stream.decompress_start();
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
    return new HoneyCursor(fh, root, offset);
}
