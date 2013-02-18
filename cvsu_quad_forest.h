/**
 * @file cvsu_quad_forest.h
 * @author Matti J. Eskelinen <matti.j.eskelinen@gmail.com>
 * @brief Quad Forest hierarchical data structure for analyzing images.
 *
 * Copyright (c) 2011-2013, Matti Johannes Eskelinen
 * All Rights Reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *   * Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright notice,
 *     this list of conditions and the following disclaimer in the documentation
 *     and/or other materials provided with the distribution.
 *   * Neither the name of the copyright holder nor the names of its
 *     contributors may be used to endorse or promote products derived from this
 *     software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL COPYRIGHT HOLDER BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef CVSU_QUAD_FOREST_H
#   define CVSU_QUAD_FOREST_H

#ifdef __cplusplus
extern "C" {
#endif

#include "cvsu_pixel_image.h"
#include "cvsu_integral.h"
#include "cvsu_edges.h"
#include "cvsu_list.h"

/** Stores the forest status as a collection of bitwise flags. */
typedef uint32 quad_forest_status;

/**
 * Stores segment information for quad_forest segmentation with union-find
 * disjoint set approach. In addition to id and rank information contains also
 * the segment bounding box and statistics.
 */
typedef struct quad_forest_segment_t
{
  /** Parent segment, that determines the segment id (may be self) */
  struct quad_forest_segment_t *parent;
  /** Rank value used for optimizing union-find process */
  uint32 rank;
  /** X-coordinate of the bounding box top left corner */
  uint32 x1;
  /** Y-coordinate of the bounding box top left corner */
  uint32 y1;
  /** X-coordinate of the bounding box bottom right corner */
  uint32 x2;
  /** Y-coordinate of the bounding box bottom right corner */
  uint32 y2;
  /** Statistics of the image region covered by this segment */
  statistics stat;
  integral_value devmean;
  integral_value devdev;
  truth_value has_boundary;
  /** Color assigned for this segment for visualizing purposes */
  byte color[4];
} quad_forest_segment;

/**
 * Stores edge information for edge and edge chain detection in quad_forest with
 * union-find disjoint set approach. In addition to id and rank information
 * contains also edge response values and local variation information.
 * TODO: Adding also edge chain links.
 */
typedef struct quad_forest_edge_t
{
  /** The parent edge, that determines the edge segment id */
  struct quad_forest_edge_t *parent;
  /** The rank value used for optimizing union-find process */
  uint32 rank;
  /** Horizontal edge response value averaged from the tree region */
  integral_value dx;
  /** Vertical edge response value averaged from the tree region */
  integral_value dy;
  /** Magnitude of the edge response average from the tree region */
  integral_value mag;
  /** The estimated dominant edge direction as averaged from the tree region */
  integral_value ang;
  /** The estimated mean response in neighborhood */
  integral_value mean;
  /** The estimated deviation of response in neighborhood */
  integral_value deviation;
  /** Stores the information about whether this tree contains an edge */
  truth_value has_edge;
  /** Direction in which edge was determined (H,V,N4) */
  direction dir;
} quad_forest_edge;

/**
 * Stores a quad tree holding image data.
 */
