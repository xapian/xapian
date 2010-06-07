/** @file brass_table.h
 * @brief Brass B-tree table.
 */
/* Copyright (C) 2010 Olly Betts
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

#ifndef XAPIAN_INCLUDED_BRASS_TABLE_H
#define XAPIAN_INCLUDED_BRASS_TABLE_H

#include "brass_defs.h"

#include <zlib.h>

#include "io_utils.h"
#include "noreturn.h"
#include "omassert.h"
#include "str.h"
#include "unaligned.h"

#include <string>

#include "xapian/visibility.h"

using namespace std;

/** How many sequential updates trigger a switch to from random-access to
 *  sequential block splitting.
 */
#define RANDOM_ACCESS_THRESHOLD 3

static const size_t MAX_INLINE_TAG_SIZE = 64;

static const int BLOCKPTR_SIZE = 4;

static const int HEADER_SIZE = 8;

const bool COMPRESS = true;
const bool DONT_COMPRESS = false;

const bool LAZY = true;
const bool NOT_LAZY = false;

// Convert to little endian form:
#ifdef WORDS_BIGENDIAN

inline uint2 LE(uint2 x) {
# ifdef HAVE_BYTESWAP_H
    return bswap_16(x);
# else
    return (x & 0xff) << 8 | ((x >> 8) & 0xff);
# endif
}

inline uint4 LE(uint4 x) {
# ifdef __GNUC__
    return __builtin_bswap32(x);
# elif defined HAVE_BYTESWAP_H
    return bswap_32(x);
# else
    return (x & 0xff) << 24 | (x & 0xff00) << 8 | ((x >> 8) & 0xff00) | ((x >> 24) & 0xff);
# endif
}

#else
# define LE(X) (X)
#endif

inline brass_block_t get_unaligned_le4(const char *p) {
    const unsigned char * a = (const unsigned char *)p;
    return (brass_block_t)*a | (brass_block_t)a[1] << 8 | (brass_block_t)a[2] << 16 | (brass_block_t)a[3] << 24;
}

inline void set_unaligned_le4(char *p, brass_block_t b) {
    unsigned char * a = (unsigned char *)p;
    *a = b;
    a[1] = b >> 8;
    a[2] = b >> 16;
    a[3] = b >> 24;
}

class BrassTable;
class BrassCursor;

class XAPIAN_VISIBILITY_DEFAULT BrassBlock {
    friend class BrassTable;
    friend class BrassCursor;
    friend class BrassCBlock;

    /// Copying is not allowed.
    BrassBlock(const BrassBlock &);

    /// Assignment is not allowed.
    void operator=(const BrassBlock &);

    void check_block(const std::string &lb = string(),
		     const std::string &ub = string());

#ifdef XAPIAN_ASSERTIONS
    void CHECK_BLOCK() { check_block(); }
#else
    void CHECK_BLOCK() { }
#endif

  protected:
    brass_block_t n;
    char * data;
    const BrassTable & table;

    /** Track if updates are random-access or sequential.
     *
     *  If non-zero, we're doing random access updates, but each access made
     *  right after the previous one decrements by one.
     *
     *  If zero, we're doing sequential updates.
     *
     *  An access which isn't in the same block and right after the previous
     *  one resets @a random_access to RANDOM_ACCESS_THRESHOLD.
     */
    mutable int random_access;

    /// Return the length of the block header for @a count items.
    int header_length(int count) const { return HEADER_SIZE + 2 * count; }

    bool decode_leaf_key(int i, const char *& key_ptr, size_t& key_len,
			 bool& slab) const {
	Assert(is_leaf());
	AssertRel(i,>=,0);
	key_ptr = data + get_ptr(i);
	byte ch = static_cast<unsigned char>(*key_ptr++);
	int tag_len;
	bool compressed;
	if ((ch & 0x7f) == 0) {
	    compressed = false;
	    slab = false;
	    tag_len = (ch >> 7);
	} else if ((ch & 0x7f) < 64) {
	    compressed = (ch & 0x80);
	    slab = false;
	    tag_len = (ch & 0x3f) + 1;
	} else {
	    compressed = (ch & 0x80);
	    slab = true;
	    tag_len = ((ch >> 2) & 0x0f) + 1;
	}
	key_len = get_endptr(i) - (key_ptr - data) - tag_len;
	return compressed;
    }

    void insert_entry(int b, const string &key, brass_block_t blk);

