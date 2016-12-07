/** @file lmweight.cc
 * @brief Xapian::LMWeight class - the Unigram Language Modelling formula.
 */
/* Copyright (C) 2012 Gaurav Arora
 * Copyright (C) 2016 Olly Betts
 * Copyright (C) 2016 Vivek Pal
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include <config.h>

#include "xapian/weight.h"

#include "debuglog.h"
#include "omassert.h"
#include "serialise-double.h"

#include "xapian/error.h"

#include <cmath>

using namespace std;

namespace Xapian {

LMWeight *
LMWeight::clone() const {
    return new LMWeight(param_log, select_smoothing, param_smoothing1, param_smoothing2);
}

void
LMWeight::init(double factor_)
{
    // weight_collection is really factor.
    weight_collection = factor_;

    /* Setting default values of the param_log to handle negative value of log.
     * It is considered to be upperbound of document length.
     * initializing param_log to upperbound of document_length.
     */

    if (param_log == 0.0) {
	param_log = get_doclength_upper_bound();
    }

    /* Since the optimal parameter for Jelinek mercer smoothing
     * is based on query length, so if query is title query changing
     * default value of smoothing parameter.
     */

    if (select_smoothing == JELINEK_MERCER_SMOOTHING ||
	select_smoothing == TWO_STAGE_SMOOTHING) {
	if (param_smoothing1 == 0.7) {
	    if (get_query_length() <= 2) {
		param_smoothing1 = 0.1;
	    }
	}
    }

    /* param_smoothing1 default value should be 2000 in case
     * DIRICHLET_SMOOTHING is selected. Tweaking param_smoothing1
     * if user supply his own value for param_smoothing1 value will not be set
     * to 2000(default value)
     */
    if (select_smoothing == DIRICHLET_SMOOTHING) {
	if (param_smoothing1 == 0.7) {
	    param_smoothing1 = 2000;
	}
    }

    /* Setting param_smoothing1 and param_smoothing2 default value to used when
     * DIRICHLET_PLUS_SMOOTHING is selected.*/
    if (select_smoothing == DIRICHLET_PLUS_SMOOTHING) {
	if (param_smoothing1 == 0.7) {
	    param_smoothing1 = 2000;
	}
    }
}

string
LMWeight::name() const
{
    return "Xapian::LMWeight";
}

string
LMWeight::serialise() const
{
    string result = serialise_double(param_log);
    result += static_cast<unsigned char>(select_smoothing);
    result += serialise_double(param_smoothing1);
    result += serialise_double(param_smoothing2);
    return result;
}

LMWeight *
LMWeight::unserialise(const string & s) const
{
    const char *ptr = s.data();
    const char *end = ptr + s.size();
    double param_log_ = unserialise_double(&ptr, end);
    type_smoothing select_smoothing_ = static_cast<type_smoothing>(*(ptr)++);
    double param_smoothing1_ = unserialise_double(&ptr, end);
    double param_smoothing2_ = unserialise_double(&ptr, end);
    if (rare(ptr != end))
	throw Xapian::SerialisationError("Extra data in LMWeight::unserialise()");
    return new LMWeight(param_log_, select_smoothing_, param_smoothing1_, param_smoothing2_);
}

