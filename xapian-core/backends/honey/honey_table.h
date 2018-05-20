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

#define SSINDEX_ARRAY
//#define SSINDEX_BINARY_CHOP
//#define SSINDEX_SKIPLIST

#define SSINDEX_BINARY_CHOP_KEY_SIZE 4
#define SSINDEX_BINARY_CHOP_PTR_SIZE 4
#define SSINDEX_BINARY_CHOP_ENTRY_SIZE \
    (SSINDEX_BINARY_CHOP_KEY_SIZE + SSINDEX_BINARY_CHOP_PTR_SIZE)

//#include "xapian/constants.h"
#include "xapian/error.h"

#include <algorithm>
#if 0
#include <iostream> // FIXME
#endif

#include <cstdio> // For EOF
#include <cstdlib> // std::abort()
#include <type_traits>
#include <utility>
#ifdef HAVE_SYS_UIO_H
# include <sys/uio.h>
#endif

#include <sys/types.h>
#include "safesysstat.h"
#include "safeunistd.h"

#include "safeerrno.h"

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

//#define DEBUGGING

#ifdef DEBUGGING
# include <iostream>
#endif

const uint4 BLK_UNUSED = uint4(-1);

class HoneyFreeListChecker;

class BufferedFile {
    int fd = -1;
    mutable off_t pos = 0;
    bool read_only = true;
    mutable size_t buf_end = 0;
    mutable char buf[4096];

    const int FORCED_CLOSE = -2;

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

    BufferedFile(int fd_, off_t pos_, bool read_only_)
	: fd(fd_), pos(pos_), read_only(read_only_) {}

    ~BufferedFile() {
//	if (fd >= 0) ::close(fd);
    }

    void close(bool fd_owned) {
	if (fd >= 0) {
	    if (fd_owned) ::close(fd);
	    fd = -1;
	}
    }

    void force_close(bool fd_owned) {
	close(fd_owned);
	fd = FORCED_CLOSE;
    }

    bool is_open() const { return fd >= 0; }

    bool was_forced_closed() const { return fd == FORCED_CLOSE; }

    bool open(const std::string& path, bool read_only_) {
//	if (fd >= 0) ::close(fd);
	read_only = read_only_;
	if (read_only) {
	    // FIXME: add new io_open_stream_rd() etc?
	    fd = io_open_block_rd(path);
	} else {
	    // FIXME: Always create anew for now...
	    fd = io_open_block_wr(path, true);
	}
	return fd >= 0;
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
	    io_write(fd, buf, buf_end);
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

	pos += buf_end + len;
#ifdef HAVE_WRITEV
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
		io_write(fd, p, len);
		buf_end = 0;
		return;
	    }
	    buf_end -= n;
	    memmove(buf, buf + n, buf_end);
	}
#else
	io_write(fd, buf, buf_end);
	if (len >= sizeof(buf)) {
	    // If it's bigger than our buffer, just write it directly.
	    io_write(fd, p, len);
	    buf_end = 0;
	    return;
	}
	memcpy(buf, p, len);
	buf_end = len;
#endif
    }

    int read() const {
#if 1
	if (buf_end == 0) {
	    // The buffer is currently empty, so we need to read at least one
	    // byte.
	    size_t r = io_pread(fd, buf, sizeof(buf), pos, 0);
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
#else
	unsigned char ch;
	if (io_pread(fd, &ch, 1, pos) != 1)
	    return EOF;
	++pos;
	return ch;
#endif
    }

    void read(char* p, size_t len) const {
#if 1
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
#endif
	size_t r = io_pread(fd, p, len, pos, len);
	// io_pread() should throw an exception if it read < len bytes.
	AssertEq(r, len);
	pos += r;
    }

    void flush() {
	if (!read_only && buf_end) {
	    io_write(fd, buf, buf_end);
	    pos += buf_end;
	    buf_end = 0;
	}
    }

    void sync() {
	io_sync(fd);
    }

    void rewind(off_t start) {
	read_only = true;
	pos = start;
	buf_end = 0;
    }
};

class HoneyCursor;

class SSIndex {
    std::string data;
#if defined SSINDEX_BINARY_CHOP
    size_t block = size_t(-1);
#elif defined SSINDEX_SKIPLIST
    size_t block = 0;
#endif
#if defined SSINDEX_BINARY_CHOP || defined SSINDEX_SKIPLIST
    std::string last_index_key;
#endif
    // Put an index entry every this much:
    // FIXME: tune - seems 64K is common elsewhere
    enum { INDEXBLOCK = 4096 };
    SSIndex* parent_index = NULL;

#ifdef SSINDEX_ARRAY
    unsigned char first, last = static_cast<unsigned char>(-1);
    off_t* pointers = NULL;
#endif

  public:
    SSIndex() {
#ifdef SSINDEX_ARRAY
	// Header added in write() method.
#elif defined SSINDEX_BINARY_CHOP
	data.assign(5, '\x01');
#elif defined SSINDEX_SKIPLIST
	data.assign(1, '\x02');
#else
# error "SSINDEX type not specified"
#endif
    }

