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

#include "cvsu_config.h"
#include "cvsu_types.h"
#include "cvsu_pixel_image.h"
#include "cvsu_integral.h"
#include "cvsu_edges.h"
#include "cvsu_list.h"
#include "cvsu_quad_tree.h"

/** Stores the forest status as a collection of bitwise flags. */
typedef uint32 quad_forest_status;

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
  /** Token for identifying processing stage */
  uint32 token;
  /** List of all trees in the forest, including the root trees */
  list trees;
  /** List of all links between the trees of the forest */
  list links;
  /** Pointer to the last root tree in the tree list for resetting the forest */
  list_item *last_root_tree;
  /** Pointer array containing the root trees in the tree grid */
  quad_tree **roots;
} quad_forest;

/**
 * Allocates memory for a quad forest structure.
 */
quad_forest *quad_forest_alloc();

/**
 * Frees the memory for a quad_forest structure allocated with
 * @see quad_forest_alloc.
 * Will also destroy the structure properly using
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

void quad_forest_set_init(quad_forest *forest);
void quad_forest_set_update(quad_forest *forest);
void quad_forest_set_parse(quad_forest *forest);
truth_value quad_forest_has_parse(quad_forest *forest);

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
 * Collects all region parents from the quad_forest structure into a list. The
 * array has to be allocated by the caller to the correct size, as indicated
 * by the segments member of the quad_forest structure.
 */
result quad_forest_get_segments
(
  /** The quad_forest where the segments will be collected. */
  quad_forest *source,
  /** A segment array, must be allocated by the caller to the correct size. */
  segment **target
);

/**
 * Collects into a list all trees belonging to a(n array of) segment(s).
 */
result quad_forest_get_segment_trees
(
  list *target,
  quad_forest *forest,
  segment **segments,
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
  segment **segments,
  uint32 segment_count
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
  segment **segments,
  uint32 segment_count,
  truth_value invert
);

/**
 * Generates a list of lines that surrounds a segment, rounding the corners.
 */
result quad_forest_get_segment_boundary
(
  quad_forest *forest,
  segment *segment,
  list *boundary
);

/**
* Generates a list of weighted lines corresponding to links between trees
*/
result quad_forest_get_links
(
  quad_forest *forest,
  list *links,
  link_visualization_mode mode
);

/**
* Collects all trees contained in a list of segments and draws them on a
* pixel_image with a red color
*/
result quad_forest_highlight_segments
(
  quad_forest *forest,
  pixel_image *target,
  segment **segments,
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
  /** How many propagation rounds to use for determining devmean and devdev */
  uint32 rounds,
  /** The bias value used for determining devdev threshold for boundary */
  integral_value bias,
  /** The minimum length of edge chains _before_ starting to fill gaps */
  uint32 min_length
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

#ifdef __cplusplus
}
#endif

#endif /* CVSU_QUAD_FOREST_H */
