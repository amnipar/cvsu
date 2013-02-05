/**
 * @file cvsu_edges.c
 * @author Matti J. Eskelinen <matti.j.eskelinen@gmail.com>
 * @brief Edge detection and handling for cvsu.
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

#include "cvsu_config.h"
#include "cvsu_macros.h"
#include "cvsu_memory.h"
#include "cvsu_edges.h"
#include "cvsu_filter.h"

/* for sqrt */
#include <math.h>

/******************************************************************************/
/* constants for reporting function names in error messages                   */

string edge_image_alloc_name = "edge_image_alloc";
string edge_image_free_name = "edge_image_free";
string edge_image_create_name = "edge_image_create";
string edge_image_destroy_name = "edge_image_destroy";
string edge_image_nullify_name = "edge_image_nullify";
string edge_image_clone_name = "edge_image_clone";
string edge_image_copy_name = "edge_image_copy";
string edgel_response_x_name = "edgel_response_x";
string edges_x_box_deviation_name = "edges_x_box_deviation";
string edge_image_update_name = "edge_image_update";
string edge_image_convert_to_grey8_name = "edge_image_convert_to_grey8";

/******************************************************************************/

edge_image *edge_image_alloc()
{
  TRY();
  edge_image *target;

  CHECK(memory_allocate((data_pointer*)&target, 1, sizeof(edge_image)));
  CHECK(edge_image_nullify(target));

  FINALLY(edge_image_alloc);
  return target;
}

/******************************************************************************/

void edge_image_free
(
  edge_image *target
)
{
  TRY();

  r = SUCCESS;

  if (target != NULL) {
    CHECK(edge_image_destroy(target));
    CHECK(memory_deallocate((data_pointer *)&target));
  }

  FINALLY(edge_image_free);
}

/******************************************************************************/

result edge_image_create
(
  edge_image *target,
  pixel_image *source,
  uint32 hstep,
  uint32 vstep,
  uint32 hmargin,
  uint32 vmargin,
  uint32 box_width,
  uint32 box_length
)
{
  TRY();

  CHECK_POINTER(target);
  CHECK_POINTER(source);
  CHECK_PARAM(source->type == p_U8);

  CHECK(integral_image_create(&target->I, source));

  target->hstep = hstep;
  target->vstep = vstep;
  target->hmargin = hmargin;
  target->vmargin = vmargin;
  target->box_width = box_width;
  target->box_length = box_length;
  target->width = (uint32)((source->width - 2 * hmargin) / hstep);
  target->height = (uint32)((source->height - 2 * vmargin) / vstep);
  target->dx = (uint32)((hstep - box_width) / 2);
  target->dy = (uint32)((vstep - box_width) / 2);

  CHECK(pixel_image_create(&target->vedges, p_S8, GREY,
                           source->width, target->height, 1, source->width));
  CHECK(pixel_image_create(&target->hedges, p_S8, GREY,
                           target->width, source->height, 1, target->width));

  FINALLY(edge_image_create);
  RETURN();
}

/******************************************************************************/

result edge_image_destroy
(
  edge_image *target
)
{
  TRY();

  CHECK_POINTER(target);

  if (target->hedges.data != NULL) {
    CHECK(pixel_image_destroy(&target->hedges));
  }
  if (target->vedges.data != NULL) {
    CHECK(pixel_image_destroy(&target->vedges));
  }
  if (target->I.original != NULL) {
    CHECK(integral_image_destroy(&target->I));
  }

  FINALLY(edge_image_destroy);
  RETURN();
}

/******************************************************************************/

result edge_image_nullify
(
  edge_image *target
)
{
  TRY();

  CHECK_POINTER(target);

  CHECK(integral_image_nullify(&target->I));
  CHECK(pixel_image_nullify(&target->hedges));
  CHECK(pixel_image_nullify(&target->vedges));
  target->width = 0;
  target->height = 0;
  target->hstep = 0;
  target->vstep = 0;
  target->hmargin = 0;
  target->vmargin = 0;
  target->box_width = 0;
  target->box_length = 0;
  target->dx = 0;
  target->dy = 0;

  FINALLY(edge_image_nullify);
  RETURN();
}

