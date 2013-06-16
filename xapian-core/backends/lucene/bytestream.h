
#ifndef XAPIAN_INCLUDED_BYTESTREAM_H
#define XAPIAN_INCLUDED_BYTESTREAM_H

#include "xapian.h"

#include "lucene_term.h"

#include <string>
#include <string.h>
#include <map>

using namespace std;

class ByteStreamReader {
    /* Database directory */
    string db_dir;

    /* File name for reading */
    string file_name;

    /* File Handler */
    FILE * handle;

  public:
    ByteStreamReader(const string & db_dir_);
    ByteStreamReader(const string & db_dir_, const string & file_name_);

    ~ByteStreamReader();

    bool set_filename(const string & file_name);

    /* Open file for reading */
    bool open_stream();

    /* Built-in type, using reference is more efficient? Maybe they have 
     * no obvious difference */

    /* Read one Byte */
    char read_byte() const;
    bool read_byte(char &) const;

    /* Read 4 Bytes */
    int read_int32() const;
    bool read_int32(int &) const;
    bool read_uint32(unsigned int &) const;

    //TODO how to express int64 
    /**
     * Using long long for int64, just available in linux i386, is it works
     * on other platform?
     **/
    long long read_int64() const;
    bool read_int64(long long &) const;
    bool read_uint64(unsigned long long &) const;

    /**
     * Prefix 'v' means it's variable-length.
     * Details about VInt, see http://lucene.apache.org/core/3_6_2/fileformats.html
     **/
    int read_vint32() const;
    void read_vint32(int &) const;

    void read_vint64(long long &) const;

    /**
     * String --> VInt, chars.
     * Details about string, see http://lucene.apache.org/core/3_6_2/fileformats.html
     */
    //copy return
    string read_string() const;
    //reference read
    bool read_string(string &) const;

    /**
     * Map --> Count, <String, String>^Count. Map just stores string
     */
    bool read_ssmap(map<string, string> &) const;

    /* Read LuceneTerm from file */
    bool read_term(LuceneTerm &) const;

    /**
     * Read LuceneTermInf
     * Probably bug exists here, pay attention. FIXME 
     **/
    bool read_terminfo(LuceneTermInfo &, const unsigned int &) const;

    /**
     * Read doc delta and docfreq together
     * 1. Read Vint32 first, if the most right bit is 1, then the freq=1
     * 2. If the most right bit is 0, the freq = the next vint32
     * 3. Doc delta = the first Vint32 >> 1;
     * More details on http://lucene.apache.org/core/3_6_2/fileformats.html#Frequencies
     */
    bool read_did_and_freq(int &, int &) const;

    /**
     * Seek to some position in this file
     * FIXME parameter is long, so just support <4G file
     */
    void seek_to(long) const;

    /**
     * Below is just for debug
     **/
    long get_ftell() const;
};

//inline function
inline int reverse_order_int32(int data) {
    int result = 0;
    result |= (data &0x000000ff) << 24;
    result |= (data &0x0000ff00) << 8;
    result |= (data &0x00ff0000) >> 8;
    result |= (data &0xff000000) >> 24;
    return result;
}

inline long long reverse_order_int64(long long data) {
    long long result = 0;
    result |= (data &0x00000000000000ff) << 56; 
    result |= (data &0x000000000000ff00) << 40;
    result |= (data &0x0000000000ff0000) << 24;
    result |= (data &0x00000000ff000000) << 8;
    result |= (data &0x000000ff00000000) >> 8;
    result |= (data &0x0000ff0000000000) >> 24;
    result |= (data &0x00ff000000000000) >> 40;
    result |= (data &0xff00000000000000) >> 56;
    return result;
}

#endif
