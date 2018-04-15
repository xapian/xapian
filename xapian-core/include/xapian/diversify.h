/** @file diversify.h
 *  @brief Diversification API
 */
/* Copyright (C) 2018 Uppinder Chugh
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301
 * USA
 */

#ifndef XAPIAN_INCLUDED_DIVERSIFY_H
#define XAPIAN_INCLUDED_DIVERSIFY_H

#if !defined XAPIAN_IN_XAPIAN_H && !defined XAPIAN_LIB_BUILD
#error "Never use <xapian/diversify.h> directly; include <xapian.h> instead."
#endif

#include "xapian/cluster.h"
#include <xapian/attributes.h>
#include <xapian/mset.h>
#include <xapian/types.h>
#include <xapian/visibility.h>

#include <map>
#include <vector>
#include <unordered_map>

namespace Xapian {

/** Class for diversifying an MSet using GLS-MPT as given
 *  in the paper: Scalable and Efficient Web Search Result
 *  Diversification Naini et al. 2016
 */
class XAPIAN_VISIBILITY_DEFAULT Diversify {
    /// Top-k documents of given mset are diversified
    unsigned int k;

    /// Store given mset as a vector of points
    std::vector<Point> points;

    /// Store relevance score for each document of given mset
    std::unordered_map<Xapian::docid, double> weights;

    /// Store pairwise cosine similarities of documents of given mset
    std::map<std::pair<Xapian::docid, Xapian::docid>, double> pairwise_sim;

    /// MPT parameters
    double lambda, b, sigma_sqr;

    /** Initial diversified document set
     *
     *  Return top-k documents of mset as vector of Points, which
     *  represent the initial diversified document set.
     *
     *  @param source	MSet object containing the documents of which
     *			top-k are to be diversified
     */
    void initialise_points(const Xapian::MSet &source);

    /** Compute pairwise similarities
     *
     *  Use for pre-computing pairwise cosine similarities of documents
     *  of given mset, which are used in evaluate_dmset
     *
     *  @param source	MSet object containing the documents of which
     *			top-k are to be diversified
     */
    void compute_similarities();

    /** Return difference of 'points' and current dmset
     *
     *  Return the difference of 'points' and the current diversified
     *  document match set
     *
     *  @param source	MSet object containing the documents of which
     *			top-k are to be diversified
     */
    std::vector<Point> compute_diff_dmset(const std::vector<Point> &dmset);

    /** Evaluate a diversified mset
     *
     *  Evaluate a diversified mset using MPT algorithm
     *
     *  @param dmset	Set of points representing candidate diversifed
     *			set of documents
     */
    double evaluate_dmset(const std::vector<Point> &dmset);

  public:
    /** Constructor specifying the number of diversified search results
     *
     *  @param  k_	Number of required diversified results
     */
    explicit Diversify(unsigned int k_,
		       double lambda_ = 0.5,
		       double b_ = 5.0,
		       double sigma_sqr_ = 1e-3);

    /** Implements diversifcation
     *
     *  Performs GLS-MPT and returns documents of which top-k
     *  are diversified.
     *
     *  @param mset	MSet object containing the documents of which
     *			top-k are to be diversified
     */
    Xapian::DocumentSet get_dmset(const MSet &mset);

    /// Return a string describing this object
    std::string get_description() const;
};
}
#endif // XAPIAN_INCLUDED_DIVERSIFY_H
