/**
 * @file cvsu_quad_forest.c
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

#include "cvsu_quad_forest.h"
#include "cvsu_macros.h"
#include "cvsu_memory.h"

#include <stdlib.h>
/*#include <stdio.h>*/
#include <sys/time.h>
#include <math.h>

/******************************************************************************/
/* some gcc versions seem to require these definitions to work properly       */
/* remove them if they cause problems with other compilers                    */

double fmin (double __x, double __y);
double fmax (double __x, double __y);

/******************************************************************************/
/* constants for reporting function names in error messages                   */

string quad_tree_nullify_name = "quad_tree_nullify";
string quad_tree_divide_name = "quad_tree_divide";
string quad_tree_get_child_statistics_name = "quad_tree_get_child_statistics";
string quad_tree_get_neighborhood_statistics_name = "quad_tree_get_neighborhood_statistics";
string quad_tree_divide_with_overlap_name = "quad_tree_divide_with_overlap";
string quad_tree_get_edge_response_name = "quad_tree_get_edge_response";
string quad_tree_get_child_edge_response_name = "quad_tree_get_child_edge_response";
string quad_tree_get_neighbors_name = "quad_tree_get_neighbors";
string quad_forest_init_name = "quad_forest_init";
string quad_forest_alloc_name = "quad_forest_alloc";
string quad_forest_free_name = "quad_forest_free";
string quad_forest_create_name = "quad_forest_create";
string quad_forest_reload_name = "quad_forest_reload";
string quad_forest_refresh_segments_name = "quad_forest_refresh_segments";
string quad_forest_destroy_name = "quad_forest_destroy";
string quad_forest_nullify_name = "quad_forest_nullify";
string quad_forest_update_name = "quad_forest_update";
string quad_forest_segment_with_deviation_name = "quad_forest_segment_with_deviation";
string quad_forest_segment_with_overlap_name = "quad_forest_segment_with_overlap";
string quad_forest_get_segments_name = "quad_forest_get_regions";
string quad_forest_get_segment_trees_name = "quad_forest_get_segment_trees";
string quad_forest_get_segment_neighbors_name = "quad_forest_get_segment_neighbors";
string quad_forest_get_segment_mask_name = "quad_forest_get_segment_mask";
string quad_forest_highlight_segments_name = "quad_forest_highlight_segments";
string quad_forest_draw_image_name = "quad_forest_draw_image";
string quad_forest_find_edges_name = "quad_forest_find_edges";
string quad_forest_find_vertical_edges_name = "quad_forest_find_vertical_edges";
string quad_forest_find_horizontal_edges_name = "quad_forest_find_horizontal_edges";
string quad_forest_segment_horizontal_edges_name = "quad_forest_segment_horizontal_edges";

/******************************************************************************/
/* quad_forest_status possible values                                         */

/* Forest has been initialized, but not yet updated. */
const quad_forest_status FOREST_INITIALIZED;
/* Forest has been updated, but no analysis performed. */
const quad_forest_status FOREST_UPDATED;
/* Segmentation operation has been performed. */
const quad_forest_status FOREST_SEGMENTED;
/* Edge detection operation has been performed. */
const quad_forest_status FOREST_EDGES_DETECTED;

/******************************************************************************/

result quad_tree_nullify
(
  quad_tree *target
)
{
  TRY();

  CHECK(memory_clear((data_pointer)target, 1, sizeof(quad_tree)));

  FINALLY(quad_tree_nullify);
  RETURN();
}

/******************************************************************************/

truth_value quad_tree_is_null
(
  quad_tree *target
)
{
  if (target != NULL) {
    /* a tree is NULL if its size has not been set */
    if (target->s != 0) {
      return FALSE;
    }
  }
  return TRUE;
}

/******************************************************************************/
/* private function for caching beighbors, to be used only when it is known  */
/* that the tree exists, and its children exist                              */

void quad_tree_cache_neighbors
(
  quad_tree *target
)
{
  target->nw->e = target->ne;
  target->nw->s = target->sw;
  target->ne->w = target->nw;
  target->ne->s = target->se;
  target->sw->e = target->se;
  target->sw->n = target->nw;
  target->se->w = target->sw;
  target->se->n = target->ne;
  if (target->n != NULL) {
    if (target->n->sw != NULL) {
      target->nw->n = target->n->sw;
      target->n->sw->s = target->nw;
    }
    else {
      target->nw->n = target->n;
    }
    if (target->n->se != NULL) {
      target->ne->n = target->n->se;
      target->n->se->s = target->ne;
    }
    else {
      target->ne->n = target->n;
    }
  }
  if (target->e != NULL) {
    if (target->e->nw != NULL) {
      target->ne->e = target->e->nw;
      target->e->nw->w = target->ne;
    }
    else {
      target->ne->e = target->e;
    }
    if (target->e->sw != NULL) {
      target->se->e = target->e->sw;
      target->e->sw->w = target->se;
    }
    else {
      target->se->e = target->e;
    }
  }
  if (target->s != NULL) {
    if (target->s->nw != NULL) {
      target->sw->s = target->s->nw;
      target->s->nw->n = target->sw;
    }
    else {
      target->sw->s = target->s;
    }
    if (target->s->ne != NULL) {
      target->se->s = target->s->ne;
      target->s->ne->n = target->se;
    }
    else {
      target->se->s = target->s;
    }
  }
  if (target->w != NULL) {
    if (target->w->ne != NULL) {
      target->nw->w = target->w->ne;
      target->w->ne->e = target->nw;
    }
    else {
      target->nw->w = target->w;
    }
    if (target->w->se != NULL) {
      target->sw->w = target->w->se;
      target->w->se->e = target->sw;
    }
    else {
      target->sw->w = target->w;
    }
  }
}

/******************************************************************************/
/* TODO: make a new generic function quad_tree_divide_with_criterion */
/* criterion function probably requires child list as parameter */

result quad_tree_divide
(
  quad_forest *forest,
  quad_tree *target
)
{
  TRY();

  CHECK_POINTER(target);

  if (target->size >= forest->tree_min_size * 2) {
    if (target->nw == NULL) {
      quad_tree children[4];

      CHECK(quad_tree_nullify(&children[0]));
      CHECK(quad_tree_nullify(&children[1]));
      CHECK(quad_tree_nullify(&children[2]));
      CHECK(quad_tree_nullify(&children[3]));

      CHECK(quad_tree_get_child_statistics(forest, target, children));
      {
        quad_tree *child_tree;
        uint32 level;

        level = target->level + 1;

        /* nw child block */
        CHECK(list_append_return_pointer(&forest->trees, (pointer)&children[0], (pointer*)&child_tree));
        /*quad_tree_segment_create(child_tree);*/
        child_tree->level = level;
        child_tree->parent = target;
        target->nw = child_tree;

        /* ne child block */
        CHECK(list_append_return_pointer(&forest->trees, (pointer)&children[1], (pointer*)&child_tree));
        /*quad_tree_segment_create(child_tree);*/
        child_tree->level = level;
        child_tree->parent = target;
        target->ne = child_tree;

        /* sw child block */
        CHECK(list_append_return_pointer(&forest->trees, (pointer)&children[2], (pointer*)&child_tree));
        /*quad_tree_segment_create(child_tree);*/
        child_tree->level = level;
        child_tree->parent = target;
        target->sw = child_tree;

        /* se child block */
        CHECK(list_append_return_pointer(&forest->trees, (pointer)&children[3], (pointer*)&child_tree));
        /*quad_tree_segment_create(child_tree);*/
        child_tree->level = level;
        child_tree->parent = target;
        target->se = child_tree;

        quad_tree_cache_neighbors(target);
      }
    }
  }

  FINALLY(quad_tree_divide);
  RETURN();
}

/******************************************************************************/

truth_value quad_tree_has_children
(
  quad_tree *tree
)
{
  /* there should be no case in which some children would not be set */
  /* the pointers are nullified at init; therefore, check only one */
  if (tree->nw != NULL) {
    return TRUE;
  }
  return FALSE;
}

/******************************************************************************/

