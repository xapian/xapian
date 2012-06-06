/** @file weight.h
 * @brief Weighting scheme API.
 */
/* Copyright (C) 2007,2008,2009,2010,2011,2012 Olly Betts
 * Copyright (C) 2009 Lemur Consulting Ltd
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#ifndef XAPIAN_INCLUDED_WEIGHT_H
#define XAPIAN_INCLUDED_WEIGHT_H

#include <string>

#include <xapian/types.h>
#include <xapian/visibility.h>

namespace Xapian {

/** Abstract base class for weighting schemes. */
class XAPIAN_VISIBILITY_DEFAULT Weight {
  protected:
    /// Stats which the weighting scheme can use (see @a need_stat()).
    typedef enum {
	COLLECTION_SIZE = 1,
	RSET_SIZE = 2,
	AVERAGE_LENGTH = 4,
	TERMFREQ = 8,
	RELTERMFREQ = 16,
	QUERY_LENGTH = 32,
	WQF = 64,
	WDF = 128,
	DOC_LENGTH = 256,
	DOC_LENGTH_MIN = 512,
	DOC_LENGTH_MAX = 1024,
	WDF_MAX = 2048
    } stat_flags;

    /** Tell Xapian that your subclass will want a particular statistic.
     *
     *  Some of the statistics can be costly to fetch or calculate, so
     *  Xapian needs to know which are actually going to be used.  You
     *  should call need_stat() from your constructor for each such
     *  statistic.
     *
     * @param flag  The stat_flags value for a required statistic.
     */
    void need_stat(stat_flags flag) {
	stats_needed = stat_flags(stats_needed | flag);
    }

    /** Allow the subclass to perform any initialisation it needs to.
     *
     *  @param factor	  Any scaling factor (e.g. from OP_SCALE_WEIGHT).
     *			  If the Weight object is for the term-independent
     *			  weight supplied by get_sumextra()/get_maxextra(),
     *			  then init(0.0) is called (starting from Xapian
     *			  1.2.11 and 1.3.1 - earlier versions failed to
     *			  call init() for such Weight objects).
     */
    virtual void init(double factor) = 0;

  private:
    /// Don't allow assignment.
    void operator=(const Weight &);

    /// A bitmask of the statistics this weighting scheme needs.
    stat_flags stats_needed;

    /// The number of documents in the collection.
    Xapian::doccount collection_size_;

    /// The number of documents marked as relevant.
    Xapian::doccount rset_size_;

    /// The average length of a document in the collection.
    Xapian::doclength average_length_;

    /// The number of documents which this term indexes.
    Xapian::doccount termfreq_;

    /// The number of relevant documents which this term indexes.
    Xapian::doccount reltermfreq_;

    /// The length of the query.
    Xapian::termcount query_length_;

    /// The within-query-frequency of this term.
    Xapian::termcount wqf_;

    /// A lower bound on the minimum length of any document in the database.
    Xapian::termcount doclength_lower_bound_;

    /// An upper bound on the maximum length of any document in the database.
    Xapian::termcount doclength_upper_bound_;

    /// An upper bound on the wdf of this term.
    Xapian::termcount wdf_upper_bound_;

  public:
    class Internal;

    /** Virtual destructor, because we have virtual methods. */
    virtual ~Weight();

    /** Clone this object.
     *
     *  This method allocates and returns a copy of the object it is called on.
     *
     *  If your subclass is called FooWeight and has parameters a and b, then
     *  you would implement FooWeight::clone() like so:
     *
     *  FooWeight * FooWeight::clone() const { return new FooWeight(a, b); }
     *
     *  Note that the returned object will be deallocated by Xapian after use
     *  with "delete".  If you want to handle the deletion in a special way
     *  (for example when wrapping the Xapian API for use from another
     *  language) then you can define a static <code>operator delete</code>
     *  method in your subclass as shown here:
     *  http://trac.xapian.org/ticket/554#comment:1
     */
    virtual Weight * clone() const = 0;

    /** Return the name of this weighting scheme.
     *
     *  This name is used by the remote backend.  It is passed along with the
     *  serialised parameters to the remote server so that it knows which class
     *  to create.
     *
     *  Return the full namespace-qualified name of your class here - if
     *  your class is called FooWeight, return "FooWeight" from this method
     *  (Xapian::BM25Weight returns "Xapian::BM25Weight" here).
     *
     *  If you don't want to support the remote backend, you can use the
     *  default implementation which simply returns an empty string.
     */
    virtual std::string name() const;

