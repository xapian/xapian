
//#include <xapian/error.h>
#include <config.h>
#include "debuglog.h"
#include <cstdio>
#include <iostream>
#include <cstdio>
#include <cassert>

#include "safeerrno.h"
#include "safefcntl.h"
#include "filetests.h"
#include "io_utils.h"
#include "posixy_wrapper.h"

#include "bytestream.h"

/** default value for ptr is block_size + 1, which indicates buffer is invalid,
 * data must be read into buffer first
 */ 
ByteStreamReader::ByteStreamReader(const string & db_dir_)
        : db_dir(db_dir_),
        file_name("")
{
#ifdef LUCENE_BLOCK_IO
    handle = -1;
    cursor = block_size + 1;
    //always use the same memery block. memery align is needed?
    block = NULL;
#else
    handle = NULL;
#endif
}

ByteStreamReader::ByteStreamReader(const string & db_dir_, const string & file_name_)
        : db_dir(db_dir_),
        file_name(file_name_)
{
    LOGCALL_CTOR(API, "ByteStreamReader", db_dir_ | file_name_);
    
    string file_path = db_dir + "/" + file_name;
    if (!file_exists(file_path.c_str())) {
        string message("Couldn't find ");
        message += file_path;
        message += " DB to read: ";
        message += strerror(errno);
        throw Xapian::DatabaseOpeningError(message);
    }

#ifdef LUCENE_BLOCK_IO
    handle = posixy_open(file_path.c_str(), O_RDONLY);
    if (handle < 0) {
        string message("Couldn't open ");
        message += file_path;
        message += " DB to read: ";
        message += strerror(errno);
        throw Xapian::DatabaseOpeningError(message);
    }

    cursor = block_size + 1;
    block = new char[block_size];
#else
    handle = fopen(file_path.c_str(), "rb");
    if (NULL == handle) {
        string message("Couldn't open ");
        message += file_path;
        message += " DB to read: ";
        message += strerror(errno);
        throw Xapian::DatabaseOpeningError(message);
    }
#endif
}

bool
ByteStreamReader::set_filename(const string & file_name_) {
    file_name = file_name_;

    return true;
}