result quad_tree_get_child_statistics
(
  quad_forest *forest,
  quad_tree *source,
  quad_tree *target
)
{
  TRY();

  CHECK_POINTER(source);
  CHECK_POINTER(target);

  /* if the tree has already been divided, return the values from the children */
  if (source->nw != NULL) {
    /* nw child block */
    CHECK(memory_copy((void*)&target[0], (void*)source->nw, 1, sizeof(quad_tree)));
    /* ne child block */
    CHECK(memory_copy((void*)&target[1], (void*)source->ne, 1, sizeof(quad_tree)));
    /* sw child block */
    CHECK(memory_copy((void*)&target[2], (void*)source->sw, 1, sizeof(quad_tree)));
    /* se child block */
    CHECK(memory_copy((void*)&target[3], (void*)source->se, 1, sizeof(quad_tree)));
  }
  /* otherwise have to calculate */
  else {
    uint32 x, y, size;
    uint32 step, stride, offset;
    statistics *stat;

    size = (uint32)(source->size / 2);

    target[0].size = target[1].size = target[2].size = target[3].size = size;

    /* if the new width is 1 or 0, no need to calculate, use the pixel values */
    /* size 0 should not happen unless someone tries to divide tree with size 1 */
    if (size < 2) {
      pixel_image *original;
      void *data;
      pixel_type type;
      integral_value mean;

      original = forest->source;
      data = original->data;
      type = original->type;
      step = original->step;
      stride = original->stride;

      /* nw child block */
      x = source->x;
      y = source->y;
      offset = y * stride + x * step;
      mean = cast_pixel_value(original->data, type, offset);

      stat = &target[0].stat;
      stat->N = 1;
      stat->sum = mean;
      stat->sum2 = mean*mean;
      stat->mean = mean;
      stat->variance = 0;
      stat->deviation = 0;

      target[0].x = x;
      target[0].y = y;

      /* ne child block */
      x = x + size;
      offset = offset + size * step;
      mean = cast_pixel_value(original->data, type, offset);

      stat = &target[1].stat;
      stat->N = 1;
      stat->sum = mean;
      stat->sum2 = mean*mean;
      stat->mean = mean;
      stat->variance = 0;
      stat->deviation = 0;

      target[1].x = x;
      target[1].y = y;

      /* se child block */
      y = y + size;
      offset = offset + size * stride;
      mean = cast_pixel_value(original->data, type, offset);

      stat = &target[3].stat;
      stat->N = 1;
      stat->sum = mean;
      stat->sum2 = mean*mean;
      stat->mean = mean;
      stat->variance = 0;
      stat->deviation = 0;

      target[3].x = x;
      target[3].y = y;

      /* sw child block */
      x = x - size;
      offset = offset - size * step;
      mean = cast_pixel_value(original->data, type, offset);

      stat = &target[2].stat;
      stat->N = 1;
      stat->sum = mean;
      stat->sum2 = mean*mean;
      stat->mean = mean;
      stat->variance = 0;
      stat->deviation = 0;

      target[2].x = x;
      target[2].y = y;
    }
    else {
      uint32 hstep, vstep, dstep;
      integral_image *I;
      integral_value *iA, *i2A, N, sum1, sum2, mean, var;

      I = &forest->integral;
      N = (integral_value)(size * size);
      step = I->step;
      stride = I->stride;
      hstep = size * step;
      vstep = size * stride;
      dstep = hstep + vstep;

      /* nw child block */
      x = source->x;
      y = source->y;
      offset = y * stride + x * step;

      iA = ((integral_value *)I->I_1.data) + offset;
      i2A = ((integral_value *)I->I_2.data) + offset;

      sum1 = *(iA + dstep) + *iA - *(iA + hstep) - *(iA + vstep);
      sum2 = *(i2A + dstep) + *i2A - *(i2A + hstep) - *(i2A + vstep);
      mean = sum1 / N;
      var = sum2 / N - mean*mean;
      if (var < 0) var = 0;

      stat = &target[0].stat;
      stat->N = N;
      stat->sum = sum1;
      stat->sum2 = sum2;
      stat->mean = mean;
      stat->variance = var;
      stat->deviation = sqrt(var);

      target[0].x = x;
      target[0].y = y;

      /* ne child block */
      x = x + size;
      iA = iA + hstep;
      i2A = i2A + hstep;

      sum1 = *(iA + dstep) + *iA - *(iA + hstep) - *(iA + vstep);
      sum2 = *(i2A + dstep) + *i2A - *(i2A + hstep) - *(i2A + vstep);
      mean = sum1 / N;
      var = sum2 / N - mean*mean;
      if (var < 0) var = 0;

      stat = &target[1].stat;
      stat->N = N;
      stat->sum = sum1;
      stat->sum2 = sum2;
      stat->mean = mean;
      stat->variance = var;
      stat->deviation = sqrt(var);

      target[1].x = x;
      target[1].y = y;

      /* se child block */
      y = y + size;
      iA = iA + vstep;
      i2A = i2A + vstep;

      sum1 = *(iA + dstep) + *iA - *(iA + hstep) - *(iA + vstep);
      sum2 = *(i2A + dstep) + *i2A - *(i2A + hstep) - *(i2A + vstep);
      mean = sum1 / N;
      var = sum2 / N - mean*mean;
      if (var < 0) var = 0;

      stat = &target[3].stat;
      stat->N = N;
      stat->sum = sum1;
      stat->sum2 = sum2;
      stat->mean = mean;
      stat->variance = var;
      stat->deviation = sqrt(var);

      target[3].x = x;
      target[3].y = y;

      /* sw child block */
      x = x - size;
      iA = iA - hstep;
      i2A = i2A - hstep;

      sum1 = *(iA + dstep) + *iA - *(iA + hstep) - *(iA + vstep);
      sum2 = *(i2A + dstep) + *i2A - *(i2A + hstep) - *(i2A + vstep);
      mean = sum1 / N;
      var = sum2 / N - mean*mean;
      if (var < 0) var = 0;

      stat = &target[2].stat;
      stat->N = N;
      stat->sum = sum1;
      stat->sum2 = sum2;
      stat->mean = mean;
      stat->variance = var;
      stat->deviation = sqrt(var);

      target[2].x = x;
      target[2].y = y;
    }
  }

  FINALLY(quad_tree_get_child_statistics);
  RETURN();
}

/******************************************************************************/

result quad_tree_get_neighborhood_statistics
(
  quad_forest *forest,
  quad_tree *tree,
  statistics *target,
  integral_value multiplier
)
{
  TRY();
  sint32 size, x, y, s;

  CHECK_POINTER(forest);
  CHECK_POINTER(tree);
  CHECK_PARAM(multiplier > 0);

  size = ((sint32)(multiplier * ((integral_value)tree->size)));
  x = ((signed)tree->x) - size;
  y = ((signed)tree->y) - size;
  s = ((signed)tree->size) + 2 * size;

  integral_image_calculate_statistics(&forest->integral, target, x, y, s, s, 0);

  FINALLY(quad_tree_get_neighborhood_statistics);
  RETURN();
}

/******************************************************************************/

result quad_tree_divide_with_overlap
(
  quad_forest *forest,
  quad_tree *target,
  integral_value alpha,
  integral_value overlap_threshold
)
{
  TRY();

  CHECK_POINTER(forest);
  CHECK_POINTER(target);

  if (target->nw == NULL) {
    if (target->size >= forest->tree_min_size * 2) {
      quad_tree children[4];
      uint32 i;
      statistics *stat;
      integral_value m, s, x1, x2, x1min, x1max, x2min, x2max, I, U, overlap;

      CHECK(quad_tree_nullify(&children[0]));
      CHECK(quad_tree_nullify(&children[1]));
      CHECK(quad_tree_nullify(&children[2]));
      CHECK(quad_tree_nullify(&children[3]));

      CHECK(quad_tree_get_child_statistics(forest, target, children));

      stat = &children[0].stat;
      m = stat->mean;
      s = fmax(alpha, alpha * stat->deviation);
      x1min = fmax(0, m - s);
      x1max = x1min;
      x2min = fmin(255, m + s);
      x2max = x2min;
      for (i = 1; i < 4; i++) {
        stat = &children[i].stat;
        m = stat->mean;
        s = fmax(alpha, alpha * stat->deviation);
        x1 = fmax(0, m - s);
        x2 = fmin(255, m + s);
        if (x1 < x1min) x1min = x1;
        else if (x1 > x1max) x1max = x1;
        if (x2 < x2min) x2min = x2;
        else if (x2 > x2max) x2max = x2;
      }

      /* it is possible that intersection is negative, this means an empty set */
      if (x1max > x2min) {
        I = 0;
      }
      else {
        I = (x2min - x1max);
        if (I < 1) I = 1;
      }
      U = (x2max - x1min);
      if (U < 1) U = 1;

      /* let us define this entropy measure as intersection divided by union */
      overlap = I / U;

      /* if union is more than double the intersection, we have high 'entropy' */
      if (overlap < overlap_threshold) {
        quad_tree *child_tree;
        uint32 level;

        level = target->level + 1;

        /* nw child block */
        CHECK(list_append_return_pointer(&forest->trees, (pointer)&children[0], (pointer*)&child_tree));
        /*quad_tree_segment_create(child_tree);*/
        child_tree->parent = target;
        child_tree->level = level;
        target->nw = child_tree;

        /* ne child block */
        CHECK(list_append_return_pointer(&forest->trees, (pointer)&children[1], (pointer*)&child_tree));
        /*quad_tree_segment_create(child_tree);*/
        child_tree->parent = target;
        child_tree->level = level;
        target->ne = child_tree;

        /* sw child block */
        CHECK(list_append_return_pointer(&forest->trees, (pointer)&children[2], (pointer*)&child_tree));
        /*quad_tree_segment_create(child_tree);*/
        child_tree->parent = target;
        child_tree->level = level;
        target->sw = child_tree;

        /* se child block */
        CHECK(list_append_return_pointer(&forest->trees, (pointer)&children[3], (pointer*)&child_tree));
        /*quad_tree_segment_create(child_tree);*/
        child_tree->parent = target;
        child_tree->level = level;
        target->se = child_tree;

        quad_tree_cache_neighbors(target);
      }
      else {
        quad_tree_segment_create(target);
      }
    }
    else {
      quad_tree_segment_create(target);
    }
  }

  FINALLY(quad_tree_divide_with_overlap);
  RETURN();
}

/******************************************************************************/

result quad_tree_get_edge_response
(
  quad_forest *forest,
  quad_tree *tree,
  integral_value *dx,
  integral_value *dy
)
{
  TRY();
  uint32 box_width, box_length, row, col, endrow, endcol;
  sint32 srow, scol;
  integral_value hsum, vsum;
  INTEGRAL_IMAGE_2BOX_VARIABLES();

  CHECK_POINTER(forest);
  CHECK_POINTER(tree);

  box_width = tree->size;
  /* box length should be at least 4 to get proper result */
  box_length = (uint32)(fmin(box_width / 2, 4));

  /* calculate horizontal cumulative gradient */
  {
    INTEGRAL_IMAGE_INIT_HBOX(&forest->integral, box_length, box_width);
    scol = ((signed)tree->x) - ((signed)box_length);
    endcol = ((unsigned)(scol + ((signed)box_width)));
    hsum = 0;
    /*printf("col %lu endcol %lu ", col, endcol);*/
    if (scol >= 0 && endcol + box_width + 1 <= forest->integral.width) {
      col = ((unsigned)scol);
      iA1 = I_1_data + tree->y * stride + col;
      i2A1 = I_2_data + tree->y * stride + col;
      while (col < endcol) {
        sum1 = INTEGRAL_IMAGE_SUM_1();
        sum2 = INTEGRAL_IMAGE_SUM_2();
        sumsqr1 = INTEGRAL_IMAGE_SUMSQR_1();
        sumsqr2 = INTEGRAL_IMAGE_SUMSQR_2();
        g = edgel_fisher_signed(N, sum1, sum2, sumsqr1, sumsqr2);
        hsum += g;
        col++;
        iA1++;
        i2A1++;
      }
    }
    hsum /= ((integral_value)box_width);
    tree->edge.dx = hsum;
    if (dx != NULL) {
      *dx = hsum;
    }
    /*printf("dx %.3f ", hsum);*/
  }
  /* calculate vertical cumulative gradient */
  {
    INTEGRAL_IMAGE_INIT_VBOX(&forest->integral, box_length, box_width);
    srow = ((signed)tree->y) - ((signed)box_length);
    endrow = ((unsigned)(srow + ((signed)box_width)));
    vsum = 0;
    /*printf("row %lu endrow %lu ", row, endrow);*/
    if (srow >= 0 && endrow + box_width + 1 <= forest->integral.height) {
      row = ((unsigned)srow);
      iA1 = I_1_data + row * stride + tree->x;
      i2A1 = I_2_data + row * stride + tree->x;
      while (row < endrow) {
        sum1 = INTEGRAL_IMAGE_SUM_1();
        sum2 = INTEGRAL_IMAGE_SUM_2();
        sumsqr1 = INTEGRAL_IMAGE_SUMSQR_1();
        sumsqr2 = INTEGRAL_IMAGE_SUMSQR_2();
        g = edgel_fisher_signed(N, sum1, sum2, sumsqr1, sumsqr2);
        vsum += g;
        row++;
        iA1 += stride;
        i2A1 += stride;
      }
    }
    vsum /= ((integral_value)box_width);
    tree->edge.dy = vsum;
    if (dy != NULL) {
      *dy = vsum;
    }
    /*printf("dy %.3f ", vsum);*/
  }

  tree->edge.mag = sqrt(hsum*hsum + vsum * vsum);
  tree->edge.ang = atan2(vsum, hsum);

  FINALLY(quad_tree_get_edge_response);
  RETURN();
}