typedef struct quad_tree_t {
  /** X-coordinate of the top left corner */
  uint32 x;
  /** Y-coordinate of the top left corner */
  uint32 y;
  /** Size of the image region covered by this tree */
  uint32 size;
  /** Level of this tree in the hierarchy (top level is 0) */
  uint32 level;
  /** Statistics of the image region covered by this tree */
  statistics stat;
  /** Region info used in segmentation */
  quad_forest_segment segment;
  /** Edge info used in edge detection */
  quad_forest_edge edge;
  /** Parent tree, NULL if this is a root tree */
  struct quad_tree_t *parent;
  /* child trees, all NULL if the tree has not beed divided */
  /** Upper left child tree */
  struct quad_tree_t *nw;
  /** Upper right child tree */
  struct quad_tree_t *ne;
  /** Lower left child tree */
  struct quad_tree_t *sw;
  /** Lower right child tree */
  struct quad_tree_t *se;
  /* direct neighbors are cached in each tree as they are determined */
  /** Direct neighbor on the top side */
  struct quad_tree_t *n;
  /** Direct neighbor on the right side */
  struct quad_tree_t *e;
  /** Direct neighbor on the bottom side */
  struct quad_tree_t *s;
  /** Direct neighbor on the left side */
  struct quad_tree_t *w;
  /** Temporary pool value used in propagation algorithms */
  integral_value pool;
  /** Temporary squared pool value used in propagation algorithms */
  integral_value pool2;
  /** Temporary accumulator value used in propagation algorithms */
  integral_value acc;
  /** Temporary squared accumulator value used in propagation algorithms */
  integral_value acc2;
} quad_tree;

/**
 * Stores a forest of image trees.
 */
typedef struct quad_forest_t {
  /** Status flags for ensuring the correct state of the forest structure */
  quad_forest_status status;
  /** Original image from where the source data is copied */
  pixel_image *original;
  /** Source data used for updating the integral image */
  pixel_image *source;
  /** Integral image used for calculating tree statistics */
  integral_image integral;
  /** Number of rows in the tree grid */
  uint32 rows;
  /** Number of cols in the tree grid */
  uint32 cols;
  /** Number of regions found from the tree (after segmentation) */
  uint32 segments;
  /** Maximum size of trees (the size of root trees) */
  uint32 tree_max_size;
  /** Minimum size of trees, no tree will be divided beyond this size */
  uint32 tree_min_size;
  /** Horizontal offset of the grid from the origin of the source image */
  uint32 dx;
  /** Vertical offset of the grid from the origin of the source image */
  uint32 dy;
  /** List of all trees in the forest, including the root trees */
  list trees;
  /** Pointer to the last root tree in the tree list for resetting the forest */
  list_item *last_root_tree;
  /** Pointer array containing the root trees in the tree grid */
  quad_tree **roots;
} quad_forest;

/**
 * Initializes the contents of a quad_tree with null values.
 */
result quad_tree_nullify
(
  /** The quad_tree structure to be nullified. */
  quad_tree *target
);

/**
 * Everything that can be nullified should be able to tell if it's null.
 */
truth_value quad_tree_is_null
(
  /** The quad_tree structure to be checked for null. */
  quad_tree *target
);

/**
 * Divides a quad_tree in four smaller trees, if width is greater than 1.
 */
result quad_tree_divide
(
  /** The quad_forest where the tree resides. */
  quad_forest *forest,
  /** The quad_tree to be divided. */
  quad_tree *target
);

/**
 * Determine whether the quad_tree has child trees.
 */
truth_value quad_tree_has_children
(
  /** The quad_tree to be checked for presence of children. */
  quad_tree *target
);

/**
 * Generates the statistics of the four child trees without dividing the tree.
 * Useful for determining consistency before deciding to divide.
 */
result quad_tree_get_child_statistics
(
  /** The quad_forest where the tree resides. */
  quad_forest *forest,
  /** The quad_tree that will will be examined. */
  quad_tree *source,
  /** The array of child quad_trees to fill, must contain at least 4. */
  quad_tree *target
);

/**
 * Generates the statistics from a neighborhoood around a quad_tree. Tree size
 * is multiplied by the multiplier value, and a region of that size is added
 * around the region covered by the tree; in other words, the neighborhood size
 * will be tree->size + 2 * multiplier * tree->size.
 */
result quad_tree_get_neighborhood_statistics
(
  /** The quad_forest where the tree resides. */
  quad_forest *forest,
  /** The quad_tree around which the neighborhood is generated. */
  quad_tree *tree,
  /** The statistics structure where the result will be stored. */
  statistics *target,
  /** The quad_tree.size multiplier for generating the neighborhood. */
  integral_value multiplier
);

