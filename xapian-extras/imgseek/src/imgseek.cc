/** @file imgseek.cc
 * @brief Image feature extraction and matching.
 */
/* Copyright 2009 Lemur Consulting Ltd.
 *
 * Partially based on imgseek code:
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

#include <config.h>

#include "xapian/error.h"
#include "xapian/imgseek.h"

#include <fstream>
#include <iterator>
#include <stdexcept>
#include <iostream>

#include <qimage.h>

/* JPEG Includes */
#define XMD_H   // avoid primitive typedef name clashes on libjpeg
extern "C"
{
#include <jpeglib.h>
}

#include "jpegloader.h"
#include "haar.h"

#include "serialise-double.h"

template<class T>
inline std::string to_string(const T& t) {
    std::stringstream ss;
    ss << t;
    return ss.str();
}


std::string
Xapian::RangeAccelerator::make_range_term(const std::pair<double, double> r) {
    return prefix+"r"+to_string(r.first)+"-"+to_string(r.second);
}

void
Xapian::RangeAccelerator::add_val(Xapian::Document& doc, double val) const {
    doc.add_value(valnum, serialise_double(val));
    for (int i = 0; i < ranges.size(); ++i) {
	if ((val >= ranges[i].first) & (val <= ranges[i].second)) {
	    doc.add_term(range_terms[i], 0);
	}
    }
}


//namespace Xapian {
//  namespace ImgSeek {
// Weights for the Haar coefficients.
// Straight from the referenced paper:
const float weights[6][3]= {
  //    Y      I      Q       idx total occurs
  { 5.00, 19.21, 34.37},  // 0   58.58      1 (`DC' component)
  { 0.83,  1.26,  0.36},  // 1    2.45      3
  { 1.01,  0.44,  0.45},  // 2    1.90      5
  { 0.52,  0.53,  0.14},  // 3    1.19      7
  { 0.47,  0.28,  0.18},  // 4    0.93      9
  { 0.30,  0.14,  0.27}   // 5    0.71      16384-25=16359
};

/* the original imgseek code made a big array with all these values
   pre-computed so that it was just looking up into an array */
unsigned int
imgbin(const int idx) {
  return std::min ( std::max(idx % num_pixels, idx / num_pixels), 5);
}

double
find_weight(const int idx, const int colour) {
  return weights[imgbin(abs(idx))][colour];
}

ImgSig
ImgSig::register_Image(const std::string& filename)
{
  /* filename is the image location
  */
  int cn;
  // The original imgseek version had these as static
  // but that presumably isn't thread safe
  colourChan cdata1, cdata2, cdata3;
  int i;
  int width, height;

  QImage image   = QImage();
  QString format = QImageIO::imageFormat(filename);

  if (format=="JPEG") { // use fast jpeg loading
    struct jpeg_decompress_struct cinfo = loadJPEG(image, filename.c_str());

    width = cinfo.image_width;
    if (!width) {  // error loading image

      if (!image.load(filename))
        throw Xapian::InvalidArgumentError("Cannot load image file: " + filename);
      width  = image.width();
      height = image.height();
    }
    else {  // fast jpeg succeeded
      height = cinfo.image_height;
    }
  }
  else { // use default QT image loading
    if (!image.load(filename))
      throw Xapian::InvalidArgumentError("Cannot load image file: " + filename);
    width  = image.width();
    height = image.height();
  }

  image = image.scale(num_pixels, num_pixels);

  for (i = 0, cn = 0; i < num_pixels; i++) {
    // Get a scanline:
    QRgb *line = (QRgb *) image.scanLine(i);

    for (int j = 0; j < num_pixels; j++) {
      QRgb pixel = line[j];

      cdata1[cn] = qRed  (pixel);
      cdata2[cn] = qGreen(pixel);
      cdata3[cn] = qBlue (pixel);
      cn++;
    }
  }
  ImgSig isig;

  transform_and_calc(cdata1, cdata2, cdata3,
                     isig.sig1, isig.sig2, isig.sig3,
                     isig.avgl);

  /* this is from the original imgseek code - the imgbuckets structure
     allows for quickly identifying images with shared coefficients in
     the signature.

     We have the capability of doing a similary by labelling documents
     with terms to indicate the signature's coefficients.

  */
  // for (i = 0; i < num_coefs; ++i) { // populate buckets
  //   for (j = i; j < 3; ++j) { // for each colour
  //     int x, t;

  //     // sig[i] never 0

  //     x = nsig->sig1[i];
  //     t = (x < 0);		/* t = 1 if x neg else 1 */
  //     /* x - 0 ^ 0 = x; i - 1 ^ 0b111..1111 = 2-compl(x) = -x */
  //     x = (x - t) ^ -t;
  //     imgbuckets[j][t][x].push_back(id);
  //   }
  // }
  return isig;
}

