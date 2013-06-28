/**
 * @file cvsu_annotation.h
 * @author Matti J. Eskelinen <matti.j.eskelinen@gmail.com>
 * @brief Annotation structures for image parsing algorithms.
 *
 * Copyright (c) 2013, Matti Johannes Eskelinen
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

#ifndef CVSU_ANNOTATION_H
#   define CVSU_ANNOTATION_H

#ifdef __cplusplus
extern "C" {
#endif

#include "cvsu_config.h"
#include "cvsu_types.h"
#include "cvsu_typed_pointer.h"
#include "cvsu_context.h"
#include "cvsu_list.h"

/******************************************************************************/
/* forward declarations                                                       */
/******************************************************************************/

struct quad_tree_t;
struct quad_tree_link_head_t;

/******************************************************************************/
/* accumulated stat structures and functions                                  */
/******************************************************************************/

typedef struct accumulated_stat_t {
  integral_value meanmean;
  integral_value meandev;
  integral_value devmean;
  integral_value devdev;
  integral_value strength;
} accumulated_stat;

/******************************************************************************/

result accumulated_stat_create
(
  struct quad_tree_t *tree,
  stat_accumulator *acc
);

/******************************************************************************/

result ensure_accumulated_stat
(
  typed_pointer *annotation,
  accumulated_stat **astat
);

/******************************************************************************/

truth_value is_accumulated_stat
(
  typed_pointer *tptr
);

/******************************************************************************/

accumulated_stat *has_accumulated_stat
(
  typed_pointer *tptr
);

/******************************************************************************/

result expect_accumulated_stat
(
  accumulated_stat **astat,
  typed_pointer *tptr
);

/******************************************************************************/
/* neighborhood stat structures and functions                                 */
/******************************************************************************/

typedef struct neighborhood_stat_t {
  integral_value mean_mean;
  integral_value mean_dev;
  integral_value dev_mean;
  integral_value dev_dev;
  integral_value strength;
  integral_value overlap;
  integral_value mean_ridge_score;
  integral_value mean_ledge_score;
  integral_value dev_ridge_score;
  integral_value dev_ledge_score;
  integral_value boundary_score;
  integral_value segment_score;
} neighborhood_stat;

/******************************************************************************/

result ensure_neighborhood_stat
(
  typed_pointer *annotation,
  neighborhood_stat **nstat
);

/******************************************************************************/

truth_value is_neighborhood_stat
(
  typed_pointer *tptr
);

/******************************************************************************/

neighborhood_stat *has_neighborhood_stat
(
  typed_pointer *tptr
);

/******************************************************************************/

result expect_neighborhood_stat
(
  neighborhood_stat **nstat,
  typed_pointer *tptr
);

/******************************************************************************/
/* edge response structures and functions                                     */
/******************************************************************************/

typedef struct edge_response_t
{
  integral_value dx;
  integral_value dy;
  integral_value mag;
  integral_value ang;
  integral_value confidence;
  uint32 hpeaks;
  uint32 vpeaks;
  integral_value peak_score;
} edge_response;

/******************************************************************************/

result ensure_edge_response
(
  typed_pointer *annotation,
  edge_response **eresp
);

/******************************************************************************/

truth_value is_edge_response
(
  typed_pointer *tptr
);

/******************************************************************************/

edge_response *has_edge_response
(
  typed_pointer *tptr,
  uint32 token
);

/******************************************************************************/

result expect_edge_response
(
  edge_response **eresp,
  typed_pointer *tptr
);

/******************************************************************************/
/* smoothed gradient structures and functions                                 */
/******************************************************************************/

typedef struct smoothed_gradient_t
{
  integral_value mag;
  integral_value ang;
  integral_value confidence;
} smoothed_gradient;

/******************************************************************************/

truth_value is_smoothed_gradient
(
  typed_pointer *tptr
);

/******************************************************************************/

smoothed_gradient *has_smoothed_gradient
(
  typed_pointer *tptr,
  uint32 token
);

/******************************************************************************/
/* link measure structures and functions                                      */
/******************************************************************************/

/**
 * Defines categories for link directions relative to edge direction.
 * Links may go in parallel or perpendicular direction relative to edge.
 * Parallel links may go towards or against the edge direction.
 * Perpendicular links may go left or right relative to the edge direction.
 * There is a two-level hierarchy for grouping the categories.
 *
 * Following macros are available for checking the top-level category:
 * IS_PARALLEL, IS_PERPENDICULAR
 */
