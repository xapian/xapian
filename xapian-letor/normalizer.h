/* normalizer.h: The Normalizer -- used to normalize RankList.
 *
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

    
    
    static virtual RankList normalize(RankList rlist_) = 0;
};

}
#endif /* NORMALIZER_H */
