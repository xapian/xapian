/** \file geospatial.h
 * \brief Geospatial search support routines.
 */
/* Copyright 2008 Lemur Consulting Ltd
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

#ifndef XAPIAN_INCLUDED_GEOSPATIAL_H
#define XAPIAN_INCLUDED_GEOSPATIAL_H

#include <xapian/enquire.h>
#include <xapian/postingsource.h>
#include <xapian/query.h>
#include <xapian/sorter.h>
#include <xapian/visibility.h>
#include <string>
#include <set>

namespace Xapian {

/** Convert from miles to metres.
 */
inline XAPIAN_VISIBILITY_DEFAULT double
miles_to_metres(double miles)
{
    return 1609.344 * miles;
}

/** Convert from metres to miles.
 */
inline XAPIAN_VISIBILITY_DEFAULT double
metres_to_miles(double metres)
{
    return metres * (1.0 / 1609.344);
}

/** Convert from feet to metres.
 */
inline XAPIAN_VISIBILITY_DEFAULT double
feet_to_metres(double feet)
{
    return 0.3048 * feet;
}

/** Convert from metres to feet.
 */
inline XAPIAN_VISIBILITY_DEFAULT double
metres_to_feet(double metres)
{
    return metres * (1.0 / 0.3048);
}

/** Convert from nautical miles to metres.
 */
inline XAPIAN_VISIBILITY_DEFAULT double
nautical_miles_to_metres(double nautical_miles)
{
    return 1852.0 * nautical_miles;
}

/** Convert from metres to nautical miles.
 */
inline XAPIAN_VISIBILITY_DEFAULT double
metres_to_nautical_miles(double metres)
{
    return metres * (1.0 / 1852.0);
}

/** A latitude-longitude coordinate.
 *
 *  Note that latitude-longitude coordinates are only precisely meaningful if
 *  the datum used to define them is specified.  This class ignores this
 *  issue - it is up to the caller to ensure that the datum used for each
 *  coordinate in a system is consistent.
 */
struct XAPIAN_VISIBILITY_DEFAULT LatLongCoord {
  public:
    /** A latitude, as decimal degrees.
     *
     *  Should be in the range -90 <= longitude <= 90
     *
     *  Postive latitudes represent the northern hemisphere.
     */
    double latitude;

    /** A longitude, as decimal degrees.
     *
     *  Should be in the range -180 < latitude <= 180
     *
     *  Positive longitudes represent the eastern hemisphere.
     */
    double longitude;

    /** Construct a coordinate.
     *
     *  If the supplied longitude is out of range, an exception will be raised.
     *
     *  If the supplied latitude is out of range, it will be normalised to the
     *  appropriate range.
     */
    LatLongCoord(double latitude_, double longitude_);

    /** Construct a coordinate by unserialising a string.
     *
     *  @param serialised the string to unserialise the coordinate from.
     *
     *  @exception Xapian::InvalidArgumentError if the string does not contain
     *  a valid serialised latitude-longitude pair, or contains extra data at
     *  the end of it.
     */
    static LatLongCoord unserialise(const std::string & serialised);

    /** Construct a coordinate by unserialising a string.
     *
     *  The string may contain further data after that for the coordinate.
     *
     *  @param ptr A pointer to the start of the string.  This will be updated
     *  to point to the end of the data representing the coordinate.
     *  @param end A pointer to the end of the string.
     *
     *  @exception Xapian::InvalidArgumentError if the string does not contain
     *  a valid serialised latitude-longitude pair.
     */
    static LatLongCoord unserialise(const char ** ptr, const char * end);

    /** Return a serialised representation of the coordinate.
     */
    std::string serialise() const;