/******************************************************************************/

truth_value edge_image_is_null
(
  edge_image *target
)
{
  if (target != NULL) {
    return integral_image_is_null(&target->I);
  }
  return TRUE;
}

/******************************************************************************/

result edge_image_clone
(
  edge_image *target,
  edge_image *source
)
{
  TRY();

  CHECK_POINTER(target);
  CHECK_POINTER(source);

  CHECK(integral_image_clone(&target->I, &source->I));
  CHECK(pixel_image_clone(&target->hedges, &source->hedges));
  CHECK(pixel_image_clone(&target->vedges, &source->vedges));

  target->width = source->width;
  target->height = source->height;
  target->hstep = source->hstep;
  target->vstep = source->vstep;
  target->hmargin = source->hmargin;
  target->vmargin = source->vmargin;
  target->box_width = source->box_width;
  target->box_length = source->box_length;
  target->dx = source->dx;
  target->dy = source->dy;

  FINALLY(edge_image_clone);
  RETURN();
}

/******************************************************************************/

result edge_image_copy
(
  edge_image *target,
  edge_image *source
)
{
  TRY();

  CHECK_POINTER(target);
  CHECK_POINTER(source);
  CHECK_PARAM(source->width == target->width);
  CHECK_PARAM(source->height == target->height);

  CHECK(integral_image_copy(&target->I, &source->I));
  CHECK(pixel_image_copy(&target->hedges, &source->hedges));
  CHECK(pixel_image_copy(&target->vedges, &source->vedges));

  FINALLY(edge_image_copy);
  RETURN();
}

/******************************************************************************/

