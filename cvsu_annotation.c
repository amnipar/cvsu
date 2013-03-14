/**
 * @file cvsu_annotation.c
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

#include "cvsu_annotation.h"

/******************************************************************************/

void quad_tree_segment_create
(
  quad_tree *tree
)
{
  quad_forest_segment *segment;
  if (tree != NULL) {
    segment = &tree->segment;
    /* proceed only if the tree doesn't have its region info initialized yet */
    if (segment->parent == NULL) {
      /* one-tree segment is it's own parent, and has the rank of 0 */
      segment->parent = segment;
      segment->rank = 0;
      segment->x1 = tree->x;
      segment->y1 = tree->y;
      segment->x2 = tree->x + tree->size - 1;
      segment->y2 = tree->y + tree->size - 1;
      memory_copy((data_pointer)&segment->stat, (data_pointer)&tree->stat, 1, sizeof(statistics));
    }
  }
}

/******************************************************************************/

void quad_tree_segment_union
(
  quad_tree *tree1,
  quad_tree *tree2
)
{
  quad_forest_segment_union(
      quad_tree_segment_find(tree1), quad_tree_segment_find(tree2));
}

/******************************************************************************/

void quad_forest_segment_union
(
  quad_forest_segment *segment1,
  quad_forest_segment *segment2
)
{
  /* if the segments are already in the same class, no need for union */
  if (segment1 != NULL && segment2 != NULL && segment1 != segment2) {
    /* otherwise set the tree with higher class rank as id of the union */
    statistics *stat;
    integral_value N, mean, variance;
    if (segment1->rank < segment2->rank) {
      segment1->parent = segment2;
      segment2->x1 = (segment1->x1 < segment2->x1) ? segment1->x1 : segment2->x1;
      segment2->y1 = (segment1->y1 < segment2->y1) ? segment1->y1 : segment2->y1;
      segment2->x2 = (segment1->x2 > segment2->x2) ? segment1->x2 : segment2->x2;
      segment2->y2 = (segment1->y2 > segment2->y2) ? segment1->y2 : segment2->y2;
      stat = &segment2->stat;
      N = (stat->N += segment1->stat.N);
      stat->sum += segment1->stat.sum;
      stat->sum2 += segment1->stat.sum2;
      mean = stat->mean = stat->sum / N;
      variance = stat->sum2 / N - mean*mean;
      if (variance < 0) variance = 0;
      stat->variance = variance;
      stat->deviation = sqrt(variance);
    }
    else {
      segment2->parent = segment1;
      /* when equal rank trees are combined, the root tree's rank is increased */
      if (segment1->rank == segment2->rank) {
        segment1->rank += 1;
      }
      segment1->x1 = (segment1->x1 < segment2->x1) ? segment1->x1 : segment2->x1;
      segment1->y1 = (segment1->y1 < segment2->y1) ? segment1->y1 : segment2->y1;
      segment1->x2 = (segment1->x2 > segment2->x2) ? segment1->x2 : segment2->x2;
      segment1->y2 = (segment1->y2 > segment2->y2) ? segment1->y2 : segment2->y2;
      stat = &segment1->stat;
      N = (stat->N += segment2->stat.N);
      stat->sum += segment2->stat.sum;
      stat->sum2 += segment2->stat.sum2;
      mean = stat->mean = stat->sum / N;
      variance = stat->sum2 / N - mean*mean;
      if (variance < 0) variance = 0;
      stat->variance = variance;
      stat->deviation = sqrt(variance);
    }
  }
}

/******************************************************************************/
/* a private function that takes a segment instead of a tree                  */
/* allows using quad_tree in the public interface                             */

quad_forest_segment *segment_find
(
  quad_forest_segment *segment
)
{
  if (segment != NULL) {
    if (segment->parent != NULL && segment->parent != segment) {
      segment->parent = segment_find(segment->parent);
    }
    return segment->parent;
  }
  return NULL;
}

/******************************************************************************/

quad_forest_segment *quad_tree_segment_find
(
  quad_tree *tree
)
{
  if (tree != NULL) {
    return segment_find(&tree->segment);
  }
  return NULL;
}

/******************************************************************************/

uint32 quad_tree_segment_get
(
  quad_tree *tree
)
{
  return (uint32)quad_tree_segment_find(tree);
}

/******************************************************************************/

truth_value quad_tree_is_segment_parent
(
  quad_tree *tree
)
{
  if (tree != NULL) {
    if (quad_tree_segment_find(tree) == &tree->segment) {
      return TRUE;
    }
  }
  return FALSE;
}

/******************************************************************************/
/* comparator function for quad_forest_segments                               */

int compare_segments(const void *a, const void *b)
{
  const quad_forest_segment *sa, *sb;

  sa = *((const quad_forest_segment* const *)a);
  if (sa == NULL) {
    printf("warning: tree is null in compare_segments\n");
    return -1;
  }
  sb = *((const quad_forest_segment* const *)b);

  if (sb == NULL) {
    printf("warning: tree is null in compare_segments\n");
    return -1;
  }

  if (sa > sb) return 1;
  else if (sa < sb) return -1;
  else return 0;
}

/* end of file                                                                */
/******************************************************************************/
