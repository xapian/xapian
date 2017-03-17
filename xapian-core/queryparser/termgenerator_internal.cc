/** @file termgenerator_internal.cc
 * @brief TermGenerator class internals
 */
/* Copyright (C) 2007,2010,2011,2012,2015,2016,2017 Olly Betts
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
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

#include <config.h>

#include "termgenerator_internal.h"

#include "api/omenquireinternal.h"
#include "api/queryinternal.h"

#include <xapian/document.h>
#include <xapian/queryparser.h>
#include <xapian/stem.h>
#include <xapian/unicode.h>

#include "stringutils.h"

#include <algorithm>
#include <cmath>
#include <deque>
#include <limits>
#include <list>
#include <string>
#include <unordered_map>
#include <vector>

#include "cjk-tokenizer.h"

using namespace std;

namespace Xapian {

inline bool
U_isupper(unsigned ch) {
    return (ch < 128 && C_isupper(static_cast<unsigned char>(ch)));
}

inline unsigned check_wordchar(unsigned ch) {
    if (Unicode::is_wordchar(ch)) return Unicode::tolower(ch);
    return 0;
}

inline bool
should_stem(const std::string & term)
{
    const unsigned int SHOULD_STEM_MASK =
	(1 << Unicode::LOWERCASE_LETTER) |
	(1 << Unicode::TITLECASE_LETTER) |
	(1 << Unicode::MODIFIER_LETTER) |
	(1 << Unicode::OTHER_LETTER);
    Utf8Iterator u(term);
    return ((SHOULD_STEM_MASK >> Unicode::get_category(*u)) & 1);
}

/** Value representing "ignore this" when returned by check_infix() or
 *  check_infix_digit().
 */
const unsigned UNICODE_IGNORE = numeric_limits<unsigned>::max();

inline unsigned check_infix(unsigned ch) {
    if (ch == '\'' || ch == '&' || ch == 0xb7 || ch == 0x5f4 || ch == 0x2027) {
	// Unicode includes all these except '&' in its word boundary rules,
	// as well as 0x2019 (which we handle below) and ':' (for Swedish
	// apparently, but we ignore this for now as it's problematic in
	// real world cases).
	return ch;
    }
    // 0x2019 is Unicode apostrophe and single closing quote.
    // 0x201b is Unicode single opening quote with the tail rising.
    if (ch == 0x2019 || ch == 0x201b) return '\'';
    if (ch >= 0x200b && (ch <= 0x200d || ch == 0x2060 || ch == 0xfeff))
	return UNICODE_IGNORE;
    return 0;
}

inline unsigned check_infix_digit(unsigned ch) {
    // This list of characters comes from Unicode's word identifying algorithm.
    switch (ch) {
	case ',':
	case '.':
	case ';':
	case 0x037e: // GREEK QUESTION MARK
	case 0x0589: // ARMENIAN FULL STOP
	case 0x060D: // ARABIC DATE SEPARATOR
	case 0x07F8: // NKO COMMA
	case 0x2044: // FRACTION SLASH
	case 0xFE10: // PRESENTATION FORM FOR VERTICAL COMMA
	case 0xFE13: // PRESENTATION FORM FOR VERTICAL COLON
	case 0xFE14: // PRESENTATION FORM FOR VERTICAL SEMICOLON
	    return ch;
    }
    if (ch >= 0x200b && (ch <= 0x200d || ch == 0x2060 || ch == 0xfeff))
	return UNICODE_IGNORE;
    return 0;
}

inline bool
is_digit(unsigned ch) {
    return (Unicode::get_category(ch) == Unicode::DECIMAL_DIGIT_NUMBER);
}

inline unsigned check_suffix(unsigned ch) {
    if (ch == '+' || ch == '#') return ch;
    // FIXME: what about '-'?
    return 0;
}

/** Templated framework for processing terms.
 *
 *  Calls action(term, positional) for each term to add, where term is a
 *  std::string holding the term, and positional is a bool indicating
 *  if this term carries positional information.
 */
