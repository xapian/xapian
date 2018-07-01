.. Copyright (C) 2007 Jenny Black
.. Copyright (C) 2007,2008,2009,2011 Olly Betts
.. Copyright (C) 2007 Deron Meranda

========
Glossary
========

This glossary defines specialized terminology you may encounter while using
Xapian.  Some of the entries are standard in the field of Information
Retrieval, while others have a specific meaning in the context of Xapian.

.. The first sentence should ideally work alone to allow us to reuse these
.. in the future to generate pop-up information when the user moves the mouse
.. over the term used in the documentation.

**BM25**
 The weighting scheme which Xapian uses by default.  BM25 is a refinement on
 the original probabilistic weighting scheme, and recent TREC tests have shown
 BM25 to be the best of the known probabilistic weighting schemes.  It's
 sometimes known as "Okapi BM25" since it was first implemented in an
 academic IR system called Okapi.

**Boolean Retrieval**
 Retrieving the set of documents that match a boolean query (e.g. a
 list of terms joined with a combination of operators such as AND, OR,
 AND_NOT).  In many systems, these documents are not ranked according to their
 relevance.  In Xapian, a pure Boolean query may be used, or alternatively a
 Boolean style query can filter the retrieved documents, which are then ranked
 using a weighting formula.

**Brass**
 Brass was the current "under development" database format in Xapian 1.2.x,
 1.3.0 and 1.3.1.  It was renamed to 'glass' in Xapian 1.3.2 because we decided
 to use backend names in ascending alphabetical order to make it easier to
 understand which backend is newest, and since 'flint' was used recently, we
 skipped over 'd', 'e' and 'f'.

**Chert**
 Chert was the stable database format used in Xapian 1.2.x.  It is similar
 to Flint in many ways, but generally faster, and uses significantly less disk
 space.  Chert is very efficient and highly scalable.  It supports incremental
 modifications, and concurrent single-writer and multiple-reader access to a
 database.

**Collection Frequency**
 The collection frequency of a term is the total number of times is occurs in
 the database.  This is equal to the sum of the within-document frequency for
 the term in all the documents it occurs in.

**Database**
 In Xapian (as opposed to a relational database system) a database consists of
 little more than indexed documents: this reflects the purpose of Xapian as an
 information retrieval system, rather than an information storage system.
 These may also occasionally be called Indexes.  Glass is the default backend
 used by Xapian 1.4; Chert was the default backend used by Xapian 1.2; Flint
 was used by Xapian 1.0; Quartz was used prior to Xapian 1.0.

**Document ID**
 A unique positive integer identifying a document in a Xapian database.

**Document data**
 The document data is one of several types of information that can be
 associated with each document, the contents can be set to be anything in any
 format, examples include fields such as URL, document title, and an excerpt of
 text from the document.  If you wish to interoperate with Omega, it should
 contain name=value pairs, one per line (recent versions of Omega also support
 one field value per line, and can assign names to line numbers in the
 query template).

**Document**
 These are the items that are being retrieved.  Often they will be text
 documents (e.g. web pages, email messages, word processor documents)
 but they could be sections within such a document, or photos, video, music,
 user profiles, or anything else you want to index.

**Edit distance**
 A measure of how many "edits" are required to turn one text string into
 another, used to suggest spelling corrections.  The algorithm Xapian uses
 counts an edit as any of inserting a character, deleting a character,
 changing a character, or transposing two adjacent characters.

**ESet (Expand Set)**
 The Expand Set (ESet) is a ranked list of terms that could be used to expand
 the original query.  These terms are those which are statistically good
 differentiators between relevant and non-relevant documents.

**Flint**
 Flint was the default database format used in Xapian 1.0.x.  It was
 deprecated in 1.2.x and removed in 1.3.0.

**Glass**
 Glass is the default database format in Xapian 1.4.

**Index**
 If a document is described by a term, this term is said to index the document.
 Also, the database in Xapian and other IR systems is sometimes called an index
 (by analogy with the index in the back of a book).

**Indexer**
 The indexer takes documents (in various formats) and processes them so that they
 can be searched efficiently, they are then stored in the database.

**Information Need**
 The information need is what the user is looking for.  They will usually
 attempt to express this as a query string.

**Information Retrieval (IR)**
 Information Retrieval is the "science of search".  It's the name used to
 refer to the study of search and related topics in academia.

**MSet (Match Set)**
 The Match Set (MSet) is a ranked list of documents resulting from a query.
 The list is ranked according to document weighting, so the top document has
 the highest probability of relevance, the second document the second highest,
 and so on.  The number of documents in the MSet can be controlled, so it does
 not usually contain all of the matching documents.