/******************************************************************************/

result quad_tree_get_child_edge_response
(
  quad_forest *forest,
  quad_tree *tree,
  integral_value dx[],
  integral_value dy[]
)
{
  TRY();
  uint32 i, x[4], y[4], box_width, box_length, row, col, endrow, endcol;
  sint32 srow, scol;
  integral_value hsum, vsum;
  INTEGRAL_IMAGE_2BOX_VARIABLES();

  CHECK_POINTER(forest);
  CHECK_POINTER(tree);
  CHECK_POINTER(dx);
  CHECK_POINTER(dy);

  box_width = (uint32)(tree->size / 2);
  /* box length should be at least 4 to get proper result */
  box_length = (uint32)(fmin(box_width / 2, 4));

  x[0] = x[2] = tree->x;
  x[1] = x[3] = tree->x + box_width;
  y[0] = y[1] = tree->y;
  y[2] = y[3] = tree->y + box_width;

  /* calculate horizontal cumulative gradient for all child trees */
  {
    INTEGRAL_IMAGE_INIT_HBOX(&forest->integral, box_length, box_width);
    for (i = 0; i < 4; i++) {
      scol = ((signed)x[i]) - ((signed)box_length);
      endcol = ((unsigned)(scol + ((signed)box_width)));
      hsum = 0;
      /*printf("col %lu endcol %lu ", col, endcol);*/
      if (scol >= 0 && endcol + box_width + 1 <= forest->integral.width) {
        col = ((unsigned)scol);
        iA1 = I_1_data + y[i] * stride + col;
        i2A1 = I_2_data + y[i] * stride + col;
        while (col < endcol) {
          sum1 = INTEGRAL_IMAGE_SUM_1();
          sum2 = INTEGRAL_IMAGE_SUM_2();
          sumsqr1 = INTEGRAL_IMAGE_SUMSQR_1();
          sumsqr2 = INTEGRAL_IMAGE_SUMSQR_2();
          g = edgel_fisher_signed(N, sum1, sum2, sumsqr1, sumsqr2);
          hsum += g;
          col++;
          iA1++;
          i2A1++;
        }
      }
      hsum /= ((integral_value)box_width);
      dx[i] = hsum;
    }
  }
  /* calculate vertical cumulative gradient for all child trees */
  {
    INTEGRAL_IMAGE_INIT_VBOX(&forest->integral, box_length, box_width);
    for (i = 0; i < 4; i++) {
      srow = ((signed)y[i]) - ((signed)box_length);
      endrow = ((unsigned)(srow + ((signed)box_width)));
      vsum = 0;
      /*printf("row %lu endrow %lu ", row, endrow);*/
      if (srow >= 0 && endrow + box_width + 1 <= forest->integral.height) {
        row = ((unsigned)srow);
        iA1 = I_1_data + row * stride + x[i];
        i2A1 = I_2_data + row * stride + x[i];
        while (row < endrow) {
          sum1 = INTEGRAL_IMAGE_SUM_1();
          sum2 = INTEGRAL_IMAGE_SUM_2();
          sumsqr1 = INTEGRAL_IMAGE_SUMSQR_1();
          sumsqr2 = INTEGRAL_IMAGE_SUMSQR_2();
          g = edgel_fisher_signed(N, sum1, sum2, sumsqr1, sumsqr2);
          vsum += g;
          row++;
          iA1 += stride;
          i2A1 += stride;
        }
      }
      vsum /= ((integral_value)box_width);
      dy[i] = vsum;
    }
  }

  FINALLY(quad_tree_get_child_edge_response);
  RETURN();
}

/******************************************************************************/
/* private functions for managing graph propagation algorithms                */

/*******************************************************************************
 * graph propagation is based on
 *  a) accumulator value that holds the current state of the node
 *  b) pool value that is the temporary state, modified by neighbors
 *  c) four neighbors that adjust the node's pool value
 * each node is first primed with an initial value, stored in acc; then each
 * node is given their chance to modify their neighbors' pool values; finally
 * the pool value is accumulated back to the acc value and possibly used in the
 * next round of propagation.
*******************************************************************************/

/* sets the acc value to the current pool value */
void quad_tree_prime_with_pool(quad_tree *tree)
{
  tree->acc = tree->pool / 2;
  tree->pool = tree->acc;
  /* pool value is not squared as it already contains squared values */
  tree->acc2 = tree->pool2 / 2;
  tree->pool2 = tree->acc2;
}

/* sets the acc value to a constant value */
void quad_tree_prime_with_constant(quad_tree *tree, integral_value constant)
{
  tree->acc = constant / 2;
  tree->pool = tree->acc;
  tree->acc2 = constant * tree->acc;
  tree->pool2 = tree->acc2;
}

/* sets the acc value to the edge response magnitude */
void quad_tree_prime_with_mag(quad_tree *tree)
{
  tree->acc = tree->edge.mag / 2;
  tree->pool = tree->acc;
  /* (a * a) / 2 = a * (a / 2) */
  tree->acc2 = tree->edge.mag * tree->acc;
  tree->pool2 = tree->acc2;
}

/* sets the acc value to the dy edge response value */
void quad_tree_prime_with_dy(quad_tree *tree)
{
  tree->acc = tree->edge.dy / 2;
  tree->pool = tree->acc;
  /* (a * a) / 2 = a * (a / 2) */
  tree->acc2 = tree->edge.dy * tree->acc;
  tree->pool2 = tree->acc2;
}

/* sets the acc value to the dy edge response value */
void quad_tree_prime_with_dx(quad_tree *tree)
{
  tree->acc = tree->edge.dx / 2;
  tree->pool = tree->acc;
  /* (a * a) / 2 = a * (a / 2) */
  tree->acc2 = tree->edge.dx * tree->acc;
  tree->pool2 = tree->acc2;
}

/* sets the acc value to acc if tree has edge, 0 otherwise */
void quad_tree_prime_with_edge(quad_tree *tree, integral_value acc)
{
  if (IS_TRUE(tree->edge.has_edge)) {
    tree->acc = acc / 2;
    tree->pool = tree->acc;
    tree->acc2 = acc * tree->acc;
    tree->pool2 = tree->acc2;
  }
  else {
    tree->acc = 0;
    tree->pool = 0;
    tree->acc2 = 0;
    tree->pool2 = 0;
  }
}

/* sets the acc value to horizontal difference */
/* (dy + left and right neighbor dy - top and bottom neighbor dy) */
void quad_tree_prime_with_hdiff(quad_tree *tree)
{
  integral_value acc;
  acc += (tree->w == NULL ? 0 : tree->w->edge.dy);
  acc += (tree->e == NULL ? 0 : tree->e->edge.dy);
  acc -= (tree->n == NULL ? 0 : tree->n->edge.dy);
  acc -= (tree->s == NULL ? 0 : tree->s->edge.dy);
  tree->acc = acc / 2;
  tree->pool = tree->acc;
  tree->acc2 = acc * tree->acc;
  tree->pool2 = tree->acc2;
}

void quad_tree_accumulate(quad_tree *tree)
{
  tree->acc = tree->pool;
  tree->acc2 = tree->pool2;
}

/* propagates one quarter of the acc value to each of the four neighbors */
void quad_tree_propagate(quad_tree *tree)
{
  integral_value pool, pool2;
  /* effectively this is one eighth of original tree value */
  pool = tree->acc / 4;
  pool2 = tree->acc2 / 4;
  /* neighbor n */
  if (tree->n != NULL) {
    tree->n->pool += pool;
    tree->n->pool2 += pool2;
  }
  /* at edge, return back to own pool */
  else {
    tree->pool += pool;
    tree->pool2 += pool;
  }
  /* neighbor e */
  if (tree->e != NULL) {
    tree->e->pool += pool;
    tree->e->pool2 += pool2;
  }
  /* at edge, return back to own pool */
  else {
    tree->pool += pool;
    tree->pool2 += pool2;
  }
  /* neighbor s */
  if (tree->s != NULL) {
    tree->s->pool += pool;
    tree->s->pool2 += pool2;
  }
  /* at edge, return back to own pool */
  else {
    tree->pool += pool;
    tree->pool2 += pool2;
  }
  /* neighbor w */
  if (tree->w != NULL) {
    tree->w->pool += pool;
    tree->w->pool2 += pool2;
  }
  /* at edge, return back to own pool */
  else {
    tree->pool += pool;
    tree->pool2 += pool2;
  }
}

/* propagates only in vertical directions */
void quad_tree_propagate_v(quad_tree *tree)
{
  integral_value pool, pool2;
  /* effectively this is one eighth of original tree value */
  pool = tree->acc / 4;
  pool2 = tree->acc2 / 4;
  /* neighbor n */
  if (tree->n != NULL) {
    tree->n->pool += pool;
    tree->n->pool2 += pool2;
  }
  /* at edge, return back to own pool */
  else {
    tree->pool += pool;
    tree->pool2 += pool;
  }
  /* neighbor s */
  if (tree->s != NULL) {
    tree->s->pool += pool;
    tree->s->pool2 += pool2;
  }
  /* at edge, return back to own pool */
  else {
    tree->pool += pool;
    tree->pool2 += pool2;
  }
}

