/** @file imgseek.h
 * @brief Image feature extraction and matching.
 */
/* Copyright 2009 Lemur Consulting Ltd.
 *
 * Design of ImgSig class based on imgseek code:
 * Copyright 2003 Ricardo Niederberger Cabral (nieder|at|mail.ru)
 * Copyright 2006 Geert Janssen <geert at ieee.org>
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

#ifndef XAPIAN_INCLUDED_IMGSEEK_H
#define XAPIAN_INCLUDED_IMGSEEK_H

#include <set>
#include <string>

#include <xapian/database.h>
#include <xapian/error.h>
#include <xapian/postingsource.h>
#include <xapian/visibility.h>

// FIXME - these are currently defined here as well as in haar.h internally
typedef int Idx;
typedef std::set<Idx> coeffs;

/** The signature for an image.
 */
struct XAPIAN_VISIBILITY_DEFAULT ImgSig {
  public:
    /** Y positions with largest magnitude */
    coeffs sig1;

    /** I positions with largest magnitude */
    coeffs sig2;

    /** Q positions with largest magnitude */
    coeffs sig3;

    /** YIQ for position [0,0] */
    double avgl[3];

    /** Construct an image signature from a image stored on disk.
     */
    static ImgSig register_Image(const std::string & filename);

    /** Produce a serialized representation of this signature.
     */
    std::string serialise() const;

    /** Construct a signature from a serialised form.
     *
     *  This accepts a signature as produced by @a serialised().
     *
     *  @param ptr A pointer to a pointer to the start of the serialised data.
     *  This will be updated to point the to the first byte after the data
     *  representing the ImgSig.
     *
     *  @param end Pointer to the end of the serialised data.
     *
     *  There may be unused data at the end of the serialised data - the return
     *  value of *ptr points to this unused data.
     */
    static ImgSig unserialise(const char ** ptr, const char * end);

    /** Construct a signature from serialised data.
     *
     *  This accepts a signature as produced by @a serialised().
     *
     *  @param serialised The serialised data.
     *
     *  This will throw an exception if there is excess data at the end of @a
     *  serialised.
     */
    static ImgSig unserialise(const std::string & serialised);

    /** Comparison operator, so that we can live in a std::set.
     *
     *  Note that the order doesn't really have any inherent meaning.
     */
    bool operator<(const ImgSig & right) const {
	return avgl < right.avgl;
    }

    /** Compute a measure of similarity between this image and another.
     *
     *  The returned scores range from 0 to 100. The higher the score the
     *  closer the two images.
     *
     *  @param other The image signatures to compare with.
     */
    double score(const ImgSig & other) const;
};

/** A collection of image signatures */
class XAPIAN_VISIBILITY_DEFAULT ImgSigs {
    std::set<ImgSig> sigs;
  public:
    std::set<ImgSig>::const_iterator begin() const {
	return sigs.begin();
    }

    std::set<ImgSig>::const_iterator end() const {
	return sigs.end();
    }

    size_t size() const {
	return sigs.size();
    }

    void insert(const ImgSig & sig) {
	sigs.insert(sig);
    }

    void erase(const ImgSig & sig) {
	sigs.erase(sig);
    }

    ImgSigs() : sigs() {}

    ImgSigs(const ImgSig& sig) : sigs() {
	sigs.insert(sig);
    }

    static ImgSigs unserialise(const char * ptr, const char * end);
    static ImgSigs unserialise(const std::string & serialised);
    std::string serialise() const;
};

/** Posting source which weights documents according to signatures.
 *
 *  This posting source returns all documents which have a signature in the
 *  specified slot.
 */

class XAPIAN_VISIBILITY_DEFAULT ImgSigSimilarityPostingSource
	: public Xapian::PostingSource {
    Xapian::Database db;
    Xapian::valueno valno;
    ImgSigs sigs;
    bool started;
    Xapian::ValueIterator it;
    Xapian::ValueIterator end;
    double score;
    Xapian::doccount termfreq_min;
    Xapian::doccount termfreq_est;
    Xapian::doccount termfreq_max;
    void calc_score();
  public:

    ImgSigSimilarityPostingSource(const ImgSigs sigs, unsigned valno);

    // Required methods for Posting source api.

    Xapian::doccount get_termfreq_min() const;
    Xapian::doccount get_termfreq_est() const;
    Xapian::doccount get_termfreq_max() const;

    Xapian::weight get_maxweight() const;

    Xapian::weight get_weight() const;
    Xapian::docid get_docid() const;

    void next(Xapian::weight min_wt);
    bool at_end() const;
    void reset(const Xapian::Database &db);
    std::string get_description() const;
};

#endif /* XAPIAN_INCLUDED_IMGSEEK_H */