    ~SSIndex() {
#ifdef SSINDEX_ARRAY
	delete [] pointers;
#endif
    }

    void maybe_add_entry(const std::string& key, off_t ptr) {
#ifdef SSINDEX_ARRAY
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
#elif defined SSINDEX_BINARY_CHOP
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
	if (last_index_key.size() == SSINDEX_BINARY_CHOP_KEY_SIZE) {
	    if (startswith(key, last_index_key)) {
		return;
	    }
	}

	// Ensure the truncated key doesn't end in a zero byte.
	if (key.size() >= SSINDEX_BINARY_CHOP_KEY_SIZE) {
	    // FIXME: Start from char N if we have N array index levels above.
	    last_index_key.assign(key, 0, SSINDEX_BINARY_CHOP_KEY_SIZE);
	    if (key[SSINDEX_BINARY_CHOP_KEY_SIZE - 1] == '\0')
		return;
	} else {
	    last_index_key = key;
	    if (key.back() == '\0')
		return;
	    // Pad with zero bytes.
	    last_index_key.resize(SSINDEX_BINARY_CHOP_KEY_SIZE);
	}

	// Thin entries to at most one per INDEXBLOCK sized block.
	size_t cur_block = ptr / INDEXBLOCK;
	if (cur_block == block)
	    return;

#if 0
	{
	    std::string esc;
	    description_append(esc, last_index_key);
	    std::cout << "Adding «" << esc << "» -> file offset " << ptr << std::endl;
	}
#endif
	data += last_index_key;
	size_t c = data.size();
	data.resize(c + 4);
	unaligned_write4(reinterpret_cast<unsigned char*>(&data[c]), ptr);

	block = cur_block;
#elif defined SSINDEX_SKIPLIST
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
# error "SSINDEX type not specified"
#endif
    }

    off_t write(BufferedFile& store) {
	off_t root = store.get_pos();

#ifdef SSINDEX_ARRAY
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
	    if (ptr > 0xffffffff)
		throw Xapian::DatabaseError("Index offset needs >4 bytes");
	    Assert(o + 4 <= data.size());
	    unaligned_write4(reinterpret_cast<unsigned char*>(&data[o]), ptr);
	}
	delete [] pointers;
	pointers = NULL;
#elif defined SSINDEX_BINARY_CHOP
	if (last_index_key.size() == SSINDEX_BINARY_CHOP_KEY_SIZE) {
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
	    last_index_key.resize(SSINDEX_BINARY_CHOP_KEY_SIZE);
	}

	{
	    data += last_index_key;
	    size_t c = data.size();
	    data.resize(c + 4);
	    unaligned_write4(reinterpret_cast<unsigned char*>(&data[c]), root);
	}

skip_adding_upper_bound:
	// Fill in bytes 1 to 4 with the number of entries.
	size_t n_index = (data.size() - 5) / SSINDEX_BINARY_CHOP_ENTRY_SIZE;
	data[1] = n_index >> 24;
	data[2] = n_index >> 16;
	data[3] = n_index >> 8;
	data[4] = n_index;
#elif defined SSINDEX_SKIPLIST
	// Already built in data.
#else
# error "SSINDEX type not specified"
#endif

	store.write(data.data(), data.size());
	// FIXME: parent stuff...
	return root;
    }

    size_t size() const {
	// FIXME: For SSINDEX_ARRAY, data.size() only correct after calling
	// write().
	size_t s = data.size();
	if (parent_index) s += parent_index->size();
	return s;
    }
};

class HoneyCursor;
class MutableHoneyCursor;

template<typename STORAGE>
class SSTable {
  protected:
    bool read_only;
    int flags;
    uint4 compress_min;
    mutable STORAGE store;
    SSIndex index;
    off_t root = -1;
    honey_tablesize_t num_entries = 0;
    bool lazy;

    /** Offset to add to pointers in this table.
     *
     *  This is zero when each table is a separate file, but likely non-zero
     *  when the tables are all embedded in one file.
     *
     *  FIXME: Should this be at the HoneyTable level?
     */
    off_t offset = 0;

  private:
    mutable std::string last_key;