  public:
    BrassBlock(const BrassTable & table_)
	: data(NULL), table(table_), random_access(RANDOM_ACCESS_THRESHOLD) { }
    void save();
    virtual ~BrassBlock();
    bool is_leaf() const { return (byte(data[5]) & 0x80) == 0; }

    brass_revision_number_t get_revision() const {
	return LE(*(brass_revision_number_t*)data);
    }

    void set_revision(brass_revision_number_t revision) {
	*(brass_revision_number_t*)data = LE(revision);
    }

    int get_level() const {
	return (int)(byte)data[7];
    }

    void set_level(int level) {
	AssertRel(level,>=,0);
	AssertRel(level,<,256);
	data[7] = (byte)level;
    }

    /// Returns the number of items in this block.
    int get_count() const {
	return LE(((uint2 *)data)[2]) & 0x7fff;
    }

    /// Sets the number of items in this block.
    void set_count(int count) {
	AssertRel(count,<,0x8000);
	((uint2 *)data)[2] = LE((uint2)count) | (is_leaf() ? 0 : 0x8000);
    }

    int get_ptr(int i) const {
	AssertRel(i,>=,0);
	AssertRel(i,<,get_count());
	// Need HEADER_SIZE / 2 because each entry is 2 bytes.
	return LE(((uint2 *)data)[(HEADER_SIZE / 2) + i]);
    }

    int get_endptr(int i) const;

    // FIXME: only currently used in assertions it seems.
    const string get_key(int i) const {
	const char * p;
	size_t key_len;
	if (is_leaf()) {
	    bool slab;
	    (void)decode_leaf_key(i, p, key_len, slab);
	} else {
	    p = data + get_ptr(i);
	    p += BLOCKPTR_SIZE;
	    key_len = get_endptr(i) - get_ptr(i) - BLOCKPTR_SIZE;
	}
	return string(p, key_len);
    }

    // Need HEADER_SIZE / 2 because each entry is 2 bytes.
    void set_ptr(int i, int ptr) {
	((uint2 *)data)[(HEADER_SIZE / 2) + i] = LE((uint2)ptr);
    }

    brass_block_t get_left_block() const;

    void set_left_block(brass_block_t b);

    brass_block_t get_block(int i) const;
    void set_block(int i, brass_block_t b) {
	Assert(i != -1);
	set_unaligned_le4(data + get_ptr(i), b);
    }
    void new_leaf_block();
    void new_branch_block();
};

class XAPIAN_VISIBILITY_DEFAULT BrassCBlock : public BrassBlock {
    /// Copying is not allowed.
    BrassCBlock(const BrassCBlock &);

    /// Assignment is not allowed.
    void operator=(const BrassCBlock &);

    void set_child_block_number(brass_block_t n_child);

    void check_block();

#ifdef XAPIAN_ASSERTIONS
    void CHECK_BLOCK() { check_block(); }
#else
    void CHECK_BLOCK() { }
#endif

    /** For a branch block, the item number of child.  If child is the left
     *  pointer, then item is -1. */
  protected: // FIXME: what?  Definitely: item
    int item;

    bool modified;

    /// If we modify this block, do we need to clone it first?
    bool needs_clone;

    BrassCBlock * parent, * child;

    bool next_() {
	if (!data)
	   item = -2;

	if (item == -2)
	    return false;

	if (++item >= get_count()) {
	    if (!parent || !parent->next_()) {
		// Root block or parent at end, so end of iteration.
		item = -2;
		return false;
	    }
	    item = (child ? -1 : 0);
	}

	if (child)
	    child->read(get_block(item));

	return true;
    }

    bool prev_() {
	if (!data)
	    item = -2;

	if (item == -2)
	    return false;

	if (item == (child ? -1 : 0)) {
	    if (!parent || !parent->prev_()) {
		// Root block or parent at end, so end of iteration.
		item = -2;
		return false;
	    }
	    item = get_count();
	}
	--item;

	if (child)
	    child->read(get_block(item));

	return true;
    }

    void read_key(std::string & key) {
	if (item == -2) {
	    key.resize(0);
	    return;
	}
	AssertRel(item,<,get_count());
	if (child) {
	    child->read_key(key);
	    return;
	}

	AssertRel(item,>=,0);
	Assert(is_leaf());
	const char * key_ptr;
	size_t key_len;
	bool slab;
	(void)decode_leaf_key(item, key_ptr, key_len, slab);
	key.assign(key_ptr, key_len);
    }

    // Always returns true to allow tail-calling.
    bool read_tag(std::string &tag);

  public:
    BrassCBlock(const BrassTable & table_)
	: BrassBlock(table_), item(-2), modified(false), needs_clone(false),
	  parent(NULL), child(NULL)
    { }

