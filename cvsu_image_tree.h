/**
 * @file cvsu_quad_forest.h
 * @author Matti J. Eskelinen <matti.j.eskelinen@gmail.com>
 * @brief Quad Forest hierarchical data structure for analyzing images.
 *
 * Copyright (c) 2011, Matti Johannes Eskelinen
 * All Rights Reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * Neither the name of the copyright holder nor the
 *     names of its contributors may be used to endorse or promote products
 *     derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL COPYRIGHT HOLDER BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
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

#include "cvsu_basic.h"
#include "cvsu_integral.h"
#include "cvsu_edges.h"
#include "cvsu_list.h"

/**
 * Stores region information for forest segmentation with union-find
 * equivalence class approach. In addition to id and rank information contains
 * also the region bounding box and statistics.
 */
typedef struct forest_region_info_t
{
  struct forest_region_info_t *id;
  uint32 rank;
  uint32 x1;
  uint32 y1;
  uint32 x2;
  uint32 y2;
  statistics stat;
  byte color[4];
} forest_region_info;

/**
 * Stores a quad tree holding image data.
 */
typedef struct quad_tree_t {
  /** The x coordinate of the top left corner. */
  uint32 x;
  /** The y coordinate of the top left corner. */
  uint32 y;
  /** Width of the image region covered by this tree. */
  uint32 w;
  /** Height of the image region covered by this tree. */
  uint32 h;
  /** Level of this tree in the hierarchy. Top level is 0. */
  uint32 level;
  /** Statistics of the image region covered by this tree. */
  statistics stat;
  /** Region info used in segmentation. */
  forest_region_info region_info;
  /** Parent tree, NULL if this is a root tree */
  struct quad_tree_t *parent;
  /* child trees, all NULL if the tree has not beed divided */
  /** Upper left child tree */
  struct image_tree_t *nw;
  /** Upper right child tree */
  struct image_tree_t *ne;
  /** Lower left child tree */
  struct image_tree_t *sw;
  /** Lower right child tree */
  struct image_tree_t *se;
  /* direct neighbors are cached in each tree as they are determined */
  /** Direct neighbor on the top side. */
  struct image_tree_t *n;
  /** Direct neighbor on the right side. */
  struct image_tree_t *e;
  /** Direct neighbor on the bottom side. */
  struct image_tree_t *s;
  /** Direct neighbor on the left side. */
  struct image_tree_t *w;
} quad_tree;

/**
 * Stores a forest of image trees.
 */
typedef struct quad_forest_t {
  /** The original image from where the source data is copied. */
  pixel_image *original;
  /** The source data used for updating the integral image. */
  pixel_image *source;
  /** The integral image used for calculating tree statistics. */
  integral_image integral;
  /** The number of rows in the tree grid. */
  uint32 rows;
  /** The number of cols in the tree grid. */
  uint32 cols;
  /** The number of regions found from the tree (after segmentation). */
  uint32 regions;
  /** The maximum size of trees (the size of root trees). */
  uint32 tree_max_size;
  /** The minimum size of trees, no tree will be divided beyond this size. */
  uint32 tree_min_size;
  /** Horizontal offset of the grid from the origin of the source image. */
  uint32 dx;
  /** Vertical offset of the grid from the origin of the source image. */
  uint32 dy;
  /** List of all trees in the forest. */
  list trees;
  /** Pointer to the end of the root tree list for resetting the forest. */
  list_item *last_root_tree;
  /** The array containing the root tree grid. */
  quad_tree *roots;
} quad_forest;

/**
 * Initializes the contents of a quad_tree with null values.
 */
result quad_tree_nullify(
  /** The quad_tree structure to be nullified. */
  quad_tree *target
);

/**
 * Everything that can be nullified should be able to tell if it's null.
 */
