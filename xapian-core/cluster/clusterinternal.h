/** @file clusterinternal.h
 *  @brief Cluster API
 */
/* Copyright (C) 2017 Richhiey Thomas
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

#include <xapian/intrusive_ptr.h>

#include <unordered_map>
#include <vector>

/** Internal class for ClusterSet
 */
class Xapian::ClusterSet::Internal : public Xapian::Internal::intrusive_base {
  private:
    /// Copies are not allowed
    Internal(const Internal &);

    /// Assignment is not allowed
    void operator=(const Internal &);

    /** A vector storing the clusters that are created by the
     *  clusterers
     */
    std::vector<Cluster> clusters;
  public:
    /// Constructor
    Internal() {}

    /// Destructor
    ~Internal() {}

    /// Add a cluster to the cluster set
    void add_cluster(const Cluster &cluster);

    /// Add the point the the cluster at position 'index'
    void add_to_cluster(const Point &x, unsigned int index);

    /// Return the number of clusters
    Xapian::doccount size() const;

    /// Return the cluster at index 'i'
    Cluster& get_cluster(Xapian::doccount i);

    /// clear all the Clusters in the ClusterSet
    void clear_clusters();

    /** Recalculate the centroids for all the centroids
     *  in the ClusterSet
     */
    void recalculate_centroids();
};

/** Internal class for Cluster
 */
class Xapian::Cluster::Internal : public Xapian::Internal::intrusive_base {
  private:
    /// Copies are not allowed
    Internal(const Internal &);

    /// Assignment is not allowed
    void operator=(const Internal &);

    /// Documents (or Points in the vector space) within the cluster
    std::vector<Point> cluster_docs;

    /// Point or Document representing the cluster centroid
    Centroid centroid;

  public:
    Internal(const Centroid &centroid_) : centroid(centroid_) {}

    Internal() {}

    ~Internal() {}

    /// Returns size of the cluster
    Xapian::doccount size() const;

    /// Add a document to the Cluster
    void add_point(const Point &point);

    /// Clear the cluster values
    void clear();

    /// Return the point at the given index in the cluster
    Point& operator[](Xapian::doccount i);

    /// Return the point at the given index in the cluster
    const Point& operator[](Xapian::doccount i) const;

    /// Return the documents that are contained within the cluster
    DocumentSet get_documents() const;

    /// Return the current centroid of the cluster
    const Centroid& get_centroid() const;

    /// Set the centroid of the Cluster to 'centroid'
    void set_centroid(const Centroid &centroid);

    /** Recalculate the centroid of the Cluster after each iteration
     *  of the KMeans algorithm by taking the mean of all document vectors (Points)
     *  that belong to the Cluster
     */
    void recalculate();
};

/** Internal class for DocumentSet
 */
class Xapian::DocumentSet::Internal : public Xapian::Internal::intrusive_base {
  private:
    /// Copies are not allowed.
    Internal(const Internal &);

    /// Assignment is not allowed.
    void operator=(const Internal &);

    /// Vector storing the documents for this DocumentSet
    std::vector<Xapian::Document> documents;

  public:
    /// Constructor
    Internal() {}

    /// Destructor
    ~Internal() {}

    /// Returns the size of the DocumentSet
    Xapian::doccount size() const;

    /** Returns the Document at the index 'i' in the DocumentSet
     *
     * @params  i	Index of required document
     */
    const Xapian::Document& get_document(Xapian::doccount i) const;

    /** Add a new Document to the DocumentSet
     *
     * @params  document	Document to be added
     */
    void add_document(const Xapian::Document &document);
};
