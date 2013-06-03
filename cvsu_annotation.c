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

#include "cvsu_macros.h"
#include "cvsu_output.h"
#include "cvsu_memory.h"
#include "cvsu_annotation.h"
#include "cvsu_typed_pointer.h"
#include "cvsu_quad_tree.h"

/******************************************************************************/
/* constants for reporting function names in error messages                   */

string accumulated_stat_create_name = "accumulated_stat_create";
string ensure_accumulated_stat_name = "annotation_ensure_accumulated_stat";
string expect_accumulated_stat_name = "expect_accumulated_stat";
string ensure_neighborhood_stat_name = "annotation_ensure_neighborhood_stat";
string expect_neighborhood_stat_name = "expect_neighborhood_stat";
string ensure_accumulated_reg_name = "annotation_ensure_accumulated_reg";
string ensure_edge_response_name = "annotation_ensure_edge_response";
string ensure_boundary_potential_name = "ensure_boundary_potential";
string ensure_boundary_message_name = "ensure_boundary_message";
string ensure_boundary_fragment_name = "ensure_boundary_fragment";
string ensure_segment_message_name = "ensure_segment_message";
string ensure_segment_potential_name = "ensure_segment_potential";

/******************************************************************************/

result accumulated_stat_create
(
  quad_tree *tree,
  stat_accumulator *acc
)
{
  TRY();
  accumulated_stat *astat;

  CHECK_POINTER(tree);
  CHECK_POINTER(acc);

  CHECK(ensure_accumulated_stat(&tree->annotation, &astat));

  astat->meanmean = 0;
  astat->meandev = 0;
  astat->devmean = 0;
  astat->devdev = 0;

  FINALLY(accumulated_stat_create);
  RETURN();
}

/******************************************************************************/

result ensure_accumulated_stat
(
  typed_pointer *annotation,
  accumulated_stat **astat
)
{
  TRY();
  typed_pointer *tptr;

  CHECK_POINTER(astat);
  *astat = NULL;

  CHECK(ensure_has(annotation, t_ASTAT, &tptr));

  *astat = (accumulated_stat*)tptr->value;

  FINALLY(ensure_accumulated_stat);
  RETURN();
}

/******************************************************************************/

truth_value is_accumulated_stat
(
  typed_pointer *tptr
)
{
  if (tptr != NULL && tptr->type == t_ASTAT) {
    return TRUE;
  }
  return FALSE;
}

/******************************************************************************/

accumulated_stat *has_accumulated_stat
(
  typed_pointer *tptr
)
{
  if (IS_TRUE(is_accumulated_stat(tptr))) {
    return (accumulated_stat*)tptr->value;
  }
  if (IS_TRUE(is_tuple(tptr))) {
    typed_pointer *element;
    element = tuple_has_type(tptr, t_ASTAT);
    if (element != NULL) {
      if (IS_FALSE(is_accumulated_stat(element))) {
        return NULL;
      }
      return (accumulated_stat*)element->value;
    }
  }
  return NULL;
}

/******************************************************************************/

result expect_accumulated_stat
(
  accumulated_stat **astat,
  typed_pointer *tptr
)
{
  TRY();

  CHECK_POINTER(astat);
  CHECK_POINTER(tptr);
  CHECK_POINTER(tptr->value);

  *astat = has_accumulated_stat(tptr);
  if (*astat == NULL) {
    ERROR(BAD_TYPE);
  }

  FINALLY(expect_accumulated_stat);
  RETURN();
}

/******************************************************************************/

result ensure_neighborhood_stat
(
  typed_pointer *annotation,
  neighborhood_stat **nstat
)
{
  TRY();
  typed_pointer *tptr;

  CHECK_POINTER(nstat);
  *nstat = NULL;

  CHECK(ensure_has(annotation, t_NSTAT, &tptr));

  *nstat = (neighborhood_stat*)tptr->value;

  FINALLY(ensure_neighborhood_stat);
  RETURN();
}