template<typename ACTION> void
parse_terms(Utf8Iterator itor, bool cjk_ngram, bool with_positions, ACTION action)
{
    while (true) {
	// Advance to the start of the next term.
	unsigned ch;
	while (true) {
	    if (itor == Utf8Iterator()) return;
	    ch = check_wordchar(*itor);
	    if (ch) break;
	    ++itor;
	}

	string term;
	// Look for initials separated by '.' (e.g. P.T.O., U.N.C.L.E).
	// Don't worry if there's a trailing '.' or not.
	if (U_isupper(*itor)) {
	    const Utf8Iterator end;
	    Utf8Iterator p = itor;
	    do {
		Unicode::append_utf8(term, Unicode::tolower(*p++));
	    } while (p != end && *p == '.' && ++p != end && U_isupper(*p));
	    // One letter does not make an acronym!  If we handled a single
	    // uppercase letter here, we wouldn't catch M&S below.
	    if (term.size() > 1) {
		// Check there's not a (lower case) letter or digit
		// immediately after it.
		if (p == end || !Unicode::is_wordchar(*p)) {
		    itor = p;
		    goto endofterm;
		}
	    }
	    term.resize(0);
	}

	while (true) {
	    if (cjk_ngram &&
		CJK::codepoint_is_cjk(*itor) &&
		Unicode::is_wordchar(*itor)) {
		const string & cjk = CJK::get_cjk(itor);
		for (CJKTokenIterator tk(cjk); tk != CJKTokenIterator(); ++tk) {
		    const string & cjk_token = *tk;
		    if (!action(cjk_token, with_positions && tk.get_length() == 1, itor))
			return;
		}
		while (true) {
		    if (itor == Utf8Iterator()) return;
		    ch = check_wordchar(*itor);
		    if (ch) break;
		    ++itor;
		}
		continue;
	    }
	    unsigned prevch;
	    do {
		Unicode::append_utf8(term, ch);
		prevch = ch;
		if (++itor == Utf8Iterator() ||
		    (cjk_ngram && CJK::codepoint_is_cjk(*itor)))
		    goto endofterm;
		ch = check_wordchar(*itor);
	    } while (ch);

	    Utf8Iterator next(itor);
	    ++next;
	    if (next == Utf8Iterator()) break;
	    unsigned nextch = check_wordchar(*next);
	    if (!nextch) break;
	    unsigned infix_ch = *itor;
	    if (is_digit(prevch) && is_digit(*next)) {
		infix_ch = check_infix_digit(infix_ch);
	    } else {
		// Handle things like '&' in AT&T, apostrophes, etc.
		infix_ch = check_infix(infix_ch);
	    }
	    if (!infix_ch) break;
	    if (infix_ch != UNICODE_IGNORE)
		Unicode::append_utf8(term, infix_ch);
	    ch = nextch;
	    itor = next;
	}

	{
	    size_t len = term.size();
	    unsigned count = 0;
	    while ((ch = check_suffix(*itor))) {
		if (++count > 3) {
		    term.resize(len);
		    break;
		}
		Unicode::append_utf8(term, ch);
		if (++itor == Utf8Iterator()) goto endofterm;
	    }
	    // Don't index fish+chips as fish+ chips.
	    if (Unicode::is_wordchar(*itor))
		term.resize(len);
	}

endofterm:
	if (!action(term, with_positions, itor))
	    return;
    }
}

void
TermGenerator::Internal::index_text(Utf8Iterator itor, termcount wdf_inc,
				    const string & prefix, bool with_positions)
{
    bool cjk_ngram = (flags & FLAG_CJK_NGRAM) || CJK::is_cjk_enabled();

    stop_strategy current_stop_mode;
    if (!stopper.get()) {
	current_stop_mode = TermGenerator::STOP_NONE;
    } else {
	current_stop_mode = stop_mode;
    }

    parse_terms(itor, cjk_ngram, with_positions,
	[=](const string & term, bool positional, const Utf8Iterator &) {
	    if (term.size() > max_word_length) return true;

	    if (current_stop_mode == TermGenerator::STOP_ALL && (*stopper)(term))
		return true;

	    if (strategy == TermGenerator::STEM_SOME ||
		strategy == TermGenerator::STEM_NONE) {
		if (positional) {
		    doc.add_posting(prefix + term, ++termpos, wdf_inc);
		} else {
		    doc.add_term(prefix + term, wdf_inc);
		}
	    }

	    if ((flags & FLAG_SPELLING) && prefix.empty())
		db.add_spelling(term);

	    if (strategy == TermGenerator::STEM_NONE ||
		!stemmer.internal.get()) return true;

	    if (strategy == TermGenerator::STEM_SOME) {
		if (current_stop_mode == TermGenerator::STOP_STEMMED &&
		    (*stopper)(term))
		    return true;

		// Note, this uses the lowercased term, but that's OK as we
		// only want to avoid stemming terms starting with a digit.
		if (!should_stem(term)) return true;
	    }

	    // Add stemmed form without positional information.
	    const string& stem = stemmer(term);
	    if (rare(stem.empty())) return true;
	    string stemmed_term;
	    if (strategy != TermGenerator::STEM_ALL) {
		stemmed_term += "Z";
	    }
	    stemmed_term += prefix;
	    stemmed_term += stem;
	    if (strategy != TermGenerator::STEM_SOME && with_positions) {
		doc.add_posting(stemmed_term, ++termpos, wdf_inc);
	    } else {
		doc.add_term(stemmed_term, wdf_inc);
	    }
	    return true;
	});
}

