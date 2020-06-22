/** @file honey_table.h
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

#ifndef XAPIAN_INCLUDED_HONEY_TABLE_H
#define XAPIAN_INCLUDED_HONEY_TABLE_H

#ifndef PACKAGE
# error config.h must be included first in each C++ source file
#endif

#define SSTINDEX_ARRAY
//#define SSTINDEX_BINARY_CHOP
//#define SSTINDEX_SKIPLIST

#define SSTINDEX_BINARY_CHOP_KEY_SIZE 4
#define SSTINDEX_BINARY_CHOP_PTR_SIZE 4
#define SSTINDEX_BINARY_CHOP_ENTRY_SIZE \
    (SSTINDEX_BINARY_CHOP_KEY_SIZE + SSTINDEX_BINARY_CHOP_PTR_SIZE)

//#include "xapian/constants.h"
#include "xapian/error.h"

#include <algorithm>
#if 0
#include <iostream> // FIXME
#endif

#include <cstdio> // For EOF
#include <cstdlib> // std::abort()
#include <type_traits>
#ifdef HAVE_SYS_UIO_H
# include <sys/uio.h>
#endif

#include <sys/types.h>
#include "safesysstat.h"
#include "safeunistd.h"

#include "compression_stream.h"
#include "honey_defs.h"
#include "honey_version.h"
#include "internaltypes.h"
#include "io_utils.h"
#include "pack.h"
#include "str.h"
#include "stringutils.h"
#include "wordaccess.h"

#include "unicode/description_append.h"

#ifdef BLK_UNUSED
# undef BLK_UNUSED
#endif // FIXME: namespace it?

const uint4 BLK_UNUSED = uint4(-1);

class HoneyFreeListChecker;

class BufferedFileCommon {
    friend class BufferedFile;
    int fd = -1;
    unsigned _refs = 0;
    off_t offset = 0;

    BufferedFileCommon(int fd_, off_t offset_)
	: fd(fd_), _refs(1), offset(offset_) {}

    BufferedFileCommon(const BufferedFileCommon&) = delete;

    BufferedFileCommon& operator=(const BufferedFileCommon&) = delete;
};

class BufferedFile {
    BufferedFileCommon* common = nullptr;

    mutable off_t pos = 0;
    bool read_only = true;
    mutable size_t buf_end = 0;
    mutable char buf[4096];

    const int FORCED_CLOSE = -2;

  public:
    BufferedFile() { }

    BufferedFile(const BufferedFile& o) : common(o.common) {
	if (!o.read_only) std::abort();
	if (common) ++common->_refs;
#if 0
	if (o.buf_end) {
	    buf_end = o.buf_end;
	    std::memcpy(buf, o.buf, buf_end);
	}
#endif
    }

    BufferedFile(int fd_, off_t offset_, off_t pos_, bool read_only_)
	: common(new BufferedFileCommon(fd_, offset_)),
	  pos(pos_), read_only(read_only_) {}

    ~BufferedFile() {
	if (common && --common->_refs == 0)
	    delete common;
    }

    off_t get_offset() const { return common->offset; }

    void close(bool fd_owned) {
	if (common && common->fd >= 0) {
	    if (fd_owned) ::close(common->fd);
	    common->fd = -1;
	}
    }

    void force_close(bool fd_owned) {
	if (common) {
	    if (fd_owned && common->fd >= 0) ::close(common->fd);
	    common->fd = FORCED_CLOSE;
	}
    }

    bool is_open() const { return common && common->fd >= 0; }

    bool was_forced_closed() const {
	return common && common->fd == FORCED_CLOSE;
    }

    bool open(const std::string& path, bool read_only_) {
	if (common && --common->_refs == 0)
	    delete common;
	common = nullptr;
	read_only = read_only_;
	int fd;
	if (read_only) {
	    // FIXME: add new io_open_stream_rd() etc?
	    fd = io_open_block_rd(path);
	} else {
	    // FIXME: Always create anew for now...
	    fd = io_open_block_wr(path, true);
	}
	if (fd < 0) return false;
	common = new BufferedFileCommon(fd, 0);
	return true;
    }

    off_t get_pos() const {
	return read_only ? pos - buf_end : pos + buf_end;
    }

    void set_pos(off_t pos_) {
	if (!read_only) flush();
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
	    io_write(common->fd, buf, buf_end);
	    pos += buf_end;
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

#ifdef HAVE_WRITEV
	pos += buf_end + len;
	while (true) {
	    struct iovec iov[2];
	    iov[0].iov_base = buf;
	    iov[0].iov_len = buf_end;
	    iov[1].iov_base = const_cast<char*>(p);
	    iov[1].iov_len = len;
	    ssize_t n_ = writev(common->fd, iov, 2);
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
		io_write(common->fd, p, len);
		buf_end = 0;
		return;
	    }
	    buf_end -= n;
	    memmove(buf, buf + n, buf_end);
	}
#else
	io_write(common->fd, buf, buf_end);
	pos += buf_end;
	if (len >= sizeof(buf)) {
	    // If it's bigger than our buffer, just write it directly.
	    io_write(common->fd, p, len);
	    pos += len;
	    buf_end = 0;
	    return;
	}
	memcpy(buf, p, len);
	buf_end = len;
#endif
    }

    int read() const {
	if (buf_end == 0) {
	    // The buffer is currently empty, so we need to read at least one
	    // byte.
	    size_t r = io_pread(common->fd, buf, sizeof(buf), pos, 0);
	    if (r < sizeof(buf)) {
		if (r == 0) {
		    return EOF;
		}
		memmove(buf + sizeof(buf) - r, buf, r);
	    }
	    pos += r;
	    buf_end = r;
	}
	return static_cast<unsigned char>(buf[sizeof(buf) - buf_end--]);
    }

    uint4 read_uint4_be() const {
	uint4 res = read() << 24;
	res |= read() << 16;
	res |= read() << 8;
	res |= read();
	return res;
    }

    void read(char* p, size_t len) const {
	if (buf_end != 0) {
	    if (len <= buf_end) {
		memcpy(p, buf + sizeof(buf) - buf_end, len);
		buf_end -= len;
		return;
	    }
	    memcpy(p, buf + sizeof(buf) - buf_end, buf_end);
	    p += buf_end;
	    len -= buf_end;
	    buf_end = 0;
	}
	// FIXME: refill buffer if len < sizeof(buf)
	size_t r = io_pread(common->fd, p, len, pos + common->offset, len);
	// io_pread() should throw an exception if it read < len bytes.
	AssertEq(r, len);
	pos += r;
    }

    void flush() {
	if (!read_only && buf_end) {
	    io_write(common->fd, buf, buf_end);
	    pos += buf_end;
	    buf_end = 0;
	}
    }

    void sync() {
	io_sync(common->fd);
    }

    void rewind(off_t start) {
	read_only = true;
	pos = start;
	buf_end = 0;
    }
};

class HoneyCursor;

class SSTIndex {
    std::string data;
#if defined SSTINDEX_BINARY_CHOP
    size_t block = size_t(-1);
#elif defined SSTINDEX_SKIPLIST
    size_t block = 0;
#endif
#if defined SSTINDEX_BINARY_CHOP || defined SSTINDEX_SKIPLIST
    std::string last_index_key;
#endif
    // Put an index entry every this much:
    // FIXME: tune - seems 64K is common elsewhere
    enum { INDEXBLOCK = 4096 };
    SSTIndex* parent_index = NULL;

#ifdef SSTINDEX_ARRAY
    unsigned char first, last = static_cast<unsigned char>(-1);
    off_t* pointers = NULL;
#endif

  public:
    SSTIndex() {
#ifdef SSTINDEX_ARRAY
	// Header added in write() method.
#elif defined SSTINDEX_BINARY_CHOP
	data.assign(5, '\x01');
#elif defined SSTINDEX_SKIPLIST
	data.assign(1, '\x02');
#else
# error SSTINDEX type not specified
#endif
    }

    ~SSTIndex() {
#ifdef SSTINDEX_ARRAY
	delete [] pointers;
#endif
    }

    void maybe_add_entry(const std::string& key, off_t ptr) {
#ifdef SSTINDEX_ARRAY
	unsigned char initial = key[0];
	if (!pointers) {
	    pointers = new off_t[256]();
	    first = initial;
	}
	// We should only be called for valid index points.
	AssertRel(int(initial), !=, last);

	while (++last != int(initial)) {
	    pointers[last] = ptr;
	    // FIXME: Perhaps record this differently so that an exact key
	    // search can return false?
	}
	pointers[initial] = ptr;
	last = initial;
#elif defined SSTINDEX_BINARY_CHOP
	// We store entries truncated to a maximum width (and trailing zeros
	// are used to indicate keys shorter than that max width).  These then
	// point to the first key that maps to this truncated value.
	//
	// We need constant width entries to allow binary chop to work, but
	// there are other ways to achieve this which could be explored.  We
	// could allow the full key width of 256 bytes, but that would take a
	// lot more space.  We could store a pointer (offset) to the key data,
	// but that's more complex to read, and adds the pointer overhead.  We
	// could use a "SKO" - a fixed width entry which encodes variable
	// length pointer and key with short keys in the entry and long keys
	// pointed to (or prefix included and rest pointed to).
	if (last_index_key.size() == SSTINDEX_BINARY_CHOP_KEY_SIZE) {
	    if (startswith(key, last_index_key)) {
		return;
	    }
	}

	// Ensure the truncated key doesn't end in a zero byte.
	if (key.size() >= SSTINDEX_BINARY_CHOP_KEY_SIZE) {
	    // FIXME: Start from char N if we have N array index levels above.
	    last_index_key.assign(key, 0, SSTINDEX_BINARY_CHOP_KEY_SIZE);
	    if (key[SSTINDEX_BINARY_CHOP_KEY_SIZE - 1] == '\0')
		return;
	} else {
	    last_index_key = key;
	    if (key.back() == '\0')
		return;
	    // Pad with zero bytes.
	    last_index_key.resize(SSTINDEX_BINARY_CHOP_KEY_SIZE);
	}

	// Thin entries to at most one per INDEXBLOCK sized block.
	size_t cur_block = ptr / INDEXBLOCK;
	if (cur_block == block)
	    return;

#if 0
	{
	    std::string esc;
	    description_append(esc, last_index_key);
	    std::cout << "Adding «" << esc << "» -> file offset " << ptr
		      << std::endl;
	}
#endif
	data += last_index_key;
	size_t c = data.size();
	data.resize(c + 4);
	unaligned_write4(reinterpret_cast<unsigned char*>(&data[c]), ptr);

	block = cur_block;
#elif defined SSTINDEX_SKIPLIST
	size_t cur_block = ptr / INDEXBLOCK;
	if (cur_block == block) return;

	size_t reuse = common_prefix_length(last_index_key, key);

	data += char(reuse);
	data += char(key.size() - reuse);
	data.append(key, reuse, key.size() - reuse);
	pack_uint(data, static_cast<std::make_unsigned<off_t>::type>(ptr));

	block = cur_block;
	// FIXME: deal with parent_index...

	last_index_key = key;
#else
# error SSTINDEX type not specified
#endif
    }

    off_t write(BufferedFile& store) {
	off_t root = store.get_pos();

#ifdef SSTINDEX_ARRAY
	if (!pointers) {
	    first = last = 0;
	    pointers = new off_t[1]();
	}
	data.resize(0);
	data.resize(3 + (last - first + 1) * 4);
	data[0] = 0;
	data[1] = first;
	data[2] = last - first;
	for (unsigned ch = first; ch <= last; ++ch) {
	    size_t o = 3 + (ch - first) * 4;
	    // FIXME: Just make offsets 8 bytes?  Or allow different widths?
	    off_t ptr = pointers[ch];
	    if (sizeof(off_t) > 4 && ptr > off_t(0xffffffff))
		throw Xapian::DatabaseError("Index offset needs >4 bytes");
	    Assert(o + 4 <= data.size());
	    unaligned_write4(reinterpret_cast<unsigned char*>(&data[o]), ptr);
	}
	delete [] pointers;
	pointers = NULL;
#elif defined SSTINDEX_BINARY_CHOP
	if (last_index_key.size() == SSTINDEX_BINARY_CHOP_KEY_SIZE) {
	    // Increment final byte(s) to give a key which is definitely
	    // at or above any key which this could be truncated from.
	    size_t i = last_index_key.size();
	    unsigned char ch;
	    do {
		if (i == 0) {
		    // We can't increment "\xff\xff\xff\xff" to give an upper
		    // bound - just skip adding one in this case as the table
		    // will handle it OK and there's not much to be gained by
		    // adding one as few keys are larger.
		    goto skip_adding_upper_bound;
		}
		--i;
		ch = static_cast<unsigned char>(last_index_key[i]) + 1;
		last_index_key[i] = ch;
	    } while (ch == 0);
	} else {
	    // Pad with zeros, which gives an upper bound.
	    last_index_key.resize(SSTINDEX_BINARY_CHOP_KEY_SIZE);
	}

	{
	    data += last_index_key;
	    size_t c = data.size();
	    data.resize(c + 4);
	    unaligned_write4(reinterpret_cast<unsigned char*>(&data[c]), root);
	}

skip_adding_upper_bound:
	// Fill in bytes 1 to 4 with the number of entries.
	size_t n_index = (data.size() - 5) / SSTINDEX_BINARY_CHOP_ENTRY_SIZE;
	data[1] = n_index >> 24;
	data[2] = n_index >> 16;
	data[3] = n_index >> 8;
	data[4] = n_index;
#elif defined SSTINDEX_SKIPLIST
	// Already built in data.
#else
# error SSTINDEX type not specified
#endif

	store.write(data.data(), data.size());
	// FIXME: parent stuff...
	return root;
    }

    size_t size() const {
	// FIXME: For SSTINDEX_ARRAY, data.size() only correct after calling
	// write().
	size_t s = data.size();
	if (parent_index) s += parent_index->size();
	return s;
    }
};

class HoneyCursor;
class MutableHoneyCursor;

class HoneyTable {
    // FIXME cleaner way?
    friend class HoneyCursor; // Allow access to store.
    friend class MutableHoneyCursor; // Allow access to store.

    std::string path;
    bool read_only;
    int flags;
    uint4 compress_min;
    mutable BufferedFile store;
    mutable std::string last_key;
    SSTIndex index;
    off_t root = -1;
    honey_tablesize_t num_entries = 0;
    bool lazy;

    bool single_file() const { return path.empty(); }

    /** Offset to add to pointers in this table.
     *
     *  This is zero when each table is a separate file, but likely non-zero
     *  when the tables are all embedded in one file.
     */
    off_t offset = 0;

    bool get_exact_entry(const std::string& key, std::string* tag) const;

    bool read_key(std::string& key, size_t& val_size, bool& compressed) const;

    void read_val(std::string& val, size_t val_size) const;

  public:
    HoneyTable(const char*, const std::string& path_, bool read_only_,
	       bool lazy_ = false)
	: path(path_ + HONEY_TABLE_EXTENSION),
	  read_only(read_only_),
	  lazy(lazy_)
    {
    }

    HoneyTable(const char*, int fd, off_t offset_, bool read_only_,
	       bool lazy_ = false)
	: read_only(read_only_),
	  store(fd, 0, offset_, read_only_),
	  lazy(lazy_),
	  offset(offset_)
    {
    }

    ~HoneyTable() {
#if 0
	size_t index_size = index.size();
	if (index_size)
	    std::cout << "*** " << path << " - index " << index_size << " for "
		      << index.get_num_entries() << " entries" << std::endl;
#endif
	bool fd_owned = !single_file();
	store.close(fd_owned);
    }

    bool is_writable() const { return !read_only; }

    int get_flags() const { return flags; }

    void create_and_open(int flags_, const Honey::RootInfo& root_info);

    void open(int flags_, const Honey::RootInfo& root_info,
	      honey_revision_number_t);

    void close(bool permanent) {
	bool fd_owned = !single_file();
	if (permanent)
	    store.force_close(fd_owned);
	else
	    store.close(fd_owned);
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
	root = index.write(store);
	store.flush();
    }

    void cancel(const Honey::RootInfo&, honey_revision_number_t) {
	std::abort();
    }

    void commit(honey_revision_number_t, Honey::RootInfo* root_info);

    bool sync() {
	store.sync();
	return true;
    }

    bool empty() const {
	return num_entries == 0;
    }

    bool get_exact_entry(const std::string& key, std::string& tag) const {
	return get_exact_entry(key, &tag);
    }

    bool key_exists(const std::string& key) const {
	return get_exact_entry(key, NULL);
    }

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

    bool is_open() const { return store.is_open(); }

    static void throw_database_closed() {
	throw Xapian::DatabaseClosedError("Closed!");
    }

    honey_tablesize_t get_entry_count() const { return num_entries; }

    /** Return an approximation of the number of entries in the table.
     *
     *  Currently this is exact, but may not be in the future.
     */
    honey_tablesize_t get_approx_entry_count() const { return num_entries; }

    off_t get_root() const { return root; }

    off_t get_offset() const { return offset; }
};

#endif // XAPIAN_INCLUDED_HONEY_TABLE_H
