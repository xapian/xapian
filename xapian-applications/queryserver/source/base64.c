/*
 * Base64 encoding and decoding.
 */

/*
The code in this file is taken from PuTTY, see
http://www.chiark.greenend.org.uk/~sgtatham/putty/

PuTTY is copyright 1997-2004 Simon Tatham.

Portions copyright Robert de Bath, Joris van Rantwijk, Delian
Delchev, Andreas Schultz, Jeroen Massar, Wez Furlong, Nicolas Barry,
Justin Bradford, Ben Harris, and CORE SDI S.A.

Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation files
(the "Software"), to deal in the Software without restriction,
including without limitation the rights to use, copy, modify, merge,
publish, distribute, sublicense, and/or sell copies of the Software,
and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT.  IN NO EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE
FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "base64.h"

/** Encode up to 3 bytes of data into 4 bytes of base64 encoded data.
 *  
 *  @param in   A pointer to the input data.
 *  @param n    The number of bytes of input data available.
 *              Must be 1, 2 or 3.
 *  @param atom A pointer to an array of characters of length 4, in which the
 *              result will be placed.
 */
static void
base64_encode_atom(const unsigned char *in, int n, char *atom)
{
    static const char base64_chars[] =
            "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    unsigned word;

    word = in[0] << 16;
    if (n > 1)
        word |= in[1] << 8;
    if (n > 2)
        word |= in[2];
    atom[0] = base64_chars[(word >> 18) & 0x3F];
    atom[1] = base64_chars[(word >> 12) & 0x3F];
    if (n > 1)
        atom[2] = base64_chars[(word >> 6) & 0x3F];
    else
        atom[2] = '=';
    if (n > 2)
        atom[3] = base64_chars[word & 0x3F];
    else
        atom[3] = '=';
}

/** Returns true if the specified character is in the base 64 alphabet,
 *  false otherwise.  The alphabet consists of "A-Za-z0-9+/=".
 */
static int
base64_is_base64_char(char c)
{
    return ((c >= 'A' && c <= 'Z') ||
            (c >= 'a' && c <= 'z') ||
            (c >= '0' && c <= '9') ||
            (c == '+') ||
            (c == '/') ||
            (c == '='));
}

/** Decode up to 4 bytes of base64 encoded data into up to 3 bytes of data.
 *  @param atom A pointer to the base64 encoded data.
 *  @param out  An array of size 3 in which the decoded data will be placed.
 *  @returns The number of bytes which were decoded.
 */
static int
base64_decode_atom(const char *atom, unsigned char *out)
{
    int vals[4];
    int i, v, len;
    unsigned word;
    char c;

    for (i = 0; i < 4; i++) {
	c = atom[i];
	if (c >= 'A' && c <= 'Z')
	    v = c - 'A';
	else if (c >= 'a' && c <= 'z')
	    v = c - 'a' + 26;
	else if (c >= '0' && c <= '9')
	    v = c - '0' + 52;
	else if (c == '+')
	    v = 62;
	else if (c == '/')
	    v = 63;
	else if (c == '=')
	    v = -1;
	else
	    return 0;		       /* invalid atom */
	vals[i] = v;
    }

    if (vals[0] == -1 || vals[1] == -1)
	return 0;
    if (vals[2] == -1 && vals[3] != -1)
	return 0;

    if (vals[3] != -1)
	len = 3;
    else if (vals[2] != -1)
	len = 2;
    else
	len = 1;

    word = ((vals[0] << 18) |
	    (vals[1] << 12) | ((vals[2] & 0x3F) << 6) | (vals[3] & 0x3F));
    out[0] = (word >> 16) & 0xFF;
    if (len > 1)
	out[1] = (word >> 8) & 0xFF;
    if (len > 2)
	out[2] = word & 0xFF;
    return len;
}

extern int
base64_encode_size(int inlen)
{
    /* 4 bytes for each atom, 1 byte for each newline. */
    return ((inlen + 2) / 3) * 4 + (inlen == 0 ? 0 : (inlen - 1) / 64);
}

extern int
base64_decode_size(int inlen)
{
    return ((inlen + 3) / 4) * 3;
}

extern int
base64_encode(const char * in, int inlen, char * out)
{
    int outlen = 0;
    int linelen = 0;
    while (inlen > 0) {
        int n = (inlen < 3 ? inlen : 3);
        if (linelen >= 64) {
            *out = '\n';
            out += 1;
            linelen = 0;
        }
	base64_encode_atom((const unsigned char *)in, n, out);
	in += n;
	inlen -= n;
        out += 4;
        outlen += 4;
        linelen += 4;
    }
    return outlen;
}

extern int
base64_decode(const char * in, int inlen, char *out)
{
    int inpos;
    int outlen = 0;

    for (inpos = 0; inpos + 4 <= inlen; inpos += 4) {
        /* Move past any invalid characters. */
        while (!base64_is_base64_char(in[inpos]) && inpos < inlen) {
            inpos += 1;
        }
        if (inpos + 4 > inlen) {
            /* If there is no data left, return decoded length. */
            if (inpos >= inlen) return outlen;
            /* If there isn't a whole atom left, return an error. */
            return -1;
        }
        int decoded = base64_decode_atom(in + inpos,
                                         (unsigned char *)out + outlen);
        if (!decoded) {
            return -1;
        }
        outlen += decoded;
    }
    return outlen;
}