struct Sniplet {
    double* relevance;

    size_t term_end;

    size_t highlight;

    Sniplet(double* r, size_t t, size_t h)
	: relevance(r), term_end(t), highlight(h) { }
};

class SnipPipe {
    deque<Sniplet> pipe;
    deque<Sniplet> best_pipe;

    // Requested length for snippet.
    size_t length;

    // Position in text of start of current pipe contents.
    size_t begin = 0;

    // Rolling sum of the current pipe contents.
    double sum = 0;

    size_t phrase_len = 0;

  public:
    size_t best_begin = 0;

    size_t best_end = 0;

    double best_sum = 0;

    // Add one to length to allow for inter-word space.
    // FIXME: We ought to correctly allow for multiple spaces.
    explicit SnipPipe(size_t length_) : length(length_ + 1) { }

    bool pump(double* r, size_t t, size_t h, unsigned flags);

    void done();

    bool drain(const string & input,
	       const string & hi_start,
	       const string & hi_end,
	       const string & omit,
	       string & output);
};

#define DECAY 2.0

inline bool
SnipPipe::pump(double* r, size_t t, size_t h, unsigned flags)
{
    if (h > 1) {
	if (pipe.size() >= h - 1) {
	    // The final term of a phrase is entering the window.  Peg the
	    // phrase's relevance onto the first term of the phrase, so it'll
	    // be removed from `sum` when the phrase starts to leave the
	    // window.
	    auto & phrase_start = pipe[pipe.size() - (h - 1)];
	    if (phrase_start.relevance) {
		*phrase_start.relevance *= DECAY;
		sum -= *phrase_start.relevance;
	    }
	    sum += *r;
	    phrase_start.relevance = r;
	    phrase_start.highlight = h;
	    *r /= DECAY;
	}
	r = NULL;
	h = 0;
    }
    pipe.emplace_back(r, t, h);
    if (r) {
	sum += *r;
	*r /= DECAY;
    }

    // If necessary, discard words from the start of the pipe until it has the
    // desired length.
    // FIXME: Also shrink the window past words with relevance < 0?
    while (t - begin > length /* || pipe.front().relevance < 0.0 */) {
	const Sniplet& word = pipe.front();
	if (word.relevance) {
	    *word.relevance *= DECAY;
	    sum -= *word.relevance;
	}
	begin = word.term_end;
	if (best_end >= begin)
	    best_pipe.push_back(word);
	pipe.pop_front();
	// E.g. can happen if the current term is longer than the requested
	// length!
	if (rare(pipe.empty())) break;
    }

    // Using > here doesn't work well, as we don't extend a snippet over terms
    // with 0 weight.
    if (sum >= best_sum) {
	// Discard any part of `best_pipe` which is before `begin`.
	if (begin >= best_end) {
	    best_pipe.clear();
	} else {
	    while (!best_pipe.empty() &&
		   best_pipe.front().term_end <= begin) {
		best_pipe.pop_front();
	    }
	}
	best_sum = sum;
	best_begin = begin;
	best_end = t;
    } else if ((flags & Xapian::MSet::SNIPPET_EXHAUSTIVE) == 0) {
	if (best_sum > 0 && best_end < begin) {
	    // We found something, and we aren't still looking near it.
	    // FIXME: Benchmark this and adjust if necessary.
	    return false;
	}
    }
    return true;
}

inline void
SnipPipe::done()
{
    // Discard any part of `pipe` which is after `best_end`.
    if (begin >= best_end) {
	pipe.clear();
    } else {
	// We should never empty the pipe (as that case should be handled
	// above).
	while (rare(!pipe.empty()) &&
	       pipe.back().term_end > best_end) {
	    pipe.pop_back();
	}
    }
}