/******************************************************************************/

truth_value is_neighborhood_stat
(
  typed_pointer *tptr
)
{
  if (tptr != NULL && tptr->type == t_NSTAT) {
    return TRUE;
  }
  return FALSE;
}

/******************************************************************************/

neighborhood_stat *has_neighborhood_stat
(
  typed_pointer *tptr
)
{
  if (IS_TRUE(is_neighborhood_stat(tptr))) {
    return (neighborhood_stat*)tptr->value;
  }
  if (IS_TRUE(is_tuple(tptr))) {
    typed_pointer *element;
    element = tuple_has_type(tptr, t_NSTAT);
    if (element != NULL) {
      if (IS_FALSE(is_neighborhood_stat(element))) {
        return NULL;
      }
      return (neighborhood_stat*)element->value;
    }
  }
  return NULL;
}

/******************************************************************************/

result expect_neighborhood_stat
(
  neighborhood_stat **nstat,
  typed_pointer *tptr
)
{
  TRY();

  CHECK_POINTER(nstat);

  *nstat = has_neighborhood_stat(tptr);
  if (*nstat == NULL) {
    ERROR(BAD_TYPE);
  }

  FINALLY(expect_neighborhood_stat);
  RETURN();
}

/******************************************************************************/

truth_value is_ridge_potential
(
  typed_pointer *tptr
)
{
  if (tptr != NULL && tptr->type == t_ridge_potential) {
    return TRUE;
  }
  return FALSE;
}

/******************************************************************************/

ridge_potential *has_ridge_potential
(
  typed_pointer *tptr,
  uint32 token
)
{
  if (IS_TRUE(is_ridge_potential(tptr)) && tptr->token == token) {
    return (ridge_potential*)tptr->value;
  }
  else {
    typed_pointer *element = tuple_has_type(tptr, t_ridge_potential);
    if (element != NULL && element->token == token) {
      return (ridge_potential*)element->value;
    }
  }
  return NULL;
}

/******************************************************************************/

result ensure_segment_message
(
  typed_pointer *annotation,
  segment_message **smsg,
  uint32 token
)
{
  TRY();
  typed_pointer *tptr;

  CHECK_POINTER(smsg);
  *smsg = NULL;

  CHECK(ensure_has(annotation, t_segment_message, &tptr));

  *smsg = (segment_message*)tptr->value;
  tptr->token = token;

  FINALLY(ensure_segment_message);
  RETURN();
}

truth_value is_segment_message
(
  typed_pointer *tptr
)
{
  if (tptr != NULL && tptr->type == t_segment_message) {
    return TRUE;
  }
  return FALSE;
}

segment_message *has_segment_message
(
  typed_pointer *tptr,
  uint32 token
)
{
  if (IS_TRUE(is_segment_message(tptr)) && tptr->token == token) {
    return (segment_message*)tptr->value;
  }
  else {
    typed_pointer *element = tuple_has_type(tptr, t_segment_message);
    if (element != NULL && element->token == token) {
      return (segment_message*)element->value;
    }
  }
  return NULL;
}

/******************************************************************************/

result ensure_segment_potential
(
  typed_pointer *annotation,
  segment_potential **spot,
  uint32 token
)
{
  TRY();
  typed_pointer *tptr;

  CHECK_POINTER(spot);
  *spot = NULL;

  CHECK(ensure_has(annotation, t_segment_potential, &tptr));

  *spot = (segment_potential*)tptr->value;
  tptr->token = token;

  FINALLY(ensure_segment_potential);
  RETURN();
}

truth_value is_segment_potential
(
  typed_pointer *tptr
)
{
  if (tptr != NULL && tptr->type == t_segment_potential) {
    return TRUE;
  }
  return FALSE;
}