    /** Return this object's parameters serialised as a single string.
     *
     *  If you don't want to support the remote backend, you can use the
     *  default implementation which simply throws Xapian::UnimplementedError.
     */
    virtual std::string serialise() const;

    /** Unserialise parameters.
     *
     *  This method unserialises parameters serialised by the @a serialise()
     *  method and allocates and returns a new object initialised with them.
     *
     *  If you don't want to support the remote backend, you can use the
     *  default implementation which simply throws Xapian::UnimplementedError.
     *
     *  Note that the returned object will be deallocated by Xapian after use
     *  with "delete".  If you want to handle the deletion in a special way
     *  (for example when wrapping the Xapian API for use from another
     *  language) then you can define a static <code>operator delete</code>
     *  method in your subclass as shown here:
     *  http://trac.xapian.org/ticket/554#comment:1
     *
     *  @param s	A string containing the serialised parameters.
     */
    virtual Weight * unserialise(const std::string & s) const;

    /** Calculate the weight contribution for this object's term to a document.
     *
     *  The parameters give information about the document which may be used
     *  in the calculations:
     *
     *  @param wdf    The within document frequency of the term in the document.
     *  @param doclen The document's length (unnormalised).
     */
    virtual Xapian::weight get_sumpart(Xapian::termcount wdf,
				       Xapian::termcount doclen) const = 0;

    /** Return an upper bound on what get_sumpart() can return for any document.
     *
     *  This information is used by the matcher to perform various
     *  optimisations, so strive to make the bound as tight as possible.
     */
    virtual Xapian::weight get_maxpart() const = 0;

    /** Calculate the term-independent weight component for a document.
     *
     *  The parameter gives information about the document which may be used
     *  in the calculations:
     *
     *  @param doclen The document's length (unnormalised).
     */
    virtual Xapian::weight get_sumextra(Xapian::termcount doclen) const = 0;

    /** Return an upper bound on what get_sumextra() can return for any
     *  document.
     *
     *  This information is used by the matcher to perform various
     *  optimisations, so strive to make the bound as tight as possible.
     */
    virtual Xapian::weight get_maxextra() const = 0;

    /** @private @internal Initialise this object to calculate weights for term
     *  @a term.
     *
     *  @param stats	  Source of statistics.
     *  @param query_len_ Query length.
     *  @param term	  The term for the new object.
     *  @param wqf_	  The within-query-frequency of @a term.
     *  @param factor	  Any scaling factor (e.g. from OP_SCALE_WEIGHT).
     */
    void init_(const Internal & stats, Xapian::termcount query_len_,
	       const std::string & term, Xapian::termcount wqf_,
	       double factor);

    /** @private @internal Initialise this object to calculate weights for a
     *  synonym.
     *
     *  @param stats	   Source of statistics.
     *  @param query_len_  Query length.
     *  @param factor	   Any scaling factor (e.g. from OP_SCALE_WEIGHT).
     *  @param termfreq    The termfreq to use.
     *  @param reltermfreq The reltermfreq to use.
     */
    void init_(const Internal & stats, Xapian::termcount query_len_,
	       double factor, Xapian::doccount termfreq,
	       Xapian::doccount reltermfreq);

    /** @private @internal Initialise this object to calculate the extra weight
     *  component.
     *
     *  @param stats	  Source of statistics.
     *  @param query_len_ Query length.
     */
    void init_(const Internal & stats, Xapian::termcount query_len_);

    /** @private @internal Return true if the document length is needed.
     *
     *  If this method returns true, then the document length will be fetched
     *  and passed to @a get_sumpart().  Otherwise 0 may be passed for the
     *  document length.
     */
    bool get_sumpart_needs_doclength_() const {
	return stats_needed & DOC_LENGTH;
    }

    /** @private @internal Return true if the WDF is needed.
     *
     *  If this method returns true, then the WDF will be fetched and passed to
     *  @a get_sumpart().  Otherwise 0 may be passed for the wdf.
     */
    bool get_sumpart_needs_wdf_() const {
	return stats_needed & WDF;
    }

