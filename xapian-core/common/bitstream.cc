/** @file bitstream.cc
 * @brief Classes to encode/decode a bitstream.
 */
/* Copyright (C) 2004,2005,2006,2008,2013,2014 Olly Betts
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

#include <config.h>

#include "bitstream.h"

#include <xapian/types.h>

#include "omassert.h"

#include <cmath>
#include <vector>

using namespace std;

static const unsigned char flstab[256] = {
    0, 1, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4,
    5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
    6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
    6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8
};

const unsigned char Xapian::OrdinaryDecoder::mask_nbits[8][9] =
{
    {0, 0x80, 0xc0, 0xe0, 0xf0, 0xf8, 0xfc, 0xfe, 0xff},
    {0, 0x40, 0x60, 0x70, 0x78, 0x7c, 0x7e, 0x7f, 0},
    {0, 0x20, 0x30, 0x38, 0x3c, 0x3e, 0x3f, 0, 0},
    {0, 0x10, 0x18, 0x1c, 0x1e, 0x1f, 0, 0, 0},
    {0, 0x8, 0xc, 0xe, 0xf, 0, 0, 0, 0},
    {0, 0x4, 0x6, 0x7, 0, 0, 0, 0, 0},
    {0, 0x2, 0x3, 0, 0, 0, 0, 0, 0},
    {0, 0x1, 0, 0, 0, 0, 0, 0, 0}
};

const unsigned int Xapian::OrdinaryEncoder::mask_low_n_bits[9] =
{
	0,0x1,0x3,0x7,0xf,0x1f,0x3f,0x7f,0xff
};

// @return : the value returned is an integer which is no less than log2(@val) when @up is ture,
//				or is no more than log2(@val) when @up is false
inline int log2(unsigned val, bool up )
{
    int result = 0;
    if (val >= 0x10000u) {
        val >>= 16;
        result = 16;
    }
    if (val >= 0x100u) {
        val >>= 8;
        result += 8;
    }
    result += flstab[val];
	if ( up )
	{
		if ( val == 1<<(result-1) )
		{
			result--;
		}
	}
	else
	{
		result--;
	}
    
	return result;
}

unsigned int get_Unary_encode_length( unsigned int n )
{
	return n;
}

unsigned int get_Gamma_encode_length( unsigned int n )
{
	return 2*log2(n,false)+1;
}

namespace Xapian {
    
    //Encoder.cpp
    
    
    inline bool Encoder::check_acc()
    {
        if ( bits == 8 )
        {
            buf += acc;
            acc = 0;
            bits = 0;
            return true;
        }
        return false;
    }
    
    void UnaryEncoder::encode( unsigned int n )
    {
        for ( int i = 0 ; i < (int)n-1 ; ++i )
        {
            acc <<= 1;
            acc |= 1;
            bits++;
            check_acc();
        }
        acc = acc << 1;
        bits++;
        check_acc();
    }
    
    void GammaEncoder::encode( unsigned int n )
    {
        int n_bin_bits = log2(n,false)+1;
        UnaryEncoder u( buf, acc, bits );
        u.encode(n_bin_bits);
        unsigned int mask = 0;
        for ( int i = 0 ; i < n_bin_bits-1 ; ++i )
        {
            mask = mask << 1;
            mask = mask | 1;
        }
        n = n & mask;
        
        n_bin_bits--;
        
        for ( int i = n_bin_bits-1 ; i >= 0 ; i-- )
        {
            acc <<= 1;
            acc |= (n&(1<<i))>>i;
            bits++;
            check_acc();
        }
        
    }
    
    void OrdinaryEncoder::encode( unsigned int n )
    {
        unsigned int cur_width = num_of_bits;
        while ( cur_width )
        {
            if ( cur_width + bits <= 8 )
            {
                acc <<= cur_width;
                acc |= mask_low_n_bits[cur_width]&n;
                bits += cur_width;
                if ( bits == 8 )
                {
                    buf += acc;
                    acc = 0;
                    bits = 0;
                }
                return;
            }
            else
            {
                acc <<= (8-bits);
                acc |= mask_low_n_bits[8-bits]&(n>>(cur_width-8+bits));
                cur_width -= 8-bits;
                buf += acc;
                acc = 0;
                bits = 0;
            }
            
        }
    }
    
    VSEncoder::VSEncoder(std::string& chunk_)
	: chunk(chunk_)
    {
        acc = 0;
        bits = 0;
        
        
    }
    
    unsigned int VSEncoder::get_optimal_split(const std::vector<unsigned int>& L, std::vector<unsigned int>& S)
    {
        unsigned int n = (unsigned)L.size();
        double min_good_bytes_ratio = 0.3;
        unsigned int pre_p, cur_p;
        pre_p = cur_p = 0;
        int max_bits = 0;
        int cur_bits = 0;
        int used_bits = 0;
        int good_bits = 0;
        S.push_back(0);
        while (true)
        {
            if (cur_p == n) {
                S.push_back(cur_p);
                break;
            }
            cur_bits = log2(L[cur_p]);
            if (max_bits < cur_bits) {
                max_bits = cur_bits;
                used_bits = (cur_p-pre_p+1)*max_bits;
            } else {
                used_bits += max_bits;
            }
            good_bits += cur_bits;
            if ((float)good_bits/(float)used_bits < min_good_bytes_ratio) {
                S.push_back(cur_p);
                max_bits = 0;
                cur_bits = 0;
                used_bits = 0;
                good_bits = 0;
                pre_p = cur_p;
                continue;
            }
            ++cur_p;
        }
        return 0;
    }
    
    
    void VSEncoder::encode( const std::vector<unsigned int>& L_ )
    {
        vector<unsigned int> L;
        L.push_back( L_[0] );
        for ( int i = 0 ; i < (int)L_.size()-1 ; ++i )
        {
            L.push_back( L_[i+1]-L_[i] );
        }
        std::vector<unsigned int> S;
        get_optimal_split( L, S );
        for ( int i = 0 ; i < (int)S.size()-1 ; ++i )
        {
            encode( L, S[i], S[i+1] );
        }
        string header;
        header += acc;
        header += (char)bits&0xff;
        unsigned int last_entry = L_.back();
        unsigned int n_entry = (unsigned)L_.size();
        pack_uint(header, n_entry);
        pack_uint(header, last_entry);
        chunk += header+buf;
    }
    
    void VSEncoder::encode( const std::vector<unsigned int>& L, int pstart, int pend )
    {
        int b = 0;
        for ( int i = pstart ; i < pend ; ++i )
        {
            int tmp = log2(L[i],false)+1;
            b = tmp > b ? tmp : b;
        }
        int k = pend-pstart;
        GammaEncoder g( buf, acc, bits );
        g.encode( b+1 );
        UnaryEncoder u( buf, acc, bits );
        u.encode( k );
        OrdinaryEncoder o( buf, acc, bits, b );
        for ( int i = pstart ; i < pend ; ++i )
        {
            o.encode( L[i] );
        }
        
    }
    
    
    unsigned int Decoder::mask[8] = { 128, 64, 32, 16, 8, 4, 2, 1 };
    
    inline unsigned int Decoder::get_bit_value( unsigned int n, int i )
    {
        if ( n & mask[i] )
        {
            return 1;
        }
        return 0;
    }
    
    unsigned int UnaryDecoder::decode()
    {
        unsigned int n = 0;
        char cur_data = 0;
        if (pos != end)
        {
            cur_data = *pos;
        }
        else
        {
            cur_data = (char)acc;
        }
        unsigned char cur_bit = 0;
        cur_bit = cur_data&mask[p_bit];
        while ( cur_bit )
        {
            n++;
            p_bit++;
            if ( p_bit < 8)
            {
                cur_bit = cur_data&mask[p_bit];
            }
            if ( p_bit == 8 )
            {
                pos++;
                if (pos != end)
                {
                    p_bit = 0;
                    cur_data = *pos;
                    cur_bit = cur_data&mask[p_bit];
                }
                else
                {
                    p_bit = 8-acc_bits;
                    cur_data = (char)acc;
                    cur_bit = cur_data&mask[p_bit];
                }
            }
        }
        n++;
        p_bit++;
        if ( p_bit == 8 )
        {
            pos++;
            if (pos != end)
            {
                p_bit = 0;
                cur_data = *pos;
                cur_bit = cur_data&mask[p_bit];
            }
            else
            {
                p_bit = 8-acc_bits;
                cur_data = (char)acc;
                cur_bit = cur_data&mask[p_bit];
            }
        }
        return n;
    }
    
    unsigned int GammaDecoder::decode()
    {
        unsigned int n_bits = p_ud->decode();
        unsigned int n = 1;
        char cur_data = 0;
        n_bits--;
        if (pos == end)
        {
            while ( n_bits-- )
            {
                int tmp = get_bit_value( acc, p_bit );
                p_bit++;
                n = n*2 + tmp;
            }
        }
        else
        {
            cur_data = *pos;
            while ( n_bits-- )
            {
                n = 2*n + (int)get_bit_value(cur_data, p_bit);
                p_bit++;
                if ( p_bit == 8 )
                {
                    p_bit = 0;
                    pos++;
                    if (pos == end)
                    {
                        cur_data = acc;
                        p_bit = 8-acc_bits;
                        while (n_bits--)
                        {
                            p_bit++;
                            n = 2*n + (int)get_bit_value(acc, p_bit);
                        }
                        break;
                    }
                    cur_data = *pos;
                }
            }
        }
        return n;
    }
    
    unsigned int OrdinaryDecoder::decode()
    {
        unsigned int n_bits = width;
        unsigned int n = 0;
        while ( n_bits )
        {
            if (pos != end)
            {
                if (n_bits <= (unsigned int)8-p_bit)
                {
                    n <<= n_bits;
                    unsigned char tmp = (*pos)&mask_nbits[p_bit][n_bits];
                    tmp >>= ( 8-p_bit-n_bits );
                    n |= tmp;
                    p_bit += n_bits;
                    if ( p_bit == 8 )
                    {
                        pos++;
                        if (pos == end)
                        {
                            p_bit = 8-acc_bits;
                        }
                        else
                        {
                            p_bit = 0;
                        }
                    }
                    return n;
                }
                else
                {
                    unsigned char tmp = (*pos)&mask_nbits[p_bit][8-p_bit];
                    n <<= (8-p_bit);
                    n |= tmp;
                    n_bits -= (8-p_bit);
                    pos++;
                    if (pos == end)
                    {
                        p_bit = 8-acc_bits;
                    }
                    else
                    {
                        p_bit = 0;
                    }
                    continue;
                }
            }
            else
            {
                n <<= n_bits;
                unsigned char tmp = acc&mask_nbits[p_bit][n_bits];
                tmp >>= ( 8-p_bit-n_bits );
                n |= tmp;
                p_bit += n_bits;
                n_bits = 0;
                return n;
            }
            
        }
        return n;
    }
    
    
    VSDecoder::VSDecoder( const std::string& buf_ )
	: buf(buf_), acc(buf[0]), acc_bits(buf[1])
    {
        bias = -1;
        pos = buf.data()+2;
        end = buf.data()+buf.size();
        unpack_uint(&pos, end, &n_entry);
        unpack_uint(&pos, end, &last_entry);
        
        if (pos == end)
        {
            p_bit = 8-acc_bits;
        }
        else
        {
            p_bit = 0;
        }
        p_gd = new GammaDecoder(pos, end, acc, acc_bits, p_bit);
        cur_num_width = p_gd->decode()-1;
        p_ud = new UnaryDecoder(pos, end, acc, acc_bits, p_bit);
        cur_remaining_nums = p_ud->decode();
        p_od = new OrdinaryDecoder(pos, end, acc, acc_bits, p_bit, cur_num_width);
        p_od->setWidth(cur_num_width);
        
    }
    
    unsigned int VSDecoder::get_first_entry()
    {
        bias = next();
        return bias;
    }
    
    unsigned int VSDecoder::get_next_entry()
    {
        bias += next();
        return bias;
    }
    
    unsigned int VSDecoder::next()
    {
        if ( pos == end && p_bit == 8 )
        {
            return ~0;
        }
        if ( cur_remaining_nums )
        {
            cur_remaining_nums--;
            return p_od->decode();
        }
        else
        {
            cur_num_width = p_gd->decode()-1;
            cur_remaining_nums = p_ud->decode();
            cur_remaining_nums--;
            p_od->setWidth( cur_num_width );
            return p_od->decode();
        }
    }
    
    unsigned int VSDecoder::get_n_entry()
    {
        return n_entry;
    }
    
    unsigned int VSDecoder::get_last_entry()
    {
        return last_entry;
    }
    VSDecoder::~VSDecoder()
    {
        if ( p_ud != NULL )
        {
            delete p_ud;
            p_ud = NULL;
        }
        if ( p_gd != NULL )
        {
            delete p_gd;
            p_gd = NULL;
        }
        if ( p_od != NULL )
        {
            delete p_od;
            p_od = NULL;
        }
    }
    
}

// Highly optimised fls() implementation.
inline int highest_order_bit(unsigned mask)
{
    int result = 0;
    if (mask >= 0x10000u) {
	mask >>= 16;
	result = 16;
    }
    if (mask >= 0x100u) {
	mask >>= 8;
	result += 8;
    }
    return result + flstab[mask];
}

namespace Xapian {

void
BitWriter::encode(size_t value, size_t outof)
{
    Assert(value < outof);
    size_t bits = highest_order_bit(outof - 1);
    const size_t spare = (1 << bits) - outof;
    if (spare) {
	/* If we have spare values, we can use one fewer bit to encode some
	 * values.  We shorten the values in the middle of the range, as
	 * testing (on positional data) shows this works best.  "Managing
	 * Gigabytes" suggests reversing this for the lowest level and encoding
	 * the end values of the range shorter, which is contrary to our
	 * testing (MG is talking about posting lists, which probably have
	 * different characteristics).
	 *
	 * For example, if outof is 11, the codes emitted are:
	 *
	 * value	output
	 * 0		0000
	 * 1		0001
	 * 2		0010
	 * 3		 011
	 * 4		 100
	 * 5		 101
	 * 6		 110
	 * 7		 111
	 * 8		1000
	 * 9		1001
	 * 10		1010
	 *
	 * Note the LSB comes first in the bitstream, so these codes need to be
	 * suffix-free to be decoded.
	 */
	const size_t mid_start = (outof - spare) / 2;
	if (value >= mid_start + spare) {
	    value = (value - (mid_start + spare)) | (1 << (bits - 1));
	} else if (value >= mid_start) {
	    --bits;
	}
    }

    if (bits + n_bits > 32) {
	// We need to write more bits than there's empty room for in
	// the accumulator.  So we arrange to shift out 8 bits, then
	// adjust things so we're adding 8 fewer bits.
	Assert(bits <= 32);
	acc |= (value << n_bits);
	buf += char(acc);
	acc >>= 8;
	value >>= 8;
	bits -= 8;
    }
    acc |= (value << n_bits);
    n_bits += bits;
    while (n_bits >= 8) {
	buf += char(acc);
	acc >>= 8;
	n_bits -= 8;
    }
}