segment_potential *has_segment_potential
(
  typed_pointer *tptr,
  uint32 token
)
{
  if (IS_TRUE(is_segment_potential(tptr)) && tptr->token == token) {
    return (segment_potential*)tptr->value;
  }
  else {
    typed_pointer *element = tuple_has_type(tptr, t_segment_potential);
    if (element != NULL && element->token == token) {
      return (segment_potential*)element->value;
    }
  }
  return NULL;
}

/******************************************************************************/

truth_value is_link_measure
(
  typed_pointer *tptr
)
{
  if (tptr != NULL && tptr->type == t_link_measure) {
    return TRUE;
  }
  return FALSE;
}

/******************************************************************************/

link_measure *has_link_measure
(
  typed_pointer *tptr,
  uint32 token
)
{
  if (IS_TRUE(is_link_measure(tptr)) && tptr->token == token) {
    return (link_measure*)tptr->value;
  }
  else {
    typed_pointer *element = tuple_has_type(tptr, t_link_measure);
    if (element != NULL && element->token == token) {
      return (link_measure*)element->value;
    }
  }
  return NULL;
}

/******************************************************************************/

truth_value is_edge_profile
(
  typed_pointer *tptr
)
{
  if (tptr != NULL && tptr->type == t_edge_profile) {
    return TRUE;
  }
  return FALSE;
}

edge_profile *has_edge_profile
(
  typed_pointer *tptr,
  uint32 token
)
{
  if (IS_TRUE(is_edge_profile(tptr)) && tptr->token == token) {
    return (edge_profile*)tptr->value;
  }
  else {
    typed_pointer *element = tuple_has_type(tptr, t_edge_profile);
    if (element != NULL && element->token == token) {
      return (edge_profile*)element->value;
    }
  }
  return NULL;
}

/******************************************************************************/

truth_value is_edge_links
(
  typed_pointer *tptr
)
{
  if (tptr != NULL && tptr->type == t_edge_links) {
    return TRUE;
  }
  return FALSE;
}

/******************************************************************************/

edge_links *has_edge_links
(
  typed_pointer *tptr,
  uint32 token
)
{
  if (IS_TRUE(is_edge_links(tptr)) && tptr->token == token) {
    return (edge_links*)tptr->value;
  }
  else {
    typed_pointer *element = tuple_has_type(tptr, t_edge_links);
    if (element != NULL && element->token == token) {
      return (edge_links*)element->value;
    }
  }
  return NULL;
}

/******************************************************************************/

truth_value is_boundary_strength
(
  typed_pointer *tptr
)
{
  if (tptr != NULL && tptr->type == t_BOUNDARY_STRENGTH) {
    return TRUE;
  }
  return FALSE;
}

/******************************************************************************/

boundary_strength *has_boundary_strength
(
  typed_pointer *tptr,
  uint32 token
)
{
  if (IS_TRUE(is_boundary_strength(tptr))) {
    if (tptr->token != token) {
      return NULL;
    }
    return (boundary_strength*)tptr->value;
  }
  if (IS_TRUE(is_tuple(tptr))) {
    typed_pointer *element;
    element = tuple_has_type(tptr, t_BOUNDARY_STRENGTH);
    if (element != NULL) {
      if (IS_FALSE(is_boundary_strength(element)) || element->token != token) {
        return NULL;
      }
      return (boundary_strength*)element->value;
    }
  }
  return NULL;
}

/******************************************************************************/

truth_value is_segment_strength
(
  typed_pointer *tptr
)
{
  if (tptr != NULL && tptr->type == t_SEGMENT_STRENGTH) {
    return TRUE;
  }
  return FALSE;
}

/******************************************************************************/

