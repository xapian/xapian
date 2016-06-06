#ifndef SCORER_H
#define SCORER_H

#include <xapian.h>
#include <xapian/intrusive_ptr.h>
#include <xapian/types.h>
#include <xapian/visibility.h>

#include "ranklist.h"

using namespace std;

namespace Xapian {

class XAPIAN_VISIBILITY_DEFAULT Scorer { //TODO: Update documentation

  public:
    Scorer();

    std::vector<double> get_labels(Xapian::RankList rl);

    virtual double score(const Xapian::RankList & rl)=0;

};

}

#endif /* SCORER_H */
