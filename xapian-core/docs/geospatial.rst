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

 - Returning a list of documents in order of distance from a query location.
   This may be used in conjunction with any Xapian query.

 - Returning a list of documents within a given distance of a query location.
   This may be used in conjunction with any other Xapian query, and with any
   Xapian sort order.

 - Returning a set of documents in a combined order based on distance from a
   query location, and relevance.

Locations are stored in value slots, allowing multiple independent locations to
be used for a single document.  It is also possible to store multiple
coordinates in a single value slot, in which case the closest coordinate will
be used for distance calculations.

Metrics
=======

A metric is a function which calculates the distance between two points.

Calculating the exact distance between two geographical points is an involved
subject.  In fact, even defining the meaning of a geographical point is very
hard to do precisely - not only do you need to define a mathematical projection
used to calculate the coordinates, you also need to choose a model of the shape
of the Earth, and identify a few sample points to identify the coordinates of
particular locations.  Since the Earth is constantly changing shape, these
coordinates also need to be defined at a particular date.

There are a few standard datums which define all these - a very common datum is
the WGS84 datum, which is the datum used by the GPS system.  Unless you have a
good reason not to, we recommend using the WGS84 datum, since this will ensure
that preset parameters of the functions built in to Xapian will have the
correct values (currently, the only such parameter is the Earth radius used by
the GreatCircleMetric, but more may be added in future).

Since there are lots of ways of calculating distances between two points, using
different assumptions about the approximations which are valid, Xapian allows
user-implemented metrics.  These are subclasses of the Xapian::LatLongMetric
class; see the API documentation for details on how to implement the various
required methods.

There is currently only one built-in metric - the GreatCircleMetric.  As the
name suggests, this calculates the distance between a latitude and longitude
based on the assumption that the world is a perfect sphere.  The radius of the
world can be specified as a constructor parameter, but defaults to a reasonable
approximation of the radius of the Earth.  The calculation uses the haversine
formula, which is accurate for points which are close together, but can have
significant error for coordinates which are on opposite sides of the sphere: on
the other hand, such points are likely to be at the end of a ranked list of
search results, so this probably doesn't matter.

Indexing
========

To index a set of documents with location, you need to store serialised
latitude-longitude coordinates in a value slot in your documents.  To do this,
use the LatLongCoord class.  For example, this is how you might store a
latitude and longitude corresponding to "London" in value slot 0::

  Xapian::Document doc;
  doc.add_value(0, Xapian::LatLongCoord(51.53, 0.08).serialise());

Of course, often a location is a bit more complicated than a single point - for
example, postcode regions in the UK can cover a fairly wide area.  If a search
were to treat such a location as a single point, the distances returned could
be incorrect by as much as a couple of miles.  Xapian therefore allows you to
store a set of points in a single slot - the distance calculation will return
the distance to the closest of these points.  This is often a good enough work
around for this problem - if you require greater accuracy, you will need to
filter the results after they are returned from Xapian.

To store multiple coordinates in a single slot, use the LatLongCoords class::

  Xapian::Document doc;
  Xapian::LatLongCoords coords;
  coords.append(Xapian::LatLongCoord(51.53, 0.08));
  coords.append(Xapian::LatLongCoord(51.51, 0.07));
  coords.append(Xapian::LatLongCoord(51.52, 0.09));
  doc.add_value(0, coords.serialise());

(Note that the serialised form of a LatLongCoords object containing a single
coordinate is exactly the same as the serialised form of the corresponding
LatLongCoord object.)

Searching
=========

Sorting results by distance
---------------------------

If you simply want your results to be returned in order of distance, you can
use the LatLongDistanceKeyMaker class to calculate sort keys.  For example, to
return results in order of distance from the coordinate (51.00, 0.50), based on
the values stored in slot 0, and using the great-circle distance::

  Xapian::Database db("my_database");
  Xapian::Enquire enq(db);
  enq.set_query(Xapian::Query("my_query"));
  GreatCircleMetric metric;
  LatLongCoord centre(51.00, 0.50);
  Xapian::LatLongDistanceKeyMaker keymaker(0, centre, metric);
  enq.set_sort_by_key(keymaker, False);