/* propagate only in horizontal directions */
void quad_tree_propagate_h(quad_tree *tree)
{
  integral_value pool, pool2;
  /* effectively this is one eighth of original tree value */
  pool = tree->acc / 4;
  pool2 = tree->acc2 / 4;
  /* neighbor e */
  if (tree->e != NULL) {
    tree->e->pool += pool;
    tree->e->pool2 += pool2;
  }
  /* at edge, return back to own pool */
  else {
    tree->pool += pool;
    tree->pool2 += pool2;
  }
  /* neighbor w */
  if (tree->w != NULL) {
    tree->w->pool += pool;
    tree->w->pool2 += pool2;
  }
  /* at edge, return back to own pool */
  else {
    tree->pool += pool;
    tree->pool2 += pool2;
  }
}

/******************************************************************************/
/* private function for adding neighboring trees to list                      */

void quad_tree_add_neighbors
(
  list *target,
  quad_tree *tree,
  direction dir
)
{
  /* in the initial run, the direction is set to whole 4-neighborhood */
  if (dir == d_N4)
  {
    /* recurse to all direct neighbors that are not NULL */
    if (tree->n != NULL) {
      quad_tree_add_neighbors(target, tree->n, d_N);
    }
    if (tree->e != NULL) {
      quad_tree_add_neighbors(target, tree->e, d_E);
    }
    if (tree->s != NULL) {
      quad_tree_add_neighbors(target, tree->s, d_S);
    }
    if (tree->w != NULL) {
      quad_tree_add_neighbors(target, tree->w, d_W);
    }
  }
  else {
    /* if the tree does not have children, add it to neighbor list */
    if (tree->nw == NULL) {
      /* using the uniqueness constraint to allow getting neighbors of many trees */
      list_append(target, &tree);
    }
    /* otherwise need to recurse to children in the given direction */
    else {
      switch (dir) {
      case d_N:
        {
          quad_tree_add_neighbors(target, tree->sw, d_N);
          quad_tree_add_neighbors(target, tree->se, d_N);
        }
        break;
      case d_E:
        {
          quad_tree_add_neighbors(target, tree->nw, d_E);
          quad_tree_add_neighbors(target, tree->sw, d_E);
        }
        break;
      case d_S:
        {
          quad_tree_add_neighbors(target, tree->nw, d_S);
          quad_tree_add_neighbors(target, tree->ne, d_S);
        }
        break;
      case d_W:
        {
          quad_tree_add_neighbors(target, tree->ne, d_E);
          quad_tree_add_neighbors(target, tree->se, d_E);
        }
        break;
      default:
        /* should never end up here */
        return;
      }
    }
  }
}

/******************************************************************************/

