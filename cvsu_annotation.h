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

/* forward declarations */

struct quad_tree_t;
struct quad_tree_link_head_t;
struct quad_forest_edge_chain_t;

/******************************************************************************/

typedef struct accumulated_stat_t {
  integral_value meanmean;
  integral_value meandev;
  integral_value devmean;
  integral_value devdev;
  integral_value strength;
} accumulated_stat;

result accumulated_stat_create
(
  struct quad_tree_t *tree,
  stat_accumulator *acc
);

result ensure_accumulated_stat
(
  typed_pointer *annotation,
  accumulated_stat **astat
);

truth_value is_accumulated_stat
(
  typed_pointer *tptr
);

accumulated_stat *has_accumulated_stat
(
  typed_pointer *tptr
);

result expect_accumulated_stat
(
  accumulated_stat **astat,
  typed_pointer *tptr
);

/******************************************************************************/

typedef struct neighborhood_stat_t {
  integral_value mean_mean;
  integral_value mean_dev;
  integral_value dev_mean;
  integral_value dev_dev;
  integral_value strength;
  integral_value strength_score;
  integral_value ridge_score;
  integral_value overlap;
} neighborhood_stat;

result ensure_neighborhood_stat
(
  typed_pointer *annotation,
  neighborhood_stat **nstat
);

truth_value is_neighborhood_stat
(
  typed_pointer *tptr
);

neighborhood_stat *has_neighborhood_stat
(
  typed_pointer *tptr
);

result expect_neighborhood_stat
(
  neighborhood_stat **nstat,
  typed_pointer *tptr
);

/******************************************************************************/

typedef struct ridge_potential_t {
  uint32 round;
  integral_value ridge_score;
  struct quad_tree_t *better_ridge;
} ridge_potential;

truth_value is_ridge_potential
(
  typed_pointer *tptr
);

ridge_potential *has_ridge_potential
(
  typed_pointer *tptr,
  uint32 token
);

/******************************************************************************/

typedef struct segment_message_t {
  uint32 round;
  uint32 extent;
  integral_value strength_diff;
} segment_message;

result ensure_segment_message
(
  typed_pointer *annotation,
  segment_message **smsg,
  uint32 token,
  integral_value strength_diff
);

truth_value is_segment_message
(
  typed_pointer *tptr
);

segment_message *has_segment_message
(
  typed_pointer *tptr,
  uint32 token
);

typedef struct segment_potential_t {
  uint32 round;
  uint32 extent;
  integral_value diff_score;
  /*integral_value overlap_score;*/
} segment_potential;

result ensure_segment_potential
(
  typed_pointer *annotation,
  segment_potential **spot,
  uint32 token
);

truth_value is_segment_potential
(
  typed_pointer *tptr
);

segment_potential *has_segment_potential
(
  typed_pointer *tptr,
  uint32 token
);

result expect_segment_potential
(
  typed_pointer *tptr,
  segment_potential **spot,
  uint32 token
);

/******************************************************************************/

typedef enum link_category_t {
  bl_UNDEF = 0,
  bl_TOWARDS,
  bl_AGAINST,
  bl_LEFT,
  bl_RIGHT,
  bl_PARALLEL,
  bl_PERPENDICULAR
} link_category;

typedef struct link_measure_t {
  link_category category;
  integral_value strength_score;
  /*integral_value magnitude_score;*/
  integral_value angle_score;
  integral_value straightness_score;
  /*integral_value profile_score;*/
} link_measure;

result ensure_link_measure
(
  typed_pointer *annotation,
  link_measure **lmeasure,
  uint32 token
);

truth_value is_link_measure
(
  typed_pointer *tptr
);

link_measure *has_link_measure
(
  typed_pointer *tptr,
  uint32 token
);

result expect_link_measure
(
  typed_pointer *tptr,
  link_measure **lmeasure,
  uint32 token
);

/******************************************************************************/

typedef struct edge_profile_t {
  integral_value mean_left;
  integral_value mean_right;
  integral_value dev_left;
  integral_value dev_right;
  integral_value mean_score;
  integral_value dev_score;
} edge_profile;

truth_value is_edge_profile
(
  typed_pointer *tptr
);

edge_profile *has_edge_profile
(
  typed_pointer *tptr,
  uint32 token
);

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

result ensure_edge_links
(
  typed_pointer *annotation,
  edge_links **elinks,
  uint32 token
);

truth_value is_edge_links
(
  typed_pointer *tptr
);

edge_links *has_edge_links
(
  typed_pointer *tptr,
  uint32 token
);

result expect_edge_links
(
  typed_pointer *tptr,
  edge_links **elinks,
  uint32 token
);

/******************************************************************************/

typedef struct edge_response_t
{
  integral_value dx;
  integral_value dy;
  integral_value mag;
  integral_value ang;
  integral_value confidence;
} edge_response;

result ensure_edge_response
(
  typed_pointer *annotation,
  edge_response **eresp
);

truth_value is_edge_response
(
  typed_pointer *tptr
);

edge_response *has_edge_response
(
  typed_pointer *tptr,
  uint32 token
);

/******************************************************************************/

typedef struct smoothed_gradient_t
{
  integral_value mag;
  integral_value ang;
  integral_value confidence;
} smoothed_gradient;

truth_value is_smoothed_gradient
(
  typed_pointer *tptr
);

smoothed_gradient *has_smoothed_gradient
(
  typed_pointer *tptr,
  uint32 token
);