segment_strength *has_segment_strength
(
  typed_pointer *tptr,
  uint32 token
)
{
  if (IS_TRUE(is_segment_strength(tptr))) {
    if (tptr->token != token) {
      return NULL;
    }
    return (segment_strength*)tptr->value;
  }
  if (IS_TRUE(is_tuple(tptr))) {
    typed_pointer *element;
    element = tuple_has_type(tptr, t_SEGMENT_STRENGTH);
    if (element != NULL) {
      if (IS_FALSE(is_segment_strength(element)) || element->token != token) {
        return NULL;
      }
      return (segment_strength*)element->value;
    }
  }
  return NULL;
}

/******************************************************************************/

result ensure_accumulated_reg
(
  typed_pointer *annotation,
  accumulated_reg **areg
)
{
  TRY();
  typed_pointer *tptr;

  CHECK_POINTER(areg);
  *areg = NULL;

  CHECK(ensure_has(annotation, t_AREG, &tptr));

  *areg = (accumulated_reg*)tptr->value;

  FINALLY(ensure_accumulated_reg);
  RETURN();
}

/******************************************************************************/

truth_value is_accumulated_reg
(
  typed_pointer *tptr
)
{
  if (tptr != NULL && tptr->type == t_AREG) {
    return TRUE;
  }
  return FALSE;
}

/******************************************************************************/

accumulated_reg *has_accumulated_reg
(
  typed_pointer *tptr
)
{
  if (IS_TRUE(is_accumulated_reg(tptr))) {
    return (accumulated_reg*)tptr->value;
  }
  if (IS_TRUE(is_tuple(tptr))) {
    typed_pointer *element;
    element = tuple_has_type(tptr, t_AREG);
    if (element != NULL) {
      if (IS_FALSE(is_accumulated_reg(element))) {
        return NULL;
      }
      return (accumulated_reg*)element->value;
    }
  }
  return NULL;
}

/******************************************************************************/

result ensure_edge_response
(
  typed_pointer *annotation,
  edge_response **eresp
)
{
  TRY();
  typed_pointer *tptr;

  CHECK_POINTER(eresp);
  *eresp = NULL;

  CHECK(ensure_has(annotation, t_EDGE_RESPONSE, &tptr));

  *eresp = (edge_response*)tptr->value;

  FINALLY(ensure_edge_response);
  RETURN();
}

/******************************************************************************/

truth_value is_edge_response
(
  typed_pointer *tptr
)
{
  if (tptr != NULL && tptr->type == t_EDGE_RESPONSE) {
    return TRUE;
  }
  return FALSE;
}

/******************************************************************************/

edge_response *has_edge_response
(
  typed_pointer *tptr,
  uint32 token
)
{
  if (IS_TRUE(is_edge_response(tptr)) && tptr->token == token) {
    return (edge_response*)tptr->value;
  }
  else {
    typed_pointer *element = tuple_has_type(tptr, t_EDGE_RESPONSE);
    if (element != NULL && element->token == token) {
      return (edge_response*)element->value;
    }
  }
  return NULL;
}

/******************************************************************************/

truth_value is_smoothed_gradient
(
  typed_pointer *tptr
)
{
  if (tptr != NULL && tptr->type == t_smoothed_gradient) {
    return TRUE;
  }
  return FALSE;
}

/******************************************************************************/

smoothed_gradient *has_smoothed_gradient
(
  typed_pointer *tptr,
  uint32 token
)
{
  if (IS_TRUE(is_smoothed_gradient(tptr)) && tptr->token == token) {
    return (smoothed_gradient*)tptr->value;
  }
  else {
    typed_pointer *element = tuple_has_type(tptr, t_smoothed_gradient);
    if (element != NULL && element->token == token) {
      return (smoothed_gradient*)element->value;
    }
  }
  return NULL;
}

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
    PRINT0("warning: tree is null in compare_segments\n");
    return -1;
  }
  sb = *((const quad_forest_segment* const *)b);

  if (sb == NULL) {
    PRINT0("warning: tree is null in compare_segments\n");
    return -1;
  }

  if (sa > sb) return 1;
  else if (sa < sb) return -1;
  else return 0;
}

/******************************************************************************/

