#ifndef SCORER_H
#define SCORER_H

#include <xapian.h>
#include <xapian/intrusive_ptr.h>
#include <xapian/types.h>
#include <xapian/visibility.h>

#include "featurevector.h"

using namespace std;

namespace Xapian {

class XAPIAN_VISIBILITY_DEFAULT Scorer { //TODO: Update documentation

  public:
    Scorer();

    std::vector<double> get_labels(const std::vector<Xapian::FeatureVector> & fvv);

    virtual double score(const std::vector<Xapian::FeatureVector> & fvv)=0;

};

}

#endif /* SCORER_H */
