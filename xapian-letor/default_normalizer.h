/* default_normalizer.h: The default Normalizer -- used to normalize RankList.
 *
 */

#ifndef DEFAULT_NORMALIZER_H
#define DEFAULT_NORMALIZER_H


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

class XAPIAN_VISIBILITY_DEFAULT DefaultNormalizer {

public:
    
    static RankList normalize(RankList rlist_);
};

}
#endif /* DEFAULT_NORMALIZER_H */
