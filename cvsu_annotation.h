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

/**
 * Generic tree annotation. Can be edge, segment, intersection.
 */
typedef struct tree_annotation_t {
  /** Actual annotation data */
  typed_pointer data;
} tree_annotation;

struct quad_tree_t;

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

result annotation_ensure_accumulated_stat
(
  tree_annotation *annotation,
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
  integral_value mean_dist;
  integral_value dev_mean;
  integral_value dev_dev;
  integral_value dev_dist;
  integral_value strength;
  integral_value overlap;
} neighborhood_stat;

result annotation_ensure_neighborhood_stat
(
  tree_annotation *annotation,
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

typedef struct accumulated_reg_t {
  integral_value mdist_mean;
  integral_value mdist_max;
  integral_value sdist_mean;
  integral_value sdist_max;
  integral_value boundary_strength;
  integral_value segment_strength;
  integral_value spread_strength;
} accumulated_reg;

result annotation_ensure_accumulated_reg
(
  tree_annotation *annotation,
  accumulated_reg **areg
);

truth_value is_accumulated_reg
(
  typed_pointer *tptr
);

accumulated_reg *has_accumulated_reg
(
  typed_pointer *tptr
);

/******************************************************************************/

typedef struct edge_response_t
{
  integral_value dx;
  integral_value dy;
  integral_value mag;
  integral_value ang;
} edge_response;

result annotation_ensure_edge_response
(
  tree_annotation *annotation,
  edge_response **eresp
);

truth_value is_edge_response
(
  typed_pointer *tptr
);

edge_response *has_edge_response
(
  typed_pointer *tptr
);

/******************************************************************************/

typedef struct quad_forest_intersection_t {
  struct quad_tree_t *tree;
  list edges;
  list chains;
} quad_forest_intersection;

/******************************************************************************/

/* forward declarations */

struct quad_tree_t;
struct quad_forest_edge_chain_t;

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
