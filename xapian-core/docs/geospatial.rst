.. Copyright (C) 2008 Lemur Consulting Ltd

================================
Geospatial searching with Xapian
================================

.. contents:: Table of contents

Introduction
============

This document describes a set of features present in Xapian which are designed
to allow geospatial searches to be supported.  Currently, the geospatial
support allows sets of locations to be stored associated with each document, as
latitude/longitude coordinates, and allows searches to be restricted or
reordered on the basis of distance from a second set of locations.

Three types of geospatial searches are supported:

 - Returning a list of the documents at least one of whose locations is within
   a given distance of a query location.  This may be used in conjunction with
   any other Xapian query, and with any Xapian sort order.

 - Returning a list of documents in (increasing) order of distance from a query
   location.  This may be used in conjunction with any Xapian query 

 - Returning a set of documents in a combined order based on distance from a
   query location, and relevance.





Implementation and performance details
======================================

The location information associated with each document is stored in a document
value.  This allows it to be looked up quickly at search time, so that the
exact distance from the query location can be calculated.

Additionally, extra "acceleration" terms are stored associated with each
document, which 


Future directions
=================

It is entirely possible that a more efficient implementation could be performed
using "R trees" or "KD trees" (or one of the many other tree structures used
for geospatial indexing - see http://en.wikipedia.org/wiki/Spatial_index for a
list of some of these).  However, the current implementation required minimal
effort and makes use of the existing, and well tested, Xapian database.
Additionally, by simply generating special terms to restrict the search, the
existing optimisations of the Xapian query parser are taken advantage of.

It would be interesting to compare performance of the current algorithm against
algorithms using specially designed multi-dimensional search trees.

The spacing at which the "acceleration" terms are generated is currently
ad-hoc.  It would be worthwhile trying to develop some kind of theoretical
analysis of the best set of spacings to use, including calculating the
worst-case scenarios, and to perform performance tests with various different
sets of spacings.


References
==========

The O-QTM algorithm is described in "Dutton, G. (1996). Encoding and handling
geospatial data with hierarchical triangular meshes. In Kraak, M.J. and
Molenaar, M. (eds.)  Advances in GIS Research II. London: Taylor & Francis,
505-518." , a copy of which is available from
http://www.spatial-effects.com/papers/conf/GDutton_SDH96.pdf

Use of the O-QTM mesh to generate location identifiers is covered in 

Some of the geometry needed to calculate the correct set of QTM IDs to cover a
particular region is detailed in
ftp://ftp.research.microsoft.com/pub/tr/tr-2005-123.pdf

Also, see:
http://www.sdss.jhu.edu/htm/doc/c++/htmInterface.html