typedef enum link_category_t {
  bl_UNDEF         = 0x00, /* 00000000 */
  bl_TOWARDS       = 0x11, /* 00010001, parallel */
  bl_AGAINST       = 0x12, /* 00010010, parallel */
  bl_LEFT          = 0x24, /* 00100100, perpendicular */
  bl_RIGHT         = 0x28, /* 00101000, perpendicular */
  bl_PARALLEL      = 0x10, /* 00010000 */
  bl_PERPENDICULAR = 0x20, /* 00100000 */
} link_category;

#define IS_PARALLEL(expr) (((expr) & bl_PARALLEL) != 0)
#define IS_PERPENDICULAR(expr) (((expr) & bl_PERPENDICULAR) != 0)

/******************************************************************************/

typedef struct link_measure_t {
  link_category category;
  integral_value magnitude_score;
  integral_value strength_score;
  integral_value angle_score;
  integral_value against_score;
  integral_value profile_score;
  integral_value parallel_score;
  integral_value perpendicular_score;
} link_measure;

/******************************************************************************/

result ensure_link_measure
(
  typed_pointer *annotation,
  link_measure **lmeasure,
  uint32 token
);

/******************************************************************************/

truth_value is_link_measure
(
  typed_pointer *tptr
);

/******************************************************************************/

link_measure *has_link_measure
(
  typed_pointer *tptr,
  uint32 token
);

/******************************************************************************/

result expect_link_measure
(
  typed_pointer *tptr,
  link_measure **lmeasure,
  uint32 token
);

/******************************************************************************/
/* edge profile structures and functions                                      */
/******************************************************************************/

typedef struct edge_profile_t {
  integral_value mean_left;
  integral_value mean_right;
  integral_value dev_left;
  integral_value dev_right;
  integral_value mean_score;
  integral_value dev_score;
} edge_profile;

/******************************************************************************/

truth_value is_edge_profile
(
  typed_pointer *tptr
);

/******************************************************************************/

edge_profile *has_edge_profile
(
  typed_pointer *tptr,
  uint32 token
);

/******************************************************************************/
/* edge links structures and functions                                        */
/******************************************************************************/

typedef struct edge_links_t {
  struct quad_tree_link_head_t *towards;
  struct quad_tree_link_head_t *against;
  /*struct quad_tree_link_head_t *other;*/
  integral_value own_angle;
  integral_value towards_angle;
  integral_value against_angle;
  integral_value straightness;
  integral_value curvature;
  integral_value own_consistency;
  integral_value towards_consistency;
  integral_value against_consistency;
  integral_value direction_consistency;
} edge_links;

/******************************************************************************/

result ensure_edge_links
(
  typed_pointer *annotation,
  edge_links **elinks,
  uint32 token
);

/******************************************************************************/

truth_value is_edge_links
(
  typed_pointer *tptr
);

/******************************************************************************/

edge_links *has_edge_links
(
  typed_pointer *tptr,
  uint32 token
);

/******************************************************************************/

result expect_edge_links
(
  typed_pointer *tptr,
  edge_links **elinks,
  uint32 token
);

/******************************************************************************/
/* ridge potential structures and functions                                   */
/******************************************************************************/

typedef struct ridge_potential_t {
  uint32 round;
  integral_value ridge_score;
  struct quad_tree_t *better_ridge;
} ridge_potential;

/******************************************************************************/

truth_value is_ridge_potential
(
  typed_pointer *tptr
);

/******************************************************************************/

ridge_potential *has_ridge_potential
(
  typed_pointer *tptr,
  uint32 token
);

/******************************************************************************/
/* boundary potential structures and functions                                */
/******************************************************************************/

typedef struct boundary_potential_t {
  uint32 length;
  integral_value angle;
  integral_value curvature;
  integral_value acc_angle;
  struct boundary_t *parent;
  struct boundary_potential_t *prev;
} boundary_potential;

/******************************************************************************/

result ensure_boundary_potential
(
  typed_pointer *annotation,
  boundary_potential **bpot,
  uint32 token
);

/******************************************************************************/

truth_value is_boundary_potential
(
  typed_pointer *tptr
);

/******************************************************************************/

boundary_potential *has_boundary_potential
(
  typed_pointer *tptr,
  uint32 token
);

/******************************************************************************/
/* boundary message structures and functions                                  */
/******************************************************************************/

typedef struct boundary_message_t {
  uint32 round;
  integral_value pool_curvature;
  integral_value acc_curvature;
  integral_value pool_distance;
  integral_value acc_distance;
  uint32 pool_length;
  uint32 acc_length;
} boundary_message;

/******************************************************************************/

result ensure_boundary_message
(
  typed_pointer *annotation,
  boundary_message **bmsg,
  uint32 token
);

/******************************************************************************/

truth_value is_boundary_message
(
  typed_pointer *tptr
);

