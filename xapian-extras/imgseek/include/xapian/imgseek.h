/* 

   Copyright (C) 2009 Lemur Consulting Ltd.

   Partially based on imgseek code:
   Copyright (C) 2003 Ricardo Niederberger Cabral (nieder|at|mail.ru)
   Copyright (C) 2006 Geert Janssen <geert at ieee.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.
 
   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.
 
   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
   02110-1301 USA

 */

#ifndef XAPIAN_INCLUDED_IMGSEEK_H
#define XAPIAN_INCLUDED_IMGSEEK_H
#include "xapian/postingsource.h"
#include "xapian/visibility.h"
#include "xapian/database.h"
#include "xapian/error.h"

#include <string>
#include <set>

#include "haar.h"

struct XAPIAN_VISIBILITY_DEFAULT ImgSig {
 public:
  coeffs sig1;		/* Y positions with largest magnitude */
  coeffs sig2;		/* I positions with largest magnitude */
  coeffs sig3;		/* Q positions with largest magnitude */
  double avgl[3];		/* YIQ for position [0,0] */

  // construct an image signature from a image stored on disk
  static ImgSig register_Image(const std::string& filename);
  
  // serialized representation of this sig.
  std::string serialise() const;

  /* construct a signature from char data pointed to by *ptr, ptr will
     be updated to point after the data representing the ImgSig.
     end points at the end of the the string.
  */
  static ImgSig unserialise(const char ** ptr, const char * end);

  // construct a signature from serialised data
  static ImgSig unserialise(std::string& serialised);

  /* so that we can put them in a std::set the order doesn't really
   have any inherent meaning.
  */
  bool operator< (const ImgSig & right) const {
    return avgl < right.avgl;
  }

  /* Compute a hueristic measure for the similarity of this image with
     other. Scores range between 0 and 100. The higher the score the
     closer the two images.
   */

  double score(const ImgSig & other) const;
};

/* A collection of image signatures */

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

  static ImgSigs unserialise (const char * ptr, const char * end);
  static ImgSigs unserialise(const std::string & serialised);
  std::string serialise() const;
};

/* A posting source that weight documents according to similarity to
   the provided signatures. 

 */ 
class XAPIAN_VISIBILITY_DEFAULT ImgSigSimilarityPostingSource : public Xapian::PostingSource {
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

  ImgSigSimilarityPostingSource(const ImgSigs sigs, unsigned valno, const Xapian::Database db);

  // Required methods for Posting source api.

  Xapian::doccount get_termfreq_min() const;
  Xapian::doccount get_termfreq_est() const;
  Xapian::doccount get_termfreq_max() const;
  
  Xapian::weight get_maxweight() const;
  
  Xapian::weight get_weight() const;
  Xapian::docid get_docid() const;
  
  void next(Xapian::weight min_wt);
  bool at_end() const;
  void reset();
  void reset(const Xapian::Database &);
  std::string get_description() const;

  /* Just use the default skip_to and check, since we don't have a
     short cut available.
  */

  //bool check(Xapian::docid min_docid, Xapian::weight min_wt);
  //void skip_to(Xapian::docid min_docid, Xapian::weight min_wt);

};

#endif /* XAPIAN_INCLUDED_IMGSEEK_H */