/**
 * Calculates the child tree statistics but divides the tree only if the
 * entropy measure of the children is higher than the given threshold.
 * Higher values will require higher overlap to divide.
 */
result quad_tree_divide_with_overlap
(
  /** The quad_forest where the tree resides. */
  quad_forest *forest,
  /** The quad_tree to be divided if its internal overlap is high. */
  quad_tree *target,
  /** Deviation multiplier used for creating the estimated intensity range. */
  integral_value alpha,
  /** Range overlap threshold value used to decide whether to divide or not. */
  integral_value overlap_threshold
);

/**
 * Calculates the cumulative edge response in the region covered by a quad_tree
 * using integral images.
 */
result quad_tree_get_edge_response
(
  /** The quad_forest where the tree resides. */
  quad_forest *forest,
  /** The quad_tree where the edge response is calculated. */
  quad_tree *target,
  /** Pointer where the horizontal scanning result is stored. */
  integral_value *dx,
  /** Pointer where the vertical scanning result is stored. */
  integral_value *dy
);

/**
 * Generates the edge responses of the four child trees without dividing the
 * tree. Useful for determining consistency before deciding to divide.
 */
result quad_tree_get_child_edge_response
(
  /** The quad_forest where the tree resides. */
  quad_forest *forest,
  /** The quad_tree that will will be examined. */
  quad_tree *source,
  /** Array where the horizontal scanning results are stored: must fit 4. */
  integral_value dx[],
  /** Array where the vertical scanning results are stored: must fit 4. */
  integral_value dy[]
);

/**
 * Adds all immediate neighbors of a quad_tree to a list.
 * The neighbor links point to the direct neighbors, or to those trees that are
 * directly adjacent at the same or lower level, thus of the same or larger
 * size. There is always only one direct neighbor on each side. But for finding
 * all immediate neighbors, it is necessary to check if the direct neighbor has
 * children, and then add recursively the child trees on the correct side into
 * the neighbor list.
 */
result quad_tree_get_neighbors
(
  list *target,
  quad_tree *tree
);

/**
 * Creates a new segment from this quad_tree.
 * Part of the Union-Find implementation for quad_trees.
 */
void quad_tree_segment_create
(
  quad_tree *tree
);

/**
 * Creates a union of two segments.
 */
void quad_forest_segment_union
(
  quad_forest_segment *segment1,
  quad_forest_segment *segment2
);

/**
 * Creates a union of the two segments these two quad_trees belong to.
 * Part of the Union-Find implementation for quad_trees.
 */
void quad_tree_segment_union
(
  quad_tree *tree1,
  quad_tree *tree2
);

/**
 * Finds the parent element in the segment this tree belongs to.
 * Part of the Union-Find implementation for quad_trees.
 */
quad_forest_segment *quad_tree_segment_find
(
  quad_tree *tree
);

/**
 * Gets the segment id for this quad_tree. Effectively the pointer cast into an
 * int. Helper function on top of the Union-Find implementation for quad_trees.
 */
uint32 quad_tree_segment_get
(
  quad_tree *tree
);

/**
 * Checks if this quad_tree is a segment parent (id == segment_info)
 */
truth_value quad_tree_is_segment_parent
(
  quad_tree *tree
);

/**
 * Allocates memory for a quad forest structure.
 */
quad_forest *quad_forest_alloc();

/**
 * Frees the memory for a quad_forest structure allocated with
 * @see quad_forest_alloc. Will also destroy the structure properly using
 * @see quad_forest_destroy.
 */
void quad_forest_free
(
  /** A pointer to the quad_forest to be freed. */
  quad_forest *target
);

/**
 * Creates a quad_forest from a pixel_image.
 */
result quad_forest_create
(
  /** The quad_forest that will store the created data structure. */
  quad_forest *target,
  /** The pixel_image where the image data is acquired. */
  pixel_image *source,
  /** The maximum size of the trees (initial size of the root trees). */
  uint32 tree_max_size,
  /** The minimum size of the trees within this quad_forest. */
  uint32 tree_min_size
);