/******************************************************************************/

boundary_message *has_boundary_message
(
  typed_pointer *tptr,
  uint32 token
);

/******************************************************************************/
/* segment message structures and functions                                   */
/******************************************************************************/

typedef struct segment_message_t {
  uint32 extent;
  truth_value echo;
  integral_value strength_diff;
} segment_message;

/******************************************************************************/

result ensure_segment_message
(
  typed_pointer *annotation,
  segment_message **smsg,
  uint32 token,
  integral_value strength_diff
);

/******************************************************************************/

truth_value is_segment_message
(
  typed_pointer *tptr
);

/******************************************************************************/

segment_message *has_segment_message
(
  typed_pointer *tptr,
  uint32 token
);

/******************************************************************************/

result expect_segment_message
(
  typed_pointer *tptr,
  segment_message **smsg,
  uint32 token
);

/******************************************************************************/
/* boundary structures and functions                                          */
/******************************************************************************/

/**
 * Enumerates the different categories of boundary fragments; each fragment can
 * be straight or curved, contain a corner (two boundaries meet) or an
 * intersection (multiple boundaries meet), or be a part of a cluttered region.
 */
typedef enum boundary_category_t {
  fc_UNDEF = 0,
  fc_UNKNOWN,
  fc_STRAIGHT,
  fc_CURVED,
  fc_CORNER,
  fc_INTERSECTION,
  fc_CLUTTER
} boundary_category;

/******************************************************************************/

/**
 * Stores boundary fragment information for quad forest parsing. Each fragment
 * is a disjoint set of tree nodes, managed with union-find approach. Boundary
 * fragments are long and thin chains of nodes, that follow edges and boundaries
 * and have uniform curvature.
 */
typedef struct boundary_t {
  /** Parent boundary, that determines the segment id (may be self) */
  struct boundary_t *parent;
  /** Boundary category */
  boundary_category category;
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
  /** Length of the boundary fragment in nodes */
  uint32 length;
  /** Average curvature (change in direction) between nodes */
  integral_value curvature_mean;
  /** Sum of curvatures for calculating the mean */
  integral_value curvature_sum;
  /** Direction in the beginning of the boundary fragment */
  integral_value dir_a;
  /** Direction in the end of the fragment */
  integral_value dir_b;
  /** Color assigned for this segment for visualizing purposes */
  byte color[4];
  /** Parent nodes will collect all hypotheses that this fragment affects */
  list *hypotheses;
} boundary;

/******************************************************************************/

/**
 * Compares boundaries numerically by pointer value.
 */
int compare_boundaries(const void *a, const void *b);

/******************************************************************************/

/**
 * Checks whether the value contained in the typed pointer is a boundary object.
 */
truth_value is_boundary
(
  typed_pointer *input_pointer
);

/******************************************************************************/

/**
 * Checks if the typed pointer contains a boundary object as a tuple element.
 */
boundary *has_boundary
(
  typed_pointer *input_pointer,
  uint32 token
);

/******************************************************************************/

/**
 * Ensures that the tree has a boundary fragment annotation, but does not init.
 * Part of the Union-Find implementation for boundary nodes.
 */
result quad_tree_ensure_boundary
(
  struct quad_tree_t *tree,
  boundary **output_boundary
);

/**
 * Initializing the boundary values needs to be separated from creating the
 * boundary fragment structure, since the final values are not always known at
 * the time of creating the structure. Remember to call this before trying to
 * merge fragments.
 */
void boundary_init
(
  boundary *input_boundary,
  edge_links *elinks
);

/**
 * Ensures that the tree has a boundary fragment annotation, and initializes it.
 */
result quad_tree_boundary_init
(
  struct quad_tree_t *tree,
  boundary **output_boundary,
  edge_links *elinks
);

/******************************************************************************/

/**
 * Creates a union of two boundary fragments.
 * Part of the Union-Find implementation for boundaries.
 */
void boundary_union
(
  boundary *input_boundary_1,
  boundary *input_boundary_2
);

/******************************************************************************/

/**
 * Creates a union of the two boundary fragments these two trees belong to.
 * Part of the Union-Find implementation for boundary nodes.
 */
void quad_tree_boundary_union
(
  struct quad_tree_t *input_tree_1,
  struct quad_tree_t *input_tree_2
);

/******************************************************************************/

/**
 * Finds the parent element of this boundary fragment.
 * Part of the Union-Find implementation for boundaries.
 */
boundary *boundary_find
(
  boundary *input_boundary
);

/******************************************************************************/

/**
 * Finds the parent element in the boundary fragment this tree belongs to.
 * Part of the Union-Find implementation for boundary nodes.
 */
