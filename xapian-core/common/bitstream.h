/** @file bitstream.h
 * @brief Classes to encode/decode a bitstream.
 */
/* Copyright (C) 2004,2005,2006,2008,2012,2013,2014 Olly Betts
 * Copyrigtt (C) 2014 Shangtong Zhang
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301
 * USA
 */

#ifndef XAPIAN_INCLUDED_BITSTREAM_H
#define XAPIAN_INCLUDED_BITSTREAM_H

#include <xapian/types.h>

#include <string>
#include <vector>

#include "math.h"
#include "pack.h"

using std::string;

/* highly optimized log2
 * @return : the value returned is an integer which is no less than log2(@val) when @up is ture,
 *			 or is no more than log2(@val) when @up is false */

inline int log2(unsigned val, bool up = true);

namespace Xapian {

/* base class for all encoders 
 * @buf : the chunk to store encoded infomation 
 * @acc : as @buf is a string, so data in it is 8-bit aligned,
 *        it is often the case that in the end there are data less than 8 bit, that data is stored in @acc 
 * @bits : the number of valid bits in @acc */
class Encoder{
  
  protected:
    std::string& buf;
    unsigned char& acc;
    int& bits;
    
    // check if @acc has 8 bits, if so, append @acc to @buf and zero @bits
    inline bool check_acc();
    
    // a mask to retrive low bits of an variable
    static const unsigned int mask_low_n_bits[33];
  
  public:
    Encoder(std::string& buf_, unsigned char& acc_, int& bits_)
		: buf(buf_), acc(acc_), bits(bits_) { }
    
    // encode an unsigned integer
    virtual void encode(unsigned int n) = 0;
    
    // default deconstructor
    virtual ~Encoder() { }
};
    
/* Implementation of Unary encoder.
 * In the unary representation, each integer value x is represented using x − 1 bits equal to ‘1’ followed by a ‘0’ that acts as a terminator. 
 * Therefore, the length of the encoding of an integer x is |Unary(x)| = x. 
 * As an example, if x = 5 we have UN(5) = 11110. */
class UnaryEncoder : public Encoder{
    
    // mask to get 1 of n bits
    static const unsigned char mask_1s[8];
    
  public:
    UnaryEncoder(std::string& buf_, unsigned char& acc_, int& bits_)
		: Encoder(buf_, acc_, bits_) { }
    void encode( unsigned int n );
};
    
/* Implementation of Gamma encoder.
 * In Gamma, an integer x > 0 is encoded by representing |bin(x)| in unary
 * followed by bin(x) without its most significant bit. 
 * As an example, if x = 5, we have bin (x) = 101, |bin(x)| = 3, 
 * and thus Gamma(5) is equal to 11001. */
class GammaEncoder : public Encoder {
  public:
    GammaEncoder(std::string& buf_, unsigned char& acc_, int& bits_)
        : Encoder(buf_, acc_, bits_) { }
    void encode(unsigned int n);
};

/* As the name suggests, it's just encode bin(x) within @num_of_bits bits. 
 * For example, 3 is encoded as 0101, if @num_of_bits is 4. */
class OrdinaryEncoder : public Encoder {
    
    // number of bits to be used to encode an integer.
    const unsigned int num_of_bits;
    
  public:
    OrdinaryEncoder(std::string& buf_, unsigned char& acc_, int& bits_, int num_of_bits_)
		: Encoder(buf_, acc_, bits_), num_of_bits(num_of_bits_) { }
    void encode( unsigned int n );
};

/* an algorithm to encode a series of numbers by groups
 * For each group, assuming we need at most b bits to encode the max number in this group using Ordinary Encoder
 * and the number of entries in this group is k, we first encode b+1 with Gamma Encoder,
 * then encode k with Unary Encoder, and encode each number in this group with Ordinary Encoder within b bits.
 * @buf : the temporary buffer to store encoded infomation
 * @acc : as @buf is a string, so data in it is 8-bit aligned,
 *        it is often the case that in the end there are data less than 8 bit, that data is stored in @acc
 * @bits : the number of valid bits in @acc, as acc is char, @bis is always less than 8.
 * the format of final string chunk ( referred to as f_chunk in the following ):
 *      f_chunk = (char)@acc + (char)@bits + number of entries + last entry + @buf
 * @chunk : f_chunk is appended to @chunk finally. */
class VSEncoder {
    int bits;
    unsigned char acc;
    string& chunk;
    string buf;
    
