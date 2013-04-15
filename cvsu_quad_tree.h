/**
 * @file cvsu_quad_tree.h
 * @author Matti J. Eskelinen <matti.j.eskelinen@gmail.com>
 * @brief Quad Tree structure for use in quad forests.
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

#ifndef CVSU_QUAD_TREE_H
#   define CVSU_QUAD_TREE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "cvsu_config.h"
#include "cvsu_types.h"
#include "cvsu_list.h"
#include "cvsu_context.h"
#include "cvsu_annotation.h"

/* forward declaration */
struct quad_tree_link_t;

/**
 * Describes one head of a link between two quad trees. Each head has a link
 * to the other head for accessing that head's values (read-only).
 */
typedef struct quad_tree_link_head_t {
  struct quad_tree_link_t *link;
  struct quad_tree_link_head_t *opposite;
  const struct quad_tree_link_head_t *other;
  struct quad_tree_t *tree;
  /** Angle of the line going away from this head */
  integral_value angle;
  /** Annotation storing various calculated values for this link head */
  typed_pointer annotation;
} quad_tree_link_head;

/**
 * Describes a link between two quad trees. Both heads of the link have their
 * own separate structure and context that they can update on their own. Each
 * link between two trees should occur only once, meaning that the two trees
 * should share the same structure and each store a pointer to the same link.
 */
typedef struct quad_tree_link_t {
  /** Head A of the link */
  quad_tree_link_head a;
  /** Head B of the link */
  quad_tree_link_head b;
  /** Link category */
  direction category;
  /** Distance between the heads */
  integral_value distance;
  /** Annotation storing various calculated values for this link */
  typed_pointer annotation;
} quad_tree_link;

void quad_tree_link_destroy
(
  quad_tree_link *target
);

typedef enum link_visualization_mode_t {
  v_LINK_NONE,
  v_LINK_DISTANCE,
  v_LINK_ANGLE_COST,
  v_LINK_SIMILARITY,
  v_LINK_MEASURE,
  v_LINK_STRENGTH,
  v_LINK_STRAIGHTNESS
} link_visualization_mode;

/* forward declaration */
struct quad_forest_t;

void quad_tree_ensure_neighborhood_stat();

void quad_tree_link_ensure_booundary_strength
(
  struct quad_forest_t *forest,
  quad_tree_link *link,
  boundary_strength **bstrength
);

void quad_tree_ensure_boundary_strength();
void quad_tree_ensure_segment_strength();

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
  /** List of links to neighboring and nearby nodes */
  list links;
  /** Context data used in image parsing operations */
  typed_pointer context;
  typed_pointer annotation;

  /* TODO: to be removed */
  quad_forest_segment segment;
} quad_tree;

void quad_tree_destroy
(
  quad_tree *target
);

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
  struct quad_forest_t *forest,
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
  struct quad_forest_t *forest,
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
  struct quad_forest_t *forest,
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
  struct quad_forest_t *forest,
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
  struct quad_forest_t *forest,
  /** The quad_tree where the edge response is calculated. */
  quad_tree *target,
  /** Pointer where the horizontal scanning result is stored. */
  integral_value *dx,
  /** Pointer where the vertical scanning result is stored. */
  integral_value *dy
);

/**
 * Ensures that a tree has an edge response annotation, when that is needed.
 */
result quad_tree_ensure_edge_response
(
  struct quad_forest_t *forest,
  quad_tree *tree,
  edge_response **eresp
);

/**
 * Generates the edge responses of the four child trees without dividing the
 * tree. Useful for determining consistency before deciding to divide.
 */
result quad_tree_get_child_edge_response
(
  /** The quad_forest where the tree resides. */
  struct quad_forest_t *forest,
  /** The quad_tree that will will be examined. */
  quad_tree *source,
  /** Array where the horizontal scanning results are stored: must fit 4. */
  integral_value dx[],
  /** Array where the vertical scanning results are stored: must fit 4. */
  integral_value dy[]
);

/**
 * Generates a line corresponding to edge response direction (if edge response
 * is found) and appends it to the list.
 */
result quad_tree_edge_response_to_line
(
  quad_tree *tree,
  list *lines
);

/**
 * Generates a line corresponding to edge response gradient direction (if edge
 * response is found) and appends it to the list.
 */
result quad_tree_gradient_to_line
(
  quad_tree *tree,
  list *lines
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
 * Comparison function for links.
 */
truth_value quad_tree_link_equals
(
  const void *a,
  const void *b
);

result quad_tree_find_link
(
  quad_tree *tree1,
  quad_tree *tree2,
  quad_tree_link_head **link
);

#ifdef __cplusplus
}
#endif

#endif /* CVSU_QUAD_TREE_H */