boundary *quad_tree_boundary_find
(
  struct quad_tree_t *input_tree
);

/******************************************************************************/

/**
 * Gets the boundary id for this tree. Effectively the pointer cast into an int.
 * Helper function on top of the Union-Find implementation for boundary nodes.
 */
uint32 quad_tree_boundary_id
(
  struct quad_tree_t *input_tree
);

/******************************************************************************/

/**
 * Checks if this tree is a boundary fragment parent (id == self)
 */
truth_value quad_tree_is_boundary_parent
(
  struct quad_tree_t *input_tree
);

/******************************************************************************/
/* segment structures and functions                                           */
/******************************************************************************/

/**
 * Enumerates the different categories of segments; each segment can be deemed
 * either foreground (belonging to objects of interest), background (not
 * belonging to objects of interest), or clutter (unidentified small details).
 */
typedef enum segment_category_t {
  sc_UNDEF = 0,
  sc_FOREGROUND,
  sc_BACKGROUND,
  sc_CLUTTER
} segment_category;

/******************************************************************************/

/**
 * Stores segment information for quad forest parsing. Each segment is a
 * disjoint set of tree nodes, managed with union-find approach. Segments
 * contain wide, uniform regions, as opposed to boundary fragments.
 */
typedef struct segment_t
{
  /** Parent segment, that determines the segment id (may be self) */
  struct segment_t *parent;
  /** Segment category */
  segment_category category;
  /** Rank value used for optimizing union-find process */
  uint32 rank;
  /** Maximum extent of the segment */
  uint32 extent;
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
  /** Color assigned for this segment for visualizing purposes */
  byte color[4];
  /** Parent nodes will collect all hypotheses that this segment affects */
  list *hypotheses;
} segment;

/******************************************************************************/

/**
 * Compares segments numerically by pointer value.
 */
int compare_segments(const void *a, const void *b);

/******************************************************************************/

/**
 * Checks whether the value contained in the typed pointer is a segment object.
 */
truth_value is_segment
(
  typed_pointer *input_pointer
);

/******************************************************************************/

/**
 * Checks if the typed pointer contains a segment object as a tuple element.
 */
segment *has_segment
(
  typed_pointer *input_pointer,
  uint32 token
);

/******************************************************************************/

/**
 * Ensures that the tree has a segment annotation, and initializes it.
 * Part of the Union-Find implementation for segment nodes.
 */
result quad_tree_ensure_segment
(
  struct quad_tree_t *input_tree,
  segment **output_segment
);

/******************************************************************************/

segment *quad_tree_get_segment
(
  struct quad_tree_t *input_tree
);

/******************************************************************************/

/**
 * Creates a union of two segments.
 * Part of the Union-Find implementation for segments.
 */
void segment_union
(
  segment *input_segment_1,
  segment *input_segment_2
);

/******************************************************************************/

/**
 * Creates a union of the two segments these two trees belong to.
 * Part of the Union-Find implementation for segment nodes.
 */
void quad_tree_segment_union
(
  struct quad_tree_t *input_tree_1,
  struct quad_tree_t *input_tree_2
);

/******************************************************************************/

/**
 * Finds the parent element of this segment.
 * Part of the Union-Find implementation for segments.
 */
segment *segment_find
(
  segment *input_segment
);

/******************************************************************************/

/**
 * Finds the parent element in the segment this tree belongs to.
 * Part of the Union-Find implementation for segment nodes.
 */
segment *quad_tree_segment_find
(
  struct quad_tree_t *input_tree
);

/******************************************************************************/

/**
 * Gets the segment id for this tree. Effectively the pointer cast into an int.
 * Helper function on top of the Union-Find implementation for segment nodes.
 */
uint32 quad_tree_segment_id
(
  struct quad_tree_t *input_tree
);

/******************************************************************************/

/**
 * Checks if this tree is a segment parent (id == self)
 */
truth_value quad_tree_is_segment_parent
(
  struct quad_tree_t *input_tree
);

/******************************************************************************/
/* object structures and functions                                            */
/******************************************************************************/

/**
 * Represents a hypothesis, that a given object is found in the given region.
 * This hypothesis requires support from the surrounding perceptual entities.
 */
typedef struct object_hypothesis_t {
  uint32 class_id;
  uncertain_rect extent;
  integral_value potential;
} object_hypothesis;

/******************************************************************************/

/**
 * Represents the support that a given perceptual entity bestows on a particular
 * object hypothesis.
 */
typedef struct hypothesis_support_t {
  object_hypothesis *hypothesis;
  integral_value support;
} hypothesis_support;

#ifdef __cplusplus
}
#endif

#endif /* CVSU_ANNOTATION_H */