double
LMWeight::get_sumpart(Xapian::termcount wdf, Xapian::termcount len,
		      Xapian::termcount uniqterm) const
{
    // Within Document Frequency of the term in document being considered.
    double wdf_double = wdf;
    // Length of the Document in terms of number of terms.
    double len_double = len;
    // variable to store weight contribution of term in the document scoring for LM.
    double weight_sum;

    /* In case the within document frequency of term is zero smoothing will
     * be required and should be return instead of returning zero, as returning
     * LM score are multiplication of contribution of all terms, due to absence
     * of single term whole document is scored zero, hence apply collection
     * frequency smoothing.
     */
    double wt_coll =
	get_collection_freq() / (get_collection_size() * get_average_length());

    // Calculating weights considering different smoothing option available to user.
    if (select_smoothing == JELINEK_MERCER_SMOOTHING) {
	/* Maximum likelihood of current term, weight contribution of term in
	 * case query term is present in the document.
	 */
	double weight_document = wdf_double / len_double;
	weight_sum = (param_smoothing1 * wt_coll) +
		     ((1 - param_smoothing1) * weight_document);
    } else if (select_smoothing == DIRICHLET_SMOOTHING) {
	weight_sum = (wdf_double + (param_smoothing1 * wt_coll)) /
		     (len_double + param_smoothing1);
    } else if (select_smoothing == DIRICHLET_PLUS_SMOOTHING) {
	/* In the Dir+ weighting formula, sumpart weight contribution is :-
	 *
	 * sum of log of (1 + (wdf/(param_smoothing1 * wt_coll))) and
	 * log of (1 + (delta/param_smoothing1 * wt_coll))).
	 * Since, sum of logs is log of product so weight_sum is calculated as product
	 * of terms in log in the Dir+ formula.
	 */
	weight_sum = (1 + (wdf_double / (param_smoothing1 * wt_coll))) *
		     (1 + (param_smoothing2 / (param_smoothing1 * wt_coll)));
    } else if (select_smoothing == ABSOLUTE_DISCOUNT_SMOOTHING) {
	double uniqterm_double = uniqterm;
	weight_sum = ((((wdf_double - param_smoothing1) > 0) ? (wdf_double - param_smoothing1) : 0) / len_double) + ((param_smoothing1 * wt_coll * uniqterm_double) / len_double);
    } else {
	weight_sum = (((1 - param_smoothing1) * (wdf_double + (param_smoothing2 * wt_coll)) / (len_double + param_smoothing2)) + (param_smoothing1 * wt_coll));
    }

    /* Since LM score is calculated with multiplication, instead of changing
     * the current implementation log trick have been used to calculate the
     * product since (sum of log is log of product and since aim is ranking
     * ranking document by product or log of product won't make a large
     * difference hence log(product) will be used for ranking.
     */
    double product = weight_sum * param_log;
    // weight_collection is really factor.
    return (product > 1.0) ? weight_collection * log(product) : 0;
}

double
LMWeight::get_maxpart() const
{
    // Variable to store the collection frequency
    double upper_bound;
    // Store upper bound on wdf in variable wdf_max
    double wdf_max = get_wdf_upper_bound();

    /* In case the within document frequency of term is zero smoothing will
     * be required and should be return instead of returning zero, as
     * returning LM score are multiplication of contribution of all terms,
     * due to absence of single term whole document is scored zero, hence
     * apply collection frequency smoothing.
     */
    double wt_coll =
	get_collection_freq() / (get_collection_size() * get_average_length());

    // Calculating upper bound considering different smoothing option available to user.
    if (select_smoothing == JELINEK_MERCER_SMOOTHING) {
	upper_bound = (param_smoothing1 * wt_coll) + (1 - param_smoothing1);
    } else if (select_smoothing == DIRICHLET_SMOOTHING) {
	upper_bound = (get_doclength_upper_bound() + (param_smoothing1 * wt_coll)) / (get_doclength_upper_bound() + param_smoothing1);
    } else if (select_smoothing == DIRICHLET_PLUS_SMOOTHING) {
	upper_bound = (1 + (wdf_max / (param_smoothing1 * wt_coll))) *
		      (1 + (param_smoothing2 / (param_smoothing1 * wt_coll)));
    } else if (select_smoothing == ABSOLUTE_DISCOUNT_SMOOTHING) {
	upper_bound = param_smoothing1 * wt_coll + 1;
    } else {
	upper_bound = (((1 - param_smoothing1) * (get_doclength_upper_bound() + (param_smoothing2 * wt_coll)) / (get_doclength_upper_bound() + param_smoothing2)) + (param_smoothing1 * wt_coll));
    }

    /* Since weight are calculated using log trick, using same with the bounds. Refer
     * comment in get_sumpart for the details.
     */
    double product = upper_bound * param_log;
    // weight_collection is really factor.
    return (product > 1.0) ? weight_collection * log(product) : 1.0;
}


/* The extra weight component in the Dir+ formula is :-
 *
 * |Q| * log (param_smoothing1 / (|D| + param_smoothing1))
 *
 * where, |Q| is total query length.
 *	  |D| is total document length.
 */
double
LMWeight::get_sumextra(Xapian::termcount len, Xapian::termcount) const
{
    if (select_smoothing == DIRICHLET_PLUS_SMOOTHING) {
	double extra_weight = param_smoothing1 / (len + param_smoothing1);
	return get_query_length() * log(extra_weight);
    }
    return 0;
}

double
LMWeight::get_maxextra() const
{
    if (select_smoothing == DIRICHLET_PLUS_SMOOTHING) {
	double extra_weight = param_smoothing1 / (get_doclength_lower_bound() + param_smoothing1);
	return get_query_length() * log(extra_weight);
    }
    return 0;
}

}