result edge_image_update
(
  edge_image *target
)
{
  TRY();
  truth_value rising, falling;
  uint32 width, height, /*stride,*/ target_stride;
  pixel_image *edges;

  CHECK_POINTER(target);
  CHECK_POINTER(target->hedges.data);
  CHECK_POINTER(target->vedges.data);
  CHECK_PARAM(target->hedges.type == p_S8);
  CHECK_PARAM(target->vedges.type == p_S8);

  CHECK(integral_image_update(&target->I));

  width = target->I.width;
  height = target->I.height;
  /*stride = target->I.stride;*/

  /* calculate vertical edges */
  edges = &target->vedges;
  {
    uint32 x, y, rows, startcol, endcol;
    integral_value prev;
    INTEGRAL_IMAGE_2BOX_VARIABLES();
    INTEGRAL_IMAGE_INIT_HBOX(&target->I, target->box_length, target->box_width);

    target_stride = target->vedges.stride;
    rows = target->vedges.height;
    startcol = target->box_length;
    endcol = width - target->box_length;

    {
      SINGLE_DISCONTINUOUS_IMAGE_VARIABLES(edges, char);

      pixel_image_clear(edges);
      for (y = 0; y < rows; y++ ) {
        iA1 = I_1_data + ((target->vmargin + target->dy + y * target->vstep) * stride);
        i2A1 = I_2_data + ((target->vmargin + target->dy + y * target->vstep) * stride);

        rising = FALSE;
        falling = FALSE;
        edges_pos = edges_rows[y] + startcol * edges_step;
        for (x = startcol; x < endcol; x++,
             iA1++, i2A1++, edges_pos += edges_step) {
          sum1 = INTEGRAL_IMAGE_SUM_1();
          sum2 = INTEGRAL_IMAGE_SUM_2();
          sumsqr1 = INTEGRAL_IMAGE_SUMSQR_1();
          sumsqr2 = INTEGRAL_IMAGE_SUMSQR_2();

          g = edgel_fisher_signed(N, sum1, sum2, sumsqr1, sumsqr2);

          if (x > startcol) {
            if (g < prev) {
              /* found maximum at previous column */
              if (IS_TRUE(rising)) {
                PIXEL_VALUE_MINUS(edges, 1) = (char)prev;
                rising = FALSE;
              }
              falling = TRUE;
            }
            else if (g > prev) {
              /* found minimum at previous column */
              if (IS_TRUE(falling)) {
                PIXEL_VALUE_MINUS(edges, 1) = (char)prev;
                falling = FALSE;
              }
              rising = TRUE;
            }
          }
          prev = g;
        }
      }
    }
  }
  /* calculate horizontal edges */
  edges = &target->hedges;
  {
    uint32 x, y, cols, startrow, endrow, edges_stride;
    integral_value prev;
    INTEGRAL_IMAGE_2BOX_VARIABLES();
    INTEGRAL_IMAGE_INIT_VBOX(&target->I, target->box_length, target->box_width);

    edges_stride = target->hedges.stride;
    cols = target->hedges.width;
    startrow = target->box_length;
    endrow = height - target->box_length;

    {
      SINGLE_DISCONTINUOUS_IMAGE_VARIABLES(edges, char);

      pixel_image_clear(edges);
      for (x = 0; x < cols; x++) {
        iA1 = I_1_data + target->hmargin + target->dx + x * target->hstep;
        i2A1 = I_2_data + target->hmargin + target->dx + x * target->hstep;

        rising = FALSE;
        falling = FALSE;
        edges_pos = edges_rows[startrow] + x * edges_step;
        for (y = startrow; y < endrow; y++,
             iA1 += stride, i2A1 += stride, edges_pos += edges_stride) {
          sum1 = INTEGRAL_IMAGE_SUM_1();
          sum2 = INTEGRAL_IMAGE_SUM_2();
          sumsqr1 = INTEGRAL_IMAGE_SUMSQR_1();
          sumsqr2 = INTEGRAL_IMAGE_SUMSQR_2();

          g = edgel_fisher_signed(N, sum1, sum2, sumsqr1, sumsqr2);

          if (y > startrow) {
            if (g < prev) {
              /* found maximum at previous row */
              if (IS_TRUE(rising)) {
                PIXEL_VALUE_MINUS(edges, edges_stride) = (char)prev;
                rising = FALSE;
              }
              falling = TRUE;
            }
            else if (g > prev) {
              /* found minimum at previous row */
              if (IS_TRUE(falling)) {
                PIXEL_VALUE_MINUS(edges, edges_stride) = (char)prev;
                falling = FALSE;
              }
              rising = TRUE;
            }
          }
          prev = g;
        }
      }
    }
  }

  FINALLY(edge_image_update);
  RETURN();
}

/******************************************************************************/