  protected:
    /** Don't allow copying.
     *
     *  This would ideally be private, but that causes a compilation error
     *  with GCC 4.1 (which appears to be a bug).
     */
    Weight(const Weight &);

    /// Default constructor, needed by subclass constructors.
    Weight() : stats_needed() { }

    /// The number of documents in the collection.
    Xapian::doccount get_collection_size() const { return collection_size_; }

    /// The number of documents marked as relevant.
    Xapian::doccount get_rset_size() const { return rset_size_; }

    /// The average length of a document in the collection.
    Xapian::doclength get_average_length() const { return average_length_; }

    /// The number of documents which this term indexes.
    Xapian::doccount get_termfreq() const { return termfreq_; }

    /// The number of relevant documents which this term indexes.
    Xapian::doccount get_reltermfreq() const { return reltermfreq_; }

    /// The length of the query.
    Xapian::termcount get_query_length() const { return query_length_; }

    /// The within-query-frequency of this term.
    Xapian::termcount get_wqf() const { return wqf_; }

    /** An upper bound on the maximum length of any document in the database.
     *
     *  This should only be used by get_maxpart() and get_maxextra().
     */
    Xapian::termcount get_doclength_upper_bound() const {
	return doclength_upper_bound_;
    }

    /** A lower bound on the minimum length of any document in the database.
     *
     *  This bound does not include any zero-length documents.
     *
     *  This should only be used by get_maxpart() and get_maxextra().
     */
    Xapian::termcount get_doclength_lower_bound() const {
	return doclength_lower_bound_;
    }

    /** An upper bound on the wdf of this term.
     *
     *  This should only be used by get_maxpart() and get_maxextra().
     */
    Xapian::termcount get_wdf_upper_bound() const {
	return wdf_upper_bound_;
    }
};

/** Class implementing a "boolean" weighting scheme.
 *
 *  This weighting scheme gives all documents zero weight.
 */
class XAPIAN_VISIBILITY_DEFAULT BoolWeight : public Weight {
    BoolWeight * clone() const;

    void init(double factor);

  public:
    /** Construct a BoolWeight. */
    BoolWeight() { }

    std::string name() const;

    std::string serialise() const;
    BoolWeight * unserialise(const std::string & s) const;

    Xapian::weight get_sumpart(Xapian::termcount wdf,
			       Xapian::termcount doclen) const;
    Xapian::weight get_maxpart() const;

    Xapian::weight get_sumextra(Xapian::termcount doclen) const;
    Xapian::weight get_maxextra() const;
};

/// Xapian::Weight subclass implementing the BM25 probabilistic formula.
class XAPIAN_VISIBILITY_DEFAULT BM25Weight : public Weight {
    /// Factor to multiply the document length by.
    mutable Xapian::doclength len_factor;

    /// Factor combining all the document independent factors.
    mutable Xapian::weight termweight;

    /// The BM25 parameters.
    double param_k1, param_k2, param_k3, param_b;

    /// The minimum normalised document length value.
    Xapian::doclength param_min_normlen;

    BM25Weight * clone() const;

    void init(double factor);