void
BitWriter::encode_interpolative(const vector<Xapian::termpos> &pos, int j, int k)
{
    // "Interpolative code" - for an algorithm description, see "Managing
    // Gigabytes" - pages 126-127 in the second edition.  You can probably
    // view those pages in google books.
    while (j + 1 < k) {
	const size_t mid = (j + k) / 2;
	// Encode one out of (pos[k] - pos[j] + 1) values
	// (less some at either end because we must be able to fit
	// all the intervening pos in)
	const size_t outof = pos[k] - pos[j] + j - k + 1;
	const size_t lowest = pos[j] + mid - j;
	encode(pos[mid] - lowest, outof);
	encode_interpolative(pos, j, mid);
	j = mid;
    }
}

Xapian::termpos
BitReader::decode(Xapian::termpos outof, bool force)
{
    (void)force;
    Assert(force == di_current.is_initialized());
    size_t bits = highest_order_bit(outof - 1);
    const size_t spare = (1 << bits) - outof;
    const size_t mid_start = (outof - spare) / 2;
    Xapian::termpos p;
    if (spare) {
	p = read_bits(bits - 1);
	if (p < mid_start) {
	    if (read_bits(1)) p += mid_start + spare;
	}
    } else {
	p = read_bits(bits);
    }
    Assert(p < outof);
    return p;
}