    /** Parse a string representation of a latitude / longitude.
     *
     *  The latitude / longitude strings may be in any of the
     *  following forms:
     *
     *   - decimal degrees, optionally followed by a "degrees" symbol.
     *   (eg: 10.51o)
     *
     *   - degrees and minutes, followed by "degrees" and minutes
     *   symbols, optionally separated by whitespace.  (eg: 10o 30.6')
     *
     *   - degrees, minutes and seconds, followed by "degrees",
     *   "minutes" and "seconds" symbols, optionally separated by
     *   whitespace.  (eg: 10o 30' 36")
     *
     *  The latitude and longitude may be followed by direction
     *  indicators (N, S, E or W).
     *
     *  @exception Xapian::LatLongParserError if the string cannot be
     *  parsed into a valid coordinate.
     */
    static LatLongCoord parse_latlong(const std::string & coord);

    /** Parse two strings into latitude and longitude.
     *
     *  @param lat_string String holding the latitude.
     *  @param long_string String holding the longitude.
     *
     *  @exception Xapian::LatLongParserError if the string cannot be
     *  parsed into a valid coordinate.
     */
    static LatLongCoord parse_latlong(const std::string & lat_string,
				      const std::string & long_string);

    /** Compare with another LatLongCoord.
     */
    bool operator<(const LatLongCoord & other) const
    {
	if (latitude < other.latitude) return true;
	return (longitude < other.longitude);
    }
};

/** A set of latitude-longitude coordinate.
 */
class XAPIAN_VISIBILITY_DEFAULT LatLongCoords {
    /// The coordinates.
    std::set<LatLongCoord> coords;

  public:
    std::set<LatLongCoord>::const_iterator begin() const
    {
	return coords.begin();
    }

    std::set<LatLongCoord>::const_iterator end() const
    {
	return coords.end();
    }

    size_t size() const
    {
	return coords.size();
    }

    size_t empty() const
    {
	return coords.empty();
    }

    void insert(const LatLongCoord & coord)
    {
	coords.insert(coord);
    }

    void erase(const LatLongCoord & coord)
    {
	coords.erase(coord);
    }

    /// Construct an empty set of coordinates.
    LatLongCoords() : coords() {}

    /// Construct a set of coordinates containing one coordinate.
    LatLongCoords(const LatLongCoord & coord) : coords()
    {
	coords.insert(coord);
    }

    /** Construct a set of coordinates by unserialising a string.
     *
     *  @param serialised the string to unserialise the coordinates from.
     *
     *  @exception Xapian::InvalidArgumentError if the string does not contain
     *  a valid serialised latitude-longitude pair, or contains junk at the end
     *  of it.
     */
    static LatLongCoords unserialise(const std::string & serialised);

    /** Construct a set of coordinates by unserialising a string.
     *
     *  The string may NOT contain further data after the coordinates (the
     *  representation of the list of coordinates is not self-terminating).
     *
     *  @param ptr A pointer to the start of the string.
     *  @param end A pointer to the end of the string.
     *
     *  @exception Xapian::InvalidArgumentError if the string does not contain
     *  a valid serialised latitude-longitude pair, or contains junk at the end
     *  of it.
     */
    static LatLongCoords unserialise(const char * ptr, const char * end);

    /** Return a serialised form of the coordinate list.
     */
    std::string serialise() const;
};

/** Base class for calculating distances between two lat/long coordinates.
 */
class XAPIAN_VISIBILITY_DEFAULT LatLongMetric {
  public:
    /// Destructor.
    virtual ~LatLongMetric();

    /** Return the distance between two coordinates, in metres.
     */
    virtual double operator()(const LatLongCoord & a, const LatLongCoord &b) const = 0;

    /** Return the distance between two coordinate lists, in metres.
     *
     *  The distance between the coordinate lists is defined to the be minimum
     *  pairwise distance between coordinates in the lists.
     *
     *  If either of the lists is empty, an InvalidArgumentError will be raised.
     */
    double operator()(const LatLongCoords & a, const LatLongCoords &b) const;
};

/** Calculate the great-circle distance between two coordinates on a sphere.
 *
 *  This uses the haversine formula to calculate the distance.  Note that this
 *  formula is subject to inaccuracy due to numerical errors for coordinates on
 *  the opposite side of the sphere.
 *
 *  See http://en.wikipedia.org/wiki/Haversine_formula
 */
