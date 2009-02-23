/***************************************************************************

    Copyright (C) 2009 Lemur Consulting. 

    Based on imgseek code - here are the original copyright notices:

    imgSeek ::  Haar 2d transform implemented in C/C++ to speed things up
                             -------------------
    begin                : Fri Jan 17 2003
    email                : nieder|at|mail.ru
    Time-stamp:            <05/01/30 19:58:56 rnc>
    ***************************************************************************
    *    Wavelet algorithms, metric and query ideas based on the paper        *
    *    Fast Multiresolution Image Querying                                  *
    *    by Charles E. Jacobs, Adam Finkelstein and David H. Salesin.         *
    *    <http://www.cs.washington.edu/homes/salesin/abstracts.html>          *
    ***************************************************************************

    Copyright (C) 2003 Ricardo Niederberger Cabral

    Clean-up and speed-ups by Geert Janssen <geert at ieee.org>, Jan 2006:
    - introduced names for various `magic' numbers
    - made coding style suitable for Emacs c-mode
    - expressly doing constant propagation by hand (combined scalings)
    - preferring pointer access over indexed access of arrays
    - introduced local variables to avoid expression re-evaluations
    - took out all dynamic allocations
    - completely rewrote calcHaar and eliminated truncq()
    - better scheme of introducing sqrt(0.5) factors borrowed from
      FXT package: author Joerg Arndt, email: arndt@jjj.de,
      http://www.jjj.de/
    - separate processing per array: better cache behavior
    - do away with all scaling; not needed except for DC component

    To do:
    - the whole Haar transform should be done using fixpoints

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include <math.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <cstring>
#include <queue>
#include <algorithm>


#include "haar.h"

// RGB -> YIQ colorspace conversion; Y luminance, I,Q chrominance.
// If RGB in [0..255] then Y in [0..255] and I,Q in [-127..127].
void rgb_2_yiq(Unit (&a) [num_pixels_squared], 
               Unit (&b) [num_pixels_squared],
               Unit (&c) [num_pixels_squared]) {
    int i;
    for (i = 0; i < num_pixels_squared; i++) { 
      Unit Y, I, Q; 
      Y = 0.299 * a[i] + 0.587 * b[i] + 0.114 * c[i];
      I = 0.596 * a[i] - 0.275 * b[i] - 0.321 * c[i];
      Q = 0.212 * a[i] - 0.523 * b[i] + 0.311 * c[i];
      a[i] = Y;
      b[i] = I;
      c[i] = Q;
    } 
}

// Do the Haar tensorial 2d transform itself.
// Here input is RGB data [0..255] in Unit arrays
// Computation is (almost) in-situ.

static void
haar2D(Unit a[])
{
  int i;
  Unit t[num_pixels >> 1];

  // Decompose rows:
  for (i = 0; i < num_pixels_squared; i += num_pixels) {
    int h, h1;
    Unit C = 1;

    for (h = num_pixels; h > 1; h = h1) {
      int j1, j2, k;

      h1 = h >> 1;		// h = 2*h1
      C *= 0.7071;		// 1/sqrt(2)
      for (k = 0, j1 = j2 = i; k < h1; k++, j1++, j2 += 2) {
        int j21 = j2+1;

        t[k]  = (a[j2] - a[j21]) * C;
        a[j1] = (a[j2] + a[j21]);
      }
      // Write back subtraction results:
      memcpy(a+i+h1, t, h1*sizeof(a[0]));
    }
    // Fix first element of each row:
    a[i] *= C;	// C = 1/sqrt(num_pixels)
  }

  // Decompose columns:
  for (i = 0; i < num_pixels; i++) {
    Unit C = 1;
    int h, h1;

    for (h = num_pixels; h > 1; h = h1) {
      int j1, j2, k;

      h1 = h >> 1;
      C *= 0.7071;		// 1/sqrt(2) = 0.7071
      for (k = 0, j1 = j2 = i; k < h1;
	   k++, j1 += num_pixels, j2 += 2 * num_pixels) {
        int j21 = j2 + num_pixels;

        t[k]  = (a[j2] - a[j21]) * C;
        a[j1] = (a[j2] + a[j21]);
      }
      // Write back subtraction results:
      for (k = 0, j1 = i + h1 * num_pixels; k < h1; k++, j1 += num_pixels) 
        a[j1]=t[k];
    }
    // Fix first element of each column:
    a[i] *= C;
  }
}

/* Do the Haar tensorial 2d transform itself.
   Here input is RGB data [0..255] in Unit arrays.
   Results are available in a, b, and c.
   Fully inplace calculation; order of result is interleaved though,
   but we don't care about that.
*/
void
transform(colourChan & a, colourChan &b, colourChan &c) {
  rgb_2_yiq(a, b, c);

  haar2D(a);
  haar2D(b);
  haar2D(c);

  /* Reintroduce the skipped scaling factors: */
  a[0] /= 256 * 128;
  b[0] /= 256 * 128;
  c[0] /= 256 * 128;
}

/* signature structure */
typedef struct valStruct_{
  Unit d;			/* [f]abs(a[i]) */
  int i;			/* index i of a[i] */
  bool operator< (const valStruct_ &right) const
  {
    return d > right.d;
  }
} valStruct;

typedef std::priority_queue < valStruct > valqueue;


// Find the NUM_COEFS largest numbers in cdata[] (in magnitude that is)
// and store their indices in sig[].
inline static void
get_m_largests(colourChan & cdata, coeffs &sig)
{
  int cnt, i;
  valStruct val;
  valqueue vq;			// dynamic priority queue of valStruct's

  // Could skip i=0: goes into separate avgl

  // Fill up the bounded queue. (Assuming num_pixels_squared > num_coefs)
  for (i = 1; i < num_coefs+1; i++) {
    val.i = i;
    val.d = abs(cdata[i]);
    vq.push(val);
  }
  // Queue is full (size is num_coefs)

  for (/*i = num_coefs+1*/; i < num_pixels_squared; i++) {
    val.d = abs(cdata[i]);

    if (val.d > vq.top().d) {
      // Make room by dropping smallest entry:
      vq.pop();
      // Insert val as new entry:
      val.i = i;
      vq.push(val);
    }
    // else discard: do nothing
  }

  // Empty the (non-empty) queue and fill-in sig:
  cnt=0;
  do {
    val = vq.top();
    sig.insert(cdata[val.i] <= 0 ? -val.i : val.i);
    vq.pop();
  } while(!vq.empty());
  // Must have cnt==num_coefs here.
}

// Determines a total of num_coefs positions in the image that have the
// largest magnitude (absolute value) in color value. Returns linearized
// coordinates in sig1, sig2, and sig3. avgl are the [0,0] values.
// The order of occurrence of the coordinates in sig doesn't matter.
// Complexity is 3 x num_pixels^2 x 2log(num_coefs).
void
calcHaar(colourChan &cdata1, colourChan & cdata2, colourChan & cdata3,
	 coeffs & sig1, coeffs &sig2, coeffs& sig3, double *avgl)
{
  avgl[0]=cdata1[0];
  avgl[1]=cdata2[0];
  avgl[2]=cdata3[0];

  // Color channel 1:
  get_m_largests(cdata1, sig1);

  // Color channel 2:
  get_m_largests(cdata2, sig2);

  // Color channel 3:
  get_m_largests(cdata3, sig3);
}

void transform_and_calc(colourChan &a, colourChan &b, colourChan & c,
                        coeffs & sig1, coeffs &sig2, coeffs& sig3,
                        double *avgl) {
  transform(a, b, c);
  calcHaar(a, b, c, sig1, sig2, sig3, avgl);
}