bool
ByteStreamReader::open_stream() {
    LOGCALL(API, bool, "ByteStreamReader::open_stream", NO_ARGS);

    //opened befor, just return
#ifdef LUCENE_BLOCK_IO
    if (-1 != handle) {
#else
    if (NULL != handle) {
#endif
        LOGLINE(API, "ByteStreamReader::open_stream, opened already");
        return true;
    }

    if ("" == db_dir || "" == file_name) {
        string message("db_dir or file_name not initialed");
        throw Xapian::DatabaseOpeningError(message);
    }

    string file_path = db_dir + "/" + file_name;
    LOGLINE(API, "ByteStreamReader::open_stream, file_path=" << file_path);

    if (!file_exists(file_path.c_str())) {
        string message("Couldn't find ");
        message += file_path;
        message += " DB to read: ";
        message += strerror(errno);
        throw Xapian::DatabaseOpeningError(message);
    }

#ifdef LUCENE_BLOCK_IO
    handle = posixy_open(file_path.c_str(), O_RDONLY);
    if (handle < 0) {
        string message("Couldn't open ");
        message += file_path;
        message += " DB to read: ";
        message += strerror(errno);
        throw Xapian::DatabaseOpeningError(message);
    }

    cursor = block_size + 1;
    block = new char[block_size];
#else
    handle = fopen(file_path.c_str(), "rb");
    if (NULL == handle) {
        string message("Couldn't fopen ");
        message += file_path;
        message += " DB to read: ";
        message += strerror(errno);
        throw Xapian::DatabaseOpeningError(message);
    }
#endif

    return true;
}

ByteStreamReader::~ByteStreamReader() {
    LOGCALL(API, void, "~ByteStreamReader", handle);
#ifdef LUCENE_BLOCK_IO
    if (-1 != handle)
        close(handle);

    if (NULL != block)
        delete block;
#else
    //TODO core dump here, ignore
    if (NULL != handle)
        fclose(handle);
#endif
}

int
ByteStreamReader::read_int_by_byte() const {
    int result = 0;
    char c;

    /* after read_int_by_byte(), reverse_order_xxx must be executed
    read_byte(c);
    result |= (c & 0x000000ff);
    read_byte(c);
    result |= ((c & 0x000000ff) << 8);
    read_byte(c);
    result |= ((c & 0x000000ff) << 16);
    read_byte(c);
    result |= ((c & 0x000000ff) << 24);
    */
    read_byte(c);
    result |= ((c & 0xff) << 24);
    read_byte(c);
    result |= ((c & 0xff) << 16);
    read_byte(c);
    result |= ((c & 0xff) << 8);
    read_byte(c);
    result |= (c & 0xff);

    return result;
}

long long
ByteStreamReader::read_int64_by_byte() const {
    long long result = 0;
    char c = 0;

    /* after read_int64_by_byte(), reverse_order_xxx must be executed
    read_byte(c);
    result |= ((long long)c & 0x00000000000000ff);
    read_byte(c);
    result |= (((long long)c & 0x00000000000000ff) << 8);
    read_byte(c);
    result |= (((long long)c & 0x00000000000000ff) << 16);
    read_byte(c);
    result |= (((long long)c & 0x00000000000000ff) << 24);
    read_byte(c);
    result |= (((long long)c & 0x00000000000000ff) << 32);
    read_byte(c);
    result |= (((long long)c & 0x00000000000000ff) << 40);
    read_byte(c);
    result |= (((long long)c & 0x00000000000000ff) << 48);
    read_byte(c);
    result |= (((long long)c & 0x00000000000000ff) << 56);
    */
    read_byte(c);
    result |= (((long long)c & 0x00000000000000ff) << 56);
    read_byte(c);
    result |= (((long long)c & 0x00000000000000ff) << 48);
    read_byte(c);
    result |= (((long long)c & 0x00000000000000ff) << 40);
    read_byte(c);
    result |= (((long long)c & 0x00000000000000ff) << 32);
    read_byte(c);
    result |= (((long long)c & 0x00000000000000ff) << 24);
    read_byte(c);
    result |= (((long long)c & 0x00000000000000ff) << 16);
    read_byte(c);
    result |= (((long long)c & 0x00000000000000ff) << 8);
    read_byte(c);
    result |= ((long long)c & 0x00000000000000ff);

    return result;
}

char
ByteStreamReader::ByteStreamReader::read_byte() const {
#ifdef LUCENE_BLOCK_IO
    assert(-1 != handle);

    unsigned char result;
    if (cursor >= block_size) {
        //if reaches the last block, how to read it
        io_read(handle, block, block_size, 0);
        //reset the cursor to start of the new block
        cursor = 0;
    }
    result = block[cursor++];

    return result;
#else
    assert(NULL != handle);

    unsigned char result;
    ssize_t c = fread(&result, sizeof(char), 1, handle);
    if (c <= 0) {
        if (0 == c) {
            throw Xapian::DatabaseError("Couldn't read enought(EOF)");
        }
        throw Xapian::DatabaseError("Error reading from file", errno);
    }

    return result;
#endif
}

bool
ByteStreamReader::read_byte(char & data) const {
#ifdef LUCENE_BLOCK_IO
    assert(-1 != handle);

    if (cursor >= block_size) {
        //the last param is 0, make reading the last block available
        io_read(handle, block, block_size, 0);
        //reset the cursor to start of the new block
        cursor = 0;
    }
    data = block[cursor++];

    return true;
#else
    assert(NULL != handle);

    ssize_t c = fread(&data, sizeof(char), 1, handle);
    if (c <= 0) {
        if (0 == c) {
            throw Xapian::DatabaseError("Couldn't read enought(EOF)");
        }
        throw Xapian::DatabaseError("Error reading from file", errno);
    }

    return true;
#endif
}

int
ByteStreamReader::read_int32() const {
#ifdef LUCENE_BLOCK_IO
    assert(-1 != handle);

    //int result = read_int_by_byte();
    return read_int_by_byte();
#else
    assert(NULL != handle);

    int result;
    ssize_t c = fread(&result, sizeof(int), 1, handle);
    if (c <= 0) {
        if (0 == c) {
            throw Xapian::DatabaseError("Couldn't read enought(EOF)");
        }
        throw Xapian::DatabaseError("Error reading from file", errno);
    }

    //little/big ending transfer
    return reverse_order_int32(result);
#endif
}

bool
ByteStreamReader::read_int32(int & data) const {
#ifdef LUCENE_BLOCK_IO
    assert(-1 != handle);
    
    data = read_int_by_byte();
#else
    assert(NULL != handle);

    ssize_t c = fread(&data, sizeof(int), 1, handle);
    if (c <= 0) {
        if (0 == c) {
            throw Xapian::DatabaseError("Couldn't read enought(EOF)");
        }
        throw Xapian::DatabaseError("Error reading from file", errno);
    }

    data = reverse_order_int32(data);
#endif

    return true;
}

bool
ByteStreamReader::read_uint32(unsigned int & data) const {
#ifdef LUCENE_BLOCK_IO
    assert(-1 != handle);

    data = (unsigned int)read_int_by_byte();
#else
    assert(NULL != handle);

    ssize_t c = fread(&data, sizeof(unsigned int), 1, handle);
    if (c <= 0) {
        if (0 == c) {
            throw Xapian::DatabaseError("Couldn't read enought(EOF)");
        }
        throw Xapian::DatabaseError("Error reading from file", errno);
    }

    data = reverse_order_int32(data);
#endif

    return true;
}


long long
ByteStreamReader::read_int64() const {
#ifdef LUCENE_BLOCK_IO
    assert(-1 != handle);

    //long long result = read_int64_by_byte();
    return read_int64_by_byte();
#else
    assert(NULL != handle);

    long long result;
    ssize_t c = fread(&result, sizeof(long long), 1, handle);
    if (c <= 0) {
        if (0 == c) {
            throw Xapian::DatabaseError("Couldn't read enought(EOF)");
        }
        throw Xapian::DatabaseError("Error reading from file", errno);
    }

    return reverse_order_int64(result);
#endif
}

bool
ByteStreamReader::read_int64(long long & data) const {
#ifdef LUCENE_BLOCK_IO
    assert(-1 != handle);

    data = read_int64_by_byte();
#else
    assert(NULL != handle);

    ssize_t c = fread(&data, sizeof(long long), 1, handle);
    if (c <= 0) {
        if (0 == c) {
            throw Xapian::DatabaseError("Couldn't read enought(EOF)");
        }
        throw Xapian::DatabaseError("Error reading from file", errno);
    }

    data = reverse_order_int64(data);
#endif

    return true;
}

bool
ByteStreamReader::read_uint64(unsigned long long & data) const {
#ifdef LUCENE_BLOCK_IO
    assert(-1 != handle);

    data = (unsigned long long)read_int64_by_byte();
#else
    assert(NULL != handle);
    
    ssize_t c = fread(&data, sizeof(unsigned long long), 1, handle);
    if (c <= 0) {
        if (0 == c) {
            throw Xapian::DatabaseError("Couldn't read enought(EOF)");
        }
        throw Xapian::DatabaseError("Error reading from file", errno);
    }
    LOGLINE(API, "ByteStreamReader::read_uint64, data" << data);

    data = reverse_order_int64(data);
#endif

    return true;
}

int
ByteStreamReader::read_vint32() const {
#ifdef LUCENE_BLOCK_IO
    assert(-1 != handle);
#else
    assert(NULL != handle);
#endif

    unsigned char b = read_byte();
    int i = b & 0x7f;
    if ((b & 0x80) == 0) return i;
    b = read_byte();
    i |= ((b & 0x7f) << 7);
    if ((b & 0x80) == 0) return i;
    b = read_byte();
    i |= ((b & 0x7f) << 14);
    if ((b & 0x80) == 0) return i;
    b = read_byte();
    i |= ((b & 0x7f) << 21);
    if ((b & 0x80) == 0) return i;
    b = read_byte();
    i |= ((b & 0x7f) << 28);
    if ((b & 0x80) == 0) return i;

    throw Xapian::DatabaseError("Invalid vInt detected(too many bytes)");
}

void
ByteStreamReader::read_vint32(int & data) const {
#ifdef LUCENE_BLOCK_IO
    assert(-1 != handle);
#else
    assert(NULL != handle);
#endif

    unsigned char b = read_byte();
    data = b & 0x7f;
    if ((b & 0x80) == 0) return ;
    b = read_byte();
    data |= ((b & 0x7f) << 7);
    if ((b & 0x80) == 0) return ;
    b = read_byte();
    data |= ((b & 0x7f) << 14);
    if ((b & 0x80) == 0) return ;
    b = read_byte();
    data |= ((b & 0x7f) << 21);
    if ((b & 0x80) == 0) return ;
    b = read_byte();
    data |= ((b & 0x7f) << 28);
    if ((b & 0x80) == 0) return ;

    throw Xapian::DatabaseError("Invalid vInt detected(too many bytes)");
}

void
ByteStreamReader::read_vint64(long long & data) const {
#ifdef LUCENE_BLOCK_IO
    assert(-1 != handle);
#else
    assert(NULL != handle);
#endif

    long long b = (long long)read_byte();
    data = b & 0x7fL;
    if ((b & 0x80) == 0) return ;
    b = read_byte();
    data |= ((b & 0x7fL) << 7);
    if ((b & 0x80) == 0) return ;
    b = read_byte();
    data |= ((b & 0x7fL) << 14);
    if ((b & 0x80) == 0) return ;
    b = read_byte();
    data |= ((b & 0x7fL) << 21);
    if ((b & 0x80) == 0) return ;
    b = read_byte();
    data |= ((b & 0x7fL) << 28);
    if ((b & 0x80) == 0) return ;
    b = read_byte();
    data |= ((b & 0x7fL) << 35);
    if ((b & 0x80) == 0) return ;
    b = read_byte();
    data |= ((b & 0x7fL) << 42);
    if ((b & 0x80) == 0) return ;
    b = read_byte();
    data |= ((b & 0x7fL) << 49);
    if ((b & 0x80) == 0) return ;
    b = read_byte();
    data |= ((b & 0x7fL) << 56);
    if ((b & 0x80) == 0) return ;

    throw Xapian::DatabaseError("Invalid vLong detected(too many bytes)");
}

//TODO declare string in local function, it causes string copy when return
string
ByteStreamReader::read_string() const {
#ifdef LUCENE_BLOCK_IO
    assert(-1 != handle);
#else
    assert(NULL != handle);
#endif

    int len = read_vint32();
    //Is there a better method to read string without new/delete?
    if (0 == len)
        return "";
    char * buf = new char[len + 1];
    memset(buf, 0, len + 1);
#ifdef LUCENE_BLOCK_IO
    for (int i = 0; i < len; ++i) {
        buf[i] = read_byte();
    }
#else
    ssize_t c = fread(buf, sizeof(char), len, handle);
    if (c < len) {
        if (0 == c) {
            throw Xapian::DatabaseError("Couldn't read enought(EOF)");
        }
        throw Xapian::DatabaseError("Error reading from file", errno);
    }
#endif

    string result(buf, len);
    delete buf;

    return result;
}

//read string in reference, avoid copy return
bool
ByteStreamReader::read_string(string & str) const {
#ifdef LUCENE_BLOCK_IO
    assert(-1 != handle);
#else
    assert(NULL != handle);
#endif

    int len = read_vint32();
    //Is there a better method to read string without new/delete?
    if (0 == len) {
        str = "";
        return true;
    }

    char * buf = new char[len + 1];
    memset(buf, 0, len + 1);
#ifdef LUCENE_BLOCK_IO
    for (int i = 0; i < len; ++i) {
        buf[i] = read_byte();
    }
#else
    ssize_t c = fread(buf, sizeof(char), len, handle);
    if (c < len) {
        if (0 == c) {
            throw Xapian::DatabaseError("Couldn't read enought(EOF)");
        }
        throw Xapian::DatabaseError("Error reading from file", errno);
    }
#endif

    str.assign(buf, len);
    delete buf;

    return true;
}



//TODO it causes map copy when return, so change it to reference
bool
ByteStreamReader::read_ssmap(map<string, string> &ssmap) const {
#ifdef LUCENE_BLOCK_IO
    assert(-1 != handle);
#else
    assert(NULL != handle);
#endif

    int count = read_int32();
    for (int i = 0; i < count; ++i) {
        string k = read_string();
        string v = read_string();
        ssmap.insert(pair<string, string>(k, v));
    }

    return true;
}

bool
ByteStreamReader::read_term(LuceneTerm & term) const {
#ifdef LUCENE_BLOCK_IO
    assert(-1 != handle);
#else
    assert(NULL != handle);
#endif

    read_vint32(term.prefix_length);
    read_string(term.suffix);
    read_vint32(term.field_num);

    return true;
}

bool
ByteStreamReader::read_terminfo(LuceneTermInfo & terminfo,
            const unsigned int & skip_interval) const {
#ifdef LUCENE_BLOCK_IO
    assert(-1 != handle);
#else
    assert(NULL != handle);
#endif

    read_term(terminfo.term);
    read_vint32(terminfo.doc_freq);
    //from lucene.3.6.2->TermInfoReaderIndex.java line126, but it's read sequence is:
    //doc_freq,skip_delta,freq_delta,prox_delta, diff from here
    read_vint32(terminfo.freq_delta);
    read_vint32(terminfo.prox_delta);
    if ((unsigned int)terminfo.doc_freq >= skip_interval) {
        read_vint32(terminfo.skip_delta);
    } else {
        terminfo.skip_delta = 0;
    }

    return true;
}

bool
ByteStreamReader::read_did_and_freq(int & docid, int & freq) const {
    int did = 0;
    read_vint32(did);
    docid = did >> 1;
    int has_freq = did & 0x00000001;
    if (1 == has_freq) {
      freq = 1;
    }
    else {
        read_vint32(freq);
    }

    return true;
}

void
ByteStreamReader::seek_to(long position) const {
#ifdef LUCENE_BLOCK_IO
    /** if doing seek_to in LUCENE_BLOCK_IO, block must be read after lseek
     */
    assert(-1 != handle);
    
    if (lseek(handle, off_t(position), SEEK_SET) == -1) {
        throw Xapian::DatabaseError("Couldn't fseek the right posion in file");
    }
    
    io_read(handle, block, block_size, 0);
    cursor = 0;
#else
    assert(NULL != handle);

    int r = fseek(handle, position, SEEK_SET);
    if (r < 0) {
        throw Xapian::DatabaseError("Couldn't fseek the right posion in file");
    }
#endif

    return ;
}

/**
 * below is just for debug
 */
long
ByteStreamReader::get_ftell() const {
#ifdef LUCENE_BLOCK_IO
    return lseek(handle, 0, SEEK_CUR);
#else
    return ftell(handle);
#endif
}
