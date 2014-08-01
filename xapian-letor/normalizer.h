/* normalizer.h: The abstract normalizer.
 *
 * Copyright (C) 2014 Jiarong Wei
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

#ifndef NORMALIZER_H
#define NORMALIZER_H


#include <xapian.h>
#include <xapian/intrusive_ptr.h>
#include <xapian/types.h>
#include <xapian/visibility.h>

#include "ranklist.h"

namespace Xapian {

class RankList;

class XAPIAN_VISIBILITY_DEFAULT Normalizer {

public:
    // Normalizer flag type
    typedef unsigned int normalizer_t;


    // The flag for different kinds of normalizers
    static const DEFAULT_NORMALIZER = 0;

    
    // The abstract function for normalizing
    static virtual RankList normalize(RankList rlist_) = 0;
};

}
#endif /* NORMALIZER_H */
