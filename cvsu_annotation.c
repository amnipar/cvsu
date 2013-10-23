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
string ensure_edge_response_name = "annotation_ensure_edge_response";
string ensure_link_measure_name = "ensure_link_measure";
string expect_link_measure_name = "expect_link_measure";
string ensure_edge_links_name = "ensure_edge_links";
string expect_edge_links_name = "expect_edge_links";
string expect_edge_response_name = "expect_edge_response";
string ensure_boundary_message_name = "ensure_boundary_message";
string ensure_segment_message_name = "ensure_segment_message";
string expect_segment_message_name = "expect_segment_message";

string quad_tree_ensure_boundary_name = "quad_tree_ensure_boundary";
string quad_tree_boundary_init_name = "quad_tree_boundary_init";
string quad_tree_ensure_segment_name = "quad_tree_ensure_segment";

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

  CHECK(ensure_has(annotation, t_accumulated_stat, &tptr));

  *astat = (accumulated_stat*)tptr->value;

  FINALLY(ensure_accumulated_stat);
  RETURN();
}

truth_value is_accumulated_stat
(
  typed_pointer *tptr
)
{
  if (tptr != NULL && tptr->type == t_accumulated_stat) {
    return TRUE;
  }
  return FALSE;
}

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
    element = tuple_has_type(tptr, t_accumulated_stat);
    if (element != NULL) {
      if (IS_FALSE(is_accumulated_stat(element))) {
        return NULL;
      }
      return (accumulated_stat*)element->value;
    }
  }
  return NULL;
}

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

  CHECK(ensure_has(annotation, t_neighborhood_stat, &tptr));

  *nstat = (neighborhood_stat*)tptr->value;

  FINALLY(ensure_neighborhood_stat);
  RETURN();
}

truth_value is_neighborhood_stat
(
  typed_pointer *tptr
)
{
  if (tptr != NULL && tptr->type == t_neighborhood_stat) {
    return TRUE;
  }
  return FALSE;
}

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
    element = tuple_has_type(tptr, t_neighborhood_stat);
    if (element != NULL) {
      if (IS_FALSE(is_neighborhood_stat(element))) {
        return NULL;
      }
      return (neighborhood_stat*)element->value;
    }
  }
  return NULL;
}

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

