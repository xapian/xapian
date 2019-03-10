.. Copyright (C) 2017 Vivek Pal

===============================================
Simplified Dynamic Bayesian Network Click Model
===============================================

.. contents:: Table of Contents

Introduction
============

Clicks in web search reflect user interests in retrieved documents and by the
use of various methods, clicks can be used to improve search results. Click
models are one such method. These are probabilistic graphical models that
make user clicks easier to understand, define and quantify.

There are various kinds of click models among which the dynamic bayesian
network (DBN) model is proved to be able to closely mimic the user’s search
behavior. The underlying idea of DBN model is that if a user is satisfied
with a document in the search engine results page (SERP), he/she stops
otherwise, continues to examine more documents with fixed probability.

The Simplified DBN model (SDBN), according to the paper_: `Chapelle, Olivier and
Zhang, Ya. A dynamic bayesian network click model for web search ranking.
Proceedings of WWW, pages 1-10, 2009.`__
is a simplified variation of DBN model wherein the inference algorithm is
relatively simpler. It contains the set of attractiveness and satisfactoriness
parameters, which both depend on a query and a document.

.. _paper: https://dl.acm.org/citation.cfm?id=1526711
__ paper_

Training the SDBN model
=======================

For training the model, we need the list of search sessions obtained from
clickstream log data. Xapian Omega has the clickstream logging functionality
which generates click log data in the required format in ``final.log`` file.

A few example entries in ``final.log``:

	QueryID,Query,Hits,Offset,Clicks
	821f03288846297c2cf43c34766a38f7,book,"45,54",0,"45:0,54:2"
	098f6bcd4621d373cade4e832627b4f6,test,"35,47",0,"35:1,47:0"

``SimplifiedDBN`` class provides ``train`` function that takes a list of search
sessions as its input.

To generate search sessions from the log data file, ``SimplifiedDBN`` class
also provides ``build_sessions`` function which takes the path to the log data
file as its input and returns a list of generated sessions.

The training process basically involves learning the values of attractiveness
and satisfactoriness parameters modelled by SDBN for each pair of query and
a corresponding document from the list of retrieved documents in search result.

Predicting the Document Relevance
=================================

``SimplifiedDBN`` class provides ``get_predicted_relevances`` function to get
the predicted relevance of all documents in a given search session based on
the trained SDBN model.

Use ``build_sessions`` function to generate a list of search sessions from an
input log data file as described above and then pass any particular search
session from the list for which you want the predicted relevances of the
documents in that session to ``get_predicted_relevances`` function which creates
a list of those predicted relevances.

Predicted relevances are the estimations of the relevance of each document in
a given session based on a trained model with their values ranging from 0.0 to
1.0. The relevance of the document is the probability that the user is satisfied
given that he examined and clicked on the document in the list of search results
presented to them by a search engine. An ideal search engine would show documents
with higher relevance on the top of search results.