    /* spilt all entries @L into groups @S.
     * 1st group is L[S[0]] - L[S[1]-1],
     * 2nd group is L[S[1]] - L[S[2]-1],
     * ... */
    unsigned int get_optimal_split(const std::vector<unsigned int>& L, std::vector<unsigned int>& S);
    
    // encode a group of entries, their indice are >= i and < j
    void encode(const std::vector<unsigned int>& L, int i, int j);
        
  public:
    
    // @chunk_ : the destination for encoded data
    VSEncoder(std::string& chunk_);
    
    // encode all entries @L
    void encode(const std::vector<unsigned int>& L);
};
 

/* base class for all decoders 
 * all these decoders is designed to read the string chunk ( referred to as s_chunk in the following ) encoded by corresponding encoder. 
 * @pos : a pointer to char being decoded in s_chunk.
 * @end : a pointer to the end of s_chunk.
 * @acc : the acc part of s_chunk.
 * @acc_bits : the number of valid bits of @acc.
 * @p_bit : p_bit is x if x-th bit of char pointed by @pos is being decoded. */
class Decoder {
  protected:
    const char*& pos;
    const char* end;
    unsigned char& acc;
    int& acc_bits;
    int& p_bit;
    
    // a mask to get x-th bit of a 8-bit variable
    static unsigned int mask[8];
    
    // get the @i-th bit of @n
    static unsigned int get_bit_value(unsigned int n, int i);
    
  public:
    
    // decode a number from s_chunk
    virtual unsigned int decode() = 0;
    
    // default constructor
    Decoder(const char*& pos_, const char* end_, unsigned char& acc_, int& acc_bits_, int& p_bit_)
		: pos(pos_), end(end_), acc(acc_), acc_bits(acc_bits_), p_bit(p_bit_) { }
    
    // default deconstructor
    virtual ~Decoder(){ }
};

// decode a number encoded by Unary Encoder
class UnaryDecoder : public Decoder {
    
  public:
    unsigned int decode();
    UnaryDecoder(const char*& pos_, const char* end_, unsigned char& acc_, int& acc_bits_, int& p_bit_)
        : Decoder(pos_, end_, acc_, acc_bits_, p_bit_){ }
};

// decode a number encoded by Gamma Encoder
class GammaDecoder : public Decoder {
    
    // a Unary Decoder
    UnaryDecoder* p_ud;
    
  public:
    unsigned int decode();
    GammaDecoder(const char*& pos_, const char* end_, unsigned char& acc_, int& acc_bits_, int& p_bit_)
    : Decoder(pos_, end_, acc_, acc_bits_, p_bit_){
        p_ud = new UnaryDecoder(pos_, end_, acc_, acc_bits_, p_bit_);
    }
    ~GammaDecoder(){
        if (p_ud != NULL) {
            delete p_ud;
            p_ud = NULL;
        }
    }
};

// decode a number encoded by Ordinary Encoder
class OrdinaryDecoder : public Decoder {
    
    // the number of bits used by Ordinary Encoder to encode each number
    int width;
    
    // a mask to get the i-th bit to j-th bit of an 8-bit variable
    static const unsigned char mask_nbits[8][9];
    
  public:
    unsigned int decode();
    OrdinaryDecoder(const char*& pos_, const char* end_, unsigned char& acc_, int& acc_bits_, int& p_bit_, int width_)
    : Decoder(pos_, end_, acc_, acc_bits_, p_bit_), width(width_) { }
    
    // set @width
    void setWidth( int width_ ) {
        width = width_;
    }
};
    
// a decoder to decode the chunk encoded by VSEncoder
class VSDecoder {
    
    // the source chunk generate by VSEncoder
    std::string buf;
    
    // the acc part of source chunk
    unsigned char acc;
    
    // the number of valid bits of @acc
    int acc_bits;
    
    /* p_bit is x if x-th bit of char pointed by @pos is being decoded.
     * the bit pointed by p_bit hasn't be handled. */
    int p_bit;
    
    // a pointer to char being decoded in source chunk.
    const char* pos;
    
    // a pointer to the end of source chunk.
    const char* end;
    
    /* In current group of entries, each entry is encoded by Ordinary Encoder within @cur_num_width bits
     * and there are @cur_remaining_nums entries havn't been decoded. */
    unsigned int cur_num_width;
    unsigned int cur_remaining_nums;
    
    // a Unary Decoder to decode current group
    UnaryDecoder* p_ud;
    
    // a Gamma Decoder to decode current group
    GammaDecoder* p_gd;
    
    // an Ordinary Decoder to decode current group
    OrdinaryDecoder* p_od;
    
