/* normalizer.h: The Normalizer -- used to normalize RankList.
 *
 */

#ifndef NORMALIZER_H
#define NORMALIZER_H


#include <xapian.h>
#include <xapian/intrusive_ptr.h>
#include <xapian/types.h>
#include <xapian/visibility.h>

#include "featurevector.h"

#include <string>
#include <vector>

using std::string;
using std::vector;

namespace Xapian {

class RankList;

class XAPIAN_VISIBILITY_DEFAULT Normalizer {

public:
    
    static virtual RankList normalize(RankList rlist_) = 0;
};

}
#endif /* NORMALIZER_H */