result quad_tree_get_neighbors
(
  list *target,
  quad_tree *tree
)
{
  TRY();

  CHECK_POINTER(target);
  CHECK_POINTER(tree);

  CHECK(list_create(target, 100, sizeof(quad_tree*), 1));

  quad_tree_add_neighbors(target, tree, d_N4);

  FINALLY(quad_tree_get_neighbors);
  RETURN();
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
/* a private function for initializing quad_forest structure                  */
/* used in create and in reload                                               */

result quad_forest_init
(
  quad_forest *target,
  uint32 tree_max_size,
  uint32 tree_min_size
)
{
  TRY();
  uint32 row, col, rows, cols, pos, size, width, height;
  quad_tree new_tree, *tree_ptr;

  /* not necessary to check target pointer, calling function should handle that */
  width = target->original->width;
  height = target->original->height;

  CHECK_PARAM(tree_max_size <= width && tree_max_size <= height);
  CHECK_PARAM(tree_min_size <= tree_max_size);

  if (target->tree_max_size != tree_max_size || target->tree_min_size != tree_min_size) {
    /* initialize values */
    rows = (uint32)(height / tree_max_size);
    cols = (uint32)(width / tree_max_size);
    target->rows = rows;
    target->cols = cols;
    target->tree_max_size = tree_max_size;
    target->tree_min_size = tree_min_size;
    target->dx = (uint32)((width - (cols * tree_max_size)) / 2);
    target->dy = (uint32)((height - (rows * tree_max_size)) / 2);

    size = rows * cols;
    if (target->roots != NULL) {
      CHECK(memory_deallocate((data_pointer*)&target->roots));
    }
    CHECK(memory_allocate((data_pointer *)&target->roots, size, sizeof(quad_tree*)));

    if (!list_is_null(&target->trees)) {
      CHECK(list_destroy(&target->trees));
    }
    CHECK(list_create(&target->trees, 8 * size, sizeof(quad_tree), 1));
  }
  else {
    rows = target->rows;
    cols = target->cols;
    size = rows * cols;
    /* TODO: need to determine also max tree depth -> max number of tree nodes */
  }

  if (target->source == NULL) {
    target->source = pixel_image_alloc();
    CHECK_POINTER(target->source);
    CHECK(pixel_image_create(target->source, p_U8, GREY, width, height, 1, width));
  }

  /*printf("create integral image\n");*/
  if (integral_image_is_null(&target->integral)) {
    CHECK(integral_image_create(&target->integral, target->source));
  }

  CHECK(list_clear(&target->trees));

  /* create tree roots and their trees and blocks */
  /* TODO: init value only once */
  quad_tree_nullify(&new_tree);
  new_tree.size = tree_max_size;
  for (row = 0, pos = 0; row < rows; row++) {
    new_tree.y = (uint32)(target->dy + row * tree_max_size);
    new_tree.x = (uint32)(target->dx);
    for (col = 0; col < cols; col++, pos++, new_tree.x += tree_max_size) {
      CHECK(list_append_return_pointer(&target->trees, (pointer)&new_tree, (pointer*)&tree_ptr));
      target->roots[pos] = tree_ptr;
    }
  }
  target->last_root_tree = target->trees.last.prev;

  /* add neighbors to roots */
  for (row = 0, pos = 0; row < rows; row++) {
    for (col = 0; col < cols; col++, pos++) {
      /* add neighbor to west */
      if (col > 0) {
        target->roots[pos]->w = target->roots[pos - 1];
      }
      /* add neighbor to north */
      if (row > 0) {
        target->roots[pos]->n = target->roots[pos - cols];
      }
      /* add neighbor to east */
      if (col < (unsigned)(cols - 1)) {
        target->roots[pos]->e = target->roots[pos + 1];
      }
      /* add neighbor to south */
      if (row < (unsigned)(rows - 1)) {
        target->roots[pos]->s = target->roots[pos + cols];
      }
    }
  }

  FINALLY(quad_forest_init);
  RETURN();
}

/******************************************************************************/

quad_forest *quad_forest_alloc()
{
  TRY();
  quad_forest *forest;

  CHECK(memory_allocate((data_pointer *)&forest, 1, sizeof(quad_forest)));
  CHECK(quad_forest_nullify(forest));

  FINALLY(quad_forest_alloc);
  return forest;
}

/******************************************************************************/

void quad_forest_free
(
  quad_forest *forest
)
{
  TRY();

  r = SUCCESS;
  if (forest != NULL) {
    CHECK(quad_forest_destroy(forest));
    CHECK(memory_deallocate((data_pointer *)&forest));
  }

  FINALLY(quad_forest_free);
}

/******************************************************************************/

result quad_forest_create
(
  quad_forest *target,
  pixel_image *source,
  uint32 tree_max_size,
  uint32 tree_min_size
)
{
  TRY();

  CHECK_POINTER(target);

  CHECK_FALSE(pixel_image_is_null(source));

  CHECK_PARAM(source->type == p_U8);
  CHECK_PARAM(source->format == GREY);

  /* nullify so the values can be checked and set in the init function */
  CHECK(quad_forest_nullify(target));

  target->original = source;

  CHECK(quad_forest_init(target, tree_max_size, tree_min_size));

  FINALLY(quad_forest_create);
  RETURN();
}

/******************************************************************************/

result quad_forest_reload
(
  quad_forest *target,
  uint32 tree_max_size,
  uint32 tree_min_size
)
{
  TRY();

  CHECK_POINTER(target);
  CHECK_POINTER(target->original);

  if (target->tree_max_size != tree_max_size || target->tree_min_size != tree_min_size) {
    quad_forest_init(target, tree_max_size, tree_min_size);
  }

  FINALLY(quad_forest_reload);
  RETURN();
}

/******************************************************************************/

result quad_forest_refresh_segments
(
  quad_forest *target
)
{
  TRY();
  list_item *trees, *end;
  quad_tree *tree;
  quad_forest_segment *parent, *segment;
  uint32 count;

  count = 0;
  /* initialize the random number generator for assigning the colors */
  srand(1234);

  trees = target->trees.first.next;
  end = &target->trees.last;
  while (trees != end) {
    tree = (quad_tree *)trees->data;
    if (tree->nw == NULL) {
      segment = &tree->segment;
      if (segment != NULL) {
        parent = quad_tree_segment_find(tree);
        if (parent == segment) {
          segment->color[0] = (byte)(rand() % 256);
          segment->color[1] = (byte)(rand() % 256);
          segment->color[2] = (byte)(rand() % 256);
          count++;
        }
      }
    }
    trees = trees->next;
  }
  target->segments = count;

  FINALLY(quad_forest_refresh_segments);
  RETURN();
}

/******************************************************************************/

result quad_forest_destroy
(
  quad_forest *target
)
{
  TRY();

  CHECK_POINTER(target);

  CHECK(list_destroy(&target->trees));
  CHECK(memory_deallocate((data_pointer*)&target->roots));
  CHECK(integral_image_destroy(&target->integral));
  pixel_image_free(target->source);
  CHECK(quad_forest_nullify(target));

  FINALLY(quad_forest_destroy);
  RETURN();
}

/******************************************************************************/

result quad_forest_nullify
(
  quad_forest *target
)
{
  TRY();

  CHECK_POINTER(target);

  target->original = NULL;
  target->source = NULL;
  CHECK(integral_image_nullify(&target->integral));
  target->rows = 0;
  target->cols = 0;
  target->segments = 0;
  target->tree_max_size = 0;
  target->tree_min_size = 0;
  target->dx = 0;
  target->dy = 0;
  CHECK(list_nullify(&target->trees));
  target->last_root_tree = NULL;
  target->roots = NULL;

  FINALLY(quad_forest_nullify);
  RETURN();
}

/******************************************************************************/

truth_value quad_forest_is_null
(
  quad_forest *target
)
{
  if (target != NULL) {
    if (target->original == NULL && target->source == NULL) {
      return TRUE;
    }
  }
  return FALSE;
}

/******************************************************************************/

result quad_forest_update
(
  quad_forest *target
)
{
  TRY();
  uint32 row, col, rows, cols, pos, size, step, stride, hstep, vstep, dstep, offset;
  quad_tree *tree;
  integral_image *I;
  statistics *stat;
  integral_value *iA, *i2A, N, sum1, sum2, mean, var;

  size = target->tree_max_size;
  I = &target->integral;
  N = (integral_value)(size * size);
  step = I->step;
  stride = I->stride;
  hstep = size * step;
  vstep = size * stride;
  dstep = hstep + vstep;

  /* create a fresh copy of the source image in case it has changed */
  CHECK(pixel_image_copy(target->source, target->original));
  CHECK(integral_image_update(&target->integral));
  /* if there are existing child nodes and blocks, remove them */

  CHECK(list_remove_rest(&target->trees, target->last_root_tree));

  rows = target->rows;
  cols = target->cols;
  pos = 0;
  for (row = 0; row < rows; row++) {
    tree = target->roots[pos];
    /* TODO: calculate offset for first row only, then add vstep */
    offset = (tree->y * stride) + (tree->x * step);
    for (col = 0; col < cols; col++, pos++, offset += hstep) {

      iA = ((integral_value *)I->I_1.data) + offset;
      i2A = ((integral_value *)I->I_2.data) + offset;

      sum1 = *(iA + dstep) + *iA - *(iA + hstep) - *(iA + vstep);
      sum2 = *(i2A + dstep) + *i2A - *(i2A + hstep) - *(i2A + vstep);
      mean = sum1 / N;
      var = sum2 / N - mean*mean;
      if (var < 0) var = 0;

      tree = target->roots[pos];
      stat = &tree->stat;

      stat->N = N;
      stat->sum = sum1;
      stat->sum2 = sum2;
      stat->mean = mean;
      stat->variance = var;
      stat->deviation = sqrt(var);

      /* TODO: decide where the segments are created */
      /*quad_tree_segment_create(tree);*/
      tree->nw = NULL;
      tree->ne = NULL;
      tree->sw = NULL;
      tree->se = NULL;
    }
  }

  FINALLY(quad_forest_update);
  RETURN();
}

/******************************************************************************/

/* a macro for calculating neighbor difference; neighbor should be statistics pointer */
#define EVALUATE_NEIGHBOR_DEVIATION(neighbor)\
  stat = neighbor;\
  nm = stat->mean;\
  dist = fabs(tm - nm)

result quad_forest_segment_with_deviation
(
  quad_forest *target,
  integral_value threshold,
  integral_value alpha
)
{
  TRY();
  list_item *trees, *end;
  quad_tree *tree, *neighbor, *best_neighbor;
  quad_forest_segment *tree_segment, *neighbor_segment;
  statistics *stat;
  integral_value tm, nm, dist, best_dist;
  uint32 min_size;

  CHECK_POINTER(target);
  CHECK_PARAM(threshold > 0);
  CHECK_PARAM(alpha > 0);

  min_size = target->tree_min_size;

  /* first, divide until all trees are consistent */
  trees = target->trees.first.next;
  end = &target->trees.last;
  while (trees != end) {
    tree = (quad_tree *)trees->data;
    if (tree->size >= 2 * min_size) {
      if (tree->stat.deviation > threshold) {
        CHECK(quad_tree_divide(target, tree));
      }
      else {
        quad_tree_segment_create(tree);
      }
    }
    else {
      quad_tree_segment_create(tree);
    }
    trees = trees->next;
  }

  /* then, make a union of those neighboring regions that are consistent together */
  /*printf("starting to merge trees\n");*/
  trees = target->trees.first.next;
  while (trees != &target->trees.last) {
    tree = (quad_tree *)trees->data;
    tree_segment = quad_tree_segment_find(tree);
    /* only consider consistent trees (those that have not been divided) */
    if (tree->nw == NULL) {
      stat = &tree->stat;
      tm = stat->mean;

      best_dist = 255;
      best_neighbor = NULL;
      /* neighbor n */
      neighbor = tree->n;
      if (neighbor != NULL && neighbor->nw == NULL) {
        neighbor_segment = quad_tree_segment_find(neighbor);
        if (tree_segment != neighbor_segment) {
          EVALUATE_NEIGHBOR_DEVIATION(&neighbor->stat);
          if (dist < best_dist) {
            best_dist = dist;
            best_neighbor = neighbor;
          }
        }
      }
      /* neighbor e */
      neighbor = tree->e;
      if (neighbor != NULL && neighbor->nw == NULL) {
        neighbor_segment = quad_tree_segment_find(neighbor);
        if (tree_segment != neighbor_segment) {
          EVALUATE_NEIGHBOR_DEVIATION(&neighbor->stat);
          if (dist < best_dist) {
            best_dist = dist;
            best_neighbor = neighbor;
          }
        }
      }
      /* neighbor s */
      neighbor = tree->s;
      if (neighbor != NULL && neighbor->nw == NULL) {
        neighbor_segment = quad_tree_segment_find(neighbor);
        if (tree_segment != neighbor_segment) {
          EVALUATE_NEIGHBOR_DEVIATION(&neighbor->stat);
          if (dist < best_dist) {
            best_dist = dist;
            best_neighbor = neighbor;
          }
        }
      }
      /* neighbor w */
      neighbor = tree->w;
      if (neighbor != NULL && neighbor->nw == NULL) {
        neighbor_segment = quad_tree_segment_find(neighbor);
        if (tree_segment != neighbor_segment) {
          EVALUATE_NEIGHBOR_DEVIATION(&neighbor->stat);
          if (dist < best_dist) {
            best_dist = dist;
            best_neighbor = neighbor;
          }
        }
      }

      if (best_dist < alpha * threshold) {
        quad_tree_segment_union(tree, best_neighbor);
      }
    }
    trees = trees->next;
  }

  /* then, merge those neighboring regions that are consistent together */
  /*printf("starting to merge regions\n");*/
  trees = target->trees.first.next;
  while (trees != &target->trees.last) {
    tree = (quad_tree *)trees->data;
    tree_segment = quad_tree_segment_find(tree);
    /* only consider consistent trees (those that have not been divided) */
    if (tree->nw == NULL) {
      stat = &tree_segment->stat;
      tm = stat->mean;

      /* neighbor n */
      neighbor = tree->n;
      if (neighbor != NULL && neighbor->nw == NULL) {
        neighbor_segment = quad_tree_segment_find(neighbor);
        if (tree_segment != neighbor_segment) {
          EVALUATE_NEIGHBOR_DEVIATION(&neighbor_segment->stat);
          if (dist < alpha * threshold) {
            quad_tree_segment_union(tree, neighbor);
          }
        }
      }
      /* neighbor e */
      neighbor = tree->e;
      if (neighbor != NULL && neighbor->nw == NULL) {
        neighbor_segment = quad_tree_segment_find(neighbor);
        if (tree_segment != neighbor_segment) {
          EVALUATE_NEIGHBOR_DEVIATION(&neighbor_segment->stat);
          if (dist < alpha * threshold) {
            quad_tree_segment_union(tree, neighbor);
          }
        }
      }
      /* neighbor s */
      neighbor = tree->s;
      if (neighbor != NULL && neighbor->nw == NULL) {
        neighbor_segment = quad_tree_segment_find(neighbor);
        if (tree_segment != neighbor_segment) {
          EVALUATE_NEIGHBOR_DEVIATION(&neighbor_segment->stat);
          if (dist < alpha * threshold) {
            quad_tree_segment_union(tree, neighbor);
          }
        }
      }
      /* neighbor w */
      neighbor = tree->w;
      if (neighbor != NULL && neighbor->nw == NULL) {
        neighbor_segment = quad_tree_segment_find(neighbor);
        if (tree_segment != neighbor_segment) {
          EVALUATE_NEIGHBOR_DEVIATION(&neighbor_segment->stat);
          if (dist < alpha * threshold) {
            quad_tree_segment_union(tree, neighbor);
          }
        }
      }
    }
    trees = trees->next;
  }

  /* finally, count regions and assign colors */
  CHECK(quad_forest_refresh_segments(target));

  FINALLY(quad_forest_segment_with_deviation);
  RETURN();
}

/******************************************************************************/

/* a macro for calculating neighbor overlap; neighbor should be statistics pointer */
#define EVALUATE_NEIGHBOR_OVERLAP(neighbor)\
  stat = neighbor;\
  nm = stat->mean;\
  ns = fmax(alpha, alpha * stat->deviation);\
  x1min = fmax(0, tm - ts);\
  x1max = x1min;\
  x2min = fmin(255, tm + ts);\
  x2max = x2min;\
  x1 = fmax(0, nm - ns);\
  x2 = fmin(255, nm + ns);\
  if (x1 < x1min) x1min = x1; else x1max = x1;\
  if (x2 < x2min) x2min = x2; else x2max = x2;\
  if (x1max > x2min) {\
    I = 0;\
  }\
  else {\
    I = (x2min - x1max);\
    if (I < 1) I = 1;\
  }\
  U = (x2max - x1min);\
  if (U < 1) U = 1;\
  overlap = I / U

result quad_forest_segment_with_overlap
(
  quad_forest *target,
  integral_value alpha,
  integral_value threshold_trees,
  integral_value threshold_segments
)
{
  TRY();
  list_item *trees, *end;
  quad_tree *tree, *neighbor, *best_neighbor;
  quad_forest_segment *tree_segment, *neighbor_segment;
  statistics *stat;
  integral_value tm, ts, nm, ns, x1, x2, x1min, x1max, x2min, x2max, I, U, overlap, best_overlap;

  CHECK_POINTER(target);
  CHECK_PARAM(alpha > 0);
  CHECK_PARAM(threshold_trees > 0);
  CHECK_PARAM(threshold_segments > 0);

  /* first, divide until all trees are consistent */
  /*printf("starting to divide trees\n");*/
  trees = target->trees.first.next;
  while (trees != &target->trees.last) {
    tree = (quad_tree *)trees->data;
    CHECK(quad_tree_divide_with_overlap(target, tree, alpha, threshold_trees));
    trees = trees->next;
  }

  /* then, merge each tree with the best neighboring tree that is close enough */
  /*printf("starting to merge trees\n");*/
  trees = target->trees.first.next;
  end = &target->trees.last;
  while (trees != end) {
    tree = (quad_tree *)trees->data;
    tree_segment = quad_tree_segment_find(tree);
    /* only consider consistent trees (those that have not been divided) */
    if (tree->nw == NULL) {
      stat = &tree->stat;
      tm = stat->mean;
      ts = fmax(alpha, alpha * stat->deviation);

      best_overlap = 0;
      best_neighbor = NULL;
      /* neighbor n */
      neighbor = tree->n;
      if (neighbor != NULL && neighbor->nw == NULL) {
        neighbor_segment = quad_tree_segment_find(neighbor);
        if (tree_segment != neighbor_segment) {
          EVALUATE_NEIGHBOR_OVERLAP(&neighbor->stat);
          if (overlap > best_overlap) {
            best_overlap = overlap;
            best_neighbor = neighbor;
          }
        }
      }
      /* neighbor e */
      neighbor = tree->e;
      if (neighbor != NULL && neighbor->nw == NULL) {
        neighbor_segment = quad_tree_segment_find(neighbor);
        if (tree_segment != neighbor_segment) {
          EVALUATE_NEIGHBOR_OVERLAP(&neighbor->stat);
          if (overlap > best_overlap) {
            best_overlap = overlap;
            best_neighbor = neighbor;
          }
        }
      }
      /* neighbor s */
      neighbor = tree->s;
      if (neighbor != NULL && neighbor->nw == NULL) {
        neighbor_segment = quad_tree_segment_find(neighbor);
        if (tree_segment != neighbor_segment) {
          EVALUATE_NEIGHBOR_OVERLAP(&neighbor->stat);
          if (overlap > best_overlap) {
            best_overlap = overlap;
            best_neighbor = neighbor;
          }
        }
      }
      /* neighbor w */
      neighbor = tree->w;
      if (neighbor != NULL && neighbor->nw == NULL) {
        neighbor_segment = quad_tree_segment_find(neighbor);
        if (tree_segment != neighbor_segment) {
          EVALUATE_NEIGHBOR_OVERLAP(&neighbor->stat);
          if (overlap > best_overlap) {
            best_overlap = overlap;
            best_neighbor = neighbor;
          }
        }
      }

      if (best_overlap > threshold_trees) {
        quad_tree_segment_union(tree, best_neighbor);
      }
    }
    trees = trees->next;
  }

  /* then, merge those neighboring regions that are consistent together */
  /*printf("starting to merge regions\n");*/
  trees = target->trees.first.next;
  end = &target->trees.last;
  while (trees != end) {
    tree = (quad_tree *)trees->data;
    tree_segment = quad_tree_segment_find(tree);
    /* only consider consistent trees (those that have not been divided) */
    if (tree->nw == NULL) {
      stat = &tree_segment->stat;
      tm = stat->mean;
      ts = fmax(alpha, alpha * stat->deviation);

      /* neighbor n */
      neighbor = tree->n;
      if (neighbor != NULL && neighbor->nw == NULL) {
        neighbor_segment = quad_tree_segment_find(neighbor);
        if (tree_segment != neighbor_segment) {
          EVALUATE_NEIGHBOR_OVERLAP(&neighbor_segment->stat);
          if (overlap > threshold_segments) {
            quad_tree_segment_union(tree, neighbor);
          }
        }
      }
      /* neighbor e */
      neighbor = tree->e;
      if (neighbor != NULL && neighbor->nw == NULL) {
        neighbor_segment = quad_tree_segment_find(neighbor);
        if (tree_segment != neighbor_segment) {
          EVALUATE_NEIGHBOR_OVERLAP(&neighbor_segment->stat);
          if (overlap > threshold_segments) {
            quad_tree_segment_union(tree, neighbor);
          }
        }
      }
      /* neighbor s */
      neighbor = tree->s;
      if (neighbor != NULL && neighbor->nw == NULL) {
        neighbor_segment = quad_tree_segment_find(neighbor);
        if (tree_segment != neighbor_segment) {
          EVALUATE_NEIGHBOR_OVERLAP(&neighbor_segment->stat);
          if (overlap > threshold_segments) {
            quad_tree_segment_union(tree, neighbor);
          }
        }
      }
      /* neighbor w */
      neighbor = tree->w;
      if (neighbor != NULL && neighbor->nw == NULL) {
        neighbor_segment = quad_tree_segment_find(neighbor);
        if (tree_segment != neighbor_segment) {
          EVALUATE_NEIGHBOR_OVERLAP(&neighbor_segment->stat);
          if (overlap > threshold_segments) {
            quad_tree_segment_union(tree, neighbor);
          }
        }
      }
    }
    trees = trees->next;
  }

  /* finally, count regions and assign colors */
  CHECK(quad_forest_refresh_segments(target));

  FINALLY(quad_forest_segment_with_overlap);
  RETURN();
}

/******************************************************************************/

result quad_forest_get_segments
(
  quad_forest *source,
  quad_forest_segment **target
)
{
  TRY();
  list_item *trees, *end;
  quad_tree *tree;
  quad_forest_segment *segment, *parent;
  uint32 count;

  CHECK_POINTER(source);
  CHECK_POINTER(target);

  if (source->segments == 0) {
    TERMINATE(SUCCESS);
  }

  /* collect all segments to the array */
  count = 0;
  trees = source->trees.first.next;
  end = &source->trees.last;
  while (trees != end) {
    tree = (quad_tree *)trees->data;
    if (tree->nw == NULL) {
      segment = &tree->segment;
      parent = quad_tree_segment_find(tree);
      if (parent == segment) {
        target[count] = segment;
        count++;
      }
    }
    trees = trees->next;
  }

  FINALLY(quad_forest_get_segments);
  RETURN();
}

/******************************************************************************/
/* a private function for collecting trees that belong to one of the segments */

void quad_forest_collect_trees
(
  quad_tree *tree,
  list *target,
  quad_forest_segment **segments,
  uint32 segment_count
)
{
  /* if the tree has children, recurse */
  if (tree->nw != NULL) {
    quad_forest_collect_trees(tree->nw, target, segments, segment_count);
    quad_forest_collect_trees(tree->ne, target, segments, segment_count);
    quad_forest_collect_trees(tree->sw, target, segments, segment_count);
    quad_forest_collect_trees(tree->se, target, segments, segment_count);
  }
  else {
    uint32 i;
    quad_forest_segment *segment;

    segment = quad_tree_segment_find(tree);

    /* loop through all segments, if the tree belongs to one of them, add to list */
    for (i = 0; i < segment_count; i++) {
      if (segment == segments[i]) {
        list_append(target, &tree);
        break;
      }
    }
  }
}

/******************************************************************************/

result quad_forest_get_segment_trees
(
  list *target,
  quad_forest *forest,
  quad_forest_segment **segments,
  uint32 segment_count
)
{
  TRY();
  uint32 i, x1, y1, x2, y2;
  uint32 pos, col, firstcol, lastcol, row, firstrow, lastrow;

  CHECK_POINTER(forest);
  CHECK_POINTER(target);
  CHECK_POINTER(segments);

  if (segment_count == 0) {
    TERMINATE(SUCCESS);
  }

  CHECK(list_create(target, 100, sizeof(quad_tree*), 1));

  /* find bounding box of the collection */
  x1 = segments[0]->x1;
  y1 = segments[0]->y1;
  x2 = segments[0]->x2;
  y2 = segments[0]->y2;
  for (i = 1; i < segment_count; i++) {
    if (segments[i]->x1 < x1) x1 = segments[i]->x1;
    if (segments[i]->y1 < y1) y1 = segments[i]->y1;
    if (segments[i]->x2 > x2) x2 = segments[i]->x2;
    if (segments[i]->y2 > y2) y2 = segments[i]->y2;
  }

  /* determine which root trees are within the bounding box */
  firstcol = (uint32)((x1 - forest->dx) / forest->tree_max_size);
  lastcol = (uint32)((x2 - forest->dx) / forest->tree_max_size);
  firstrow = (uint32)((y1 - forest->dy) / forest->tree_max_size);
  lastrow = (uint32)((y2 - forest->dy) / forest->tree_max_size);

  /* loop through the root trees */
  for (row = firstrow; row <= lastrow; row++) {
    pos = row * forest->cols + firstcol;
    for (col = firstcol; col <= lastcol; col++) {
      quad_forest_collect_trees(forest->roots[pos], target, segments, segment_count);
      pos++;
    }
  }

  FINALLY(quad_forest_get_segment_trees);
  RETURN();
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

/******************************************************************************/
/* private function for adding neighboring segments to list                   */

void quad_tree_add_neighbor_segments
(
  list *target,
  quad_tree *tree,
  quad_forest_segment **segments,
  uint32 segment_count,
  direction dir
)
{
  /* in the initial run, the direction is set to whole 4-neighborhood */
  if (dir == d_N4) {
    /* recurse to all direct neighbors that are not NULL */
    if (tree->n != NULL) {
      quad_tree_add_neighbor_segments(target, tree->n, segments, segment_count, d_N);
    }
    if (tree->e != NULL) {
      quad_tree_add_neighbor_segments(target, tree->e, segments, segment_count, d_E);
    }
    if (tree->s != NULL) {
      quad_tree_add_neighbor_segments(target, tree->s, segments, segment_count, d_S);
    }
    if (tree->w != NULL) {
      quad_tree_add_neighbor_segments(target, tree->w, segments, segment_count, d_W);
    }
  }
  else {
    /* if the tree does not have children, check the segments and add */
    if (tree->nw == NULL) {
      uint32 i;
      quad_forest_segment *segment;

      segment = quad_tree_segment_find(tree);
      if (segment != NULL) {
        /* loop through all segments, if the tree doesn't belong to any of them, add to list */
        for (i = 0; i < segment_count; i++) {
          if (segment == segments[i]) {
            goto found;
          }
        }
        /* segment was not found, it is a neighboring segment */
        list_insert_unique(target, &segment, &compare_segments);
      }
      found: ;
      /* otherwise the neighbor belongs to same collection of segments, don't add it */
    }
    /* otherwise need to recurse to children in the given direction */
    else {
      switch (dir) {
      case d_N:
        {
          quad_tree_add_neighbor_segments(target, tree->sw, segments, segment_count, d_N);
          quad_tree_add_neighbor_segments(target, tree->se, segments, segment_count, d_N);
        }
        break;
      case d_E:
        {
          quad_tree_add_neighbor_segments(target, tree->nw, segments, segment_count, d_E);
          quad_tree_add_neighbor_segments(target, tree->sw, segments, segment_count, d_E);
        }
        break;
      case d_S:
        {
          quad_tree_add_neighbor_segments(target, tree->nw, segments, segment_count, d_S);
          quad_tree_add_neighbor_segments(target, tree->ne, segments, segment_count, d_S);
        }
        break;
      case d_W:
        {
          quad_tree_add_neighbor_segments(target, tree->ne, segments, segment_count, d_E);
          quad_tree_add_neighbor_segments(target, tree->se, segments, segment_count, d_E);
        }
        break;
      default:
        /* should never end up here */
        return;
      }
    }
  }
}

/******************************************************************************/

result quad_forest_get_segment_neighbors
(
  list *target,
  quad_forest *forest,
  quad_forest_segment **segments,
  uint32 segment_count
)
{
  TRY();
  list tree_list;
  list_item *trees;
  quad_tree *tree;

  CHECK_POINTER(forest);
  CHECK_POINTER(target);
  CHECK_POINTER(segments);

  list_nullify(&tree_list);

  if (segment_count == 0) {
    TERMINATE(SUCCESS);
  }

  CHECK(list_create(target, 100, sizeof(quad_forest_segment*), 1));

  CHECK(quad_forest_get_segment_trees(&tree_list, forest, segments, segment_count));

  trees = tree_list.first.next;
  while (trees != &tree_list.last) {
    tree = *((quad_tree **)trees->data);
    quad_tree_add_neighbor_segments(target, tree, segments, segment_count, d_N4);
    trees = trees->next;
  }

  FINALLY(quad_forest_get_segment_neighbors);
  if (IS_FALSE(list_is_null(&tree_list))) {
    list_destroy(&tree_list);
  }
  RETURN();
}

/******************************************************************************/
/* private function for drawing recursively the trees into the bitmask        */

void quad_forest_draw_segments
(
  quad_tree *tree,
  pixel_image *target,
  uint32 dx,
  uint32 dy,
  quad_forest_segment **segments,
  uint32 segment_count,
  byte color[4],
  uint32 channels
)
{
  if (tree->nw != NULL) {
    quad_forest_draw_segments(tree->nw, target, dx, dy, segments, segment_count, color, channels);
    quad_forest_draw_segments(tree->ne, target, dx, dy, segments, segment_count, color, channels);
    quad_forest_draw_segments(tree->sw, target, dx, dy, segments, segment_count, color, channels);
    quad_forest_draw_segments(tree->se, target, dx, dy, segments, segment_count, color, channels);
  }
  else {
    uint32 i, x, y, width, height, row_step;
    byte *target_pos;
    quad_forest_segment *segment;

    segment = quad_tree_segment_find(tree);

    /* loop through all segments, if the tree belongs to one of them, draw the pixels */
    if (channels == 1) {
      for (i = 0; i < segment_count; i++) {
        if (segment == segments[i]) {
          width = tree->size;
          height = width;
          row_step = target->stride - width;
          target_pos = (byte*)target->data + (tree->y - dy) * target->stride + (tree->x - dx);
          for (y = 0; y < height; y++, target_pos += row_step) {
            for (x = 0; x < width; x++) {
              *target_pos = color[0];
              target_pos++;
            }
          }
          break;
        }
      }
    }
    else
    if (channels == 3) {
      for (i = 0; i < segment_count; i++) {
        if (segment == segments[i]) {
          width = tree->size;
          height = width;
          row_step = target->stride - width * target->step;
          target_pos = (byte*)target->data + (tree->y - dy) * target->stride + (tree->x - dx) * target->step;
          for (y = 0; y < height; y++, target_pos += row_step) {
            for (x = 0; x < width; x++) {
              *target_pos = color[0];
              target_pos++;
              *target_pos = color[1];
              target_pos++;
              *target_pos = color[2];
              target_pos++;
            }
          }
          break;
        }
      }
    }
  }
}

/******************************************************************************/

result quad_forest_get_segment_mask
(
  quad_forest *forest,
  pixel_image *target,
  quad_forest_segment **segments,
  uint32 segment_count,
  truth_value invert
)
{
  TRY();
  uint32 i, x1, y1, x2, y2, width, height;
  byte value[4];

  CHECK_POINTER(forest);
  CHECK_POINTER(target);
  CHECK_POINTER(segments);

  if (segment_count == 0) {
    TERMINATE(SUCCESS);
  }

  /*
  loop all root trees under the segment bounding box
  recurse to leaf trees
  if parent is one of the segments, draw pixels to image
  */
  /* find bounding box of the collection */
  x1 = segments[0]->x1;
  y1 = segments[0]->y1;
  x2 = segments[0]->x2;
  y2 = segments[0]->y2;
  for (i = 1; i < segment_count; i++) {
    if (segments[i]->x1 < x1) x1 = segments[i]->x1;
    if (segments[i]->y1 < y1) y1 = segments[i]->y1;
    if (segments[i]->x2 > x2) x2 = segments[i]->x2;
    if (segments[i]->y2 > y2) y2 = segments[i]->y2;
  }

  /* create the image */
  width = x2 - x1;
  height = y2 - y1;
  CHECK(pixel_image_create(target, p_U8, GREY, width, height, 1, width));

  if (IS_FALSE(invert)) {
    CHECK(pixel_image_clear(target));
    value[0] = 255;
  }
  else {
    SINGLE_CONTINUOUS_IMAGE_VARIABLES(target, byte);
    FOR_CONTINUOUS_IMAGE(target)
      PIXEL_VALUE(target) = 255;
    value[0] = 0;
  }

  {
    uint32 pos, col, firstcol, lastcol, row, firstrow, lastrow;

    /* determine which root trees are within the bounding box */
    firstcol = (uint32)((x1 - forest->dx) / forest->tree_max_size);
    lastcol = (uint32)((x2 - forest->dx) / forest->tree_max_size);
    firstrow = (uint32)((y1 - forest->dy) / forest->tree_max_size);
    lastrow = (uint32)((y2 - forest->dy) / forest->tree_max_size);

    /* loop through the root trees */
    for (row = firstrow; row <= lastrow; row++) {
      pos = row * forest->cols + firstcol;
      for (col = firstcol; col <= lastcol; col++) {
        quad_forest_draw_segments(forest->roots[pos], target, x1, y1, segments, segment_count, value, 1);
        pos++;
      }
    }
  }

  FINALLY(quad_forest_get_segment_mask);
  RETURN();
}

/******************************************************************************/

result quad_forest_highlight_segments
(
  quad_forest *forest,
  pixel_image *target,
  quad_forest_segment **segments,
  uint32 segment_count,
  byte color[4]
)
{
  TRY();
  uint32 i, x1, y1, x2, y2;

  CHECK_POINTER(forest);
  CHECK_POINTER(target);
  CHECK_POINTER(segments);

  if (segment_count == 0) {
    TERMINATE(SUCCESS);
  }

  /* find bounding box of the collection */
  x1 = segments[0]->x1;
  y1 = segments[0]->y1;
  x2 = segments[0]->x2;
  y2 = segments[0]->y2;
  for (i = 1; i < segment_count; i++) {
    if (segments[i]->x1 < x1) x1 = segments[i]->x1;
    if (segments[i]->y1 < y1) y1 = segments[i]->y1;
    if (segments[i]->x2 > x2) x2 = segments[i]->x2;
    if (segments[i]->y2 > y2) y2 = segments[i]->y2;
  }

  {
    uint32 pos, col, firstcol, lastcol, row, firstrow, lastrow;

    /* determine which root trees are within the bounding box */
    firstcol = (uint32)((x1 - forest->dx) / forest->tree_max_size);
    lastcol = (uint32)((x2 - forest->dx) / forest->tree_max_size);
    firstrow = (uint32)((y1 - forest->dy) / forest->tree_max_size);
    lastrow = (uint32)((y2 - forest->dy) / forest->tree_max_size);

    /* loop through the root trees */
    for (row = firstrow; row <= lastrow; row++) {
      pos = row * forest->cols + firstcol;
      for (col = firstcol; col <= lastcol; col++) {
        /*printf("(%lu %lu) ", row, col);*/
        quad_forest_draw_segments(forest->roots[pos], target, 0, 0, segments, segment_count, color, 3);
        pos++;
      }
    }
  }

  FINALLY(quad_forest_highlight_segments);
  RETURN();
}

/******************************************************************************/

result quad_forest_draw_image
(
  quad_forest *forest,
  pixel_image *target,
  truth_value use_segments,
  truth_value use_colors
)
{
  TRY();
  list_item *trees;
  quad_tree *tree;
  quad_forest_segment *parent;
  uint32 x, y, width, height, stride, row_step;
  byte *target_data, *target_pos, color0, color1, color2;

  CHECK_POINTER(forest);
  CHECK_POINTER(forest->source);
  CHECK_POINTER(target);

  width = forest->source->width;
  height = forest->source->height;

  CHECK(pixel_image_create(target, p_U8, RGB, width, height, 3, 3 * width));
  CHECK(pixel_image_clear(target));

  stride = target->stride;
  target_data = (byte*)target->data;

  /* draw using tree mean value */
  if (IS_FALSE(use_segments)) {
    statistics *stat;
    trees = forest->trees.first.next;
    while (trees != &forest->trees.last) {
      tree = (quad_tree *)trees->data;
      if (tree->nw == NULL) {
        stat = &tree->stat;
        /* TODO: maybe could create only a grayscale image..? */
        color0 = (byte)stat->mean;
        width = tree->size;
        height = width;
        row_step = stride - 3 * width;
        target_pos = target_data + tree->y * stride + tree->x * 3;
        for (y = 0; y < height; y++, target_pos += row_step) {
          for (x = 0; x < width; x++) {
            *target_pos = color0;
            target_pos++;
            *target_pos = color0;
            target_pos++;
            *target_pos = color0;
            target_pos++;
          }
        }
      }
      trees = trees->next;
    }
  }
  else {
    /* draw using region mean value */
    if (IS_FALSE(use_colors)) {
      trees = forest->trees.first.next;
      while (trees != &forest->trees.last) {
        tree = (quad_tree *)trees->data;
        if (tree->nw == NULL) {
          parent = quad_tree_segment_find(tree);
          if (parent != NULL) {
            color0 = (byte)parent->stat.mean;
            color1 = color0;
            color2 = color0;
            width = tree->size;
            height = width;
            row_step = stride - 3 * width;
            target_pos = target_data + tree->y * stride + tree->x * 3;
            for (y = 0; y < height; y++, target_pos += row_step) {
              for (x = 0; x < width; x++) {
                *target_pos = color0;
                target_pos++;
                *target_pos = color1;
                target_pos++;
                *target_pos = color2;
                target_pos++;
              }
            }
          }
        }
        trees = trees->next;
      }
    }
    /* draw using region color */
    else {
      trees = forest->trees.first.next;
      while (trees != &forest->trees.last) {
        tree = (quad_tree *)trees->data;
        if (tree->nw == NULL) {
          parent = quad_tree_segment_find(tree);
          if (parent != NULL) {
            color0 = parent->color[0];
            color1 = parent->color[1];
            color2 = parent->color[2];
            width = tree->size;
            height = width;
            row_step = stride - 3 * width;
            target_pos = target_data + tree->y * stride + tree->x * 3;
            for (y = 0; y < height; y++, target_pos += row_step) {
              for (x = 0; x < width; x++) {
                *target_pos = color0;
                target_pos++;
                *target_pos = color1;
                target_pos++;
                *target_pos = color2;
                target_pos++;
              }
            }
          }
        }
        trees = trees->next;
      }
    }
  }

  FINALLY(quad_forest_draw_image);
  RETURN();
}

/******************************************************************************/

result quad_forest_find_edges
(
  quad_forest *forest,
  uint32 rounds,
  integral_value bias,
  direction dir
)
{
  TRY();
  uint32 remaining, i, size;
  integral_value mean, dev, value;
  quad_tree *tree;

  CHECK_POINTER(forest);
  CHECK_PARAM(rounds > 0);
  CHECK_PARAM(dir == d_H || dir == d_V || dir == d_N4);

  size = forest->rows * forest->cols;

  /* first get the edge responses */
  /* TODO: add forest state to determine if this has been done already */
  for (i = 0; i < size; i++) {
    CHECK(quad_tree_get_edge_response(forest, forest->roots[i], NULL, NULL));
  }

  /* before propagation, prime all trees */
  /* for finding horizontal edges, prime with dy */
  if (dir == d_H) {
    for (i = 0; i < size; i++) {
      quad_tree_prime_with_dy(forest->roots[i]);
    }
  }
  else
  /* for finding vertical edges, prime with dx */
  if (dir == d_V) {
    for (i = 0; i < size; i++) {
      quad_tree_prime_with_dx(forest->roots[i]);
    }
  }
  /* otherwise, prime with magnitude */
  else {
    for (i = 0; i < size; i++) {
      quad_tree_prime_with_mag(forest->roots[i]);
    }
  }

  /* then, propagate three rounds */
  for (remaining = rounds; remaining--;) {
    for (i = 0; i < size; i++) {
      quad_tree_propagate(forest->roots[i]);
    }
    /* on other rounds except the last, prime for the new run */
    if (remaining > 0) {
      for (i = 0; i < size; i++) {
        quad_tree_prime_with_pool(forest->roots[i]);
      }
    }
  }

  for (i = 0; i < size; i++) {
    tree = forest->roots[i];
    mean = tree->pool;
    dev = tree->pool2;
    dev -= mean*mean;
    if (dev < 0) dev = 0; else dev = sqrt(dev);
    tree->edge.mean = mean;
    tree->edge.deviation = dev;
    tree->edge.dir = dir;

    if (dir == d_H) {
      value = tree->edge.dy;
    }
    else
    if (dir == d_V) {
      value = tree->edge.dx;
    }
    else {
      value = tree->edge.mag;
    }

    if (value > mean + bias - dev) {
      /*printf("value %.3f mean %.3f dev %.3f\n", tree->mag, mean, dev);*/
      tree->edge.has_edge = TRUE;
    }
    else {
      tree->edge.has_edge = FALSE;
    }
  }

  FINALLY(quad_forest_find_edges);
  RETURN();
}

/******************************************************************************/

result quad_forest_segment_edges
(
  quad_forest *target,
  uint32 detect_rounds,
  integral_value detect_bias,
  direction detect_dir,
  uint32 propagate_rounds,
  integral_value propagate_threshold,
  direction propagate_dir,
  direction merge_dir
)
{
  TRY();
  uint32 i, size, remaining;
  quad_tree *tree, *neighbor;

  CHECK_POINTER(target);
  CHECK_PARAM(detect_rounds > 0);
  CHECK_PARAM(detect_dir == d_H || detect_dir == d_V || detect_dir == d_N4);
  CHECK_PARAM(propagate_dir == d_H || propagate_dir == d_V || propagate_dir == d_N4);
  CHECK_PARAM(merge_dir == d_H || merge_dir == d_V || merge_dir == d_N4);

  CHECK(quad_forest_find_edges(target, detect_rounds, detect_bias, detect_dir));

  size = target->rows * target->cols;

  /* prepare to propagate detected edges */
  for (i = 0; i < size; i++) {
    quad_tree_prime_with_edge(target->roots[i], 10);
  }

  /* propagate in desired direction */
  for (remaining = propagate_rounds; remaining--;) {
    if (propagate_dir == d_H) {
      for (i = 0; i < size; i++) {
        quad_tree_propagate_h(target->roots[i]);
      }
    }
    else
    if (propagate_dir == d_V) {
      for (i = 0; i < size; i++) {
        quad_tree_propagate_v(target->roots[i]);
      }
    }
    else {
      for (i = 0; i < size; i++) {
        quad_tree_propagate(target->roots[i]);
      }
    }
    if (remaining > 0) {
      for (i = 0; i < size; i++) {
        quad_tree_prime_with_pool(target->roots[i]);
      }
    }
  }

  /* now trees with pool value higher than threshold have edge */
  for (i = 0; i < size; i++) {
    tree = target->roots[i];
    if (tree->pool > propagate_threshold) {
      tree->edge.has_edge = TRUE;
    }
  }

  /* initialize segments with trees that have edge */
  for (i = 0; i < size; i++) {
    tree = target->roots[i];
    if (IS_TRUE(tree->edge.has_edge)) {
      quad_tree_segment_create(tree);
    }
  }

  /* then merge segments in the desired direction */
  if (merge_dir == d_H) {
    for (i = 0; i < size; i++) {
      tree = target->roots[i];
      if (IS_TRUE(tree->edge.has_edge)) {
        neighbor = tree->w;
        if (neighbor != NULL && IS_TRUE(neighbor->edge.has_edge)) {
          quad_tree_segment_union(tree, neighbor);
        }
        neighbor = tree->e;
        if (neighbor != NULL && IS_TRUE(neighbor->edge.has_edge)) {
          quad_tree_segment_union(tree, neighbor);
        }
      }
    }
  }
  else
  if (merge_dir == d_V) {
    for (i = 0; i < size; i++) {
      tree = target->roots[i];
      if (IS_TRUE(tree->edge.has_edge)) {
        neighbor = tree->n;
        if (neighbor != NULL && IS_TRUE(neighbor->edge.has_edge)) {
          quad_tree_segment_union(tree, neighbor);
        }
        neighbor = tree->s;
        if (neighbor != NULL && IS_TRUE(neighbor->edge.has_edge)) {
          quad_tree_segment_union(tree, neighbor);
        }
      }
    }
  }
  else {
    for (i = 0; i < size; i++) {
      tree = target->roots[i];
      if (IS_TRUE(tree->edge.has_edge)) {
        neighbor = tree->w;
        if (neighbor != NULL && IS_TRUE(neighbor->edge.has_edge)) {
          quad_tree_segment_union(tree, neighbor);
        }
        neighbor = tree->e;
        if (neighbor != NULL && IS_TRUE(neighbor->edge.has_edge)) {
          quad_tree_segment_union(tree, neighbor);
        }
        neighbor = tree->n;
        if (neighbor != NULL && IS_TRUE(neighbor->edge.has_edge)) {
          quad_tree_segment_union(tree, neighbor);
        }
        neighbor = tree->s;
        if (neighbor != NULL && IS_TRUE(neighbor->edge.has_edge)) {
          quad_tree_segment_union(tree, neighbor);
        }
      }
    }
  }

  /* finally, count regions and assign colors */
  {
    quad_forest_segment *parent, *segment;
    list_item *trees;
    uint32 count;

    count = 0;
    /* initialize the random number generator for assigning the colors */
    srand(1234);

    trees = target->trees.first.next;
    while (trees != &target->trees.last) {
      tree = (quad_tree *)trees->data;
      if (tree->nw == NULL) {
        segment = &tree->segment;
        parent = quad_tree_segment_find(tree);
        if (parent == segment) {
          segment->color[0] = (byte)(rand() % 256);
          segment->color[1] = (byte)(rand() % 256);
          segment->color[2] = (byte)(rand() % 256);
          count++;
        }
      }
      trees = trees->next;
    }
    target->segments = count;
    printf("segmentation finished, %lu segments found\n", count);
  }

  FINALLY(quad_forest_segment_horizontal_edges);
  RETURN();
}

/* end of file                                                                */
/******************************************************************************/
