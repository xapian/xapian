/** @file honey_table.h
 * @brief HoneyTable class
 */
/* Copyright (C) 2017 Olly Betts
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

#ifndef XAPIAN_INCLUDED_HONEY_TABLE_H
#define XAPIAN_INCLUDED_HONEY_TABLE_H

//#include "xapian/constants.h"
#include "xapian/error.h"

#include <algorithm>
#include<iostream> // FIXME

//#include <cstdio>
#include <cstdlib> // std::abort()
#include <type_traits>
#include <sys/uio.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "safeerrno.h"

#include "compression_stream.h"
#include "honey_defs.h"
#include "honey_version.h"
#include "internaltypes.h"
#include "io_utils.h"
#include "pack.h"
#include "str.h"

#include "unicode/description_append.h"

#ifdef BLK_UNUSED
# undef BLK_UNUSED
#endif // FIXME: namespace it?

const uint4 BLK_UNUSED = uint4(-1);

class HoneyFreeListChecker;

class BufferedFile {
    int fd = -1;
    mutable off_t pos = 0;
    bool read_only = true;
    mutable size_t buf_end = 0;
    mutable char buf[4096];

  public:
    BufferedFile() { }

    BufferedFile(const BufferedFile& o) : fd(o.fd) {
	if (!o.read_only) std::abort();
#if 0
	if (o.buf_end) {
	    buf_end = o.buf_end;
	    std::memcpy(buf, o.buf, buf_end);
	}
#endif
    }

    ~BufferedFile() {
//	if (fd >= 0) ::close(fd);
    }

    void close() {
	if (fd >= 0) {
	    ::close(fd);
	    fd = -1;
	}
    }

    bool is_open() const { return fd >= 0; }

    bool open(const std::string& path, bool read_only_) {
//	if (fd >= 0) ::close(fd);
	read_only = read_only_;
	if (read_only) {
	    // FIXME: add new io_open_stream_rd() etc?
	    fd = io_open_block_rd(path);
	} else {
	    fd = io_open_block_wr(path, true); // FIXME: Always create anew for now...
	}
	return fd >= 0;
    }

    off_t get_pos() const {
	return read_only ?
	    pos - buf_end :
	    pos + buf_end;
    }

    void set_pos(off_t pos_) const {
	if (!read_only) std::abort();
	if (false && pos_ >= pos) { // FIXME: need to take buf_end into account
	    skip(pos_ - pos);
	} else {
	    // FIXME: salvage some of the buffer if we can?
	    buf_end = 0;
	    pos = pos_;
	}
    }

    void skip(size_t delta) const {
	if (!read_only) std::abort();
	// Keep any buffered data we can.
	if (delta > buf_end) {
	    pos -= buf_end;
	    pos += delta;
	    buf_end = 0;
	} else {
	    buf_end -= delta;
	}
    }

#if 0
    bool empty() const {
	if (buf_end) return false;
	struct stat sbuf;
	if (fd == -1 || fstat(fd, &sbuf) < 0) return true;
	return (sbuf.st_size == 0);
    }
#endif

    void write(unsigned char ch) {
	if (buf_end == sizeof(buf)) {
	    // writev()?
	    if (::write(fd, buf, buf_end)) {
		// FIXME: retry short write
	    }
	    buf_end = 0;
	}
	buf[buf_end++] = ch;
    }

    void write(const char* p, size_t len) {
	if (buf_end + len <= sizeof(buf)) {
	    memcpy(buf + buf_end, p, len);
	    buf_end += len;
	    return;
	}

	while (true) {
	    struct iovec iov[2];
	    iov[0].iov_base = buf;
	    iov[0].iov_len = buf_end;
	    iov[1].iov_base = const_cast<char*>(p);
	    iov[1].iov_len = len;
	    ssize_t n_ = writev(fd, iov, 2);
	    if (n_ < 0) std::abort();
	    size_t n = n_;
	    if (n == buf_end + len) {
		// Wrote everything.
		buf_end = 0;
		return;
	    }
	    if (n >= buf_end) {
		// Wrote all of buf.
		n -= buf_end;
		p += n;
		len -= n;
		if (::write(fd, p, len)) {
		    // FIXME: retry short write
		}
		buf_end = 0;
		return;
	    }
	    buf_end -= n;
	    memmove(buf, buf + n, buf_end);
	}
    }

    int read() const {
#if 1
	if (buf_end == 0) {
retry:
	    ssize_t r = pread(fd, buf, sizeof(buf), pos);
	    if (r == 0) return EOF;
	    if (r < 0) {
		if (errno == EINTR) goto retry;
		throw Xapian::DatabaseError("pread failed", errno);
	    }
	    pos += r;
	    buf_end = r;
	}
	return static_cast<unsigned char>(buf[sizeof(buf) - buf_end--]);
#else
	unsigned char ch;
	if (pread(fd, &ch, 1, pos) != 1)
	    return EOF;
	++pos;
	return ch;
#endif
    }

    bool read(char* p, size_t len) const {
#if 1
	if (buf_end != 0) {
	    if (len <= buf_end) {
		memcpy(p, buf + sizeof(buf) - buf_end, len);
		buf_end -= len;
		return true;
	    }
	    memcpy(p, buf + sizeof(buf) - buf_end, buf_end);
	    p += buf_end;
	    len -= buf_end;
	    buf_end = 0;
	}
	// FIXME: refill buffer if len < sizeof(buf)
#endif
	ssize_t r = pread(fd, p, len, pos);
	if (r > 0) pos += r;
	return r == ssize_t(len);
    }

    void flush() {
	if (!read_only && buf_end) {
	    if (::write(fd, buf, buf_end)) {
		// FIXME: retry short write
	    }
	    buf_end = 0;
	}
    }

    void sync() {
	io_sync(fd);
    }

    void rewind() {
	read_only = true;
	pos = 0;
	buf_end = 0;
    }
};

class HoneyCursor;

class SSIndex {
    std::string data;
    size_t block = 0;
    size_t n_index = 0;
    std::string last_index_key;
    // Put an index entry every this much:
    // FIXME: tune - seems 64K is common elsewhere
    enum { INDEXBLOCK = 1024 };
    SSIndex* parent_index = NULL;

  public:
    SSIndex() { }

    void maybe_add_entry(const std::string& key, off_t ptr) {
	size_t cur_block = ptr / INDEXBLOCK;
	if (cur_block == block)
	    return;

	size_t len = std::min(last_index_key.size(), key.size());
	size_t reuse;
	for (reuse = 0; reuse < len; ++reuse) {
	    if (last_index_key[reuse] != key[reuse]) break;
	}

	data += char(reuse);
	data += char(key.size() - reuse);
	data.append(key, reuse, key.size() - reuse);
	pack_uint(data, static_cast<std::make_unsigned<off_t>::type>(ptr));

	block = cur_block;
	last_index_key = key;

	++n_index;

	// FIXME: deal with parent_index...

	// FIXME: constant width entries would allow binary chop, but take a
	// lot more space.  could impose max key width and just insert based on
	// that, but still more space than storing key by length.  Or "SKO" -
	// fixed width entry which encodes variable length pointer and key with
	// short keys in the entry and long keys pointed to (or prefix included
	// and rest pointed to).
    }

    off_t write(BufferedFile& fh) {
	off_t root = fh.get_pos();
	fh.write(data.data(), data.size());
	// FIXME: parent stuff...
	return root;
    }

    size_t size() const {
	size_t s = data.size();
	if (parent_index) s += parent_index->size();
	return s;
    }

    size_t get_num_entries() const { return n_index; }
};

class HoneyCursor;
class MutableHoneyCursor;

class HoneyTable {
    friend class HoneyCursor; // Allow access to fh.  FIXME cleaner way?
    friend class MutableHoneyCursor; // Allow access to fh.  FIXME cleaner way?

    std::string path;
    bool read_only;
    int flags;
    uint4 compress_min;
    mutable BufferedFile fh;
    mutable std::string last_key;
    SSIndex index;
    off_t root = -1;
    honey_tablesize_t num_entries;
    bool lazy;

  public:
    HoneyTable(const char*, const std::string& path_, bool read_only_, bool lazy_ = false)
	: path(path_ + HONEY_TABLE_EXTENSION),
	  read_only(read_only_),
	  num_entries(0),
	  lazy(lazy_)
    {
    }

    HoneyTable(const char*, int fd, off_t offset, bool read_only_, bool lazy_ = false)
    {
	(void)fd;
	(void)offset;
	(void)read_only_;
	(void)lazy_;
	std::abort();
    }

    static size_t total_index_size;

    ~HoneyTable() {
#if 0
	size_t index_size = index.size();
	total_index_size += index_size;
	if (index_size)
	    std::cout << "*** " << path << " - index " << index_size << " for " << index.get_num_entries() << " entries; total_size = " << total_index_size << std::endl;
#endif
	fh.close();
    }

    bool is_writable() const { return !read_only; }

    int get_flags() const { return flags; }

    void set_full_compaction(bool) { }

    void set_max_item_size(unsigned) { }

    void create_and_open(int flags_, const Honey::RootInfo& root_info);

    void open(int flags_, const Honey::RootInfo& root_info, honey_revision_number_t);

    void close(bool permanent) {
	(void)permanent;
	fh.close();
    }

    const std::string& get_path() const { return path; }

    void add(const std::string& key,
	     const char* val,
	     size_t val_size,
	     bool compressed = false);

    void add(const std::string& key,
	     const std::string& val,
	     bool compressed = false) {
	add(key, val.data(), val.size(), compressed);
    }

    void flush_db() {
	root = index.write(fh);
	fh.flush();
    }

    void cancel(const Honey::RootInfo&, honey_revision_number_t) {
	std::abort();
    }

    void commit(honey_revision_number_t, Honey::RootInfo* root_info);

    bool sync() {
	fh.sync();
	return true;
    }

    bool empty() const {
	return num_entries == 0;
    }

    bool read_item(std::string& key, std::string& val, bool& compressed) const;

    bool get_exact_entry(const std::string& key, std::string& tag) const;

    bool key_exists(const std::string& key) const;

    bool del(const std::string&) {
	std::abort();
    }

    // readahead probably not useful?  (FIXME)
    bool readahead_key(const std::string&) const { return false; }

    bool is_modified() const { return !read_only && !empty(); }

    HoneyCursor* cursor_get() const;

    bool exists() const {
	struct stat sbuf;
	return stat(path.c_str(), &sbuf) == 0;
    }

    bool is_open() const { return fh.is_open(); }

    void set_changes(HoneyChanges*) { }

    static void throw_database_closed() { throw Xapian::DatabaseError("Closed!"); }

    honey_tablesize_t get_entry_count() const { return num_entries; }

    off_t get_root() const { return root; }
};

#endif // XAPIAN_INCLUDED_HONEY_TABLE_H