    BrassCBlock(const BrassTable & table_, BrassCBlock * parent_)
	: BrassBlock(table_), item(-2), modified(false), needs_clone(false),
	  parent(parent_), child(NULL)
    { }

    BrassCBlock(const BrassTable & table_, BrassCBlock * child_, brass_block_t blk)
	: BrassBlock(table_), item(-1), modified(true), needs_clone(false),
	  parent(NULL), child(child_)
    {
	new_branch_block();
	set_level(child->get_level() + 1);
	set_left_block(blk);
    }

    virtual ~BrassCBlock();

    BrassCBlock * lose_level() {
	Assert(!parent);
	BrassCBlock * new_root = child;
	if (child) {
	    AssertEq(child->parent, this);
	    child->parent = NULL;
	    child = NULL; // So the destructor doesn't delete it.
	}
	delete this;
	return new_root;
    }

    void read(brass_block_t blk);

    /** Find which child block @a key belongs in, set item to it and load it
     *  into the child cursor.
     *
     *  @param key The key we want to find or add.
     */
    void find_child(const std::string & key);

    /** Search for @a key in this block.
     *
     *  Searches through the (ordered) entries in this block (which must be a
     *  leaf block) for @a key.  If there's an exact match, @a item is set to
     *  it, otherwise @a item is set to the item with the first key after @a
     *  key.
     *
     *  @param key  Key to search for.
     *  @param mode Currently ignored.
     *
     *  @return True if key was found exactly.
     */
    bool binary_chop_leaf(const std::string & key, int mode);

    void insert(const std::string &key, brass_block_t blk, brass_block_t n_child);
    void insert(const std::string &key, const char * tag, size_t tag_len,
		bool compressed);
    void del();
    bool del(const std::string &key);
    void commit() {
	if (modified) {
	    save();
	    modified = false;
	}
	if (child) child->commit();
    }
    void cancel();

    enum { EQ = 1, LT = 2, LE = 3, GE = 5 };

    bool find(const std::string &key, int mode);

    bool key_exists(const std::string &key);

    bool get(const std::string &key, std::string &tag);

    bool next() {
	if (!child) {
	    Assert(!data || is_leaf());
	    return next_();
	}
	return child->next();
    }

    bool prev() {
	if (!child) {
	    Assert(!data || is_leaf());
	    return prev_();
	}
	return child->prev();
    }

    // NB Doesn't need to be virtual since we never store a BrassCBlock * in a
    // BrassBlock * and then call functions on it.
    void new_leaf_block() {
	needs_clone = false;
	BrassBlock::new_leaf_block();
    }

    void set_needs_clone() { needs_clone = true; }

    /// Recursively check this block and its descendants.
    void check();
};

class BrassCompressor {
    z_stream * deflate_stream;
    z_stream * inflate_stream;

    // FIXME: Perhaps where supported we should use anon mmap() to allocate
    // this temporary buffer, and expand it with mremap() (if that fails
    // falling back to munmap() and mmap()).
    char * zlib_buf;
    size_t zlib_buf_size;

    enum { COMPRESS_MIN_SIZE = 5 };

    XAPIAN_NORETURN(static void throw_zlib_error(int res, z_stream * zstream,
						 const char * msg_));

  public:

    BrassCompressor()
	: deflate_stream(NULL), inflate_stream(NULL),
	  zlib_buf(NULL), zlib_buf_size(0) { }

    ~BrassCompressor() {
	if (deflate_stream) {
	    deflateEnd(deflate_stream);
	    delete deflate_stream;
	}
	if (inflate_stream) {
	    inflateEnd(inflate_stream);
	    delete inflate_stream;
	}
	delete [] zlib_buf;
    }