Filtering results by distance
-----------------------------

To return only those results within a given distance, you can use the
LatLongDistancePostingSource.  For example, to return only those results within
5 miles of coordinate (51.00, 0.50), based on the values stored in slot 0, and
using the great-circle distance::

  Xapian::Database db("my_database");
  Xapian::Enquire enq(db);
  Xapian::Query q("my_query");
  GreatCircleMetric metric;
  LatLongCoord centre(51.00, 0.50);
  double max_range = Xapian::miles_to_metres(5);
  Xapian::LatLongDistancePostingSource ps(0, centre, metric, max_range)
  q = Xapian::Query(Xapian::Query::OP_FILTER, q, Xapian::Query(ps));
  enq.set_query(q);

Ranking results on a combination of distance and relevance
----------------------------------------------------------

To return results ranked by a combination of their relevance and their
distance, you can also use the LatLongDistancePostingSource.  Beware that
getting the right balance of weights is tricky: there is little solid
theoretical basis for this, so the best approach is often to try various
different parameters, evaluate the results, and settle on the best.  The
LatLongDistancePostingSource returns a weight of 1.0 for a document which is at
the specified location, and a lower, but always positive, weight for points
further away. It has two parameters, k1 and k2, which control how fast the
weight decays, which can be specified to the constructor (but aren't in this
example) - see the API documentation for details of these parameters.::

  Xapian::Database db("my_database");
  Xapian::Enquire enq(db);
  Xapian::Query q("my_query");
  GreatCircleMetric metric;
  LatLongCoord centre(51.00, 0.50);
  double max_range = Xapian::miles_to_metres(5);
  Xapian::LatLongDistancePostingSource ps(0, centre, metric, max_range)
  q = Xapian::Query(Xapian::Query::AND, q, Xapian::Query(ps));
  enq.set_query(q);


Performance
===========

The location information associated with each document is stored in a document
value.  This allows it to be looked up quickly at search time, so that the
exact distance from the query location can be calculated.  However, this method
requires that the distance of each potential match is checked, which can be
expensive.

To gain a performance boost, it is possible to store additional terms in
documents to identify regions at various scales.  There are various ways to
generate such terms (for example, the O-QTM algorithm referenced below).
However, the encoding for coordinates that Xapian uses has some nice properties
which help here.  Specifically, the standard encoded form for a coordinate used
is a 6 byte representation, which identifies a point on the surface of the
earth to an accuracy of 1/16 of a second (ie, at worst slightly less than 2
metre accuracy).  However, this representation can be truncated to 2 bytes to
represent a bounding box 1 degree on a side, or to 3, 4 or 5 bytes to get
successively more accurate bounding boxes.

It would therefore be possible to gain considerable efficiency for range
restricted searches by storing terms holding each of these successively more
accurate representations, and to construct a query combining an appropriate set
of these terms to ensure that only documents which are potentially in a range
of interest are considered.

It is entirely possible that a more efficient implementation could be performed
using "R trees" or "KD trees" (or one of the many other tree structures used
for geospatial indexing - see http://en.wikipedia.org/wiki/Spatial_index for a
list of some of these).  However, using the QTM approach will require minimal
effort and make use of the existing, and well tested, Xapian database.
Additionally, by simply generating special terms to restrict the search, the
existing optimisations of the Xapian query parser are taken advantage of.

References
==========

The following may be of interest.

The O-QTM algorithm is described in "Dutton, G. (1996). Encoding and handling
geospatial data with hierarchical triangular meshes. In Kraak, M.J. and
Molenaar, M. (eds.)  Advances in GIS Research II. London: Taylor & Francis,
505-518." , a copy of which is available from
http://www.spatial-effects.com/papers/conf/GDutton_SDH96.pdf

Some of the geometry needed to calculate the correct set of QTM IDs to cover a
particular region is detailed in
ftp://ftp.research.microsoft.com/pub/tr/tr-2005-123.pdf

Also, see:
http://www.sdss.jhu.edu/htm/doc/c++/htmInterface.html
