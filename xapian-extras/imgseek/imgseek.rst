.. Copyright (C) 2009 Lemur Consulting Ltd.

=============================
Image Similarity with Xapian.
=============================

.. contents:: Table of Contents


Introduction
============

This document describes features in Xapian for image similarity
searching. These features are based on ideas and code from the imgseek
project http://imgseek.net. The underlying techique is based on
ideas in the paper "Fast Multiresolution Image Querying":
http://grail.cs.washington.edu/projects/query/mrquery.pdf.

These features allow signatures for one or more images to be stored in
a document, allowing searches to be restricted or reordered based on
similarity to the (signatures of) images associated with a specified
document.

Implementation
==============

