/***************************************************************************
    Copyright (C) 2009 Lemur Consulting.

    Based on imgseek code:

    Copyright (C) 2003 Ricardo Niederberger Cabral
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

#ifndef HAAR_H
#define HAAR_H
#include <set>

/* Number of pixels on one side of image; required to be a power of 2. */
const int num_pixels = 128;

/* Totals pixels in a square image. */
const int num_pixels_squared = num_pixels * num_pixels;

/* Number of Haar coeffients we retain as signature for an image. */
const int num_coefs = 40;

typedef double Unit;

// Needs to be enough to index num_pixel_squared.  can't be unsigned
// as the sign is used to distinguish between positive and negative
// coeeficients.
typedef int Idx;

// A set of indexes, representing the most significant wavelet
// coefficients
typedef std::set<Idx> coeffs;

// Colour data from an image
typedef Unit colourChan [num_pixels_squared];

// transform image data into wavelet coefficient (in-place)
// for each colour channel - in a, b, c;
// then put most significant num_coefs for each in sig1, sig2, sig3
void transform_and_calc(colourChan & a, colourChan & b, colourChan & c, 
                        coeffs & sig1, coeffs &sig2, coeffs& sig3,
                        double *avgl);

#endif //ifndef HAAR_H