inline bool
SnipPipe::drain(const string & input,
		const string & hi_start,
		const string & hi_end,
		const string & omit,
		string & output)
{
    if (best_pipe.empty() && !pipe.empty()) {
	swap(best_pipe, pipe);
    }

    if (best_pipe.empty()) {
	size_t tail_len = input.size() - best_end;
	if (tail_len == 0) return false;

	// See if this is the end of a sentence.
	// FIXME: This is quite simplistic - look at the Unicode rules:
	// http://unicode.org/reports/tr29/#Sentence_Boundaries
	bool punc = false;
	Utf8Iterator i(input.data() + best_end, tail_len);
	while (i != Utf8Iterator()) {
	    unsigned ch = *i;
	    if (punc && Unicode::is_whitespace(ch)) break;

	    // Allow "...", "!!", "!?!", etc...
	    punc = (ch == '.' || ch == '?' || ch == '!');

	    if (Unicode::is_wordchar(ch)) break;
	    ++i;
	}

	if (punc) {
	    // Include end of sentence punctuation.
	    output.append(input.data() + best_end, i.raw());
	} else {
	    // Append "..." or equivalent if this doesn't seem to be the start
	    // of a sentence.
	    output += omit;
	}

	return false;
    }

    const Sniplet & word = best_pipe.front();

    if (output.empty()) {
	// Start of the snippet.
	enum { NO, PUNC, YES } sentence_boundary = (best_begin == 0) ? YES : NO;

	Utf8Iterator i(input.data() + best_begin, word.term_end - best_begin);
	while (i != Utf8Iterator()) {
	    unsigned ch = *i;
	    switch (sentence_boundary) {
		case NO:
		    if (ch == '.' || ch == '?' || ch == '!') {
			sentence_boundary = PUNC;
		    }
		    break;
		case PUNC:
		    if (Unicode::is_whitespace(ch)) {
			sentence_boundary = YES;
		    } else if (ch == '.' || ch == '?' || ch == '!') {
			// Allow "...", "!!", "!?!", etc...
		    } else {
			sentence_boundary = NO;
		    }
		    break;
		case YES:
		    break;
	    }
	    if (Unicode::is_wordchar(ch)) {
		// Start the snippet at the start of the first word.
		best_begin = i.raw() - input.data();
		break;
	    }
	    ++i;
	}

	// Add "..." or equivalent if this doesn't seem to be the start of a
	// sentence.
	if (sentence_boundary != YES) {
	    output += omit;
	}
    }

    if (word.highlight) {
	// Don't include inter-word characters in the highlight.
	Utf8Iterator i(input.data() + best_begin, input.size() - best_begin);
	while (i != Utf8Iterator()) {
	    unsigned ch = *i;
	    if (Unicode::is_wordchar(ch)) {
		const char * p = input.data() + best_begin;
		output.append(p, i.raw() - p);
		best_begin = i.raw() - input.data();
		break;
	    }
	    ++i;
	}
    }

    if (!phrase_len) {
	phrase_len = word.highlight;
	if (phrase_len) output += hi_start;
    }

    while (best_begin != word.term_end) {
	char ch = input[best_begin++];
	switch (ch) {
	    case '&':
		output += "&amp;";
		break;
	    case '<':
		output += "&lt;";
		break;
	    case '>':
		output += "&gt;";
		break;
	    default:
		output += ch;
	}
    }

    if (phrase_len && --phrase_len == 0) output += hi_end;

    best_pipe.pop_front();
    return true;
}