    bool try_to_compress(const std::string & input) {
	if (input.size() < COMPRESS_MIN_SIZE)
	    return false;

	int res;
	if (rare(!deflate_stream)) {
	    deflate_stream = new z_stream;
	    deflate_stream->zalloc = Z_NULL;
	    deflate_stream->zfree = Z_NULL;
	    deflate_stream->opaque = Z_NULL;

	    const int RAW_DEFLATE_WITH_32K_LZ77_WINDOW = -15;
	    const int MAX_MEMLEVEL = 9;
	    res = deflateInit2(deflate_stream, Z_DEFAULT_COMPRESSION,
			       Z_DEFLATED, RAW_DEFLATE_WITH_32K_LZ77_WINDOW,
			       MAX_MEMLEVEL, Z_DEFAULT_STRATEGY);
	} else {
	    res = deflateReset(deflate_stream);
	}

	if (rare(res != Z_OK))
	    throw_zlib_error(res, deflate_stream,
			     "zlib failed to initialise deflate stream: ");

	// We only want to compress if the compressed version is smaller, so
	// set the output buffer size to one less than the input size so zlib
	// will give up in that case.
	size_t max_output_len = input.size() - 1;

	if (zlib_buf && zlib_buf_size < max_output_len) {
	    delete [] zlib_buf;
	    zlib_buf = NULL;
	}
	if (!zlib_buf) {
	    zlib_buf = new char[max_output_len];
	    zlib_buf_size = max_output_len;
	}

	// Zlib's API requires us to cast away const.
	deflate_stream->next_in =
	    reinterpret_cast<Bytef *>(const_cast<char *>(input.data()));
	deflate_stream->avail_in = static_cast<uInt>(input.size());

	deflate_stream->next_out = reinterpret_cast<Bytef *>(zlib_buf);
	deflate_stream->avail_out = static_cast<uInt>(max_output_len);

	return (deflate(deflate_stream, Z_FINISH) == Z_STREAM_END);
    }

    const char * compressed_data() const { return zlib_buf; }

    size_t compressed_data_len() const { return deflate_stream->total_out; }

    void decompress(const char * in, size_t in_len, std::string & output) {
	int res;
	if (rare(!inflate_stream)) {
	    inflate_stream = new z_stream;
	    inflate_stream->zalloc = Z_NULL;
	    inflate_stream->zfree = Z_NULL;
	    inflate_stream->opaque = Z_NULL;

	    const int RAW_DEFLATE_WITH_32K_LZ77_WINDOW = -15;
	    res = inflateInit2(inflate_stream,
			       RAW_DEFLATE_WITH_32K_LZ77_WINDOW);
	} else {
	    res = inflateReset(inflate_stream);
	}

	if (rare(res != Z_OK))
	    throw_zlib_error(res, inflate_stream,
			     "zlib failed to initialise inflate stream: ");

	// Ensure we have at least in_len bytes of buffer so we don't need to
	// make a ludicrous number of calls to inflate().
	if (zlib_buf && zlib_buf_size < in_len) {
	    delete [] zlib_buf;
	    zlib_buf = NULL;
	}
	if (!zlib_buf) {
	    zlib_buf = new char[in_len];
	    zlib_buf_size = in_len;
	}

	output.resize(0);
	// FIXME better guesstimate?
	output.reserve(in_len * 2);

	// Zlib's API requires us to cast away const.
	inflate_stream->next_in = (Bytef*)const_cast<char *>(in);
	inflate_stream->avail_in = (uInt)in_len;

	do {
	    inflate_stream->next_out = reinterpret_cast<Bytef *>(zlib_buf);
	    inflate_stream->avail_out = static_cast<uInt>(zlib_buf_size);

	    res = inflate(inflate_stream, Z_SYNC_FLUSH);

	    if (res != Z_OK && res != Z_STREAM_END)
		throw_zlib_error(res, inflate_stream, "zlib inflate failed: ");

	    output.append(zlib_buf, (char*)inflate_stream->next_out - zlib_buf);
	} while (res != Z_STREAM_END);
    }
};

class XAPIAN_VISIBILITY_DEFAULT BrassTable {
    friend class BrassBlock;
    friend class BrassCBlock;
    friend class BrassCursor;

    /// Copying is not allowed.
    BrassTable(const BrassTable &);

    /// Assignment is not allowed.
    void operator=(const BrassTable &);

    /// Throw an exception indicating that the database is closed.
    XAPIAN_NORETURN(static void throw_database_closed());

  protected:
    enum {
	FD_CLOSED = -2, // Database::close() called explicitly.
	FD_NOT_OPEN = -1 // Table not yet opened (maybe non-existent lazy table).
    };

    int fd;
    int fd_slab;
    unsigned int blocksize;
    std::string path;
    bool readonly;
    std::string errmsg;
    brass_block_t next_free;
    off_t next_free_slab;
    brass_revision_number_t revision;

    BrassBlock split;
    BrassCBlock * my_cursor;

    bool compress;
    bool lazy;
    bool modified;

    mutable BrassCompressor compressor;

    bool read_block(char *buf, brass_block_t n) const;
    bool write_block(const char *buf, brass_block_t n) const;

    bool read_slab(char *buf, size_t len, off_t offset) const;
    bool write_slab(const char *buf, size_t len, off_t offset) const;