/**
 * Reloads the quad_forest structure using the same image, but possibly with
 * different tree sizes. Also used in the @see quad_forest_create function, to
 * initialize the structures for the first time.
 */
result quad_forest_reload
(
  /** The quad_forest that will be reloaded. */
  quad_forest *target,
  /** The maximum size of the trees (initial size of the root trees). */
  uint32 tree_max_size,
  /** The minimum size of the trees within this quad_forest. */
  uint32 tree_min_size
);

/**
 * Destroys a quad_forest structure and deallocates all data.
 */
result quad_forest_destroy
(
  /** The quad_forest structure to be destroyed. */
  quad_forest *target
);

/**
 * Refreshes the segment count and colors. MUST be called after segmentation and
 * BEFORE calling @see quad_forest_get_segments.
 */
result quad_forest_refresh_segments
(
  quad_forest *target
);

/**
 * Initializes the contents of a quad_forest to null. Does not deallocate data.
 */
result quad_forest_nullify
(
  /** The quad_forest structure to be nullified. */
  quad_forest *target
);

/**
* Everything that can be nullified should be able to tell if it's null.
*/
truth_value quad_forest_is_null
(
  /** The quad_forest structure to be checked for null. */
  quad_forest *target
);

/**
 * Updates a quad_forest structure; copies data from the original image,
 * possibly converting the format, cleans the data structure, updates the
 * integral_image and updates the statistics of all trees.
 */
result quad_forest_update
(
  /** The quad_forest structure to be updated. */
  quad_forest *target
);

/**
 * Segments the quad_forest structure using a deviation threshold as
 * consistency and similarity criteria. Divides all trees that have deviation
 * larger than the threshold, and then merges trees and regions that have the
 * difference of means smaller than the threshold.
 */
result quad_forest_segment_with_deviation
(
  /** The quad_forest structure to be segmented. */
  quad_forest *target,
  /** Threshold value for deviation, trees with larger value are divided. */
  integral_value threshold,
  /** Deviation multiplier used for creating the estimated intensity range. */
  integral_value alpha
);

/**
 * Segments the quad_forest structure using an entropy measure as consistency
 * and similarity criteria.
 * TODO: maybe add some region size constraint as a parameter.
 */
result quad_forest_segment_with_overlap
(
  /** The quad_forest structure to be segmented. */
  quad_forest *target,
  /** Deviation multiplier used for creating the estimated intensity range. */
  integral_value alpha,
  /** Range overlap threshold used for determining the trees to merge. */
  integral_value threshold_trees,
  /** Range overlap threshold used for determining the segments to merge. */
  integral_value threshold_segments
);

/**
 * Collects all region parents from the quad_forest structure into a list. The
 * array has to be allocated by the caller to the correct size, as indicated
 * by the segments member of the quad_forest structure.
 */
result quad_forest_get_segments
(
  /** The quad_forest where the segments will be collected. */
  quad_forest *source,
  /** A segment array, must be allocated by the caller to the correct size. */
  quad_forest_segment **target
);

/**
 * Collects into a list all trees belonging to a(n array of) segment(s).
 */
result quad_forest_get_segment_trees
(
  list *target,
  quad_forest *forest,
  quad_forest_segment **segments,
  uint32 segment_count
);

/**
 * Collects into a list all segments that are neighbors of a(n array of)
 * segment(s).
 */
result quad_forest_get_segment_neighbors
(
  list *target,
  quad_forest *forest,
  quad_forest_segment **segments,
  uint32 segment_count
);

/**
 * Draw tree values over an rgb image. For segmented images, use_segments
 * can be set to true and segment colors will be used for trees, otherwise
 * mean value with deviation encoded into the red component is used. In this
 * case, also draws the boundary trees with yellow color, if boundaries have
 * been detected.
 */
result quad_forest_draw_trees
(
  quad_forest *forest,
  pixel_image *target,
  truth_value use_segments
);