result edge_image_convert_to_grey8
(
  edge_image *source,
  pixel_image *temp,
  pixel_image *target
)
{
  TRY();
  pixel_image *edges;
  uint32 i, x, y;
  char value;
  SINGLE_DISCONTINUOUS_IMAGE_VARIABLES(temp, long);

  CHECK_POINTER(source);
  CHECK_POINTER(temp);
  CHECK_POINTER(target);
  CHECK_POINTER(source->hedges.data);
  CHECK_POINTER(source->vedges.data);
  CHECK_POINTER(temp->data);
  CHECK_POINTER(target->data);
  CHECK_PARAM(temp->type == p_S32);
  CHECK_PARAM(target->type == p_U8);
  CHECK_PARAM(temp->width == source->I.width);
  CHECK_PARAM(temp->height == source->I.height);
  CHECK_PARAM(target->width == source->I.width);
  CHECK_PARAM(target->height == source->I.height);

  pixel_image_clear(temp);

  /* process vertical edge image */
  edges = &source->vedges;
  {
    SINGLE_DISCONTINUOUS_IMAGE_VARIABLES(edges, char);
    for (y = 0; y < edges->height; y++) {
      edges_pos = edges_rows[y];
      temp_pos = temp_rows[y * source->vstep + source->vmargin + source->dy];
      for (x = 0; x < edges->width; x++,
           edges_pos += edges_step, temp_pos += temp_step) {
        value = PIXEL_VALUE(edges);
        if (value != 0) {
          for (i = 0; i < source->box_width; i++) {
              PIXEL_VALUE_PLUS(temp, i * temp->stride) = value;
          }
        }
      }
    }
  }
  edges = &source->hedges;
  /* process horizontal edge image */
  {
    uint32 edges_stride;
    SINGLE_DISCONTINUOUS_IMAGE_VARIABLES(edges, char);
    edges_stride = edges->stride;
    for (x = 0; x < edges->width; x++) {
      edges_pos = edges_rows[0] + x * edges_step;
      temp_pos = temp_rows[0] + (x * source->hstep + source->hmargin + source->dx) * temp_step;
      for (y = 0; y < edges->height; y++,
           edges_pos += edges_stride, temp_pos += temp->stride) {
        value = PIXEL_VALUE(edges);
        if (value != 0) {
          for (i = 0; i < source->box_width; i++) {
              PIXEL_VALUE_PLUS(temp, i * temp_step) = value;
          }
        }
      }
    }
  }

  normalize(temp, target);

  FINALLY(edge_image_convert_to_grey8);
  RETURN();
}

/******************************************************************************/

result edge_image_overlay_to_grey8
(
  edge_image *source,
  pixel_image *target
)
{
  TRY();
  pixel_image *edges;
  uint32 i, x, y;
  sint32 grey_value;
  char edge_value;
  SINGLE_DISCONTINUOUS_IMAGE_VARIABLES(target, byte);

  CHECK_POINTER(source);
  CHECK_POINTER(target);
  CHECK_POINTER(source->hedges.data);
  CHECK_POINTER(source->vedges.data);
  CHECK_POINTER(target->data);
  CHECK_PARAM(target->type == p_U8);
  CHECK_PARAM(target->width == source->I.width);
  CHECK_PARAM(target->height == source->I.height);

  /* process vertical edge image */
  edges = &source->vedges;
  {
    SINGLE_DISCONTINUOUS_IMAGE_VARIABLES(edges, char);
    for (y = 0; y < edges->height; y++) {
      edges_pos = edges_rows[y];
      target_pos = target_rows[y * source->vstep + source->vmargin + source->dy];
      for (x = 0; x < edges->width; x++,
           edges_pos += edges_step, target_pos += target_step) {
        edge_value = PIXEL_VALUE(edges);
        if (edge_value != 0) {
          for (i = 0; i < source->box_width; i++) {
            grey_value = PIXEL_VALUE_PLUS(target, i * target->stride);
            if (grey_value < 128) {
              grey_value = 255;
            }
            else {
              grey_value = 0;
            }
            PIXEL_VALUE_PLUS(target, i * target->stride) = (byte)grey_value;
          }
        }
      }
    }
  }
  edges = &source->hedges;
  /* process horizontal edge image */
  {
    uint32 edges_stride;
    SINGLE_DISCONTINUOUS_IMAGE_VARIABLES(edges, char);
    edges_stride = edges->stride;
    for (x = 0; x < edges->width; x++) {
      edges_pos = edges_rows[0] + x * edges_step;
      target_pos = target_rows[0] + (x * source->hstep + source->hmargin + source->dx) * target_step;
      for (y = 0; y < edges->height; y++,
           edges_pos += edges_stride, target_pos += target->stride) {
        edge_value = PIXEL_VALUE(edges);
        if (edge_value != 0) {
          for (i = 0; i < source->box_width; i++) {
            grey_value = PIXEL_VALUE_PLUS(target, i * target->stride);
            if (grey_value < 128) {
              grey_value = 255;
            }
            else {
              grey_value = 0;
            }
            PIXEL_VALUE_PLUS(target, i * target_step) = (byte)grey_value;
          }
        }
      }
    }
  }

  FINALLY(edge_image_convert_to_grey8);
  RETURN();
}