static void
check_query(const Xapian::Query & query,
	    list<vector<string>> & exact_phrases,
	    unordered_map<string, double> & loose_terms,
	    list<string> & wildcards,
	    size_t & longest_phrase)
{
    // FIXME: OP_NEAR, non-tight OP_PHRASE, OP_PHRASE with non-term subqueries
    size_t n_subqs = query.get_num_subqueries();
    Xapian::Query::op op = query.get_type();
    if (op == query.LEAF_TERM) {
	const Xapian::Internal::QueryTerm & qt =
	    *static_cast<const Xapian::Internal::QueryTerm *>(query.internal.get());
	loose_terms.insert(make_pair(qt.get_term(), 0));
    } else if (op == query.OP_WILDCARD) {
	const Xapian::Internal::QueryWildcard & qw =
	    *static_cast<const Xapian::Internal::QueryWildcard *>(query.internal.get());
	wildcards.push_back(qw.get_pattern());
    } else if (op == query.OP_PHRASE) {
	const Xapian::Internal::QueryPhrase & phrase =
	    *static_cast<const Xapian::Internal::QueryPhrase *>(query.internal.get());
	if (phrase.get_window() == n_subqs) {
	    // Tight phrase.
	    for (size_t i = 0; i != n_subqs; ++i) {
		if (query.get_subquery(i).get_type() != query.LEAF_TERM)
		    goto non_term_subquery;
	    }

	    // Tight phrase of terms.
	    exact_phrases.push_back(vector<string>());
	    vector<string> & terms = exact_phrases.back();
	    terms.reserve(n_subqs);
	    for (size_t i = 0; i != n_subqs; ++i) {
		Xapian::Query q = query.get_subquery(i);
		const Xapian::Internal::QueryTerm & qt =
		    *static_cast<const Xapian::Internal::QueryTerm *>(q.internal.get());
		terms.push_back(qt.get_term());
	    }
	    if (n_subqs > longest_phrase) longest_phrase = n_subqs;
	    return;
	}
    }
non_term_subquery:
    for (size_t i = 0; i != n_subqs; ++i)
	check_query(query.get_subquery(i), exact_phrases, loose_terms,
		    wildcards, longest_phrase);
}

static double*
check_term(unordered_map<string, double> & loose_terms,
	   const Xapian::Weight::Internal * stats,
	   const string & term,
	   double max_tw)
{
    auto it = loose_terms.find(term);
    if (it == loose_terms.end()) return NULL;

    if (it->second == 0.0) {
	double relevance;
	if (!stats->get_termweight(term, relevance)) {
	    // FIXME: Assert?
	    loose_terms.erase(it);
	    return NULL;
	}

	it->second = relevance + max_tw;
    }
    return &it->second;
}

