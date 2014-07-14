/* default_normalizer.cc: The default Normalizer -- used to normalize RankList.
 *
 */

#include <xapian.h>
#include <xapian/intrusive_ptr.h>
#include <xapian/types.h>
#include <xapian/visibility.h>

#include "featurevector.h"

#include <string>
#include <vector>

using namespace Xapian;
using std::string;
using std::vector;

static RankList
DefaultNormalizer::normalize(RankList rlist_) {
	RankList rlist;

	rlist.set_qid( rlist_.get_qid() );

	vector<FeatureVector> feature_vector_list = rlist_.get_feature_vector_list();

	int feature_num = rlist_.get_feature_num();

	double feature_max_value[feature_num];
	memset(feature_max_value, 0, sizeof(double)*feature_num);

	// look for the max value for each feature
	for (vector<FeatureVector>::iterator fv_list_it = feature_vector_list.begin();
			fv_list_it != feature_vector_list.end(); ++fv_list_it) {
		for (int feature_idx = 0; feature_idx < feature_num; ++feature_idx) {
			if ((*fv_list_it)[feature_idx] > feature_max_value[feature_idx])
				feature_max_value[feature_idx] = (*fv_list_it)[feature_idx];
		}
	}

	// perform the normalization
	for (vector<FeatureVector>::iterator fv_list_it = feature_vector_list.begin();
			fv_list_it != feature_vector_list.end(); ++fv_list_it) {
		for (int feature_idx = 0; feature_idx < feature_num; ++feature_idx) {
			if (feature_max_value[feature_idx] != 0)
				(*fv_list_it)[feature_idx] /= feature_max_value[feature_idx];
		}
	}

	rlist.set_feature_vector_list(feature_vector_list)

	return rlist;
}

#endif /* DEFAULT_NORMALIZER_H */