class XAPIAN_VISIBILITY_DEFAULT GreatCircleMetric : public LatLongMetric {
    /** The radius of the sphere in metres.
     */
    double radius;

  public:
    /** Construct a GreatCircleMetric.
     *
     *  The (quadratic mean) radius of the earth will be used by this
     *  calculator.
     */
    GreatCircleMetric();

    /** Construct a GreatCircleMetric using a specified radius.
     *
     *  @param radius_ The radius of to use, in metres.
     */
    GreatCircleMetric(double radius_);

    /** Return the great-circle distance between points on the sphere.
     */
    double operator()(const LatLongCoord & a, const LatLongCoord &b) const;
};

/** A set of nodes.
 */
typedef std::set<std::string> HTMNodeSet;

/** Calculate Heirarchical Triangular Mesh nodes for a given location.
 */
class XAPIAN_VISIBILITY_DEFAULT HTMCalculator {
    int min_depth;
    int max_depth;
  public:
    /** Construct an HTM calculator.
     *
     *  The calculator will return nodes at a set of different depths.  The
     *  depths in use can be specified by various parameters of the
     *  constructor.
     *
     *  @param min_depth_ The minimum depth of nodes to generate.
     *  @param max_depth_ The maximum depth of nodes to generate.
     */
    HTMCalculator(int min_depth_, int max_depth_);

    /** Get the nodes (at all depths in use) corresponding to a given location.
     *
     *  Any existing nodes in the set will be preserved, so this can be called
     *  repeatedly to build up the set of nodes corresponding to a set of
     *  locations.
     *
     *  @param nodes A set which the nodes will be added to.
     *  @param location The location to calculate the nodes for.
     */
    void get_nodes_for_location(HTMNodeSet & nodes,
				const LatLongCoord & location) const;

    /** Get the nodes (at all depths in use) corresponding to a set of
     *  locations.
     *
     *  @param nodes A set which the nodes will be added to.
     *  @param locations The locations to calculate the nodes for.
     */
    void get_nodes_for_locations(HTMNodeSet & nodes,
				 const LatLongCoords & locations) const
    {
	std::set<LatLongCoord>::const_iterator i;
	for (i = locations.begin(); i != locations.end(); ++i)
	{
	    get_nodes_for_location(nodes, *i);
	}
    }

    /** Simplify a set of nodes, by replacing sets of nodes at one
     *  level by a node at a higher level whenever possible.
     *
     *  @param nodes A set which the nodes will be added to.
     */
    void simplify(HTMNodeSet & nodes) const;

    /** Reduce a set of nodes to a smaller set by reducing the depth.
     *
     *  Groups of nodes are replaced by nodes at a lesser depth until there are
     *  only max_nodes left.
     *
     *  In some cases, it will not be possible to reduce the set of nodes to
     *  the desired size - this will generally happen if a limited set of
     *  depths are in use, and there are thus insufficiient common higher level
     *  nodes.
     *
     *  @param nodes     The set of nodes to reduce.
     *  @param max_nodes The desired maximum number of nodes to return.
     */
    void reduce(HTMNodeSet & nodes, int max_nodes) const;

    /** Get a set of nodes which cover all locations within a given radius of a
     *  central location.  The returned nodes may be at a variety of depths.
     *
     *  Any existing nodes in the set will be preserved, so this can be called
     *  repeatedly to build up the set of nodes covering a set of circles.
     *
     *  @param nodes A set which the nodes will be added to.
     *  @param centre The centre of the circle to cover.
     *  @param radius The radius of the circle to cover (in metres).
     *  @param metric The distance measure to use - the circle is defined as
     *  all points within the specified radius of the centre according to this
     *  measure.
     *  @param desired_nodes The desired number of nodes to use to cover the
     *  circle.  A higher number will result in closer coverage.  The actual
     *  number of nodes used will often differ slightly from this number, and
     *  may differ significantly (particularly if only a limited number of HTM
     *  depths are being used).
     */
    void get_nodes_covering_circle(HTMNodeSet & nodes,
				   const LatLongCoord & centre,
				   double radius,
				   const LatLongMetric * metric,
				   int desired_nodes) const;