string
MSet::Internal::snippet(const string & text,
			size_t length,
			const Xapian::Stem & stemmer,
			unsigned flags,
			const string & hi_start,
			const string & hi_end,
			const string & omit) const
{
    if (hi_start.empty() && hi_end.empty() && text.size() <= length) {
	// Too easy!
	return text;
    }

    bool cjk_ngram = CJK::is_cjk_enabled();

    size_t term_start = 0;
    double min_tw = 0, max_tw = 0;
    if (stats) stats->get_max_termweight(min_tw, max_tw);
    if (max_tw == 0.0) {
	max_tw = 1.0;
    } else {
	// Scale up by (1 + 1/64) so that highlighting works better for terms
	// with termweight 0 (which happens for terms not in the database, and
	// also with some weighting schemes for terms which occur in almost all
	// documents.
	max_tw *= 1.015625;
    }

    SnipPipe snip(length);

    list<vector<string>> exact_phrases;
    unordered_map<string, double> loose_terms;
    list<string> wildcards;
    size_t longest_phrase = 0;
    check_query(enquire->get_query(), exact_phrases, loose_terms,
		wildcards, longest_phrase);

    vector<double> exact_phrases_relevance;
    exact_phrases_relevance.reserve(exact_phrases.size());
    for (auto&& terms : exact_phrases) {
	// FIXME: What relevance to use?
	exact_phrases_relevance.push_back(max_tw * terms.size());
    }

    vector<double> wildcards_relevance;
    wildcards_relevance.reserve(exact_phrases.size());
    for (auto&& pattern : wildcards) {
	// FIXME: What relevance to use?
	(void)pattern;
	wildcards_relevance.push_back(max_tw + min_tw);
    }

    // Background relevance is the same for a given MSet, so cache it
    // between calls to MSet::snippet() on the same object.
    unordered_map<string, double>& background = snippet_bg_relevance;

    vector<string> phrase;
    if (longest_phrase) phrase.resize(longest_phrase - 1);
    size_t phrase_next = 0;
    bool matchfound = false;
    parse_terms(Utf8Iterator(text), cjk_ngram, true,
	[&](const string & term, bool positional, const Utf8Iterator & it) {
	    // FIXME: Don't hardcode this here.
	    const size_t max_word_length = 64;

	    if (!positional) return true;
	    if (term.size() > max_word_length) return true;

	    // We get segments with any "inter-word" characters in front of
	    // each word, e.g.:
	    // [The][ cat][ sat][ on][ the][ mat]
	    size_t term_end = text.size() - it.left();

	    double* relevance = 0;
	    size_t highlight = 0;
	    if (stats) {
		size_t i = 0;
		for (auto&& terms : exact_phrases) {
		    if (term == terms.back()) {
			size_t n = terms.size() - 1;
			bool match = true;
			while (n--) {
			    if (terms[n] != phrase[(n + phrase_next) % (longest_phrase - 1)]) {
				match = false;
				break;
			    }
			}
			if (match) {
			    // FIXME: Sort phrases, highest score first!
			    relevance = &exact_phrases_relevance[i];
			    highlight = terms.size();
			    goto relevance_done;
			}
		    }
		    ++i;
		}

		relevance = check_term(loose_terms, stats, term, max_tw);
		if (relevance) {
		    // Matched unstemmed term.
		    highlight = 1;
		    goto relevance_done;
		}

		string stem = "Z";
		stem += stemmer(term);
		relevance = check_term(loose_terms, stats, stem, max_tw);
		if (relevance) {
		    // Matched stemmed term.
		    highlight = 1;
		    goto relevance_done;
		}

		// Check wildcards.
		// FIXME: Sort wildcards, shortest pattern first or something?
		i = 0;
		for (auto&& pattern : wildcards) {
		    if (startswith(term, pattern)) {
			relevance = &wildcards_relevance[i];
			highlight = 1;
			goto relevance_done;
		    }
		    ++i;
		}

		if (flags & Xapian::MSet::SNIPPET_BACKGROUND_MODEL) {
		    // Background document model.
		    auto bgit = background.find(term);
		    if (bgit == background.end()) bgit = background.find(stem);
		    if (bgit == background.end()) {
			Xapian::doccount tf = enquire->db.get_termfreq(term);
			if (!tf) {
			    tf = enquire->db.get_termfreq(stem);
			} else {
			    stem = term;
			}
			double r = 0.0;
			if (tf) {
			    // Add one to avoid log(0) when a term indexes all
			    // documents.
			    Xapian::doccount num_docs = stats->collection_size + 1;
			    r = max_tw * log((num_docs - tf) / double(tf));
			    r /= (length + 1) * log(double(num_docs));
#if 0
			    if (r <= 0) {
				Utf8Iterator i(text.data() + term_start, text.data() + term_end);
				while (i != Utf8Iterator()) {
				    if (Unicode::get_category(*i++) == Unicode::UPPERCASE_LETTER) {
					r = max_tw * 0.05;
				    }
				}
			    }
#endif
			}
			bgit = background.emplace(make_pair(stem, r)).first;
		    }
		    relevance = &bgit->second;
		}
	    } else {
#if 0
		// In the absence of weight information, assume longer terms
		// are more relevant, and that unstemmed matches are a bit more
		// relevant than stemmed matches.
		if (queryterms.find(term) != queryterms.end()) {
		    relevance = term.size() * 3;
		} else {
		    string stem = "Z";
		    stem += stemmer(term);
		    if (queryterms.find(stem) != queryterms.end()) {
			relevance = term.size() * 2;
		    }
		}
#endif
	    }

	    // FIXME: Allow Enquire without a DB set or an empty MSet() to be
	    // used if you don't want the collection model?

#if 0
	    // FIXME: Punctuation should somehow be included in the model, but this
	    // approach is problematic - we don't want the first word of a sentence
	    // to be favoured when it's at the end of the window.

	    // Give first word in each sentence a relevance boost.
	    if (term_start == 0) {
		relevance += 10;
	    } else {
		for (size_t i = term_start; i + term.size() < term_end; ++i) {
		    if (text[i] == '.' && Unicode::is_whitespace(text[i + 1])) {
			relevance += 10;
			break;
		    }
		}
	    }
#endif

relevance_done:
	    if (longest_phrase) {
		phrase[phrase_next] = term;
		phrase_next = (phrase_next + 1) % (longest_phrase - 1);
	    }

	    if (highlight) matchfound = true;

	    if (!snip.pump(relevance, term_end, highlight, flags)) return false;

	    term_start = term_end;
	    return true;
	});

    snip.done();

    // Put together the snippet.
    string result;
    if (matchfound || (flags & SNIPPET_EMPTY_WITHOUT_MATCH) == 0) {
	while (snip.drain(text, hi_start, hi_end, omit, result)) { }
    }

    return result;
}

}
