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

/** Calculate the size of buffer required to safely encode a block of data
 *  as base64 coded data.
 *  @param inlen The number of bytes of uncoded data.
 *  @returns The number of bytes required for the base64 coded data.
 */
extern int
base64_encode_size(int inlen);

/** Calculate the size of buffer required to safely decode a block of base64
 *  coded data.
 *  @param inlen The number of bytes of base64 coded data.
 *  @returns The number of bytes required for the uncoded data.
 */
extern int
base64_decode_size(int inlen);

/** Encode a piece of data to base64.
 *  @param in    The data to encode.
 *  @param inlen The length of the data to encode.
 *  @param out   A buffer (of size at least \a base64_encode_size(\a inlen))
 *               in which the encoded output will be placed.
 *  @returns The length of the encoded data, in bytes.
 */
extern int
base64_encode(const char *in, int inlen, char * out);

/** Decode a piece of base64 encoded data.
 *  @param in    The base64 coded data to decode.
 *  @param inlen The length of the data to decode.  This must be a multiple of 4.
 *  @param out   A buffer (of size at least \a base64_decode_size(\a inlen))
 *               in which the decoded output will be placed.
 *  @returns The length of the decoded data, in bytes, or -1 on error.
 */
extern int
base64_decode(const char * in, int inlen, char *out);

#include <string>
class Base64
{
    public:
        static std::string encode(const char * in, int inlen)
        {
            char * out = new char[base64_encode_size(inlen)];
            int outlen = base64_encode(in, inlen, out);
            std::string result(out, outlen);
            delete out;
            return result;
        }

        static std::string decode(const char * in, int inlen)
        {
            char * out = new char[base64_decode_size(inlen)];
            int outlen = base64_decode(in, inlen, out);
            if (outlen == -1) throw std::string("Invalid base64 input");
            std::string result((char *)out, outlen);
            delete out;
            return result;
        }
};