/**
 * Collects all trees contained in a list of segments and generates a
 * pixel_image where all pixels within the segments are white (or black if the
 * invert parameter is set to true)
 */
result quad_forest_get_segment_mask
(
  quad_forest *forest,
  pixel_image *target,
  quad_forest_segment **segments,
  uint32 segment_count,
  truth_value invert
);

/**
 * Generates a list of lines that surrounds a segment, rounding the corners.
 */
result quad_forest_get_segment_boundary
(
  quad_forest *forest,
  quad_forest_segment *segment,
  list *boundary
);

/**
* Collects all trees contained in a list of segments and draws them on a
* pixel_image with a red color (TODO: add parameter for selecting color)
*/
result quad_forest_highlight_segments
(
  quad_forest *forest,
  pixel_image *target,
  quad_forest_segment **segments,
  uint32 segment_count,
  byte color[4]
);

/**
 * Draws an image of the quad_forest structure using the current division and
 * segment info. Each quad_tree will be painted as a square with uniform color,
 * using the color assigned to the segment parent, the mean value from the
 * segment statistics, or the mean value from the quad_tree statistics.
 */
result quad_forest_draw_image
(
  /** The quad_forest to be drawn into an image. */
  quad_forest *forest,
  /** Pointer to a pixel_image, will be (re)created to fit the forest image. */
  pixel_image *target,
  /** Should we use segment statistics or individual tree statistics? */
  truth_value use_segments,
  /** For segments, should we use mean or colors? No effect for trees. */
  truth_value use_colors
);

/**
 * Uses edge responses and graph propagation to find trees containing strong
 * magnitude edges.
 */
result quad_forest_find_edges
(
  /** Forest where edges are searched */
  quad_forest *forest,
  /** How many rounds to propagate */
  uint32 rounds,
  /** Bias value added to mean, for triggering presence of edge */
  integral_value bias,
  /** Direction of edges to search (H,V,N4) */
  direction dir
);

/**
 * Uses deviation propagation to find potential segment boundaries.
 */
result quad_forest_find_boundaries
(
  quad_forest *forest,
  uint32 rounds,
  integral_value bias
);

/**
 * Uses deviation propagation to find strong boundary trees, calculates edge
 * response for the found points, and then uses hysteresis guided by edge
 * direction to continue boundary chains from their endpoints, hopefully closing
 * gaps. Weak boundary trees are removed.
 */
result quad_forest_find_boundaries_with_hysteresis
(
  quad_forest *forest,
  uint32 rounds,
  integral_value high_bias,
  integral_value low_factor
);

result quad_forest_prune_boundaries
(
  quad_forest *forest
);

/**
 * Segments the forest by finding first all horizontal edges with edge
 * propagation, then merging segments that have edges in neighboring trees.
 */
result quad_forest_segment_edges
(
  /** Forest to be segmented */
  quad_forest *target,
  /** How many rounds to propagate while determining trees with edges */
  uint32 detect_rounds,
  /** Bias value used in edge detection */
  integral_value detect_bias,
  /** Direction of edges to search (H,V,N4) */
  direction detect_dir,
  /** How many rounds to propagate the found edges to close gaps */
  uint32 propagate_rounds,
  /** Acceptance threshold for propagated edges */
  integral_value propagate_threshold,
  /** The direction in which to propagate */
  direction propagate_dir,
  /** The direction in which to merge segments */
  direction merge_dir
);

/**
 * Segments the forest by using boundaries found using deviation propagation
 * and hysteresis to limit the expansion of segments.
 */
result quad_forest_segment_with_boundaries
(
  quad_forest *forest,
  uint32 rounds,
  integral_value high_bias,
  integral_value low_factor,
  integral_value tree_alpha,
  integral_value segment_alpha,
  truth_value use_hysteresis,
  truth_value use_pruning
);

#ifdef __cplusplus
}
#endif

#endif /* CVSU_QUAD_FOREST_H */
