/** @file image_terms.cc
 * @brief Generate terms from an image signature.
 */
/* Copyright 2009 Lemur Consulting Ltd.
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301
 * USA
 */

#include <config.h>

#include "xapian/error.h"
#include "xapian/imgseek.h"
#include "xapian/postingsource.h"

#include "haar.h" // For num_pixels_squared - FIXME - should be supplied to ImgTerms constructor.
#include "serialise-double.h"
#include "serialise.h"

#include <vector>

// Declarations of functions in other file: FIXME - should have a shared internal header.
double find_weight(const int idx, const int colour);
const float weights[6][3]= {
  //    Y      I      Q       idx total occurs
  { 5.00, 19.21, 34.37},  // 0   58.58      1 (`DC' component)
  { 0.83,  1.26,  0.36},  // 1    2.45      3
  { 1.01,  0.44,  0.45},  // 2    1.90      5
  { 0.52,  0.53,  0.14},  // 3    1.19      7
  { 0.47,  0.28,  0.18},  // 4    0.93      9
  { 0.30,  0.14,  0.27}   // 5    0.71      16384-25=16359
};


template<class T>
inline std::string to_string(const T& t) {
    std::stringstream ss;
    ss << t;
    return ss.str();
}

std::string
ImgTerms::colourprefix(int c) const {
    return prefix + "012"[c];
}

std::string 
ImgTerms::make_coeff_term(int x, int c) const {
    if (x < 0)
	return colourprefix(c) + "-" + encode_length(-x);
    else
	return colourprefix(c) + "+" + encode_length(x);
}

WeightMap
ImgTerms::make_weightmap() const {
    WeightMap wm;
    for (int x = -num_pixels_squared; x < num_pixels_squared; ++x) {
	for (int c=0; c< 3; c++) {
	    wm[make_coeff_term(x, c)] = find_weight(x, c);
	}
    }
    return wm;
}

/* Ranges for the Y, I, and Q values in the YIQ colourspace. */
#define Y_MIN 0.0
#define Y_MAX 1.0
#define I_MIN -0.523
#define I_MAX 0.523
#define Q_MIN -0.596
#define Q_MAX 0.596

ImgTerms::ImgTerms(const std::string& prefix_,
		   unsigned int buckets)
	: prefix(prefix_),
	  weightmap(make_weightmap())
{
    // Y - ranges from 0.0 to 1.0
    colour_average_accels.push_back(
	Xapian::RangeAccelerator(prefix + "3",
				 Y_MIN, Y_MAX,
				 (Y_MAX - Y_MIN) / buckets));
    // I - ranges from -0.523 to 0.523
    colour_average_accels.push_back(
	Xapian::RangeAccelerator(prefix + "4",
				 I_MIN, I_MAX,
				 (I_MAX - I_MIN) / buckets));
    // Q - ranges from -0.596 to 0.596
    colour_average_accels.push_back(
	Xapian::RangeAccelerator(prefix + "5",
				 Q_MIN, Q_MAX,
				 (Q_MAX - Q_MIN) / buckets));
}

void 
ImgTerms::add_coeff_terms(const coeffs& s, int c, CoeffTerms& r) const {
    coeffs::const_iterator it;
    for (it = s.begin(); it != s.end(); ++it)
	r.insert(make_coeff_term(*it, c));
}
      
CoeffTerms
ImgTerms::make_coeff_terms(const ImgSig& sig) const {
    CoeffTerms terms;
    add_coeff_terms(sig.sig1, 0, terms);
    add_coeff_terms(sig.sig2, 1, terms);
    add_coeff_terms(sig.sig3, 2, terms);
    return terms;
}

void
ImgTerms::AddTerms(Xapian::Document& doc, const ImgSig& sig) const {
    CoeffTerms terms = make_coeff_terms(sig);
    CoeffTerms::const_iterator it;
    for (it = terms.begin(); it != terms.end(); ++it) {
	doc.add_term(*it);
    }
    for (int c = 0; c < 3; ++c) {
	colour_average_accels[c].add_val_terms(doc, sig.avgl[c]);
    }
}

// this could be faster - it checks the whole string
bool 
startswith(const std::string& s, const std::string& start){
    return s.find(start) == 0;
}

Xapian::Query
ImgTerms::make_coeff_query(const Xapian::Document& doc) const
{
    std::vector<Xapian::Query> subqs;
    Xapian::TermIterator it;
    for (int c = 0; c < 3; ++c) {
	it = doc.termlist_begin();
	std::string cprefix = colourprefix(c);
	it.skip_to(cprefix);
	while (it != doc.termlist_end() && startswith(*it, cprefix)) {
	    WeightMap::const_iterator pos = weightmap.find(*it);
	    Xapian::FixedWeightPostingSource ps(pos->second);
	    subqs.push_back(Xapian::Query(Xapian::Query::OP_FILTER,
					  Xapian::Query(&ps),
					  Xapian::Query(*it)));
	    ++it;
	} 
    }
    return Xapian::Query(Xapian::Query::OP_OR, subqs.begin(), subqs.end());
}

// FIXME - refactor common bits with above method
Xapian::Query
ImgTerms::make_coeff_query(const ImgSig& sig) const
{
    std::vector<Xapian::Query> subqs;
    CoeffTerms terms = make_coeff_terms(sig);
    CoeffTerms::const_iterator it;
    for (it = terms.begin(); it != terms.end(); ++it) {
	WeightMap::const_iterator pos = weightmap.find(*it);
	Xapian::FixedWeightPostingSource ps(pos->second);
	subqs.push_back(Xapian::Query(Xapian::Query::OP_FILTER,
				      Xapian::Query(&ps),
				      Xapian::Query(*it)));
    }
    return Xapian::Query(Xapian::Query::OP_OR, subqs.begin(), subqs.end());
}



/*

FIXME - should be using templated version, but problem with swig wrappers
template<class T>
Xapian::Query 
ImgTerms::querySimilar(const T& img_src) const {
    return Xapian::Query(Xapian::Query::OP_OR,
			 make_coeff_query(img_src),
			 make_averages_query(img_src));
}
*/

Xapian::Query 
ImgTerms::querySimilarDoc(const Xapian::Document& img_src) const {
    return Xapian::Query(Xapian::Query::OP_OR,
			 make_coeff_query(img_src),
			 make_averages_query(img_src));
}

Xapian::Query 
ImgTerms::querySimilarSig(const ImgSig& img_src) const {
    return Xapian::Query(Xapian::Query::OP_OR,
			 make_coeff_query(img_src),
			 make_averages_query(img_src));
}

Xapian::Query
ImgTerms::make_averages_query(const Xapian::Document& doc) const {
    Xapian::Query query;
    for (int c = 0; c < 3; ++c) {
	Xapian::Query subq = Xapian::Query(Xapian::Query::OP_SCALE_WEIGHT, 
					   colour_average_accels[c].query_for_distance(doc),
					   weights[0][c]);
	query = Xapian::Query(Xapian::Query::OP_OR,
			      query,
			      subq);
    }
    return query;
}

// FIXME - refactor to reuse common bits with above method
Xapian::Query
ImgTerms::make_averages_query(const ImgSig& sig) const {
  Xapian::Query query;
      for (int c = 0; c < 3; ++c) {
	Xapian::Query subq = Xapian::Query(Xapian::Query::OP_SCALE_WEIGHT, 
					   colour_average_accels[c].query_for_distance(sig.avgl[c]),
					   weights[0][c]);
	query = Xapian::Query(Xapian::Query::OP_OR,
			      query,
			      subq);
    }
    return query;
}
