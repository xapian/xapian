/** @file snipper.cc
 * @brief Generate snippets from text relevant to MSet.
 */
/* Copyright (C) 2012 Mihai Bivol
 * Copyright (C) 2014 Olly Betts
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

#include <config.h>

#include "xapian/snipper.h"
#include "snipperinternal.h"

#include <xapian/document.h>
#include <xapian/enquire.h>
#include <xapian/termgenerator.h>
#include <xapian/stem.h>

#include <algorithm>
#include <map>
#include <string>
#include <vector>

#include "str.h"

using namespace std;

namespace Xapian {

Snipper::Snipper() : internal(new Snipper::Internal)
{
}

Snipper::Snipper(const Snipper & other) : internal(other.internal)
{
}

Snipper::~Snipper()
{
}

void
Snipper::set_mset(const MSet & mset, unsigned int rm_docno)
{
    internal->calculate_rm(mset, rm_docno);
}

string
Snipper::generate_snippet(const string & text,
			  unsigned int window_size,
			  double smoothing_coef)
{
    return internal->generate_snippet(text, window_size, smoothing_coef);
}

void
Snipper::set_stemmer(const Stem & stemmer)
{
    internal->stemmer = stemmer;
}

bool
Snipper::Internal::is_stemmed(const string & term)
{
    return (term.length() > 0 && term[0] == 'Z');
}

void
Snipper::Internal::calculate_rm(const MSet & mset, Xapian::doccount rm_docno)
{
    rm_docid current_id = 0;

    // Initialize global relevance model info.
    rm_total_weight = 0;
    rm_coll_size = 0;
    rm_documents.clear();
    rm_term_data.clear();

    Xapian::doccount current_doc = 0;
    // Index document and term data.
    for (MSetIterator ms_it = mset.begin(); ms_it != mset.end(); ++ms_it) {
	if (++current_doc > rm_docno)
	    break;
	rm_total_weight += ms_it.get_weight();
	const Document & doc = ms_it.get_document();
	Xapian::termcount doc_size = 0;
	for (TermIterator term_it = doc.termlist_begin(); term_it != doc.termlist_end(); ++term_it) {
	    if (is_stemmed(*term_it)) {
		// Increase current document size.
		doc_size += term_it.get_wdf();

		// Update term information.
		RMTermInfo & term_info = rm_term_data[*term_it];
		term_info.coll_occurrence += term_it.get_wdf();

		// Add new document that indexes the term.
		TermDocInfo td_data(current_id, term_it.get_wdf());
		term_info.indexed_docs_freq.push_back(td_data);
	    }
	}

	rm_coll_size += doc_size;

	rm_documents.push_back(RMDocumentInfo(current_id++, doc_size, ms_it.get_weight()));
    }
}

string
Snipper::Internal::generate_snippet(const string & text,
				    Xapian::termcount window_size,
				    double smoothing_coef)
{
    Document text_doc;
    TermGenerator term_gen;

    term_gen.set_document(text_doc);
    term_gen.set_stemmer(stemmer);
    term_gen.index_text(text);

    const Document & snippet_doc = term_gen.get_document();

    // Document terms.
    vector<TermPositionInfo> docterms;
    for (TermIterator it = snippet_doc.termlist_begin(); it != snippet_doc.termlist_end(); ++it) {
	if (is_stemmed(*it))
	    continue;

	for (PositionIterator pit = it.positionlist_begin(); pit != it.positionlist_end(); ++pit) {
	    docterms.push_back(TermPositionInfo(*it, *pit));
	}
    }

    sort(docterms.begin(), docterms.end());

    // Relevance model for each term.
    map<string, double> term_score;

    // Smoothing coefficient for relevance probability.
    double alpha = smoothing_coef;

    // Init docterms score.
    for (vector<TermPositionInfo>::iterator it = docterms.begin(); it < docterms.end(); ++it) {
	string term = "Z" + stemmer(it->term);
	term_score[term] = 0;
    }

    for (map<string, double>::iterator it = term_score.begin(); it != term_score.end(); ++it) {
	const string & term = it->first;

	const RMTermInfo & term_info = rm_term_data[term];
	double irrelevant_prob = (double)term_info.coll_occurrence / rm_coll_size;
	double relevant_prob = 0;
	vector<TermDocInfo>::const_iterator i;
	for (i = term_info.indexed_docs_freq.begin();
	     i != term_info.indexed_docs_freq.end();
	     ++i) {
	    rm_docid c_docid = i->docid;
	    // Occurrence of term in document.
	    Xapian::doccount doc_freq = i->freq;

	    // Document info.
	    const RMDocumentInfo & rm_doc_info = rm_documents[c_docid];
	    // Probability for term to be relevant in the current document.
	    double term_doc_prob = alpha * ((double)doc_freq / rm_doc_info.document_size)
				   + (1 - alpha) * irrelevant_prob;

	    // Probability for the current document to be relevant to the query.
	    double doc_query_prob = rm_doc_info.weight / rm_total_weight;

	    relevant_prob += term_doc_prob * doc_query_prob;
	}

	// Add relevance to map.
	it->second = relevant_prob - irrelevant_prob;
    }

    // Calculate basic snippet.
    unsigned int snippet_begin = 0;
    unsigned int snippet_end = window_size < docterms.size() ? window_size : docterms.size();
    double sum = 0;
    double max_sum = 0;
    vector<double> docterms_relevance;

    for (size_t i = 0; i < docterms.size(); ++i) {
	string term = "Z" + stemmer(docterms[i].term);
	docterms_relevance.push_back(term_score[term]);
    }

    // Remove interrupts.
    for (size_t i = 0; i < docterms.size(); ++i) {
	double prev_score = i > 0 ? docterms_relevance[i - 1] : 0;
	double next_score = i < (docterms.size() - 1) ? docterms_relevance[i + 1] : 0;
	if (docterms_relevance[i] < prev_score &&
	    docterms_relevance[i] < next_score) {
	    docterms_relevance[i] = (prev_score + next_score) / 2;
	}
    }

    for (unsigned int i = snippet_begin; i < snippet_end; ++i) {
	double score = docterms_relevance[i];
	sum += score;
    }
    max_sum = sum;

    for (size_t i = snippet_end; i < docterms.size(); ++i) {
	double score = docterms_relevance[i];
	sum += score;

	double head_score = docterms_relevance[i - window_size];
	sum -= head_score;

	if (sum > max_sum) {
	    max_sum = sum;
	    snippet_begin = i - window_size + 1;
	    snippet_end = i + 1;
	}
    }

    size_t last_pos = 0;

    // Retrieve actual snippet.
    string snippet;
    // Snippet size in bytes.
    unsigned int snippet_size = 200;

    unsigned int current_size = 0;
    do {
	if (last_pos == text.length()) {
	    break;
	}

	size_t new_pos = text.find(". ", last_pos);
	if (new_pos == string::npos) {
	    new_pos = text.length() - 1;
	}

	string sentence(text, last_pos, new_pos - last_pos + 1);
	Document sent_doc;
	term_gen.set_document(sent_doc);
	term_gen.index_text(sentence);

	int sent_size = 0;
	for (TermIterator it = sent_doc.termlist_begin(); it != sent_doc.termlist_end(); ++it) {
	    if (is_stemmed(*it)) {
		continue;
	    }

	    sent_size += it.positionlist_count();
	}

	current_size += sent_size;
	if (current_size > snippet_begin)
	    snippet += sentence;

	last_pos = new_pos + 1;
    } while (true);

    snippet.resize(snippet_size);

    snippet += "...";

    return snippet;
}

string
Snipper::get_description() const
{
    string desc = "Snipper(rm_doccount=";
    desc += str(internal->rm_documents.size());
    desc += ", rm_termcount=";
    desc += str(internal->rm_term_data.size());
    desc += ", rm_collection_size=";
    desc += str(internal->rm_coll_size);
    return desc;
}

}