double
ImgSig::score(const ImgSig & other) const{
  double result = 0;
  const coeffs sig[3] = {sig1,  sig2 , sig3};
  const coeffs osig[3] = {other.sig1, other.sig2, other.sig3};
  for (int c = 0; c < 3; ++c) { // for each colour
    //average luminance score for each colour
    result += weights[0][c] * abs(other.avgl[c] - avgl[c]);

    //score for each coefficient in common
    std::vector<int> intersection;
    std::vector<int>::const_iterator it;
    std::set_intersection(sig[c].begin(), sig[c].end(),
                          osig[c].begin(), osig[c].end(),
                          std::back_inserter(intersection));
    for (it = intersection.begin(); it != intersection.end(); ++it) {
      result -= find_weight(*it, c);
    }
  }
  // I don't really know why this value in particular, but it's what
  // the original imgseek python code does.
  const double magic_factor = -100.0/38.7;
  return std::max(0.0, std::min(100.0, result * magic_factor));
}

void
serialise_coefs(const coeffs &cos, std::string& res) {
  coeffs::const_iterator co;
  for (co = cos.begin(); co != cos.end(); ++co) {
    res += serialise_double(*co);
  }
}

std::string
ImgSig::serialise() const {
  std::string result;
  serialise_coefs(sig1, result);
  serialise_coefs(sig2, result);
  serialise_coefs(sig3, result);
  for (int i=0; i < 3; ++i) {
    result += serialise_double(avgl[i]);
  }
  return result;
}

void
unserialise_coeffs(const char** ptr, const char * end, coeffs &res, int count) {
  for (int i=0; i<count; ++i) {
    res.insert(unserialise_double(ptr, end));
  }
}

ImgSig
ImgSig::unserialise(const char ** ptr, const char * end) {
  ImgSig result;
  try {
    unserialise_coeffs(ptr, end, result.sig1, num_coefs);
    unserialise_coeffs(ptr, end, result.sig2, num_coefs);
    unserialise_coeffs(ptr, end, result.sig3, num_coefs);
    for (int i=0; i<3; ++i) {
      result.avgl[i] = unserialise_double(ptr, end);
    }
  } catch (const Xapian::NetworkError & e) {
    throw Xapian::InvalidArgumentError(e.get_msg());
  }
  return result;
}

ImgSig
ImgSig::unserialise(const std::string & serialised) {
    const char * ptr = serialised.data();
    const char * end = ptr + serialised.size();
    ImgSig result = unserialise(&ptr, end);
    if (ptr != end)
	throw Xapian::InvalidArgumentError("Junk found at end of serialised ImgSig");
    return result;
}

ImgSigs
ImgSigs::unserialise (const char * ptr, const char * end) {
  ImgSigs result;
  try {
    while (ptr != end) {
      result.insert(ImgSig::unserialise(&ptr, end));
    }
  } catch (const Xapian::NetworkError & e) {
    throw Xapian::InvalidArgumentError(e.get_msg());
  }
  if (ptr != end)
    throw Xapian::InvalidArgumentError("Junk at end of serialised ImgSigs");
  return result;
}

ImgSigs
ImgSigs::unserialise(const std::string & serialised) {
  const char * ptr = serialised.data();
  const char * end = ptr + serialised.size();
  ImgSigs results = unserialise(ptr, end);
  return results;
}

std::string
ImgSigs::serialise() const {
  std::string result;
  std::set<ImgSig>::const_iterator sig;
  for (sig = sigs.begin(); sig != sigs.end(); ++sig) {
    result += sig->serialise();
  }
  return result;
}

//ImgSigSimilarityPostingSource::ImgSigSimilarityPostingSource(const Xapian::docid did, Xapian::valueno valno, Xapian::Database db) {};

ImgSigSimilarityPostingSource::ImgSigSimilarityPostingSource(const ImgSigs sigs_, Xapian::valueno valno_) :
  sigs(sigs_), valno(valno_), started(false) {

  if (sigs.size() < 1) {
    throw Xapian::InvalidArgumentError("Cannot construct an ImgSigSimilarityPostingSource from an empty set of signatures.");
  }
  try {
      termfreq_max = db.get_value_freq(valno);
      termfreq_min = termfreq_max;
      termfreq_est = termfreq_max;
  } catch (const Xapian::UnimplementedError &) {
      termfreq_max = db.get_doccount();
      termfreq_est = termfreq_max / 2;
      termfreq_min = 0;
  }
}

Xapian::doccount ImgSigSimilarityPostingSource::get_termfreq_min() const {
  return termfreq_min;
}

Xapian::doccount ImgSigSimilarityPostingSource::get_termfreq_est() const {
  return termfreq_est;
}

Xapian::doccount ImgSigSimilarityPostingSource::get_termfreq_max() const {
  return termfreq_max;
}

Xapian::weight ImgSigSimilarityPostingSource::get_maxweight() const {
  return 100.0;
}

void ImgSigSimilarityPostingSource::next(Xapian::weight min_wt) {
  if (!started) {
    started = true;
    it = db.valuestream_begin(valno);
    end = db.valuestream_end(valno);
  } else {
    ++it;
  }
  if (!at_end()) {
    calc_score();
  }
}