/******************************************************************************/

typedef struct boundary_potential_t {
  uint32 round;
  uint32 length;
  integral_value strength_score;
  integral_value angle_score;
  integral_value straightness_score;
  /*integral_value profile_score;*/
} boundary_potential;

result ensure_boundary_potential
(
  typed_pointer *annotation,
  boundary_potential **bpot,
  uint32 token
);

truth_value is_boundary_potential
(
  typed_pointer *tptr
);

boundary_potential *has_boundary_potential
(
  typed_pointer *tptr,
  uint32 token
);

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

result ensure_boundary_message
(
  typed_pointer *annotation,
  boundary_message **bmsg,
  uint32 token
);

truth_value is_boundary_message
(
  typed_pointer *tptr
);

boundary_message *has_boundary_message
(
  typed_pointer *tptr,
  uint32 token
);

/******************************************************************************/

typedef enum fragment_type_t {
  ft_UNDEF = 0,
  ft_STRAIGHT,
  ft_CURVED,
  ft_CORNER,
  ft_INTERSECTION
} fragment_type;

typedef struct boundary_fragment_t {
  struct boundary_fragment_t *parent;
  fragment_type type;
  rect extent;
  uint32 round;
  /** Average change in direction between nodes (later curvature?) */
  integral_value dir_change;
  /** Direction in the beginning of the fragment */
  integral_value dir_a;
  /** Direction in the end of the fragment */
  integral_value dir_b;
  /** Parent nodes will collect all hypotheses that this fragment affects */
  list *hypotheses;
} boundary_fragment;

result ensure_boundary_fragment
(
  typed_pointer *annotation,
  boundary_fragment **bfrag,
  uint32 token
);

truth_value is_boundary_fragment
(
  typed_pointer *tptr
);

boundary_fragment *has_boundary_fragment
(
  typed_pointer *tptr,
  uint32 token
);

void boundary_fragment_create
(
  struct quad_tree_t *tree
);

void boundary_fragment_union
(
  struct quad_tree_t *tree1,
  struct quad_tree_t *tree2
);

boundary_fragment *boundary_fragment_find
(
  struct quad_tree_t *tree
);

/******************************************************************************/

typedef struct object_hypothesis_t {
  uint32 class_id;
  uncertain_rect extent;
  integral_value potential;
} object_hypothesis;

typedef struct hypothesis_support_t {
  object_hypothesis *hypothesis;
  integral_value support;
} hypothesis_support;

/******************************************************************************/

typedef struct quad_forest_intersection_t {
  struct quad_tree_t *tree;
  list edges;
  list chains;
} quad_forest_intersection;

/******************************************************************************/

/**
 * Stores edge information for edge and edge chain detection in quad_forest with
 * union-find disjoint set approach. In addition to id and rank information
 * contains also edge response values and local variation information.
 * TODO: Adding also edge chain links.
 */
typedef struct quad_forest_edge_t
{
  struct quad_forest_edge_chain_t *chain;
  /** The parent edge, that determines the edge segment id */
  struct quad_forest_edge_t *parent;
  /** The previous edge in the edge chain */
  struct quad_forest_edge_t *prev;
  /** The next edge in the edge chain */
  struct quad_forest_edge_t *next;
  /** TODO: temp - later will be tree pointer after establishing annotation struct */
  struct quad_tree_t *tree;
  /** length of the edge chain - initially 1 */
  uint32 length;
  /** The rank value used for optimizing union-find process */
  uint32 rank;
  /** Edge strength used in forming edge chains */
  integral_value strength;
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
  /** Temporary flag to annotate this edge node as a potential corner/intersection */
  truth_value is_intersection;
  /** Direction in which edge was determined (H,V,N4) */
  direction dir;
  uint32 token;
} quad_forest_edge;

/**
 * Stores an edge chain by its endpoints. Also stores the edge chain length.
 */
typedef struct quad_forest_edge_chain_t
{
  /** Parent node of the edge chain */
  quad_forest_edge *parent;
  /** First node of the edge chain */
  quad_forest_edge *first;
  /** Last node of the edge chain */
  quad_forest_edge *last;
  /** Chain length in nodes */
  uint32 length;
  /** Total cost of traversing the chain (up-down movement in cost space) */
  integral_value cost;
  uint32 token;
} quad_forest_edge_chain;

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

int compare_segments(const void *a, const void *b);

/**
 * Creates a new segment from this quad_tree.
 * Part of the Union-Find implementation for quad_trees.
 */
void quad_tree_segment_create
(
  struct quad_tree_t *tree
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
  struct quad_tree_t *tree1,
  struct quad_tree_t *tree2
);

/**
 * Finds the parent element in the segment this tree belongs to.
 * Part of the Union-Find implementation for quad_trees.
 */
quad_forest_segment *quad_tree_segment_find
(
  struct quad_tree_t *tree
);

/**
 * Gets the segment id for this quad_tree. Effectively the pointer cast into an
 * int. Helper function on top of the Union-Find implementation for quad_trees.
 */
uint32 quad_tree_segment_get
(
  struct quad_tree_t *tree
);

/**
 * Checks if this quad_tree is a segment parent (id == segment_info)
 */
truth_value quad_tree_is_segment_parent
(
  struct quad_tree_t *tree
);

#ifdef __cplusplus
}
#endif

#endif /* CVSU_ANNOTATION_H */