result ensure_boundary_potential
(
  typed_pointer *annotation,
  boundary_potential **bpot,
  uint32 token
)
{
  TRY();
  typed_pointer *tptr;

  CHECK_POINTER(bpot);
  *bpot = NULL;

  CHECK(ensure_has(annotation, t_boundary_potential, &tptr));

  *bpot = (boundary_potential*)tptr->value;
  tptr->token = token;

  FINALLY(ensure_boundary_potential);
  RETURN();
}

truth_value is_boundary_potential
(
  typed_pointer *tptr
)
{
  if (tptr != NULL && tptr->type == t_boundary_potential) {
    return TRUE;
  }
  return FALSE;
}

boundary_potential *has_boundary_potential
(
  typed_pointer *tptr,
  uint32 token
)
{
  if (IS_TRUE(is_boundary_potential(tptr)) && tptr->token == token) {
    return (boundary_potential*)tptr->value;
  }
  else {
    typed_pointer *element = tuple_has_type(tptr, t_boundary_potential);
    if (element != NULL && element->token == token) {
      return (boundary_potential*)element->value;
    }
  }
  return NULL;
}

/******************************************************************************/

result ensure_boundary_message
(
  typed_pointer *annotation,
  boundary_message **bmsg,
  uint32 token
)
{
  TRY();
  typed_pointer *tptr;

  CHECK_POINTER(bmsg);
  *bmsg = NULL;

  CHECK(ensure_has(annotation, t_boundary_message, &tptr));

  *bmsg = (boundary_message*)tptr->value;
  tptr->token = token;

  FINALLY(ensure_boundary_message);
  RETURN();
}

truth_value is_boundary_message
(
  typed_pointer *tptr
)
{
  if (tptr != NULL && tptr->type == t_boundary_message) {
    return TRUE;
  }
  return FALSE;
}

boundary_message *has_boundary_message
(
  typed_pointer *tptr,
  uint32 token
)
{
  if (IS_TRUE(is_boundary_message(tptr)) && tptr->token == token) {
    return (boundary_message*)tptr->value;
  }
  else {
    typed_pointer *element = tuple_has_type(tptr, t_boundary_message);
    if (element != NULL && element->token == token) {
      return (boundary_message*)element->value;
    }
  }
  return NULL;
}

/******************************************************************************/

result ensure_boundary_fragment
(
  typed_pointer *annotation,
  boundary_fragment **bfrag,
  uint32 token
)
{
  TRY();
  typed_pointer *tptr;

  CHECK_POINTER(bfrag);
  *bfrag = NULL;

  CHECK(ensure_has(annotation, t_boundary_fragment, &tptr));

  *bfrag = (boundary_fragment*)tptr->value;
  tptr->token = token;

  FINALLY(ensure_boundary_fragment);
  RETURN();
}

truth_value is_boundary_fragment
(
  typed_pointer *tptr
)
{
  if (tptr != NULL && tptr->type == t_boundary_fragment) {
    return TRUE;
  }
  return FALSE;
}

boundary_fragment *has_boundary_fragment
(
  typed_pointer *tptr,
  uint32 token
)
{
  if (IS_TRUE(is_boundary_fragment(tptr)) && tptr->token == token) {
    return (boundary_fragment*)tptr->value;
  }
  else {
    typed_pointer *element = tuple_has_type(tptr, t_boundary_fragment);
    if (element != NULL && element->token == token) {
      return (boundary_fragment*)element->value;
    }
  }
  return NULL;
}

/******************************************************************************/
/* union-find implementation for boundary fragments                           */

void boundary_fragment_create
(
  quad_tree *tree
)
{

}

void boundary_fragment_union
(
  quad_tree *tree1,
  quad_tree *tree2
)
{

}

boundary_fragment *boundary_fragment_find
(
  quad_tree *tree
)
{
  return NULL;
}

/* end of file                                                                */
/******************************************************************************/
