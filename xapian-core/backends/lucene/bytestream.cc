
#include <cstdio>
#include <xapian/error.h>
#include <config.h>
#include <iostream>
#include <cstdio>
#include <cassert>

#include "config.h"
#include "safeerrno.h"
#include "safefcntl.h"
#include "debuglog.h"
#include "filetests.h"

#include "bytestream.h"

ByteStreamReader::ByteStreamReader(const string & db_dir_)
{
    db_dir = db_dir_;
    file_name = "";
    handle = NULL;
}

ByteStreamReader::ByteStreamReader(const string & db_dir_, const string & file_name_) {
    LOGCALL_CTOR(API, "ByteStreamReader", db_dir_ | file_name);
    
    db_dir = db_dir_;
    file_name = file_name_;
    string file_path = db_dir + "/" + file_name;

    if (!file_exists(file_path.c_str())) {
        string message("Couldn't find ");
        message += file_path;
        message += " DB to read: ";
        message += strerror(errno);
        throw Xapian::DatabaseOpeningError(message);
    }

    handle = fopen(file_path.c_str(), "rb");
    if (handle < 0) {
        string message("Couldn't open ");
        message += file_path;
        message += " DB to read: ";
        message += strerror(errno);
        throw Xapian::DatabaseOpeningError(message);
    }
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
    if (NULL != handle) {
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

    handle = fopen(file_path.c_str(), "rb");
    if (handle < 0) {
        string message("Couldn't open ");
        message += file_path;
        message += " DB to read: ";
        message += strerror(errno);
        throw Xapian::DatabaseOpeningError(message);
    }

    return true;
}

ByteStreamReader::~ByteStreamReader() {
    //TODO core dump here, ignore
    /*
    if (NULL != handle) {
        fclose(handle);
    }
    */
}

char
ByteStreamReader::ByteStreamReader::read_byte() const {
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
}

bool
ByteStreamReader::ByteStreamReader::read_byte(char & data) const {
    assert(NULL != handle);

    ssize_t c = fread(&data, sizeof(char), 1, handle);
    if (c <= 0) {
        if (0 == c) {
            throw Xapian::DatabaseError("Couldn't read enought(EOF)");
        }
        throw Xapian::DatabaseError("Error reading from file", errno);
    }

    return true;
}

int
ByteStreamReader::read_int32() const {
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
}

bool
ByteStreamReader::read_int32(int & data) const {
    assert(NULL != handle);

    ssize_t c = fread(&data, sizeof(int), 1, handle);
    if (c <= 0) {
        if (0 == c) {
            throw Xapian::DatabaseError("Couldn't read enought(EOF)");
        }
        throw Xapian::DatabaseError("Error reading from file", errno);
    }

    data = reverse_order_int32(data);

    return true;
}

bool
ByteStreamReader::read_uint32(unsigned int & data) const {
    assert(NULL != handle);

    ssize_t c = fread(&data, sizeof(unsigned int), 1, handle);
    if (c <= 0) {
        if (0 == c) {
            throw Xapian::DatabaseError("Couldn't read enought(EOF)");
        }
        throw Xapian::DatabaseError("Error reading from file", errno);
    }

    data = reverse_order_int32(data);

    return true;
}


long long
ByteStreamReader::read_int64() const {
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
}

bool
ByteStreamReader::read_int64(long long & data) const {
    assert(NULL != handle);

    ssize_t c = fread(&data, sizeof(long long), 1, handle);
    if (c <= 0) {
        if (0 == c) {
            throw Xapian::DatabaseError("Couldn't read enought(EOF)");
        }
        throw Xapian::DatabaseError("Error reading from file", errno);
    }

    data = reverse_order_int64(data);

    return true;
}

bool
ByteStreamReader::read_uint64(unsigned long long & data) const {
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

    return true;
}

int
ByteStreamReader::read_vint32() const {
    assert(NULL != handle);

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
    assert(NULL != handle);

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
    assert(NULL != handle);

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
    assert(NULL != handle);

    int len = read_vint32();
    //Is there a better method to read string without new/delete?
    if (0 == len)
        return "";
    char * buf = new char[len + 1];
    memset(buf, 0, len + 1);
    ssize_t c = fread(buf, sizeof(char), len, handle);
    if (c < len) {
        if (0 == c) {
            throw Xapian::DatabaseError("Couldn't read enought(EOF)");
        }
        throw Xapian::DatabaseError("Error reading from file", errno);
    }

    string result(buf, len);
    delete buf;

    return result;
}

//read string in reference, avoid copy return
bool
ByteStreamReader::read_string(string & str) const {
    assert(NULL != handle);

    int len = read_vint32();
    //cout << "ByteStreamReader::read_string, len:" << len << endl;
    //Is there a better method to read string without new/delete?
    if (0 == len) {
        str = "";
        return true;
    }

    char * buf = new char[len + 1];
    memset(buf, 0, len + 1);
    ssize_t c = fread(buf, sizeof(char), len, handle);
    if (c < len) {
        if (0 == c) {
            throw Xapian::DatabaseError("Couldn't read enought(EOF)");
        }
        throw Xapian::DatabaseError("Error reading from file", errno);
    }

    str.assign(buf, len);
    delete buf;

    return true;
}



//TODO it causes map copy when return, so change it to reference
bool
ByteStreamReader::read_ssmap(map<string, string> &ssmap) const {
    assert(NULL != handle);

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
    assert(NULL != handle);

    read_vint32(term.prefix_length);
    read_string(term.suffix);
    read_vint32(term.field_num);
    /*
    cout << "prefix_length:" << term.prefix_length << " suffix:" << term.suffix <<
        " field_num:" << term.field_num << endl;
        */

    return true;
}

bool
ByteStreamReader::read_terminfo(LuceneTermInfo & terminfo,
            const unsigned int & skip_interval) const {
    assert(NULL != handle);

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
    /*
    cout << "doc_freq:" << terminfo.doc_freq << " freq_delta:" <<
        terminfo.freq_delta << " prox_delta:" << terminfo.prox_delta <<
        " skip_delta:" << terminfo.skip_delta << endl;
        */

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
    assert(NULL != handle);

    int r = fseek(handle, position, SEEK_SET);
    if (r < 0) {
        throw Xapian::DatabaseError("Couldn't fseek the right posion in file");
    }

    return ;
}

/**
 * below is just for debug
 */
long
ByteStreamReader::get_ftell() const {
    return ftell(handle);
}
