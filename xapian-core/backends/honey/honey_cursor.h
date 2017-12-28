#ifndef XAPIAN_INCLUDED_HONEY_CURSOR_H
#define XAPIAN_INCLUDED_HONEY_CURSOR_H

#include "honey_table.h"

#define DEBUGGING false

class HoneyCursor {
    void rewind() {
	fh.set_pos(0 * root);
	last_key = std::string();
	is_at_end = false;
	index = root;
    }

  public:
    BufferedFile fh;
    std::string current_key, current_tag;
    mutable size_t val_size = 0;
    bool current_compressed = false;
    mutable CompressionStream comp_stream;
    bool is_at_end = false;
    mutable std::string last_key;

    // File offset to start of index and to current position in index.
    off_t root, index;

    // Forward to next constructor form.
    explicit HoneyCursor(const HoneyTable* table)
	: HoneyCursor(table->fh, table->get_root()) {}

    HoneyCursor(const BufferedFile& fh_, off_t root_)
	: fh(fh_),
	  comp_stream(Z_DEFAULT_STRATEGY),
	  root(root_),
	  index(root_)
    {
	fh.set_pos(0 * root);
    }

    HoneyCursor(const HoneyCursor& o)
	: fh(o.fh),
	  current_key(o.current_key),
	  current_tag(o.current_tag), // FIXME really copy?
	  val_size(o.val_size),
	  current_compressed(o.current_compressed),
	  comp_stream(Z_DEFAULT_STRATEGY),
	  is_at_end(o.is_at_end),
	  last_key(o.last_key),
	  root(o.root),
	  index(o.index)
    {
	fh.set_pos(o.fh.get_pos());
    }

    bool after_end() const { return is_at_end; }

    bool next() {
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
	    if (ch == EOF) throw Xapian::DatabaseError("EOF/error while reading key length", errno);
	}
	size_t key_size = ch;
	char buf[256];
	if (!fh.read(buf, key_size))
	    throw Xapian::DatabaseError("read of " + str(key_size) + " bytes of key data failed", errno);
	current_key.assign(last_key, 0, reuse);
	current_key.append(buf, key_size);
	last_key = current_key;

	if (DEBUGGING) {
	    std::string esc;
	    description_append(esc, current_key);
	    std::cerr << "K:" << esc << std::endl;
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
	if (p != end) std::abort();
	current_compressed = val_size & 1;
	val_size >>= 1;

	// FIXME: Always resize to 0?  Not doing so avoids always having to
	// clear all the data before reading it.
	if (true && val_size == 0)
	    current_tag.resize(0);

	is_at_end = false;
	return true;
    }

    bool read_tag(bool keep_compressed = false) {
	if (val_size) {
	    current_tag.resize(val_size);
	    if (!fh.read(&(current_tag[0]), val_size))
		throw Xapian::DatabaseError("read of " + str(val_size) + " bytes of value data failed", errno);
	    if (DEBUGGING) {
		std::cerr << "read " << val_size << " bytes of value data ending @" << fh.get_pos() << std::endl;
	    }
	    val_size = 0;
	    if (DEBUGGING) {
		std::string esc;
		description_append(esc, current_tag);
		std::cerr << "V:" << esc << std::endl;
	    }
	}
	if (!keep_compressed && current_compressed) {
	    // Need to decompress.
	    comp_stream.decompress_start();
	    std::string new_tag;
	    if (!comp_stream.decompress_chunk(current_tag.data(), current_tag.size(), new_tag)) {
		// Decompression didn't complete.
		abort();
	    }
	    swap(current_tag, new_tag);
	    current_compressed = false;
	    if (DEBUGGING) {
		std::cerr << "decompressed to " << current_tag.size() << " bytes of value data" << std::endl;
	    }
	}
	return current_compressed;
    }

    bool find_exact(const std::string& key) {
	if (DEBUGGING) {
	    std::string esc;
	    description_append(esc, key);
	    std::cerr << "find_exact(" << esc << ") @" << fh.get_pos() << std::endl;
	}
	// FIXME: use index
	int cmp0 = current_key.compare(key);
	if (cmp0 == 0) return true;
	if (cmp0 > 0) {
	    rewind();
	}

	while (next()) {
	    int cmp = current_key.compare(key);
	    if (cmp == 0) return true;
	    if (cmp > 0) break;
	}
	return false;
    }

    bool find_entry_ge(const std::string& key) {
	if (DEBUGGING) {
	    std::string esc;
	    description_append(esc, key);
	    std::cerr << "find_entry_ge(" << esc << ") @" << fh.get_pos() << std::endl;
	}
	// FIXME: use index
	int cmp0 = current_key.compare(key);
	if (cmp0 == 0) return true;
	if (cmp0 > 0) {
	    rewind();
	}

	while (next()) {
	    int cmp = current_key.compare(key);
	    if (cmp == 0) return true;
	    if (cmp > 0) return false;
	}
	is_at_end = true;
	return false;
    }

