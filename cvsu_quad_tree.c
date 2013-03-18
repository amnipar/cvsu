/**
 * @file cvsu_quad_tree.c
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

#include "cvsu_macros.h"
#include "cvsu_quad_tree.h"
#include "cvsu_quad_forest.h"
#include "cvsu_memory.h"
#include <math.h>

/******************************************************************************/
/* constants for reporting function names in error messages                   */

string quad_tree_destroy_name = "quad_tree_destroy";
string quad_tree_nullify_name = "quad_tree_nullify";
string quad_tree_divide_name = "quad_tree_divide";
string quad_tree_get_child_statistics_name = "quad_tree_get_child_statistics";
string quad_tree_get_neighborhood_statistics_name = "quad_tree_get_neighborhood_statistics";
string quad_tree_divide_with_overlap_name = "quad_tree_divide_with_overlap";
string quad_tree_get_edge_response_name = "quad_tree_get_edge_response";
string quad_tree_get_child_edge_response_name = "quad_tree_get_child_edge_response";
string quad_tree_get_neighbors_name = "quad_tree_get_neighbors";

/******************************************************************************/

void quad_tree_destroy
(
  quad_tree *tree
)
{
  if (tree != NULL) {
    /* must deallocate the memory pointed to by typed pointers, if set */
    typed_pointer_destroy(&tree->annotation.data);
    typed_pointer_destroy(&tree->context.data);

    /* later will need a special function for destroying annotation and context */
    list_destroy(&tree->intersection.edges);
    list_destroy(&tree->intersection.chains);

    list_destroy(&tree->links);

    quad_tree_nullify(tree);
  }
}

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
      s = getmax(alpha, alpha * stat->deviation);
      x1min = getmax(0, m - s);
      x1max = x1min;
      x2min = getmin(255, m + s);
      x2max = x2min;
      for (i = 1; i < 4; i++) {
        stat = &children[i].stat;
        m = stat->mean;
        s = getmax(alpha, alpha * stat->deviation);
        x1 = getmax(0, m - s);
        x2 = getmin(255, m + s);
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
  integral_value hsum, vsum, ang;
  INTEGRAL_IMAGE_2BOX_VARIABLES();

  CHECK_POINTER(forest);
  CHECK_POINTER(tree);

  box_width = tree->size;
  /* box length should be at least 4 to get proper result */
  box_length = (uint32)(getmax(((integral_value)box_width) / 2.0, 4.0));

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

  tree->edge.mag = sqrt(hsum*hsum + vsum*vsum);
  ang = atan2(hsum, vsum);
  if (ang < 0) ang = ang + 2 * M_PI;
  tree->edge.ang = ang;

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

  box_width = (uint32)(((integral_value)tree->size) / 2.0);
  /* box length should be at least 4 to get proper result */
  box_length = (uint32)(getmax(((integral_value)box_width) / 2.0, 4.0));

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
          /* when comparing children, only interested in magnitude -> unsigned */
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
          /* when comparing children, only interested in magnitude -> unsigned */
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

/* sets the acc value to deviation */
void quad_tree_prime_with_dev(quad_tree *tree)
{
  tree->acc = tree->stat.deviation / 2;
  tree->pool = tree->acc;
  tree->acc2 = tree->stat.deviation * tree->acc;
  tree->pool2 = tree->acc2;
}

/* sets the acc value to mean */
void quad_tree_prime_with_mean(quad_tree *tree)
{
  tree->acc = tree->stat.mean / 2;
  tree->pool = tree->acc;
  tree->acc2 = tree->stat.mean * tree->acc;
  tree->pool2 = tree->acc2;
}

/* sets the new accumulator value from pool */
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
    tree->pool2 += pool2;
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

/* propagate in proportion of dx and dy */
void quad_tree_propagate_m(quad_tree *tree)
{
  integral_value pool, pool2, dx, dy, m, mx, my;

  dx = fabs(tree->edge.dx);
  dy = fabs(tree->edge.dy);
  m = dx + dy;
  if (m < 0.01) {
    mx = my = 0.5;
  }
  else {
    mx = dx / m;
    my = dy / m;
  }

  /* divide only by two, as the value is further divided in proportion of dx/dy */
  /* effectively this is one fourth of original tree value */
  pool = tree->acc / 2;
  pool2 = tree->acc2 / 2;

  /* neighbor n */
  if (tree->n != NULL) {
    tree->n->pool += mx * pool;
    tree->n->pool2 += mx * pool2;
  }
  /* at edge, return back to own pool */
  else {
    tree->pool += mx * pool;
    tree->pool2 += mx * pool2;
  }
  /* neighbor e */
  if (tree->e != NULL) {
    tree->e->pool += my * pool;
    tree->e->pool2 += my * pool2;
  }
  /* at edge, return back to own pool */
  else {
    tree->pool += my * pool;
    tree->pool2 += my * pool2;
  }
  /* neighbor s */
  if (tree->s != NULL) {
    tree->s->pool += mx * pool;
    tree->s->pool2 += mx * pool2;
  }
  /* at edge, return back to own pool */
  else {
    tree->pool += mx * pool;
    tree->pool2 += mx * pool2;
  }
  /* neighbor w */
  if (tree->w != NULL) {
    tree->w->pool += my * pool;
    tree->w->pool2 += my * pool2;
  }
  /* at edge, return back to own pool */
  else {
    tree->pool += my * pool;
    tree->pool2 += my * pool2;
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

/* end of file                                                                */
/******************************************************************************/