**Normalised document length (ndl)**
 The normalised document length (ndl) is the length of a document (the number
 of terms it contains) divided by the average length of the documents
 within the system.  So an average length document would have ndl equal to 1,
 while shorter documents have ndl less than 1, and longer documents greater
 than 1.

**Omega**
 Omega comprises two indexers and a CGI search application built using the
 Xapian library.

**Posting List**
 A posting list is a list of the documents which a specific term indexes.  This
 can be thought of as a list of numbers - the document IDs.

**Posting**
 An instance of a particular term indexing a particular document.

**Precision**
 Precision is the density of relevant documents amongst those retrieved: the
 number of relevant documents returned divided by the total number of documents
 returned.

**Probabilistic IR**
 Probabilistic IR is retrieval using a weighting formula derived from
 probability theory to produce a ranked list of documents based upon estimated
 relevance.  Xapian supports several families of weighting schemes, some of
 which are based on probabilistic methods.

**Quartz**
 Quartz was the database format used by Xapian prior to version 1.0.  Support
 was dropped completely as of Xapian 1.1.0.

**Query**
 A query is the information need expressed in a form that an IR system can
 read.  It is usually a text string containing terms, and may include Boolean
 operators such as AND or OR, etc.

**Query Expansion**
 Modifying a query in an attempt to broaden the search results.

.. _rset:

**RSet (Relevance Set)**
 The Relevance Set (RSet) is the set of documents which have been marked by the
 user as relevant.  They can be used to suggest terms that the user may want to
 add to the query (these terms form an ESet), and also to adjust term weights
 to reorder query results.

**Recall**
 Recall is the proportion of relevant documents retrieved - the number of
 relevant documents retrieved divided by the total number of relevant
 documents.

**Relevance**
 Essentially, a document is relevant if it is what the user wanted.  Ideally,
 the retrieved documents will all be relevant, and the non-retrieved ones all
 non-relevant.

**Searcher**
 The searcher is a part of the IR system, it takes queries and reads the
 database to return a list of relevant documents.

**Stemming**
 A stemming algorithm performs linguistic normalisation by reducing variant
 forms of a word to a common form.  In English, this mainly involves removing
 suffixes - such as converting any of the words "talking", "talks", or "talked"
 to the stem form "talk".

**Stop word**
 A word which is ignored during indexing and/or searching, usually because it
 is very common or doesn't convey meaning.  For example, "the", "a", "to".

**Synonyms**
 Xapian can store synonyms for terms, and use these to implement one approach
 to query expansion.

**Term List**
 A term list is the list of terms that index a specific document.  In some
 systems this may be a list of numbers (with each term represented by a number
 internally), in Xapian it is a list of strings (the terms).

**Term frequency**
 The term frequency of a specific term is the number of documents in the system
 that are indexed by that term.

**Term**
 A term is a string of bytes (often a word or word stem) which describes a
 document.  Terms are similar to the index entries found in the back of a book
 and each document may be described by many terms.  A query is composed from
 a list of terms (perhaps linked by Boolean operators).

**Term Prefix**
 By convention, terms in Xapian can be prefixed to indicate a field in the
 document which they come from, or some other form of type information.
 The term prefix is usually a single capital letter.

**Test Collection**
 A test collection consists of a set of documents and a set of queries each of
 which has a complete set of relevance assignments - this is used to test how
 well different IR methods perform.

**UTF-8**
 A standard variable-length byte-oriented encoding for Unicode.

**Value**
 A discrete meta-data attribute attached to a document.  Each document can
 have many values, each stored in a different numbered slot.  Values are
 designed to be fast to access during the matching process, and can be used for
 sorting, collapsing redundant documents, implementing ranges, and other uses.
 If you're just wanting to store "fields" for displaying results, it's better
 to store them in the document data.

**Within-document frequency (wdf)**
 The within-document frequency (wdf) of a term in a specific document is the
 number of times it is pulled out of the document in the indexing process.
 Usually this is the size of the wdp vector, but in Xapian it can exceed it,
 since we can apply extra wdf to some parts of the document text.

**Within-document positions (wdp)**
 In the case where a term derives from words actually in the document, the
 within-document positions (wdp) are the positions at which that word occurs
 within the document.  So if the term derives from a word that occurs three
 times in the document as the fifth, 22nd and 131st word, the wdps will be 5,
 22 and 131.

**Within-query frequency (wqf)**
 The within-query frequency (wqf) is the number of times a term occurs in the
 query.  This statistic is used in the BM25 weighing scheme.

.. wqp?  nql?  Is it is worth adding these - they're not referenced much.