  public:
    /** Construct a BM25Weight.
     *
     *  @param k1  A non-negative parameter controlling how influential
     *		   within-document-frequency (wdf) is.  k1=0 means that
     *		   wdf doesn't affect the weights.  The larger k1 is, the more
     *		   wdf influences the weights.  (default 1)
     *
     *  @param k2  A non-negative parameter which controls the strength of a
     *		   correction factor which depends upon query length and
     *		   normalised document length.  k2=0 disable this factor; larger
     *		   k2 makes it stronger.  (default 0)
     *
     *  @param k3  A non-negative parameter controlling how influential
     *		   within-query-frequency (wqf) is.  k3=0 means that wqf
     *		   doesn't affect the weights.  The larger k3 is, the more
     *		   wqf influences the weights.  (default 1)
     *
     *  @param b   A parameter between 0 and 1, controlling how strong the
     *		   document length normalisation of wdf is.  0 means no
     *		   normalisation; 1 means full normalisation.  (default 0.5)
     *
     *  @param min_normlen  A parameter specifying a minimum value for
     *		   normalised document length.  Normalised document length
     *		   values less than this will be clamped to this value, helping
     *		   to prevent very short documents getting large weights.
     *		   (default 0.5)
     */
    BM25Weight(double k1, double k2, double k3, double b, double min_normlen)
	: param_k1(k1), param_k2(k2), param_k3(k3), param_b(b),
	  param_min_normlen(min_normlen)
    {
	if (param_k1 < 0) param_k1 = 0;
	if (param_k2 < 0) param_k2 = 0;
	if (param_k3 < 0) param_k3 = 0;
	if (param_b < 0) {
	    param_b = 0;
	} else if (param_b > 1) {
	    param_b = 1;
	}
	need_stat(COLLECTION_SIZE);
	need_stat(RSET_SIZE);
	need_stat(TERMFREQ);
	need_stat(RELTERMFREQ);
	need_stat(WDF);
	need_stat(WDF_MAX);
	if (param_k2 != 0 || (param_k1 != 0 && param_b != 0)) {
	    need_stat(DOC_LENGTH_MIN);
	    need_stat(AVERAGE_LENGTH);
	}
	if (param_k1 != 0 && param_b != 0) need_stat(DOC_LENGTH);
	if (param_k2 != 0) need_stat(QUERY_LENGTH);
	if (param_k3 != 0) need_stat(WQF);
    }

    BM25Weight()
	: param_k1(1), param_k2(0), param_k3(1), param_b(0.5),
	  param_min_normlen(0.5)
    {
	need_stat(COLLECTION_SIZE);
	need_stat(RSET_SIZE);
	need_stat(TERMFREQ);
	need_stat(RELTERMFREQ);
	need_stat(WDF);
	need_stat(WDF_MAX);
	need_stat(DOC_LENGTH_MIN);
	need_stat(AVERAGE_LENGTH);
	need_stat(DOC_LENGTH);
	need_stat(WQF);
    }

    std::string name() const;

    std::string serialise() const;
    BM25Weight * unserialise(const std::string & s) const;

    Xapian::weight get_sumpart(Xapian::termcount wdf,
			       Xapian::termcount doclen) const;
    Xapian::weight get_maxpart() const;

    Xapian::weight get_sumextra(Xapian::termcount doclen) const;
    Xapian::weight get_maxextra() const;
};

/** Xapian::Weight subclass implementing the traditional probabilistic formula.
 *
 * This class implements the "traditional" Probabilistic Weighting scheme, as
 * described by the early papers on Probabilistic Retrieval.  BM25 generally
 * gives better results.
 *
 * TradWeight(k) is equivalent to BM25Weight(k, 0, 0, 1, 0), except that
 * the latter returns weights (k+1) times larger.
 */
class XAPIAN_VISIBILITY_DEFAULT TradWeight : public Weight {
    /// Factor to multiply the document length by.
    mutable Xapian::doclength len_factor;

    /// Factor combining all the document independent factors.
    mutable Xapian::weight termweight;

    /// The parameter in the formula.
    double param_k;

    TradWeight * clone() const;

    void init(double factor);

  public:
    /** Construct a TradWeight.
     *
     *  @param k  A non-negative parameter controlling how influential
     *		  within-document-frequency (wdf) and document length are.
     *		  k=0 means that wdf and document length don't affect the
     *		  weights.  The larger k is, the more they do.  (default 1)
     */
    explicit TradWeight(double k = 1.0) : param_k(k) {
	if (param_k < 0) param_k = 0;
	if (param_k != 0.0) {
	    need_stat(AVERAGE_LENGTH);
	    need_stat(DOC_LENGTH);
	}
	need_stat(COLLECTION_SIZE);
	need_stat(RSET_SIZE);
	need_stat(TERMFREQ);
	need_stat(RELTERMFREQ);
	need_stat(DOC_LENGTH_MIN);
	need_stat(WDF);
	need_stat(WDF_MAX);
    }

    std::string name() const;

    std::string serialise() const;
    TradWeight * unserialise(const std::string & s) const;

    Xapian::weight get_sumpart(Xapian::termcount wdf,
			       Xapian::termcount doclen) const;
    Xapian::weight get_maxpart() const;

    Xapian::weight get_sumextra(Xapian::termcount doclen) const;
    Xapian::weight get_maxextra() const;
};

}

#endif // XAPIAN_INCLUDED_WEIGHT_H
