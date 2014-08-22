/* default_normalizer.cc: The default normalizer.
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110=1301
 * USA
 */

#ifndef DEFAULT_NORMALIZER_H
#define DEFAULT_NORMALIZER_H


#include <xapian.h>
#include <xapian/intrusive_ptr.h>
#include <xapian/types.h>
#include <xapian/visibility.h>

#include "normalizer.h"
#include "ranklist.h"

#include <string>
#include <vector>

using std::string;
using std::vector;

namespace Xapian {

class RankList;

class XAPIAN_VISIBILITY_DEFAULT DefaultNormalizer : public Normalizer {

public:

	virtual ~DefaultNormalizer();
    
    RankList normalize(RankList rlist_);
};

}

#endif /* DEFAULT_NORMALIZER_H */
