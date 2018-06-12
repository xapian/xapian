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

#include <xapian/attributes.h>
#include <xapian/cluster.h>
#include <xapian/mset.h>
#include <xapian/types.h>
#include <xapian/visibility.h>

#include <map>
#include <unordered_map>
#include <vector>

namespace Xapian {

/** Class for diversifying an MSet using GLS-MPT as given
 *  in the paper: Scalable and Efficient Web Search Result
 *  Diversification Naini et al. 2016
 */
class XAPIAN_VISIBILITY_DEFAULT Diversify {
    /// Top-k documents of given mset are diversified
    unsigned int k;

    /// MPT parameters
    double lambda, b, sigma_sqr;

    /// Store each document from given mset as a point
    std::unordered_map<Xapian::docid, Xapian::Point> points;

    /// Store the relevance score of each document
    std::unordered_map<Xapian::docid, double> scores;

    /// Store pairwise cosine similarities of documents of given mset
    std::map<std::pair<Xapian::docid, Xapian::docid>, double> pairwise_sim;

    /// Store docids of top k diversified documents
    std::vector<Xapian::docid> main_dmset;

    /** Initial diversified document set
     *
     *  Return top-k documents of mset as vector of Points, which
     *  represent the initial diversified document set.
     *
     *  @param source	MSet object containing the documents of which
     *			top-k are to be diversified
     */
    void initialise_points(const Xapian::MSet& source);

    /** Return a key for a pair of documents
     *
     *  Returns a key as a pair of given documents ids
     *
     *  @param docid_a	Document id of the first document
     *  @param docid_b	Document id of the second document
     */
    std::pair<Xapian::docid, Xapian::docid>
    get_key(Xapian::docid docid_a, Xapian::docid docid_b);

    /** Compute pairwise similarities
     *
     *  Used for pre-computing pairwise cosine similarities of documents
     *  of given mset, which is used to speed up evaluate_dmset
     */
    void compute_similarities();

    /** Return difference of 'points' and current dmset
     *
     *  Return the difference of 'points' and the current diversified
     *  document match set
     *
     *  @param dmset	Document set representing a diversified document set
     */
    std::vector<Xapian::docid>
    compute_diff_dmset(const std::vector<Xapian::docid>& dmset);

    /** Evaluate a diversified mset
     *
     *  Evaluate a diversified mset using MPT algorithm
     *
     *  @param dmset	Set of points representing candidate diversifed
     *			set of documents
     */
    double evaluate_dmset(const std::vector<Xapian::docid>& dmset);

  public:
    /** Constructor specifying the number of diversified search results
     *
     *  @param  k_	Number of required diversified documents in the
     *  		diversified document set
     *  @param  lambda_	Trade-off between relevance of top-k diversified
     *  		document set and its similarity to the rest of the
     *  		documents in the document match set. Belongs to the
     *  		the range [0,1] with '0' meaning no weightage to
     *  		relevance of the diversified document set and '1'
     *  		allowing for full weightage to relevance of the
     *  		diversified document set.
     *  @param  b_	Parameter for MPT, normally in the range [1,10]
     *  @param  sigma_sqr_	Parameter for MPT, normally in the range
     *	  			[1e-6,1]
     */
    explicit Diversify(unsigned int k_,
		       double lambda_ = 0.5,
		       double b_ = 5.0,
		       double sigma_sqr_ = 1e-3);

    /** Implements diversification
     *
     *  Performs GLS-MPT and returns documents of which top-k
     *  are diversified.
     *
     *  @param mset	MSet object containing the documents of which
     *			top-k are to be diversified
     */
    Xapian::DocumentSet get_dmset(const MSet& mset);

    /// Return a string describing this object
    std::string get_description() const;
};
}
#endif // XAPIAN_INCLUDED_DIVERSIFY_H