    bool find_entry(const std::string& key) {
	if (DEBUGGING) {
	    std::string desc;
	    description_append(desc, key);
	    std::cerr << "find_entry(" << desc << ") [LE]" << std::endl;
	}
#if 1
	// FIXME: use index
	int cmp0 = current_key.compare(key);
	if (cmp0 == 0) return true;
	if (cmp0 > 0) {
	    rewind();
	}

	off_t pos;
	std::string k;
	size_t vs;
	bool compressed;
	while (pos = fh.get_pos(), k = current_key, vs = val_size, compressed = current_compressed, next()) {
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
	int cmp0 = current_key.compare(key);
	if (cmp0 == 0) {
	    if (DEBUGGING) std::cerr << " already on it" << std::endl;
	    return true;
	}
#if 0
	if (cmp0 > 0) {
	    rewind();
	}
#else
	// FIXME: If "close" just seek forwards?  Or consider seeking from current index pos?
	//off_t pos = fh.get_pos();
	fh.set_pos(root);
	std::string index_key, prev_index_key;
	std::make_unsigned<off_t>::type ptr = 0;
	while (true) {
	    int reuse = fh.read();
	    if (reuse == -1) break;
	    int len = fh.read();
	    if (len == -1) std::abort(); // FIXME
	    prev_index_key = index_key;
	    index_key.resize(reuse + len);
	    fh.read(&index_key[reuse], len);

	    if (DEBUGGING) {
		std::string desc;
		description_append(desc, index_key);
		std::cerr << "Index key: " << desc << std::endl;
	    }

	    cmp0 = index_key.compare(key);
	    if (cmp0 > 0) break;
	    last_key = ptr ? index_key : std::string(); // for now (a lie, but the reuse part is correct).
	    char buf[8];
	    char* e = buf;
	    while (true) {
		int b = fh.read();
		*e++ = b;
		if ((b & 0x80) == 0) break;
	    }
	    const char* p = buf;
	    if (!unpack_uint(&p, e, &ptr) || p != e) std::abort(); // FIXME
	    if (DEBUGGING) std::cerr << " -> " << ptr << std::endl;
	    if (cmp0 == 0) {
		if (DEBUGGING) std::cerr << " hit straight from index" << std::endl;
		fh.set_pos(ptr);
		current_key = index_key;
		int r;
		{
		    // FIXME: rework to take advantage of buffering that's happening anyway?
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
		    throw Xapian::DatabaseError("val_size unpack_uint invalid");
		}
		bool& compressed = current_compressed;
		compressed = val_size & 1;
		val_size >>= 1;
		if (p != end) std::abort();
		return true;
	    }
	}
	if (DEBUGGING) std::cerr << " cmp0 = " << cmp0 << ", going to " << ptr << std::endl;
	fh.set_pos(ptr);

	// FIXME: crude for now
	if (ptr != 0) {
	    current_key = prev_index_key;
	    char buf[4096];
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
	    bool& compressed = current_compressed;
	    auto& val = current_tag;
	    compressed = val_size & 1;
	    val_size >>= 1;
	    val.assign(p, end);
	    if (p != end) std::abort();
	    val_size -= (end - p);
	    while (val_size) {
		size_t n = std::min(val_size, sizeof(buf));
		if (!fh.read(buf, n))
		    throw Xapian::DatabaseError("read of " + str(n) + "/" + str(val_size) + " bytes of value data failed", errno);
		val.append(buf, n);
		val_size -= n;
	    }
	} else {
	    if (!next()) {
		std::abort();
	    }
	}

	if (DEBUGGING) {
	    std::string desc;
	    description_append(desc, current_key);
	    std::cerr << "Dropped to data layer on key: " << desc << std::endl;
	}

	// FIXME: need to put us in the "read key not tag" state but persist that more?
	// if (cmp0 == 0) this is an exact hit from the index...
#endif

	off_t pos;
	std::string k;
	size_t vs;
	bool compressed;
	while (true) {
	    pos = fh.get_pos();
	    k = current_key;
	    vs = val_size;
	    compressed = current_compressed;

	    if (DEBUGGING) {
		std::string desc;
		description_append(desc, current_key);
		std::cerr << "@ " << pos << " key: " << desc << std::endl;
	    }

	    if (!next()) break;

	    int cmp = current_key.compare(key);
	    if (cmp == 0) {
		if (DEBUGGING) std::cerr << " found it" << std::endl;
		return true;
	    }
	    if (cmp > 0) break;
	}

	if (DEBUGGING) {
	    std::string desc;
	    description_append(desc, current_key);
	    std::cerr << " NOT found - reached " << desc << std::endl;
	}
	// No match so back up to previous entry.
	is_at_end = false;
	last_key = current_key = k;
	val_size = vs;
	current_compressed = compressed;
	fh.set_pos(pos);
	if (DEBUGGING) {
	    std::string desc;
	    description_append(desc, current_key);
	    std::cerr << " NOT found - leaving us on " << desc << std::endl;
	}
	return false;
#endif
    }

    void find_entry_lt(const std::string& key) {
	if (DEBUGGING) {
	    std::string esc;
	    description_append(esc, key);
	    std::cerr << "find_entry_lt(" << esc << ") @" << fh.get_pos() << std::endl;
	}
	// FIXME: use index
	if (key < current_key) {
	    rewind();
	    current_key.resize(0);
	}

	off_t pos;
	std::string k;
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

    HoneyCursor * clone() const {
	return new HoneyCursor(*this);
    }

    bool del() { return false; }
};

class MutableHoneyCursor : public HoneyCursor {
  public:
    MutableHoneyCursor(HoneyTable* table_)
	: HoneyCursor(table_->fh, table_->get_root()) { }
};

#endif // XAPIAN_INCLUDED_HONEY_CURSOR_H
