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
string quad_tree_ensure_edge_response_name = "quad_tree_ensure_edge_response";
string quad_tree_get_child_edge_response_name = "quad_tree_get_child_edge_response";
string quad_tree_edge_response_to_line_name = "quad_tree_edge_response_to_line";
string quad_tree_gradient_to_line_name = "quad_tree_gradient_to_line";
string quad_tree_get_neighbors_name = "quad_tree_get_neighbors";
string quad_tree_find_link_name = "quad_tree_find_link";

/******************************************************************************/

void quad_tree_link_destroy
(
  quad_tree_link *target
)
{
  if (target != NULL) {
    typed_pointer_destroy(&target->a.annotation);
    typed_pointer_destroy(&target->b.annotation);
    typed_pointer_destroy(&target->annotation);
  }
}

/******************************************************************************/

void quad_tree_destroy
(
  quad_tree *tree
)
{
  if (tree != NULL) {
    /* must deallocate the memory pointed to by typed pointers, if set */
    typed_pointer_destroy(&tree->annotation);
    typed_pointer_destroy(&tree->context);

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
        child_tree->annotation.token = target->annotation.token;
        target->nw = child_tree;

        /* ne child block */
        CHECK(list_append_return_pointer(&forest->trees, (pointer)&children[1], (pointer*)&child_tree));
        /*quad_tree_segment_create(child_tree);*/
        child_tree->level = level;
        child_tree->parent = target;
        child_tree->annotation.token = target->annotation.token;
        target->ne = child_tree;

        /* sw child block */
        CHECK(list_append_return_pointer(&forest->trees, (pointer)&children[2], (pointer*)&child_tree));
        /*quad_tree_segment_create(child_tree);*/
        child_tree->level = level;
        child_tree->parent = target;
        child_tree->annotation.token = target->annotation.token;
        target->sw = child_tree;

        /* se child block */
        CHECK(list_append_return_pointer(&forest->trees, (pointer)&children[3], (pointer*)&child_tree));
        /*quad_tree_segment_create(child_tree);*/
        child_tree->level = level;
        child_tree->parent = target;
        child_tree->annotation.token = target->annotation.token;
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
      pixel_type type;
      integral_value mean;

      original = forest->source;
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
        child_tree->annotation.token = target->annotation.token;
        target->nw = child_tree;

        /* ne child block */
        CHECK(list_append_return_pointer(&forest->trees, (pointer)&children[1], (pointer*)&child_tree));
        /*quad_tree_segment_create(child_tree);*/
        child_tree->parent = target;
        child_tree->level = level;
        child_tree->annotation.token = target->annotation.token;
        target->ne = child_tree;

        /* sw child block */
        CHECK(list_append_return_pointer(&forest->trees, (pointer)&children[2], (pointer*)&child_tree));
        /*quad_tree_segment_create(child_tree);*/
        child_tree->parent = target;
        child_tree->level = level;
        child_tree->annotation.token = target->annotation.token;
        target->sw = child_tree;

        /* se child block */
        CHECK(list_append_return_pointer(&forest->trees, (pointer)&children[3], (pointer*)&child_tree));
        /*quad_tree_segment_create(child_tree);*/
        child_tree->parent = target;
        child_tree->level = level;
        child_tree->annotation.token = target->annotation.token;
        target->se = child_tree;

        quad_tree_cache_neighbors(target);
      }
      else {
        CHECK(quad_tree_ensure_segment(target, NULL));
      }
    }
    else {
      CHECK(quad_tree_ensure_segment(target, NULL));
    }
  }

  FINALLY(quad_tree_divide_with_overlap);
  RETURN();
}

/******************************************************************************/
/* TODO: find all places that use this function and fix to use ensure */
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
    /* TODO: tree->edge.dx = hsum;*/
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
    /* tree->edge.dy = vsum; */
    if (dy != NULL) {
      *dy = vsum;
    }
    /*printf("dy %.3f ", vsum);*/
  }

  /* tree->edge.mag = sqrt(hsum*hsum + vsum*vsum); */
  /*if (tree->edge.mag > )*/
  ang = atan2(hsum, vsum);
  if (ang < 0) ang = ang + 2 * M_PI;
  /*tree->edge.ang = ang;*/

  FINALLY(quad_tree_get_edge_response);
  RETURN();
}

