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


/**
 * Generic tree annotation. Can be edge, segment, intersection.
 */
typedef struct tree_annotation_t {
  /** Actual annotation data */
  typed_pointer data;
} tree_annotation;

typedef struct accumulated_stat_t {
  
} accumulated_stat;

typedef struct quad_forest_intersection_t {
  struct quad_tree_t *tree;
  list edges;
  list chains;
} quad_forest_intersection;


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

int compare_segments(const void *a, const void *b)

#ifdef __cplusplus
}
#endif

#endif /* CVSU_ANNOTATION_H */