    bool get_exact_entry(const std::string& key, std::string* tag) const {
	if (!read_only) std::abort();
	if (!store.is_open()) {
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
		unsigned char first = key[0] - store.read();
		unsigned char range = store.read();
		if (first > range)
		    return false;
		store.skip(first * 4); // FIXME: pointer width
		off_t jump = store.read() << 24;
		jump |= store.read() << 16;
		jump |= store.read() << 8;
		jump |= store.read();
		store.rewind(jump);
		// The jump point will be an entirely new key (because it is the
		// first key with that initial character), and we drop in as if
		// this was the first key so set last_key to be empty.
		last_key = std::string();
		break;
	    }
	    case 0x01: {
		size_t j = store.read() << 24;
		j |= store.read() << 16;
		j |= store.read() << 8;
		j |= store.read();
		if (j == 0)
		    return false;
		off_t base = store.get_pos();
		char kkey[SSINDEX_BINARY_CHOP_KEY_SIZE];
		size_t kkey_len = 0;
		size_t i = 0;
		while (j - i > 1) {
		    size_t k = i + (j - i) / 2;
		    store.set_pos(base + k * SSINDEX_BINARY_CHOP_ENTRY_SIZE);
		    store.read(kkey, SSINDEX_BINARY_CHOP_KEY_SIZE);
		    kkey_len = 4;
		    while (kkey_len > 0 && kkey[kkey_len - 1] == '\0') --kkey_len;
		    int r = key.compare(0, SSINDEX_BINARY_CHOP_KEY_SIZE, kkey, kkey_len);
		    if (r < 0) {
			j = k;
		    } else {
			i = k;
			if (r == 0) {
			    break;
			}
		    }
		}
		store.set_pos(base + i * SSINDEX_BINARY_CHOP_ENTRY_SIZE);
		store.read(kkey, SSINDEX_BINARY_CHOP_KEY_SIZE);
		kkey_len = 4;
		while (kkey_len > 0 && kkey[kkey_len - 1] == '\0') --kkey_len;
		off_t jump = store.read() << 24;
		jump |= store.read() << 16;
		jump |= store.read() << 8;
		jump |= store.read();
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
		std::string index_key, prev_index_key;
		std::make_unsigned<off_t>::type ptr = 0;
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
			char * p = buf;
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
		    last_key = std::string();
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
		std::string m = "HoneyTable: Unknown index type ";
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

    bool read_key(std::string& key, size_t& val_size, bool& compressed) const {
#ifdef DEBUGGING
	{
	    string desc;
	    description_append(desc, key);
	    cerr << "SSTable::read_key(" << desc << ", ...) for path=" << path << endl;
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
	    char * p = buf;
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
	return true;
    }

    void read_val(std::string& val, size_t val_size) const {
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

  public:
    template<typename... Args>
    SSTable(bool read_only_, off_t offset_, bool lazy_,
	    Args&&... args)
	: read_only(read_only_),
	  store(std::forward<Args>(args)...),
	  lazy(lazy_),
	  offset(offset_)
    {
    }

    bool is_writable() const { return !read_only; }

    int get_flags() const { return flags; }

    void set_full_compaction(bool) { }

    void set_max_item_size(unsigned) { }

    void add(const std::string& key,
	     const char* val,
	     size_t val_size,
	     bool compressed = false) {
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
	size_t reuse = common_prefix_length(last_key, key);

#ifdef SSINDEX_ARRAY
	if (reuse == 0) {
	    index.maybe_add_entry(key, store.get_pos());
	}
#elif defined SSINDEX_BINARY_CHOP
	// For a binary chop index, the index point is before the key info - the
	// index key must have the same N first bytes as the previous key, where
	// N >= the keep length.
	index.maybe_add_entry(key, store.get_pos());
#elif defined SSINDEX_SKIPLIST
	// Handled below.
#else
# error "SSINDEX type not specified"
#endif

	store.write(static_cast<unsigned char>(reuse));
	store.write(static_cast<unsigned char>(key.size() - reuse));
	store.write(key.data() + reuse, key.size() - reuse);
	++num_entries;

#ifdef SSINDEX_SKIPLIST
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

    void add(const std::string& key,
	     const std::string& val,
	     bool compressed = false) {
	add(key, val.data(), val.size(), compressed);
    }

    void cancel(const Honey::RootInfo&, honey_revision_number_t) {
	std::abort();
    }

    void commit(honey_revision_number_t, Honey::RootInfo* root_info) {
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
	store.rewind(offset);
	last_key = std::string();
    }

    void flush_db() {
	root = index.write(store);
	store.flush();
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

    static void throw_database_closed() {
	throw Xapian::DatabaseError("Closed!");
    }

    honey_tablesize_t get_entry_count() const { return num_entries; }

    off_t get_root() const { return root; }
};

class HoneyTable : public SSTable<BufferedFile> {
    // FIXME cleaner way?
    friend class HoneyCursor; // Allow access to store.
    friend class MutableHoneyCursor; // Allow access to store.

    std::string path;

    bool single_file() const { return path.empty(); }

  public:
    HoneyTable(const char*, const std::string& path_, bool read_only_,
	       bool lazy_ = false)
	: SSTable(read_only_, 0, lazy_),
	  path(path_ + HONEY_TABLE_EXTENSION)
    {
    }

    HoneyTable(const char*, int fd, off_t offset_, bool read_only_,
	       bool lazy_ = false)
	: SSTable(read_only_, offset_, lazy_,
		  fd, offset_, read_only_)
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

    bool sync() {
	store.sync();
	return true;
    }

    HoneyCursor* cursor_get() const;

    bool exists() const {
	struct stat sbuf;
	return stat(path.c_str(), &sbuf) == 0;
    }

    bool is_open() const { return store.is_open(); }

    void set_changes(HoneyChanges*) { }

    off_t get_offset() const { return offset; }
};

#endif // XAPIAN_INCLUDED_HONEY_TABLE_H
