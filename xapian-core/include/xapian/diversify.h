/** @file
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
#error Never use <xapian/diversify.h> directly; include <xapian.h> instead.
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
  public:
    class Internal;
    /// @private @internal Reference counted internals.
    Xapian::Internal::intrusive_ptr_nonnull<Internal> internal;

    /** Copying is allowed.  The internals are reference counted, so
     *  copying is cheap.
     *
     *  @param other	The object to copy.
     */
    Diversify(const Diversify& other);

    /** Assignment is allowed.  The internals are reference counted,
     *  so assignment is cheap.
     *
     *  @param other	The object to copy.
     */
    Diversify& operator=(const Diversify& other);

    /** Move constructor.
     *
     *  @param other	The object to move.
     */
    Diversify(Diversify&& other);

    /** Move assignment operator.
     *
     *  @param other	The object to move.
     */
    Diversify& operator=(Diversify&& other);

    /** Constructor specifying the number of diversified search results
     *
     *  @param  k_	Number of required diversified documents in the
     *  		diversified document set
     *  @param  r_	Number of documents from each cluster used for
     *  		building topC
     *  @param  lambda_	Trade-off between relevance of top-k diversified
     *  		document set and its similarity to the rest of the
     *  		documents in the document match set. Belongs to the
     *  		range [0,1] with 0 meaning no weighting to
     *  		relevance of the diversified document set and 1
     *  		allowing for full weighting to relevance of the
     *  		diversified document set.
     *  @param  b_	Parameter for MPT, normally in the range [1,10]
     *  @param  sigma_sqr_	Parameter for MPT, normally in the range
     *	  			[1e-6,1]
     */
    explicit Diversify(Xapian::doccount k_,
		       Xapian::doccount r_,
		       double lambda_ = 0.5,
		       double b_ = 5.0,
		       double sigma_sqr_ = 1e-3);

    /// Destructor
    ~Diversify();

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