void ImgSigSimilarityPostingSource::calc_score() {
  std::string val(*it);
  ImgSigs current_sigs = ImgSigs::unserialise(val);
  std::set<ImgSig>::const_iterator targ_sig;
  score = 0;
  for (targ_sig = sigs.begin(); targ_sig != sigs.end(); ++targ_sig) {
    ImgSigs cur_sigs = ImgSigs::unserialise(*it);
    std::set<ImgSig>::const_iterator cur_sig;
    for (cur_sig = cur_sigs.begin(); cur_sig != cur_sigs.end(); ++ cur_sig) {
      score = std::max( score, targ_sig->score(*cur_sig));
    }
  }
}

std::string
ImgSigSimilarityPostingSource::get_description() const {
  std::string d = "FIXME";
  return d;
}

Xapian::docid
ImgSigSimilarityPostingSource::get_docid() const {
  return it.get_docid();
}

bool
ImgSigSimilarityPostingSource::at_end() const {
  return it == end;
}

void
ImgSigSimilarityPostingSource::reset(const Xapian::Database & db_) {
  started = false;
  db = db_;
}

Xapian::weight
ImgSigSimilarityPostingSource::get_weight() const {
  return score;
}

std::string
ImgTerms::colourprefix(int c) const {
  return prefix + to_string(c);
}

std::string 
ImgTerms::make_coeff_term(int x, int c) const {
  return colourprefix(c) + to_string(x);
}

WeightMap
ImgTerms::make_weightmap() const {
  WeightMap wm;
  for (int x=-num_pixels_squared; x < num_pixels_squared; ++x) {
    for (int c=0; c< 3; c++) {
      wm[make_coeff_term(x, c)] = find_weight(x, c);
    }
  }
  return wm;
}

ImgTerms::ImgTerms(const std::string& prefix_, Xapian::valueno v1, Xapian::valueno v2, Xapian::valueno v3):
  prefix(prefix_),
  weightmap(make_weightmap()) {

  colour_vals.push_back(v1);
  colour_vals.push_back(v2);
  colour_vals.push_back(v3);

  // Put the values into 255 buckets.

  // Y - ranges from 0.0 to 1.0
  colour_average_accels.push_back(Xapian::RangeAccelerator(prefix+"A0",
							   colour_vals[0],
							   0.0, 1.0, 1.0/255.0));
  // I - ranges from -0.523 to 0.523
  colour_average_accels.push_back(Xapian::RangeAccelerator(prefix+"A1",
							   colour_vals[1],
							   -0.523, 0.523, 
							   (0.523*2.0)/255.0));
  // Q - ranges from -0.596 to 0.596
  colour_average_accels.push_back(Xapian::RangeAccelerator(prefix+"A2",
							   colour_vals[2],
							   -0.596, 0.596, 
							   (0.596*2.0)/255.0));
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
    colour_average_accels[c].add_val(doc, sig.avgl[c]);
  }
}

// this could be faster - it checks the whole string
bool 
startswith(const std::string& s, const std::string& start){
  return s.find(start) == 0;
}

Xapian::Query::Query 
ImgTerms::make_coeff_query(const Xapian::Document& doc) const {
  Xapian::Query::Query query;
  Xapian::TermIterator it;
  for (int c = 0; c < 3; ++c){
    it = doc.termlist_begin();
    std::string cprefix = colourprefix(c);
    it.skip_to(cprefix);
    while (it != doc.termlist_end() & startswith(*it, cprefix)) {
      Xapian::Query::Query subq = Xapian::Query(*it);
      WeightMap::const_iterator pos = weightmap.find(*it);
      subq = Xapian::Query(Xapian::Query::OP_SCALE_WEIGHT,
                           subq,
                           pos->second);
      query = Xapian::Query(Xapian::Query::OP_OR, 
                            query,
                            subq);
      ++it;
    } 
  }
  return query;
}

Xapian::Query 
ImgTerms::querySimilar(const Xapian::Document& doc) const {
  return Xapian::Query(Xapian::Query::OP_OR,
                       make_coeff_query(doc),
                       make_averages_query(doc));
}

Xapian::Query
ImgTerms::make_averages_query(const Xapian::Document& doc) const {
  Xapian::Query query;
  for (int c = 0; c < 3; ++c) {
    std::string doc_val = doc.get_value(colour_vals[c]);
    const char* ptr = doc_val.data();
    const char* end = ptr + doc_val.size();
    double val;
    try {
      val = unserialise_double(&ptr, end);
    } catch (const Xapian::NetworkError & e) {
      throw Xapian::InvalidArgumentError(e.get_msg());
    }
    Xapian::Query subq = Xapian::Query(Xapian::Query::OP_SCALE_WEIGHT, 
                                       colour_average_accels[c].query_for_val_distance(val),
                                       weights[0][c]);
    query = Xapian::Query(Xapian::Query::OP_OR,
                          query,
                          subq);
  }
  return query;
}

//  } // namespace ImgSeek
//} // namespace Xapian