result ensure_segment_message
(
  typed_pointer *annotation,
  segment_message **smsg,
  uint32 token,
  integral_value strength_diff
)
{
  TRY();
  typed_pointer *tptr;
  segment_message *message;

  CHECK_POINTER(smsg);
  *smsg = NULL;

  CHECK(ensure_has(annotation, t_segment_message, &tptr));

  message = (segment_message*)tptr->value;
  if (tptr->token != token) {
    tptr->token = token;
    message->extent = 0;
    message->echo = FALSE;
    message->strength_diff = strength_diff;
  }
  *smsg = message;

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

result expect_segment_message
(
  typed_pointer *tptr,
  segment_message **smsg,
  uint32 token
)
{
  TRY();

  CHECK_POINTER(smsg);

  *smsg = has_segment_message(tptr, token);
  if (*smsg == NULL) {
    ERROR(NOT_FOUND);
  }

  FINALLY(expect_segment_message);
  RETURN();
}

/******************************************************************************/

result ensure_link_measure
(
  typed_pointer *annotation,
  link_measure **lmeasure,
  uint32 token
)
{
  TRY();
  typed_pointer *tptr;
  link_measure *measure;

  CHECK(ensure_has(annotation, t_link_measure, &tptr));
  measure = (link_measure*)tptr->value;
  if (tptr->token != token) {
    tptr->token = token;
    measure->category = bl_UNDEF;
    measure->magnitude_score = 0;
    measure->strength_score = 0;
    measure->angle_score = 0;
    measure->against_score = 0;
    measure->profile_score = 0;
    measure->parallel_score = 0;
    measure->perpendicular_score = 0;
  }

  if (lmeasure != NULL) {
    *lmeasure = measure;
  }

  FINALLY(ensure_link_measure);
  RETURN();
}

truth_value is_link_measure
(
  const typed_pointer *tptr
)
{
  if (tptr != NULL && tptr->type == t_link_measure) {
    return TRUE;
  }
  return FALSE;
}

link_measure *has_link_measure
(
  const typed_pointer *tptr,
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

result expect_link_measure
(
  const typed_pointer *tptr,
  link_measure **lmeasure,
  uint32 token
)
{
  TRY();

  CHECK_POINTER(lmeasure);

  *lmeasure = has_link_measure(tptr, token);
  if (*lmeasure == NULL) {
    ERROR(NOT_FOUND);
  }

  FINALLY(expect_link_measure);
  RETURN();
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

result ensure_edge_links
(
  typed_pointer *annotation,
  edge_links **elinks,
  uint32 token
)
{
  TRY();
  typed_pointer *tptr;
  edge_links *links;

  CHECK_POINTER(elinks);
  *elinks = NULL;

  CHECK(ensure_has(annotation, t_edge_links, &tptr));
  links = (edge_links*)tptr->value;
  if (tptr->token != token) {
    tptr->token = token;
    links->towards = NULL;
    links->against = NULL;
    links->own_angle = 0;
    links->own_curvature = 0;
  }
  *elinks = links;

  FINALLY(ensure_edge_links);
  RETURN();
}

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

result expect_edge_links
(
  typed_pointer *tptr,
  edge_links **elinks,
  uint32 token
)
{
  TRY();

  CHECK_POINTER(elinks);

  *elinks = has_edge_links(tptr, token);
  if (*elinks == NULL) {
    ERROR(NOT_FOUND);
  }

  FINALLY(expect_edge_links);
  RETURN();
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

  CHECK(ensure_has(annotation, t_edge_response, &tptr));

  *eresp = (edge_response*)tptr->value;

  FINALLY(ensure_edge_response);
  RETURN();
}

truth_value is_edge_response
(
  typed_pointer *tptr
)
{
  if (tptr != NULL && tptr->type == t_edge_response) {
    return TRUE;
  }
  return FALSE;
}

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
    typed_pointer *element = tuple_has_type(tptr, t_edge_response);
    if (element != NULL && element->token == token) {
      return (edge_response*)element->value;
    }
  }
  return NULL;
}

result expect_edge_response
(
  edge_response **eresp,
  typed_pointer *tptr
)
{
  TRY();

  CHECK_POINTER(eresp);

  *eresp = has_edge_response(tptr, tptr->token);
  if (*eresp == NULL) {
    ERROR(NOT_FOUND);
  }

  FINALLY(expect_edge_response);
  RETURN();
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
  boundary_message *message;

  CHECK_POINTER(bmsg);
  *bmsg = NULL;

  CHECK(ensure_has(annotation, t_boundary_message, &tptr));
  message = (boundary_message*)tptr->value;
  if (tptr->token != token) {
    tptr->token = token;
    message->round = 0;
    message->pool_curvature = 0;
    message->acc_curvature = 0;
    message->pool_distance = 0;
    message->acc_distance = 0;
    message->pool_length = 0;
    message->acc_length = 0;
  }
  *bmsg = message;

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
/* boundary structures and functions                                           */
/******************************************************************************/

int compare_boundaries_by_quality(const void *a, const void *b)
{
  const boundary *sa, *sb;
  integral_value quality_diff;

  sa = *((const boundary* const *)a);
  if (sa == NULL) {
    PRINT0("warning: tree is null in compare_boundaries\n");
    return -1;
  }

  sb = *((const boundary* const *)b);
  if (sb == NULL) {
    PRINT0("warning: tree is null in compare_boundaries\n");
    return -1;
  }

  quality_diff = sa->quality - sb->quality;
  if (quality_diff > 0.0001) return -1;
  else if (quality_diff < -0.0001) return 1;
  else return 0;
}

int compare_boundaries_by_length(const void *a, const void *b)
{
  const boundary *sa, *sb;
  sint32 quality_diff;

  sa = *((const boundary* const *)a);
  if (sa == NULL) {
    PRINT0("warning: tree is null in compare_boundaries_by_length\n");
    return -1;
  }

  sb = *((const boundary* const *)b);
  if (sb == NULL) {
    PRINT0("warning: tree is null in compare_boundaries_by_length\n");
    return -1;
  }

  quality_diff = ((signed)sa->length) - ((signed)sb->length);
  if (quality_diff > 0) return -1;
  else if (quality_diff < 0) return 1;
  else return 0;
}

/******************************************************************************/

truth_value is_boundary
(
  typed_pointer *input_pointer
)
{
  if (input_pointer != NULL && input_pointer->type == t_boundary) {
    return TRUE;
  }
  return FALSE;
}

/******************************************************************************/

boundary *has_boundary
(
  typed_pointer *input_pointer,
  uint32 token
)
{
  if (input_pointer != NULL) {
    if (token == 0) {
      token = input_pointer->token;
    }
    if (IS_TRUE(is_boundary(input_pointer)) && input_pointer->token == token) {
      return (boundary*)input_pointer->value;
    }
    else {
      typed_pointer *element = tuple_has_type(input_pointer, t_boundary);
      if (element != NULL && element->token == token) {
        return (boundary*)element->value;
      }
    }
  }
  return NULL;
}

/******************************************************************************/
/* union-find implementation for boundary fragments                           */

result quad_tree_ensure_boundary
(
  quad_tree *input_tree,
  boundary **output_boundary
)
{
  TRY();
  typed_pointer *annotation, *new_pointer;
  boundary *tree_boundary;

  CHECK_POINTER(input_tree);

  annotation = &input_tree->annotation;
  CHECK(ensure_has(annotation, t_boundary, &new_pointer));
  tree_boundary = (boundary*)new_pointer->value;
  /* proceed only if the boundary hasn't been initialized yet in this frame */
  if (new_pointer->token != annotation->token) {
    new_pointer->token = annotation->token;
    /* one-tree segment is it's own parent, and has the rank of 0 */
    tree_boundary->parent = tree_boundary;
    tree_boundary->prev = NULL;
    tree_boundary->next = NULL;
    tree_boundary->tree = input_tree;
    tree_boundary->category = fc_UNDEF;
    tree_boundary->rank = 0;
    tree_boundary->length = 0;
    tree_boundary->x = 0;
    tree_boundary->y = 0;
    tree_boundary->dx = 0;
    tree_boundary->dy = 0;
    tree_boundary->angle = 0;
    tree_boundary->smoothed_angle = 0;
    tree_boundary->curvature = 0;
  }

  if (output_boundary != NULL) {
    *output_boundary = tree_boundary;
  }

  FINALLY(quad_tree_ensure_boundary);
  RETURN();
}

/******************************************************************************/

void boundary_init
(
  boundary *input_boundary,
  edge_links *elinks
)
{
  integral_value angle;
  if (input_boundary != NULL && elinks != NULL) {
    boundary *parent;
    parent = boundary_find(input_boundary);
    if (parent == input_boundary && parent->length < 1) {
      angle = elinks->own_angle;
      /* for a fragment of one node, the category is still unknown. */
      input_boundary->category = fc_UNKNOWN;
      input_boundary->length = 1;
      input_boundary->angle = angle;
    }
  }
}

/******************************************************************************/

result quad_tree_boundary_init
(
  struct quad_tree_t *tree,
  boundary **output_boundary,
  edge_links *elinks
)
{
  TRY();
  CHECK(quad_tree_ensure_boundary(tree, output_boundary));
  boundary_init(*output_boundary, elinks);
  FINALLY(quad_tree_boundary_init);
  RETURN();
}

/******************************************************************************/

void boundary_union
(
  boundary *input_boundary_1,
  boundary *input_boundary_2
)
{
  boundary *parent_1, *parent_2;

  parent_1 = boundary_find(input_boundary_1);
  parent_2 = boundary_find(input_boundary_2);

  if (parent_1 != NULL && parent_2 != NULL && parent_1 != parent_2) {
    if (parent_1->rank < parent_2->rank) {
      parent_1->parent = parent_2;
      parent_2->length += parent_1->length;
      /*
      parent_2->curvature_sum1 += parent_1->curvature_sum1;
      parent_2->curvature_mean =
          parent_2->curvature_sum1 /
          ((integral_value)parent_2->length);
      parent_2->curvature_sum2 += parent_1->curvature_sum2;
      parent_2->curvature_dev =
          parent_2->curvature_sum2 /
          ((integral_value)parent_2->length) -
          pow(parent_2->curvature_mean, 2.0);
      */
    }
    else {
      parent_2->parent = parent_1;
      if (parent_1->rank == parent_2->rank) {
        parent_1->rank += 1;
      }
      parent_1->length += parent_2->length;
      /*
      parent_1->curvature_sum1 += parent_2->curvature_sum1;
      parent_1->curvature_mean =
          parent_1->curvature_sum1 /
          ((integral_value)parent_1->length);
      parent_1->curvature_sum2 += parent_2->curvature_sum2;
      parent_1->curvature_dev =
          parent_1->curvature_sum2 /
          ((integral_value)parent_1->length) -
          pow(parent_1->curvature_mean, 2.0);
      */
    }
  }
}

/******************************************************************************/

void quad_tree_boundary_union
(
  quad_tree *input_tree_1,
  quad_tree *input_tree_2
)
{
  boundary_union(
      quad_tree_boundary_find(input_tree_1),
      quad_tree_boundary_find(input_tree_2));
}

/******************************************************************************/

boundary *boundary_find
(
  boundary *input_boundary
)
{
  if (input_boundary != NULL) {
    if (input_boundary->parent != NULL && input_boundary->parent != input_boundary) {
      input_boundary->parent = boundary_find(input_boundary->parent);
    }
    return input_boundary->parent;
  }
  return NULL;
}

/******************************************************************************/

boundary *quad_tree_boundary_find
(
  quad_tree *input_tree
)
{
  if (input_tree != NULL) {
    boundary *tree_boundary = has_boundary(&input_tree->annotation, 0);
    return boundary_find(tree_boundary);
  }
  return NULL;
}

/******************************************************************************/

uint32 quad_tree_boundary_id
(
  quad_tree *input_tree
)
{
  return (uint32)quad_tree_boundary_find(input_tree);
}

/******************************************************************************/

truth_value quad_tree_is_boundary_parent
(
  quad_tree *input_tree
)
{
  if (input_tree != NULL) {
    boundary *tree_boundary = has_boundary(&input_tree->annotation, 0);
    if (tree_boundary != NULL) {
      if (boundary_find(tree_boundary) == tree_boundary) {
        return TRUE;
      }
    }
  }
  return FALSE;
}

/******************************************************************************/
/* segment structures and functions                                           */
/******************************************************************************/

int compare_segments(const void *a, const void *b)
{
  const segment *sa, *sb;

  sa = *((const segment* const *)a);
  if (sa == NULL) {
    PRINT0("warning: tree is null in compare_segments\n");
    return -1;
  }

  sb = *((const segment* const *)b);
  if (sb == NULL) {
    PRINT0("warning: tree is null in compare_segments\n");
    return -1;
  }

  if (sa > sb) return 1;
  else if (sa < sb) return -1;
  else return 0;
}

/******************************************************************************/

truth_value is_segment
(
  typed_pointer *input_pointer
)
{
  if (input_pointer != NULL && input_pointer->type == t_segment) {
    return TRUE;
  }
  return FALSE;
}

/******************************************************************************/

segment *has_segment
(
  typed_pointer *input_pointer,
  uint32 token
)
{
  if (input_pointer != NULL) {
    if (token == 0) {
      token = input_pointer->token;
    }
    if (IS_TRUE(is_segment(input_pointer)) && input_pointer->token == token) {
      return (segment*)input_pointer->value;
    }
    else {
      typed_pointer *element = tuple_has_type(input_pointer, t_segment);
      if (element != NULL && element->token == token) {
        return (segment*)element->value;
      }
    }
  }
  return NULL;
}

/******************************************************************************/

result quad_tree_ensure_segment
(
  quad_tree *input_tree,
  segment **output_segment
)
{
  TRY();
  typed_pointer *annotation, *new_pointer;
  segment *tree_segment;

  CHECK_POINTER(input_tree);

  tree_segment = NULL;

  annotation = &input_tree->annotation;
  CHECK(ensure_has(annotation, t_segment, &new_pointer));
  tree_segment = (segment*)new_pointer->value;
  /* proceed only if the segment hasn't been initialized yet in this frame */
  if (new_pointer->token != annotation->token) {
    /* still need to check also the parent? */
    /* if (tree_segment->parent == NULL) ?? */
    new_pointer->token = annotation->token;

    /* one-tree segment is its own parent, and has the rank of 0 */
    tree_segment->parent = tree_segment;
    tree_segment->category = sc_UNDEF;
    tree_segment->rank = 0;
    tree_segment->extent = 0;
    memory_copy((data_pointer)&tree_segment->stat,
                (data_pointer)&input_tree->stat, 1, sizeof(statistics));
  }
  if (output_segment != NULL) {
    *output_segment = tree_segment;
  }

  FINALLY(quad_tree_ensure_segment);
  RETURN();
}

/******************************************************************************/

segment *quad_tree_get_segment
(
  quad_tree *input_tree
)
{
  if (input_tree != NULL) {
    return has_segment(&input_tree->annotation, 0);
  }
  return NULL;
}

/******************************************************************************/

void segment_union
(
  segment *input_segment_1,
  segment *input_segment_2
)
{
  /* if the segments are already in the same class, no need for union */
  if (input_segment_1 != NULL && input_segment_2 != NULL && input_segment_1 != input_segment_2) {
    /* otherwise set the tree with higher class rank as id of the union */
    statistics *stat;
    integral_value N, mean, variance;
    if (input_segment_1->rank < input_segment_2->rank) {
      input_segment_1->parent = input_segment_2;
      stat = &input_segment_2->stat;
      N = (stat->N += input_segment_1->stat.N);
      stat->sum += input_segment_1->stat.sum;
      stat->sum2 += input_segment_1->stat.sum2;
      mean = stat->mean = stat->sum / N;
      variance = stat->sum2 / N - mean*mean;
      if (variance < 0) variance = 0;
      stat->variance = variance;
      stat->deviation = sqrt(variance);

      if (input_segment_1->extent > input_segment_2->extent) {
        input_segment_2->extent = input_segment_1->extent;
      }
      if (input_segment_2->extent < 3) {
        input_segment_2->category = sc_CLUTTER;
      }
      else {
        input_segment_2->category = sc_FOREGROUND;
      }
    }
    else {
      input_segment_2->parent = input_segment_1;
      /* when equal rank trees are combined, the root tree's rank is increased */
      if (input_segment_1->rank == input_segment_2->rank) {
        input_segment_1->rank += 1;
      }
      stat = &input_segment_1->stat;
      N = (stat->N += input_segment_2->stat.N);
      stat->sum += input_segment_2->stat.sum;
      stat->sum2 += input_segment_2->stat.sum2;
      mean = stat->mean = stat->sum / N;
      variance = stat->sum2 / N - mean*mean;
      if (variance < 0) variance = 0;
      stat->variance = variance;
      stat->deviation = sqrt(variance);

      if (input_segment_2->extent > input_segment_1->extent) {
        input_segment_1->extent = input_segment_2->extent;
      }
      if (input_segment_1->extent < 3) {
        input_segment_1->category = sc_CLUTTER;
      }
      else {
        input_segment_1->category = sc_FOREGROUND;
      }
    }
  }
}

/******************************************************************************/

void quad_tree_segment_union
(
  quad_tree *input_tree_1,
  quad_tree *input_tree_2
)
{
  segment_union(
      quad_tree_segment_find(input_tree_1),
      quad_tree_segment_find(input_tree_2));
}

/******************************************************************************/

segment *segment_find
(
  segment *input_segment
)
{
  if (input_segment != NULL) {
    if (input_segment->parent != NULL && input_segment->parent != input_segment) {
      input_segment->parent = segment_find(input_segment->parent);
    }
    return input_segment->parent;
  }
  return NULL;
}

/******************************************************************************/

segment *quad_tree_segment_find
(
  quad_tree *input_tree
)
{
  if (input_tree != NULL) {
    segment *tree_segment = has_segment(&input_tree->annotation, 0);
    return segment_find(tree_segment);
  }
  return NULL;
}

/******************************************************************************/

uint32 quad_tree_segment_id
(
  quad_tree *input_tree
)
{
  return (uint32)quad_tree_segment_find(input_tree);
}

/******************************************************************************/

truth_value quad_tree_is_segment_parent
(
  quad_tree *input_tree
)
{
  if (input_tree != NULL) {
    segment *tree_segment = has_segment(&input_tree->annotation, 0);
    if (tree_segment != NULL) {
      if (segment_find(tree_segment) == tree_segment) {
        return TRUE;
      }
    }
  }
  return FALSE;
}

/* end of file                                                                */
/******************************************************************************/