/******************************************************************************/

result quad_tree_ensure_edge_response
(
  quad_forest *forest,
  quad_tree *tree,
  edge_response **eresp,
  truth_value use_max
)
{
  TRY();
  uint32 box_width, box_length, row, col, endrow, endcol, hpeaks, vpeaks, x, y;
  sint32 srow, scol;
  integral_value hmax, hsum, vmax, vsum, ang, g_1, g_2, hold, vold;
  edge_response *resp;
  typed_pointer *tptr;
  INTEGRAL_IMAGE_2BOX_VARIABLES();

  CHECK_POINTER(forest);
  CHECK_POINTER(tree);

  CHECK(ensure_has(&tree->annotation, t_edge_response, &tptr));
  resp = (edge_response*)tptr->value;
  if (tptr->token != tree->annotation.token) {
    tptr->token = tree->annotation.token;
    box_width = tree->size;
    /* box length should be at least 4 to get proper result */
    box_length = (uint32)(getmax(((integral_value)box_width) / 2.0, 4.0));

    /* calculate horizontal cumulative gradient */
    {
      INTEGRAL_IMAGE_INIT_HBOX(&forest->integral, box_length, box_width);
      scol = ((signed)tree->x) - ((signed)box_length);
      endcol = ((unsigned)(scol + ((signed)box_width)));
      hsum = 0;
      hpeaks = 0;
      x = 0;
      /*printf("col %lu endcol %lu ", col, endcol);*/
      if (scol >= 0 && endcol + box_width + 1 <= forest->integral.width) {
        if (IS_FALSE(use_max)) {
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
        else {
          hold = 0;
          hmax = 0;
          g_2 = g_1 = 0;
          col = ((unsigned)scol);
          if (col > 1) {
            col -= 1;
          }
          if (endcol + box_width + 2 < forest->integral.width) {
            endcol += 1;
          }

          iA1 = I_1_data + tree->y * stride + col;
          i2A1 = I_2_data + tree->y * stride + col;
          while (col < endcol) {
            sum1 = INTEGRAL_IMAGE_SUM_1();
            sum2 = INTEGRAL_IMAGE_SUM_2();
            sumsqr1 = INTEGRAL_IMAGE_SUMSQR_1();
            sumsqr2 = INTEGRAL_IMAGE_SUMSQR_2();
            g = edgel_fisher_signed(N, sum1, sum2, sumsqr1, sumsqr2);
            hsum += g;
            if (g < -0.000001) {
              if (g_2 < 0 && g_1 < (g_2 - 0.1) && g_1 < (g - 0.1)) {
                if (fabs(g_1) > fabs(hmax)) {
                  if (fabs(hmax) > fabs(hold)) {
                    hold = hmax;
                  }
                  hmax = g_1;
                  x = col;
                }
                else {
                  if (fabs(g_1) > fabs(hold)) {
                    hold = g_1;
                  }
                }
                hpeaks += 1;
              }
              g_2 = g_1;
              g_1 = g;
            }
            else
            if (g > 0.000001) {
              if (g_2 > 0 && g_1 > (g_2 + 0.1) && g_1 > (g + 0.1)) {
                if (fabs(g_1) > fabs(hmax)) {
                  if (fabs(hmax) > fabs(hold)) {
                    hold = hmax;
                  }
                  hmax = g_1;
                  x = col;
                }
                else {
                  if (fabs(g_1) > fabs(hold)) {
                    hold = g_1;
                  }
                }
                hpeaks += 1;
              }
              g_2 = g_1;
              g_1 = g;
            }
            col++;
            iA1++;
            i2A1++;
          }
        }
        hsum /= ((integral_value)box_width);
      }
    }
    /* calculate vertical cumulative gradient */
    {
      INTEGRAL_IMAGE_INIT_VBOX(&forest->integral, box_length, box_width);
      srow = ((signed)tree->y) - ((signed)box_length);
      endrow = ((unsigned)(srow + ((signed)box_width)));
      vsum = 0;
      vpeaks = 0;
      y = 0;
      /*printf("row %lu endrow %lu ", row, endrow);*/
      if (srow >= 0 && endrow + box_width + 1 <= forest->integral.height) {
        if (IS_FALSE(use_max)) {
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
        else {
          vold = 0;
          vmax = 0;
          g_2 = g_1 = 0;
          row = ((unsigned)srow);
          if (row > 1) {
            row -= 1;
          }
          if (endrow + box_width + 2 < forest->integral.height) {
            endrow += 1;
          }

          iA1 = I_1_data + row * stride + tree->x;
          i2A1 = I_2_data + row * stride + tree->x;
          while (row < endrow) {
            sum1 = INTEGRAL_IMAGE_SUM_1();
            sum2 = INTEGRAL_IMAGE_SUM_2();
            sumsqr1 = INTEGRAL_IMAGE_SUMSQR_1();
            sumsqr2 = INTEGRAL_IMAGE_SUMSQR_2();
            g = edgel_fisher_signed(N, sum1, sum2, sumsqr1, sumsqr2);
            vsum += g;
            if (g < -0.000001) {
              if (g_2 < 0 && g_1 < (g_2 - 0.1) && g_1 < (g - 0.1)) {
                if (fabs(g_1) > fabs(vmax)) {
                  if (fabs(vmax) > fabs(vold)) {
                    vold = vmax;
                  }
                  vmax = g_1;
                  y = row;
                }
                else {
                  if (fabs(g_1) > fabs(vold)) {
                    vold = g_1;
                  }
                }
                vpeaks += 1;
              }
              g_2 = g_1;
              g_1 = g;
            }
            else
            if (g > 0.000001) {
              if (g_2 > 0 && g_1 > (g_2 + 0.1) && g_1 > (g + 0.1)) {
                if (fabs(g_1) > fabs(vmax)) {
                  if (fabs(vmax) > fabs(vold)) {
                    vold = vmax;
                  }
                  vmax = g_1;
                  y = row;
                }
                else {
                  if (fabs(g_1) > fabs(vold)) {
                    vold = g_1;
                  }
                }
                vpeaks += 1;
              }
              g_2 = g_1;
              g_1 = g;
            }
            row++;
            iA1 += stride;
            i2A1 += stride;
          }
        }
        vsum /= ((integral_value)box_width);
      }
    }

    if (hpeaks > 0) {
      hsum = hmax;
      if (fabs(hmax) > fabs(0.5 * vmax)) {
        resp->x = x + box_length;
      }
      else {
        resp->x = tree->x + (tree->size / 2);
      }
    }
    else {
      resp->x = tree->x + (tree->size / 2);
    }

    if (vpeaks > 0) {
      vsum = vmax;
      if (fabs(vmax) > fabs(0.5 * hmax)) {
        resp->y = y + box_length;
      }
      else {
        resp->y = tree->y + (tree->size / 2);
      }
    }
    else {
      resp->y = tree->y + (tree->size / 2);
    }

    resp->dx = hsum;
    resp->dy = vsum;
    resp->mag = sqrt(hsum*hsum + vsum*vsum);
    ang = atan2(-vsum, hsum);
    if (ang < 0) ang = ang + 2 * M_PI;
    resp->ang = ang;
    resp->hpeaks = hpeaks;
    resp->vpeaks = vpeaks;
    resp->peak_score = 0;
    /*resp->peak_score = (fabs((hmax - hold) / hmax) + fabs((vmax - vold) / vmax)) / 2;*/
  }

  if (eresp != NULL) {
    *eresp = resp;
  }

  FINALLY(quad_tree_ensure_edge_response);
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

result quad_tree_edge_response_to_line
(
  quad_forest *forest,
  quad_tree *tree,
  list *lines
)
{
  TRY();
  edge_response *eresp;
  uint32 x, y, d, dx, dy;
  integral_value m, radius;
  line new_line;

  CHECK_POINTER(tree);
  CHECK_POINTER(lines);

  eresp = has_edge_response(&tree->annotation, forest->token);
  if (eresp != NULL && eresp->mag > 0 && (eresp->hpeaks <= 2 && eresp->vpeaks <= 2)) {
    x = tree->x;
    y = tree->y;
    radius = ((integral_value)tree->size) / 2.0;
    m = getmax(fabs(eresp->dx), fabs(eresp->dy));
    d = (unsigned)getlround(radius);
    dx = (unsigned)getlround(eresp->dy / m * radius);
    dy = (unsigned)getlround(eresp->dx / m * radius);

    new_line.start.x = (signed)(x + d + dx);
    new_line.start.y = (signed)(y + d - dy);
    new_line.end.x = (signed)(x + d - dx);
    new_line.end.y = (signed)(y + d + dy);

    CHECK(list_append(lines, (pointer)&new_line));
  }

  FINALLY(quad_tree_edge_response_to_line);
  RETURN();
}

/******************************************************************************/

result quad_tree_gradient_to_line
(
  quad_forest *forest,
  quad_tree *tree,
  list *lines
)
{
  TRY();
  edge_response *eresp;
  sint32 x, y, dx, dy;
  integral_value lineang, radius;
  weighted_line new_line;

  CHECK_POINTER(tree);
  CHECK_POINTER(lines);

  eresp = has_edge_response(&tree->annotation, forest->token);
  if (eresp != NULL && eresp->mag > 0.001) {
    radius = ((integral_value)tree->size) / 2.0;
    x = getlround((integral_value)tree->x + radius);
    y = getlround((integral_value)tree->y + radius);

    new_line.weight = 1;
    new_line.start.x = x;
    new_line.start.y = y;

    lineang = eresp->ang;
    dx = getlround(cos(lineang) * radius);
    dy = getlround(sin(lineang) * radius);

    new_line.end.x = x + dx;
    new_line.end.y = y - dy;

    CHECK(list_append(lines, (pointer)&new_line));

    lineang = eresp->ang - M_PI_2;
    if (lineang < 0) lineang += 2 * M_PI;
    dx = getlround(cos(lineang) * radius);
    dy = getlround(sin(lineang) * radius);

    new_line.weight = 0.5;
    new_line.end.x = x + dx;
    new_line.end.y = y - dy;

    CHECK(list_append(lines, (pointer)&new_line));
  }

  FINALLY(quad_tree_gradient_to_line);
  RETURN();
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

truth_value quad_tree_link_equals
(
  const void *a,
  const void *b
)
{
  const quad_tree_link *sa, *sb;
  if (a == NULL || b == NULL) return FALSE;
  sa = (const quad_tree_link *)a;
  sb = (const quad_tree_link *)b;
  if ((sa->a.tree == sb->a.tree && sa->b.tree == sb->b.tree) ||
      (sa->a.tree == sb->b.tree && sa->b.tree == sb->a.tree)) {
    return TRUE;
  }
  return FALSE;
}

/******************************************************************************/

result quad_tree_find_link
(
  quad_tree *tree1,
  quad_tree *tree2,
  quad_tree_link_head **link
)
{
  TRY();
  list_item *links, *endlinks;
  quad_tree_link_head *head;

  CHECK_POINTER(link);
  *link = NULL;
  CHECK_POINTER(tree1);
  CHECK_POINTER(tree2);

  links = tree1->links.first.next;
  endlinks = &tree1->links.last;
  while (links != endlinks) {
    head = *((quad_tree_link_head**)links->data);
    if (head->other->tree == tree2) {
      *link = head->other;
      break;
    }
    links = links->next;
  }

  FINALLY(quad_tree_find_link);
  RETURN();
}

/* end of file                                                                */
/******************************************************************************/