    /** Get a set of nodes which cover all locations within a given radius of
     *  one of several locations.  The returned nodes may be at a variety of
     *  depths.
     *
     *  Any existing nodes in the set will be preserved, so this can be called
     *  repeatedly to build up the set of nodes covering several sets of circles.
     *
     *  @param nodes A set which the nodes will be added to.
     *  @param centres The centres of the circles to cover.
     *  @param radius The radius of the circles to cover (in metres).
     *  @param metric The distance measure to use - the circles are defined as
     *  all points within the specified radius of the centre according to this
     *  measure.
     *  @param desired_nodes The desired number of nodes to use to cover each
     *  circle.  A higher number will result in closer coverage.  The actual
     *  number of nodes used will often differ slightly from this number, and
     *  may differ significantly (particularly if only a limited number of HTM
     *  depths are being used).
     */
    void get_nodes_covering_circles(HTMNodeSet & nodes,
				    const LatLongCoords & centres,
				    double radius,
				    const LatLongMetric * metric,
				    int desired_nodes) const
    {
	std::set<LatLongCoord>::const_iterator i;
	for (i = centres.begin(); i != centres.end(); ++i)
	{
	    get_nodes_covering_circle(nodes, *i, radius, metric, desired_nodes);
	}
	simplify(nodes);
    }

    /** Add terms to a document for a set of nodes.
     *
     *  @param doc       The document to add the terms to.
     *  @param locations The locations to calculate the terms for.
     *  @param prefix    The prefix to use.  (This shouldn't end with a colon:
     *  a colon will be added automatically where appropriate).
     */
    void add_nodeterms_to_document(Document & doc,
				   const HTMNodeSet & nodes,
				   const std::string & prefix) const;

    /** Build a query corresponding to a set of nodes.
     *
     *  Such a query can be used to filter a query to return only documents in
     *  the region specified by the nodes.
     */
    Xapian::Query query_for_nodes(const HTMNodeSet & nodes,
				  const std::string & prefix) const;
};

/** Match decider which returns only those documents within a given range.
 */
class XAPIAN_VISIBILITY_DEFAULT LatLongRangeMatchDecider : public MatchDecider
{
    Xapian::valueno valno;
    const LatLongCoords & centre;
    double range;
    const LatLongMetric & metric;

  public:
    /** Construct a new match decider which returns only documents within
     *  range of one of the central coordinates.
     */
    LatLongRangeMatchDecider(Xapian::valueno valno_,
			     const LatLongCoords & centre_,
			     double range_,
			     const LatLongMetric & metric_)
	    : valno(valno_), centre(centre_), range(range_), metric(metric_)
    {}

    /** Implementation of virtual operator().
     *
     *  Returns true iff the value stored in the document is a coordinate set
     *  containing at least one item within the specified range of the centre.
     *
     *  @exception InvalidArgumentError if the value stored is not a valid
     *  coordinate set.
     */
    bool operator()(const Xapian::Document &doc) const;
};

/** Posting source which returns a weight based on geospatial distance.
 *
 *  Results are weighted by the distance from a fixed point, or list of points,
 *  calculated according to the metric supplied.  If multiple points are
 *  supplied (either in the constructor, or in the coordinates stored in a
 *  document) , the closest pointwise distance is returned.
 *
 *  Documents further away than a specified maximum range (or with no location
 *  stored in the specified slot) will not be returned.
 *
 *  The weight returned will be computed from the distance using the formula:
 *  k1 * (distance + k1) ** (- k2)
 *
 *  (Where k1 and k2 are (strictly) positive, floating point, constants, and
 *  default to 1000 and 1, respectively.  Distance is measured in metres, so
 *  this means that something at the centre gets a weight of 1.0, something 1km
 *  away gets a weight of 0.5, and something 3km away gets a weight of 0.25,
 *  etc)
 */