    // as the entries is encoded as delta, @bias the value to be added to recently decoded delta value.
    unsigned int bias;
    
    // the number of entries in source chunk
    unsigned int n_entry;
    
    // last entry in source chunk
    unsigned int last_entry;
    
    // get the next value in source chunk
    unsigned int next();
    
  public:
    
    // @buf_ : source chunk
    VSDecoder(const std::string& buf_);
    
    // get next entry in source chunk
    unsigned int get_next_entry();
    
    // get first entry in source chunk
    unsigned int get_first_entry();
    
    // get the numer of entries in source chunk
    unsigned int get_n_entry();
    
    // get the last entry in source chunk
    unsigned int get_last_entry();
    
    // default deconstructor
    ~VSDecoder();
};

/// Create a stream to which non-byte-aligned values can be written.
class BitWriter {
    std::string buf;
    int n_bits;
    unsigned int acc;

  public:
    /// Construct empty.
    BitWriter() : n_bits(0), acc(0) { }

    /// Construct with the contents of seed already in the stream.
    explicit BitWriter(const std::string & seed)
	: buf(seed), n_bits(0), acc(0) { }

    /// Encode value, known to be less than outof.
    void encode(size_t value, size_t outof);

    /// Finish encoding and return the encoded data as a std::string.
    std::string & freeze() {
	if (n_bits) {
	    buf += char(acc);
	    n_bits = 0;
	    acc = 0;
	}
	return buf;
    }

    /// Perform interpolative encoding of pos elements between j and k.
    void encode_interpolative(const std::vector<Xapian::termpos> &pos, int j, int k);
};

/// Read a stream created by BitWriter.
class BitReader {
    std::string buf;
    size_t idx;
    int n_bits;
    unsigned int acc;

    unsigned int read_bits(int count);

    struct DIStack {
	int j, k;
	Xapian::termpos pos_k;
    };

    struct DIState : public DIStack {
	Xapian::termpos pos_j;

	void set_j(int j_, Xapian::termpos pos_j_) {
	    j = j_;
	    pos_j = pos_j_;
	}
	void set_k(int k_, Xapian::termpos pos_k_) {
	    k = k_;
	    pos_k = pos_k_;
	}
	void uninit()  {
	    j = 1;
	    k = 0;
	}
	DIState() { uninit(); }
	DIState(int j_, int k_,
		Xapian::termpos pos_j_, Xapian::termpos pos_k_) {
	    set_j(j_, pos_j_);
	    set_k(k_, pos_k_);
	}
	void operator=(const DIStack & o) {
	    j = o.j;
	    set_k(o.k, o.pos_k);
	}
	bool is_next() const { return j + 1 < k; }
	bool is_initialized() const {
	    return j <= k;
	}
	// Given pos[j] = pos_j and pos[k] = pos_k, how many possible position
	// values are there for the value midway between?
	Xapian::termpos outof() const {
	    return pos_k - pos_j + j - k + 1;
	}
    };

    std::vector<DIStack> di_stack;
    DIState di_current;

  public:
    // Construct.
    BitReader() { }

    // Construct with the contents of buf_.
    explicit BitReader(const std::string &buf_)
	: buf(buf_), idx(0), n_bits(0), acc(0) { }

    // Construct with the contents of buf_, skipping some bytes.
    BitReader(const std::string &buf_, size_t skip)
	: buf(buf_, skip), idx(0), n_bits(0), acc(0) { }

    // Initialise from buf_, optionally skipping some bytes.
    void init(const std::string &buf_, size_t skip = 0) {
	buf.assign(buf_, skip, std::string::npos);
	idx = 0;
	n_bits = 0;
	acc = 0;
	di_stack.clear();
	di_current.uninit();
    }

    // Decode value, known to be less than outof.
    Xapian::termpos decode(Xapian::termpos outof, bool force = false);

    // Check all the data has been read.  Because it'll be zero padded
    // to fill a byte, the best we can actually do is check that
    // there's less than a byte left and that all remaining bits are
    // zero.
    bool check_all_gone() const {
	return (idx == buf.size() && n_bits <= 7 && acc == 0);
    }

    /// Perform interpolative decoding between elements between j and k.
    void decode_interpolative(int j, int k,
			      Xapian::termpos pos_j, Xapian::termpos pos_k);

    /// Perform on-demand interpolative decoding.
    Xapian::termpos decode_interpolative_next();
};

}

using Xapian::BitWriter;
using Xapian::BitReader;
using Xapian::VSEncoder;
using Xapian::VSDecoder;

#endif // XAPIAN_INCLUDED_BITSTREAM_H
