/** @file
 * @brief Unigram Language Modelling weighting classes
 */
/* Copyright (C) 2012 Gaurav Arora
 * Copyright (C) 2016,2019,2024 Olly Betts
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
#include "weightinternal.h"

#include "debuglog.h"
#include "omassert.h"
#include "serialise-double.h"
#include "stringutils.h"

#include "xapian/error.h"

#include <cmath>

using namespace std;

[[noreturn]]
static inline void
parameter_error(const char* message, const std::string& scheme,
		const char* params)
{
    Xapian::Weight::Internal::parameter_error(message, scheme, params);
}

/* The equations for Jelinek-Mercer, Dirichlet, and Absolute Discount smoothing
 * are taken from:
 *
 * Zhai, C., & Lafferty, J.D. (2004). A study of smoothing methods for language
 * models applied to information retrieval. ACM Trans. Inf. Syst., 22, 179-214.
 *
 * The equations for Two-Stage smoothing are also from this paper, but aren't
 * given explicitly there, so we have derived them using the approach
 * described in the paper.
 *
 * Dir+ comes from:
 *
 * Lv, Y., & Zhai, C. (2011). Lower-bounding term frequency normalization.
 * International Conference on Information and Knowledge Management.
 */

namespace Xapian {

void
LMJMWeight::init(double factor_)
{
    factor = factor_ * get_wqf();

    Xapian::totallength total_length = get_total_length();
    if (rare(total_length == 0)) {
	// Avoid dividing by zero in the corner case where the database has no
	// terms with wdf.
	AssertEq(get_collection_freq(), 0);
	multiplier = 0;
	return;
    }

    double lambda = param_lambda;
    if (lambda <= 0.0 || lambda >= 1.0) {
	auto query_len = get_query_length();
	if (query_len <= 2) {
	    lambda = 0.1;
	} else if (query_len < 8) {
	    lambda = (query_len - 1) * 0.1;
	} else {
	    lambda = 0.7;
	}
    }

    // Pre-calculate multiplier.
    auto collection_freq = get_collection_freq();
    multiplier = (1.0 - lambda) * total_length / (lambda * collection_freq);
}

double
LMJMWeight::get_sumpart(Xapian::termcount wdf, Xapian::termcount len,
		        Xapian::termcount, Xapian::termcount) const
{
    double w = multiplier * wdf / len;
    return factor * log(1.0 + w);
}

double
LMJMWeight::get_maxpart() const
{
    Xapian::termcount wdf_max = get_wdf_upper_bound();
    Xapian::termcount len_min = get_doclength_lower_bound();
    double w = multiplier;
    if (wdf_max < len_min) {
	// Clearly wdf / len <= wdf_max / len_min, but we also know that
	// wdf <= len so wdf / len <= 1 so we use whichever bound is tighter.
	w *= double(wdf_max) / len_min;
    }
    return factor * log(1.0 + w);
}

LMJMWeight*
LMJMWeight::clone() const {
    return new LMJMWeight(param_lambda);
}

string
LMJMWeight::name() const
{
    return "lmjm";
}

string
LMJMWeight::serialise() const
{
    return serialise_double(param_lambda);
}

LMJMWeight*
LMJMWeight::unserialise(const string& s) const
{
    const char *ptr = s.data();
    const char *end = ptr + s.size();
    double lambda = unserialise_double(&ptr, end);
    if (rare(ptr != end))
	throw Xapian::SerialisationError("Extra data in "
					 "LMJMWeight::unserialise()");
    return new LMJMWeight(lambda);
}


LMJMWeight*
LMJMWeight::create_from_parameters(const char* params) const
{
    const char* p = params;
    double lambda = 0.0;
    if (*p && !Xapian::Weight::Internal::double_param(&p, &lambda))
	parameter_error("Parameter lambda is invalid", "lmjm", params);
    if (*p)
	parameter_error("Extra data after parameter", "lmjm", params);
    return new Xapian::LMJMWeight(lambda);
}

void
LMDirichletWeight::init(double factor_)
{
    factor = factor_ * get_wqf();

    double mu = param_mu;

    auto doclen_max = get_doclength_upper_bound();
    extra_offset = get_query_length() * log(doclen_max + mu);
    if (factor == 0.0) {
	// This object is for the term-independent contribution.
	return;
    }

    Xapian::totallength total_length = get_total_length();
    if (rare(total_length == 0)) {
	// Avoid dividing by zero in the corner case where the database has no
	// terms with wdf.
	AssertEq(get_collection_freq(), 0);
	multiplier = 0;
	return;
    }

    // Pre-calculate the factor to multiply by.
    multiplier = total_length / (get_collection_freq() * mu);

    double delta = param_delta;
    if (delta != 0.0) {
	// Include the query-independent Dir+ extra contribution in factor.
	factor *= log(1.0 + delta * multiplier);
    }
}

double
LMDirichletWeight::get_sumpart(Xapian::termcount wdf, Xapian::termcount,
			       Xapian::termcount, Xapian::termcount) const
{
    return factor * log(1.0 + wdf * multiplier);
}

double
LMDirichletWeight::get_maxpart() const
{
    Xapian::termcount wdf_max = get_wdf_upper_bound();
    return factor * log(1.0 + wdf_max * multiplier);
}

double
LMDirichletWeight::get_sumextra(Xapian::termcount doclen,
				Xapian::termcount,
				Xapian::termcount) const
{
    // Formula (6) in Zhai and Lafferty 2004 includes "a document-dependent
    // constant" which is:
    //
    //   query_len * log(mu / (doclen + mu))
    //
    // (The equation for Dir+ is the same.)
    //
    // Both mu and doclen are positive, so this value will be negative but
    // get_sumextra() must return a non-negative value.  To solve this we
    // decompose this into:
    //
    //   query_len * log(mu) - query_len * log(doclen + mu)
    //
    // The first part is constant for a given query so we ignore it.  The second
    // part is still negative, but we can add query_len * log(doclen_max + mu)
    // (also constant for a given query) to address that, giving:
    //
    //   query_len * log(doclen_max + mu) - query_len * log(doclen + mu)
    //
    // We pre-calculate the first part in init() and save it in member variable
    // extra_weight.
    return extra_offset - get_query_length() * log(doclen + param_mu);
}

double
LMDirichletWeight::get_maxextra() const
{
    auto doclen_min = get_doclength_lower_bound();
    return extra_offset - get_query_length() * log(doclen_min + param_mu);
}

LMDirichletWeight*
LMDirichletWeight::clone() const {
    return new LMDirichletWeight(param_mu, param_delta);
}

string
LMDirichletWeight::name() const
{
    return "lmdirichlet";
}

string
LMDirichletWeight::serialise() const
{
    string result = serialise_double(param_mu);
    result += serialise_double(param_delta);
    return result;
}

LMDirichletWeight*
LMDirichletWeight::unserialise(const string& s) const
{
    const char *ptr = s.data();
    const char *end = ptr + s.size();
    double mu = unserialise_double(&ptr, end);
    double delta = unserialise_double(&ptr, end);
    if (rare(ptr != end))
	throw Xapian::SerialisationError("Extra data in "
					 "LMDirichletWeight::unserialise()");
    return new LMDirichletWeight(mu, delta);
}


LMDirichletWeight*
LMDirichletWeight::create_from_parameters(const char* params) const
{
    const char* p = params;
    double mu = 2000.0;
    double delta = 0.05;
    if (*p && !Xapian::Weight::Internal::double_param(&p, &mu))
	parameter_error("Parameter mu is invalid", "lmdirichlet", params);
    if (*p && !Xapian::Weight::Internal::double_param(&p, &delta))
	parameter_error("Parameter delta is invalid", "lmdirichlet", params);
    if (*p)
	parameter_error("Extra data after parameters", "lmdirichlet", params);
    return new Xapian::LMDirichletWeight(mu, delta);
}

void
LMAbsDiscountWeight::init(double factor_)
{
    factor = factor_ * get_wqf();

    auto doclen_max = get_doclength_upper_bound();
    extra_offset = get_query_length() * log(double(doclen_max));

    Xapian::totallength total_length = get_total_length();
    if (rare(total_length == 0)) {
	// Avoid dividing by zero in the corner case where the database has no
	// terms with wdf.
	AssertEq(get_collection_freq(), 0);
	multiplier = 0;
	return;
    }

    // Pre-calculate the factor to multiply by.
    multiplier = total_length / (param_delta * get_collection_freq());
}

double
LMAbsDiscountWeight::get_sumpart(Xapian::termcount wdf,
				 Xapian::termcount,
				 Xapian::termcount uniqterms,
				 Xapian::termcount) const
{
    return factor * log(1.0 + (wdf - param_delta) / uniqterms * multiplier);
}

double
LMAbsDiscountWeight::get_maxpart() const
{
    Xapian::termcount doclen_min = get_doclength_lower_bound();
    Xapian::termcount wdf_max = get_wdf_upper_bound();
    double x = (wdf_max - param_delta) * multiplier;
    // We need a lower bound on uniqterms.  We have doclen = sum(wdf) so:
    //
    //   uniqterms >= ceil(doclen_min / wdf_max)
    //
    // We also know uniqterms >= 1 (since documents without terms won't match).
    if (doclen_min > wdf_max)
	x *= (doclen_min - 1) / wdf_max + 1;
    return factor * log(1.0 + x);
}

double
LMAbsDiscountWeight::get_sumextra(Xapian::termcount doclen,
				  Xapian::termcount uniqterms,
				  Xapian::termcount) const
{
    // Formula (6) in Zhai and Lafferty 2004 includes "a document-dependent
    // constant" which is:
    //
    //   query_len * log(delta * uniqterms / doclen)
    //
    // We know delta < 1 and uniqterms <= doclen so this value will be negative
    // but get_sumextra() must return a non-negative value.  To solve this we
    // decompose this into:
    //
    //   query_len * log(delta) + query_len * log(uniqterms / doclen)
    //
    // The first part is constant for a given query so we ignore it.  The second
    // part is still negative, but we can add query_len * log(1 / doclen_min)
    // (also constant for a given query) to address that, giving:
    //
    //   -query_len * log(1 / doclen_max) + query_len * log(uniqterms / doclen)
    //      = query_len * log(doclen_max) + query_len * log(uniqterms / doclen)
    //
    // We pre-calculate the first part in init() and save it in member variable
    // extra_weight.
    return extra_offset + get_query_length() * log(double(uniqterms) / doclen);
}

double
LMAbsDiscountWeight::get_maxextra() const
{
    // Our best bound seems to be based on uniqterms <= doclen, which gives
    // log(uniqterms / doclen> <= 0
    return extra_offset;
}

LMAbsDiscountWeight*
LMAbsDiscountWeight::clone() const {
    return new LMAbsDiscountWeight(param_delta);
}

string
LMAbsDiscountWeight::name() const
{
    return "lmabsdiscount";
}

string
LMAbsDiscountWeight::serialise() const
{
    return serialise_double(param_delta);
}

LMAbsDiscountWeight*
LMAbsDiscountWeight::unserialise(const string& s) const
{
    const char *ptr = s.data();
    const char *end = ptr + s.size();
    double delta = unserialise_double(&ptr, end);
    if (rare(ptr != end))
	throw Xapian::SerialisationError("Extra data in "
					 "LMAbsDiscountWeight::unserialise()");
    return new LMAbsDiscountWeight(delta);
}


LMAbsDiscountWeight*
LMAbsDiscountWeight::create_from_parameters(const char* params) const
{
    const char* p = params;
    double delta = 0.7;
    if (*p && !Xapian::Weight::Internal::double_param(&p, &delta))
	parameter_error("Parameter delta is invalid", "lmabsdiscount", params);
    if (*p)
	parameter_error("Extra data after parameter", "lmabsdiscount", params);
    return new Xapian::LMAbsDiscountWeight(delta);
}

void
LM2StageWeight::init(double factor_)
{
    factor = factor_ * get_wqf();

    double lambda = param_lambda;
    double mu = param_mu;

    auto doclen_max = get_doclength_upper_bound();
    extra_offset = -log((lambda * doclen_max + mu) / (doclen_max + mu));
    extra_offset *= get_query_length();

    Xapian::totallength total_length = get_total_length();
    if (rare(total_length == 0)) {
	// Avoid dividing by zero in the corner case where the database has no
	// terms with wdf.
	AssertEq(get_collection_freq(), 0);
	multiplier = 0;
	return;
    }

    // Pre-calculate the factor to multiply by.
    multiplier = (1 - lambda) * total_length / get_collection_freq();
}

double
LM2StageWeight::get_sumpart(Xapian::termcount wdf,
			    Xapian::termcount doclen,
			    Xapian::termcount,
			    Xapian::termcount) const
{
    // Termweight formula is: log{ 1 + (1-λ) c(w;d) / ( (λ|d|+μ) p(w|C) ) }
    double lambda = param_lambda;
    double mu = param_mu;
    return factor * log(1.0 + wdf / (lambda * doclen + mu) * multiplier);
}

double
LM2StageWeight::get_maxpart() const
{
    double lambda = param_lambda;
    double mu = param_mu;
    Xapian::termcount doclen_min = get_doclength_lower_bound();
    Xapian::termcount wdf_max = get_wdf_upper_bound();
    // We know wdf <= doclen so if the bounds don't rule out them being equal
    // we want to find wdf value w to maximise (w / (lambda * w + mu)) which is
    // just a case of maximising w, i.e. wdf_max.  Otherwise we evaluate at
    // wdf = wdf_max, doclen = doclen_min.
    double x = wdf_max / (lambda * max(doclen_min, wdf_max) + mu);
    return factor * log(1.0 + x * multiplier);
}

double
LM2StageWeight::get_sumextra(Xapian::termcount doclen,
			     Xapian::termcount,
			     Xapian::termcount) const
{
    // Formula (6) in Zhai and Lafferty 2004 includes "a document-dependent
    // constant" which is:
    //
    //   query_len * log αd
    //
    // where αd = ( λ|d| + μ ) / ( |d| + μ ) for two-stage smoothing, so:
    //
    //   query_len * log{ (lamba * doclen + mu) / (doclen + mu) }
    //
    // We know lambda < 1 so this will be negative but get_sumextra() must
    // return a non-negative value.  To achieve this we subtract:
    //
    //   query_len * log{ (lamba * doclen_max + mu) / (doclen_max + mu) }
    //
    // We pre-calculate the negation of this and store it in extra_offset.
    double lambda = param_lambda;
    double mu = param_mu;
    return extra_offset +
	   get_query_length() * log((lambda * doclen + mu) / (doclen + mu));
}

double
LM2StageWeight::get_maxextra() const
{
    // Our best bound is at the doclen value which maximises
    //
    //   log((lambda * doclen + mu) / (doclen + mu))
    //
    // which means maximising
    //
    //   (lambda * doclen + mu) / (doclen + mu)
    //
    // Putting d' = doclen + mu we want to maximise
    //
    //   lambda + (1 - lambda) * mu / d'
    //
    // (1 - lambda) > 0 so this is achieved by minimising d' which means
    // minimising doclen.
    double lambda = param_lambda;
    double mu = param_mu;
    auto doclen = get_doclength_lower_bound();
    return extra_offset +
	   get_query_length() * log((lambda * doclen + mu) / (doclen + mu));
}

LM2StageWeight*
LM2StageWeight::clone() const {
    return new LM2StageWeight(param_lambda, param_mu);
}

string
LM2StageWeight::name() const
{
    return "lm2stage";
}

string
LM2StageWeight::serialise() const
{
    string result = serialise_double(param_lambda);
    result += serialise_double(param_mu);
    return result;
}

LM2StageWeight *
LM2StageWeight::unserialise(const string & s) const
{
    const char *ptr = s.data();
    const char *end = ptr + s.size();
    double lambda = unserialise_double(&ptr, end);
    double mu = unserialise_double(&ptr, end);
    if (rare(ptr != end))
	throw Xapian::SerialisationError("Extra data in "
					 "LM2StageWeight::unserialise()");
    return new LM2StageWeight(lambda, mu);
}

LM2StageWeight*
LM2StageWeight::create_from_parameters(const char* params) const
{
    const char* p = params;
    double lambda = 0.7;
    double mu = 2000.0;
    if (*p && !Xapian::Weight::Internal::double_param(&p, &lambda))
	parameter_error("Parameter lambda is invalid", "lm2stage", params);
    if (*p && !Xapian::Weight::Internal::double_param(&p, &mu))
	parameter_error("Parameter mu is invalid", "lm2stage", params);
    if (*p)
	parameter_error("Extra data after parameters", "lm2stage", params);
    return new Xapian::LM2StageWeight(lambda, mu);
}

}