class XAPIAN_VISIBILITY_DEFAULT LatLongDistancePostingSource : public PostingSource
{
    /// The database we're reading values from.
    Xapian::Database db;

    /// The slot we're reading values from.
    Xapian::valueno valno;

    /// Value stream iterator.
    Xapian::ValueIterator it;

    /// End iterator corresponding to it.
    Xapian::ValueIterator end;

    /// Flag indicating if we've started (true if we have).
    bool started;

    /// Current distance from centre.
    double dist;

    /// An upper bound on the weight returned.
    double max_weight;

    /// A lower bound on the term frequency.
    Xapian::doccount termfreq_min;

    /// An estimate of the term frequency.
    Xapian::doccount termfreq_est;

    /// An upper bound on the term frequency.
    Xapian::doccount termfreq_max;

    /// Centre, to compute distance from.
    const LatLongCoords & centre;

    /// Metric to compute the distance with.
    const LatLongMetric & metric;

    /// Maximum range to allow.  If set to 0, there is no maximum range.
    double max_range;

    /// Constant used in weighting function.
    double k1;

    /// Constant used in weighting function.
    double k2;

    /** Calculate the distance for the current document.
     *
     *  Returns true if the distance was calculated ok, or false if the
     *  document didn't contain a valid serialised set of coordinates in the
     *  appropriate value slot.
     */
    void calc_distance();

  public:
    /** Construct a new match decider which returns only documents within
     *  range of one of the central coordinates.
     *
     *  @param db_ The database to read values from.
     *  @param valno_ The value slot to read values from.
     *  @param centre_ The centre point to use for distance calculations.
     *  @param metric_ The metric to use for distance calculations.
     *  @param max_range_ The maximum distance for documents which are returned.
     *  @param k1_ The k1 constant to use in the weighting function.
     *  @param k2_ The k2 constant to use in the weighting function.
     */
    LatLongDistancePostingSource(Xapian::Database db_,
				 Xapian::valueno valno_,
				 const LatLongCoords & centre_,
				 const LatLongMetric & metric_,
				 double max_range_ = 0.0,
				 double k1_ = 1000.0,
				 double k2_ = 1.0);

    Xapian::doccount get_termfreq_min() const;
    Xapian::doccount get_termfreq_est() const;
    Xapian::doccount get_termfreq_max() const;

    Xapian::weight get_maxweight() const;
    Xapian::weight get_weight() const;

    void next(Xapian::weight min_wt);
    void skip_to(Xapian::docid min_docid, Xapian::weight min_wt);
    bool check(Xapian::docid min_docid, Xapian::weight min_wt);

    bool at_end() const;

    Xapian::docid get_docid() const;

    void reset();

    std::string get_description() const;
};

/** Sorter subclass which sorts by distance from a latitude/longitude.
 *
 *  Results are ordered by the distance from a fixed point, or list of points,
 *  calculated according to the metric supplied.  If multiple points are
 *  supplied (either in the constructor, or in the coordinates stored in a
 *  document) , the closest pointwise distance is returned.
 *
 *  Note that you will usually want to use the "descending" sort order with
 *  this sorter, in order to sort in descending order of distance.
 */
class XAPIAN_VISIBILITY_DEFAULT LatLongDistanceSorter : public Sorter {
    Xapian::valueno valno;
    LatLongCoords centre;
    const LatLongMetric & metric;

  public:
    LatLongDistanceSorter(Xapian::valueno valno_,
			  const LatLongCoords & centre_,
			  const LatLongMetric & metric_)
	    : valno(valno_),
	      centre(centre_),
	      metric(metric_)
    {}

    LatLongDistanceSorter(Xapian::valueno valno_,
			  const LatLongCoord & centre_,
			  const LatLongMetric & metric_)
	    : valno(valno_),
	      centre(),
	      metric(metric_)
    {
	centre.insert(centre_);
    }

    std::string operator()(const Xapian::Document & doc) const;
};

}

#endif /* XAPIAN_INCLUDED_GEOSPATIAL_H */