/******************************************************************************/

integral_value edgel_fisher_unsigned
(
  integral_value N,
  integral_value sum1,
  integral_value sum2,
  integral_value sumsqr1,
  integral_value sumsqr2
)
{
  integral_value mean1, mean2, diff, var1, var2, var;
  mean1 = sum1 / N;
  mean2 = sum2 / N;
  diff = mean2 - mean1;
  diff = diff * diff;
  var1 = (sumsqr1 / N) - (mean1 * mean1);
  var2 = (sumsqr2 / N) - (mean2 * mean2);
  var = var1 + var2;
  if (var < 1) var = 1;
  return (diff / var);
}

/******************************************************************************/

integral_value edgel_fisher_signed
(
  integral_value N,
  integral_value sum1,
  integral_value sum2,
  integral_value sumsqr1,
  integral_value sumsqr2
)
{
  integral_value mean1, mean2, var1, var2, var;

  mean1 = sum1 / N;
  mean2 = sum2 / N;
  var1 = (sumsqr1 / N) - (mean1 * mean1);
  var2 = (sumsqr2 / N) - (mean2 * mean2);
  var = var1 + var2;
  if (var < 1) var = 1;
  return ((mean2 - mean1) / sqrt(var));
}

/******************************************************************************/

result edgel_response_x
(
  integral_image *I,
  pixel_image *target,
  uint32 hsize,
  uint32 vsize,
  edgel_criterion_calculator criterion
)
{
  INTEGRAL_IMAGE_2BOX_VARIABLES();
  uint32 i, x, y, width, height, target_stride;
  TRY();

  CHECK_POINTER(I);
  CHECK_POINTER(target);
  CHECK_POINTER(I->I_1.data);
  CHECK_POINTER(I->I_2.data);
  CHECK_POINTER(target->data);
  CHECK_PARAM(target->type == p_S32);
  CHECK_PARAM(I->width == target->width);
  CHECK_PARAM(I->height == target->height);

  width = I->width;
  height = I->height;
  target_stride = target->stride;

  INTEGRAL_IMAGE_INIT_HBOX(I, hsize, vsize);
  pixel_image_clear(target);
  {
    SINGLE_DISCONTINUOUS_IMAGE_VARIABLES(target, long);

    for (y = 0; y < height - vsize; y += vsize) {
      iA1 = I_1_data + (y * width);
      i2A1 = I_2_data + (y * width);
      target_pos = target_rows[y] + hsize + 1;
      for (x = hsize + 1; x < width - hsize; x++,
           iA1++, i2A1++, target_pos += target_step) {
        sum1 = INTEGRAL_IMAGE_SUM_1();
        sum2 = INTEGRAL_IMAGE_SUM_2();
        sumsqr1 = INTEGRAL_IMAGE_SUMSQR_1();
        sumsqr2 = INTEGRAL_IMAGE_SUMSQR_2();

        g = criterion(N, sum1, sum2, sumsqr1, sumsqr2);
        for (i = 0; i < vsize; i++) {
          PIXEL_VALUE_PLUS(target, i * target_stride) = ((long)g);
        }
      }
    }
  }

  FINALLY(edgel_response_x);
  RETURN();
}

/******************************************************************************/

result edges_x_box_deviation
(
  integral_image *I,
  pixel_image *temp,
  pixel_image *target,
  uint32 hsize,
  uint32 vsize
)
{
  TRY();

  CHECK(integral_image_update(I));
  CHECK(edgel_response_x(I, temp, hsize, vsize, &edgel_fisher_unsigned));
  CHECK(extrema_x(temp, temp));
  CHECK(normalize(temp, target));

  FINALLY(edges_x_box_deviation);
  RETURN();
}

/* end of file                                                                */
/******************************************************************************/