bool quad_tree_is_null(
  /** The quad_tree structure to be checked for null. */
  quad_tree *target
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
void quad_forest_free(
  /** A pointer to the quad_forest to be freed. */
  quad_forest *target
);

/**
 * Creates a quad_forest from a pixel_image.
 */
result quad_forest_create(
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
result quad_forest_reload(
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
result quad_forest_destroy(
  /** The quad_forest structure to be destroyed. */
  quad_forest *target
);

/**
 * Initializes the contents of a quad_forest to null. Does not deallocate data.
 */
result quad_forest_nullify(
  /** The quad_forest structure to be nullified. */
  quad_forest *target
);

/**
* Everything that can be nullified should be able to tell if it's null.
*/
bool quad_forest_is_null(
  /** The quad_forest structure to be checked for null. */
  quad_forest *target
);

/**
 * Prepares an image forest for the update stage.
 * This is separate from the actual update to allow easier parallelization.
 * All root trees are separate from each other, so arbitrary tree collections
 * can be handled in parallel.
 * In this stage new image content is copied from the source
 * (possibly converting the format) and data structures cleaned for a new
 * iteration.
 */
/* TODO: what to do with this? */
result image_tree_forest_update_prepare(
  image_tree_forest *target
);

/**
 * Updates a quad_forest structure; copies data from the original image,
 * possibly converting the format, cleans the data structure, updates the
 * integral_image and updates the statistics of all trees.
 */
result quad_forest_update(
  /** The quad_forest structure to be updated. */
  quad_forest *target
);

/**
 * Segments the quad_forest structure using a deviation threshold as
 * consistency and similarity criteria. Divides all trees that have deviation
 * larger than the threshold, and then merges trees and regions that have the
 * difference of means smaller than the threshold.
 */
result quad_forest_segment_with_deviation(
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
result quad_forest_segment_with_entropy(
  /** The quad_forest structure to be segmented. */
  image_tree_forest *target,
  /** Deviation multiplier used for creating the estimated intensity range. */
  integral_value alpha,
  /** Range overlap ratio used for determining the trees to merge. */
  integral_value overlap_trees,
  /** Range overlap ratio used for determining the regions to merge. */
  integral_value overlap_regions
);

/**
 * Collects all region parents from the quad_forest structure into a list. The
 * array has to be allocated by the caller to the correct size, as indicated
 * by the regions member of the quad_tree structure.
 */
result quad_forest_get_regions(
  /** The quad_forest where the regions will be collected. */
  quad_forest *source,
  /** A region array, must be allocated by the caller to the correct size. */
  forest_region_info **target
);

/**
 * Draws an image of the quad_forest structure using the current division and
 * region info. Each quad_tree will be painted as a square with uniform color,
 * using the color assigned to the region id, the mean value from the region
 * statistics, or the mean value from the quad_tree statistics.
 */
result quad_forest_draw_image
(
  /** The quad_forest to be drawn into an image. */
  image_tree_forest *forest,
  /** Pointer to a pixel_image, will be (re)created to fit the forest image. */
  pixel_image *target,
  /** Should we use region statistics or individual tree statistics? */
  uint32 use_regions,
  /** For regions, should we use mean or colors? No effect for trees. */
  uint32 use_colors
);

/**
 * Divides a quad_tree in four smaller trees, if width is greater than 1.
 */
result quad_tree_divide(
  quad_tree *target
);

/**
 * Determine whether the quad_tree has child trees.
 */
bool quad_tree_has_children(
  quad_tree *target
);

/**
 * Generates the statistics of the four child trees without dividing the tree.
 * Useful for determining consistency before deciding to divide.
 */
result quad_tree_get_child_statistics(
  /** The quad_tree that will will be examined. */
  quad_tree *source,
  /** The array of statistics structures to fill, must contain at least 4. */
  statistics *target,
  /** The quad_tree structures to fill, can be null, if not must contain 4. */
  quad_tree *children
);

/**
 * Calculates the child tree statistics but divides the tree only if the
 * entropy measure of the children is higher than the given threshold.
 * Higher values will require higher overlap to divide.
 */
result quad_tree_divide_with_entropy(
  /** The quad_tree to be divided in case its entropy is sufficiently high. */
  quad_tree *target,
  /** The threshold value used to decide whether to divide or not. */
  integral_value overlap_threshold
);

/**
 * Creates and initializes a neighbor list
 */
result image_tree_create_neighbor_list(
    list *target
);

/**
 * Finds the direct neighbor (directly adjacent on the same level) of a tree
 * and returns it in the neighbor reference. If the tree does not have a
 * neighbor on the same level, the direct neighbor on the highest level is
 * returned.
 *
 * 1. Check if the direct neighbor has already been cached.
 * 2. If not, go to parent and check if the neighbor is in the same parent.
 * 3. If not, recursively get the neighbor of the parent.
 * 4. Check if the neighbor of the parent has children; if yes, choose the
 *    correct one as neighbor, if not, return the parent as neighbor
 * 5. It is possible that there is no neighbor (tree is on the edge); in this
 *    case, the neighbor is set to NULL
 *
 * This requires that the neighbors of top level trees have been set properly.
 */

/**
 * Recursive function for adding child trees from the highest level as
 * immediate neighbors to another tree
 *
 * 1. If tree has no childen, add it to list and return
 * 2. If tree has chilren, call recursively for the two children in the proper
 *    direction
 */
result image_tree_add_children_as_immediate_neighbors(
    list *target,
    image_tree *tree,
    direction dir
);

/**
 * Finds all immediate neighbors (directly adjacent neighbors on the highest
 * level) of a tree in the given direction and stores them in the list.
 *
 * 1. Find the direct neighbor; if it has children, call recursively for the
 *    two children adjacent to this tree
 *
 * 2. Peek the item at the top of stack; if it has children, pop it and push the
 *    two children adjacent to this tree into the stack; if not, add it to the
 *    end of the list
 * 3.
 */
result image_tree_find_all_immediate_neighbors(
  list *target,
  image_tree *tree
);

/**
 * Creates a segment from this quad_tree.
 * Part of the Union-Find implementation for quad_trees.
 */
void quad_tree_segment_create(quad_tree *tree);

/**
 * Creates a union of the two segments these two trees belong to.
 * Part of the Union-Find implementation for quad_trees.
 */
void quad_tree_segment_union(quad_tree *tree1, quad_tree *tree2);

/**
 * Finds the parent element in the segment this tree belongs to.
 * Part of the Union-Find implementation for quad_trees.
 */
forest_region_info *quad_tree_segment_find(forest_region_info *region);

/**
 * Gets the class label for this tree. Effectively the pointer cast into int.
 * Helper function on top of the Union-Find implementation for image trees.
 */
uint32 quad_tree_segment_get(quad_tree *tree);

/**
 * Checks if this image is a class parent (id == region_info)
 */
bool quad_tree_is_segment_parent(quad_tree *tree);

#ifdef __cplusplus
}
#endif

#endif /* IMAGE_TREE_H */