    BrassCBlock * gain_level(brass_block_t child) {
	my_cursor = new BrassCBlock(*this, my_cursor, child);
	return my_cursor;
    }

    void lose_level();

    brass_block_t get_free_block();

    void mark_free(brass_block_t n) {
	(void)n;
	Assert(n != brass_block_t(-1));
	if (next_free) {
	    // If next_free is 0, that means we've not allocated a block since
	    // opening the file this time.
	    AssertRel(n,<,next_free);
	}
	// FIXME: add "n" to the freelist for revision "revision".
    }

    off_t get_free_slab(size_t size);

    void mark_free_slab(off_t slab) {
	(void)slab;
	if (next_free_slab) {
	    // If next_free_slab is 0, that means we've not allocated a slab
	    // since opening the file this time.
	    AssertRel(slab,<,next_free_slab);
	}
	// FIXME: add "slab" to the slab freelist for revision "revision".
    }

    void write_slab(const char * buf, size_t len, off_t slab);

    off_t create_slab(const char * buf, size_t len) {
	off_t slab = get_free_slab(len);
	write_slab(buf, len, slab);
	return slab;
    }

    virtual int compare_keys(const void *k1, size_t l1,
			     const void *k2, size_t l2) const;
    virtual std::string divide(const char *k1, size_t l1,
			       const char *k2, size_t l2) const;

    brass_block_t get_root() const {
       if (!my_cursor)
	   return brass_block_t(-1);
       return my_cursor->n;
    }

  public:
    BrassTable(const char * name_, const std::string & path_, bool readonly_,
	       bool compress_ = DONT_COMPRESS, bool lazy_ = NOT_LAZY)
	: fd(FD_NOT_OPEN), blocksize(0), path(path_), readonly(readonly_),
	  next_free(0), next_free_slab(0), revision(0), split(*this),
	  my_cursor(NULL), compress(compress_), lazy(lazy_), modified(false)
    {
	(void)name_; // FIXME
    }

    virtual ~BrassTable();

    bool exists() const;

    /** Create the table on disk.
     *
     *  @param blocksize_	The blocksize to use for this table.
     *  @param from_scratch	True if this table is know to not be present
     *				already (e.g. because the parent directory
     *				was just created).
     */
    bool create(unsigned int blocksize_, bool from_scratch);

    void erase();

    void set_block_size(unsigned int blocksize_) { blocksize = blocksize_; }

    unsigned int get_block_size() const { return blocksize; }

    /** Open (or reopen) the table with the specified root block. */
    bool open(unsigned blocksize_, brass_block_t root_);

    void open_slab_file();
    void close();

    void add(const std::string & key, const std::string & tag,
	     bool already_compressed = false);

    bool del(const std::string & key);

    bool is_modified() const { return modified; }

    // FIXME: Re-merge flush() and commit()?  or do they still need to stay
    // split for replication now per-table base files are gone?
    void flush() { }

    /** Returns the new root block. */
    brass_block_t commit(brass_revision_number_t revision_);

    void sync() {
	if (fd >= 0)
	    io_sync(fd);
	if (fd_slab >= 0)
	    io_sync(fd_slab);
    }

    void cancel();

    bool key_exists(const std::string & key) const;

    bool get(const std::string & key, std::string & tag) const;

    bool is_open() const {
	if (rare(fd == FD_CLOSED))
	    BrassTable::throw_database_closed();
	return (fd >= 0);
    }

    BrassCursor * get_cursor() const;

    //brass_revision_number_t get_open_revision_number() const { return revision; }

    bool empty() const {
	if (rare(fd == -2))
	    throw_database_closed();
	return my_cursor == NULL;
    }

    void set_full_compaction(bool) { } // FIXME support?

    void set_max_item_size(size_t) { } // FIXME support?

    /// Check the B-tree table structure.
    void check();
};

inline void BrassCBlock::set_child_block_number(brass_block_t n_child) {
    AssertRel(item,>=,-1);
    if (item == -1)
	set_left_block(n_child);
    else
	set_block(item, n_child);

    if (!needs_clone)
	return;
    needs_clone = false;
    n = const_cast<BrassTable&>(table).get_free_block();
    Assert(parent);
    parent->set_child_block_number(n);
    CHECK_BLOCK();
}

inline brass_block_t BrassBlock::get_block(int i) const {
    if (i == -1)
	return get_left_block();
    return get_unaligned_le4(data + get_ptr(i));
}

#endif // XAPIAN_INCLUDED_BRASS_TABLE_H