unsigned int
BitReader::read_bits(int count)
{
    unsigned int result;
    if (count > 25) {
	// If we need more than 25 bits, read in two goes to ensure that we
	// don't overflow acc.  This is a little more conservative than it
	// needs to be, but such large values will inevitably be rare (because
	// you can't fit very many of them into 2^32!)
	Assert(count <= 32);
	result = read_bits(16);
	return result | (read_bits(count - 16) << 16);
    }
    while (n_bits < count) {
	Assert(idx < buf.size());
	acc |= static_cast<unsigned char>(buf[idx++]) << n_bits;
	n_bits += 8;
    }
    result = acc & ((1u << count) - 1);
    acc >>= count;
    n_bits -= count;
    return result;
}

void
BitReader::decode_interpolative(int j, int k,
				Xapian::termpos pos_j, Xapian::termpos pos_k)
{
    Assert(!di_current.is_initialized());
    di_stack.reserve(highest_order_bit(pos_k - pos_j));
    di_current.set_j(j, pos_j);
    di_current.set_k(k, pos_k);
}

Xapian::termpos
BitReader::decode_interpolative_next()
{
    Assert(di_current.is_initialized());
    while (!di_stack.empty() || di_current.is_next()) {
	if (!di_current.is_next()) {
	    Xapian::termpos pos_ret = di_current.pos_k;
	    di_current = di_stack.back();
	    di_stack.pop_back();
	    int mid = (di_current.j + di_current.k) / 2;
	    di_current.set_j(mid, pos_ret);
	    return pos_ret;
	}
	di_stack.push_back(di_current);
	int mid = (di_current.j + di_current.k) / 2;
	int pos_mid = decode(di_current.outof(), true) +
	    (di_current.pos_j + mid - di_current.j);
	di_current.set_k(mid, pos_mid);
    }
#ifdef XAPIAN_ASSERTIONS
    di_current.uninit();
#endif
    return di_current.pos_k;
}

}
