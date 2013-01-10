/**
 * @file cvsu_integral.c
 * @author Matti J. Eskelinen <matti.j.eskelinen@gmail.com>
 * @brief Integral image types and operations for the cvsu module.
 *
 * Copyright (c) 2011, Matti Johannes Eskelinen
 * All Rights Reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * Neither the name of the copyright holder nor the
 *     names of its contributors may be used to endorse or promote products
 *     derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL COPYRIGHT HOLDER BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "cvsu_config.h"
#include "cvsu_macros.h"
#include "cvsu_integral.h"
#include "cvsu_memory.h"

#include <math.h>

double fmax (double __x, double __y);

/******************************************************************************/
/* constants for storing the function names                                   */
/* used in error reporting macros                                             */

string integral_image_alloc_name = "integral_image_alloc";
string integral_image_free_name = "integral_image_free";
string integral_image_create_name = "integral_image_create";
string integral_image_destroy_name = "integral_image_destroy";
string integral_image_nullify_name = "integral_image_nullify";
string integral_image_clone_name = "integral_image_clone";
string integral_image_copy_name = "integral_image_copy";
string integral_image_update_name = "integral_image_update";
string integral_image_threshold_sauvola_name = "integral_image_threshold_sauvola";
string integral_image_threshold_feng_name = "integral_image_threshold_feng";
string small_integral_image_create_name = "small_integral_image_create";
string small_integral_image_update_name = "small_integral_image_update";
string small_integral_image_box_create_name = "small_integral_image_box_create";
string small_integral_image_update_block_name = "small_integral_image_update_block";

/******************************************************************************/
/* constants for lookup tables                                                */

bool tables_initialized = false;
integral_value pixel_squared[256];
#ifdef INTEGRAL_IMAGE_HIGHER_ORDER_STATISTICS
integral_value pixel_cubed[256];
integral_value pixel_fourth[256];
#endif
SI_2_t small_pixel_squared[256];

/******************************************************************************/

void init_tables()
{
  if (!tables_initialized) {
    uint32 i;
    for (i = 0; i < 256; i++) {
      pixel_squared[i] = (integral_value)(i * i);
#ifdef INTEGRAL_IMAGE_HIGHER_ORDER_STATISTICS
      pixel_cubed[i] = (integral_value)(i * i * i);
      pixel_fourth[i] = (integral_value)(i * i * i * i);
#endif
      small_pixel_squared[i] = (SI_2_t)(i * i);
    }
    tables_initialized = true;
  }
}

/******************************************************************************/

integral_image *integral_image_alloc()
{
  TRY();
  integral_image *ptr;

  CHECK(memory_allocate((data_pointer *)&ptr, 1, sizeof(integral_image)));
  CHECK(integral_image_nullify(ptr));

  FINALLY(integral_image_alloc);
  return ptr;
}

/******************************************************************************/

void integral_image_free
  (
  integral_image *ptr
  )
{
  TRY();

  r = SUCCESS;

  if (ptr != NULL) {
    CHECK(integral_image_destroy(ptr));
    CHECK(memory_deallocate((data_pointer *)&ptr));
  }
  FINALLY(integral_image_free);
}

/******************************************************************************/

result integral_image_create
  (
  integral_image *target,
  pixel_image *source
  )
{
  TRY();

  CHECK_POINTER(target);
  CHECK_POINTER(source);
  CHECK_POINTER(source->data);
  CHECK_PARAM(source->type == p_U8);

  target->original = source;
  target->width = source->width;
  target->height = source->height;
  target->step = source->step;

  /* integral image requires one extra row and column at top and left side */
  target->stride = (target->width + 1) * target->step;

  CHECK(pixel_image_create(&target->I_1, p_I, source->format,
          target->width+1, target->height+1, target->step, target->stride));
  CHECK(pixel_image_create(&target->I_2, p_I, source->format,
          target->width+1, target->height+1, target->step, target->stride));
#ifdef INTEGRAL_IMAGE_HIGHER_ORDER_STATISTICS
  CHECK(pixel_image_create(&target->I_3, p_I, source->format,
          target->width+1, target->height+1, target->step, target->stride));
  CHECK(pixel_image_create(&target->I_4, p_I, source->format,
          target->width+1, target->height+1, target->step, target->stride));
#endif
  init_tables();

  FINALLY(integral_image_create);
  RETURN();
}

/******************************************************************************/

result integral_image_destroy
  (
  integral_image *target
  )
{
  TRY();

  CHECK_POINTER(target);

  CHECK(pixel_image_destroy(&target->I_1));
  CHECK(pixel_image_destroy(&target->I_2));
#ifdef INTEGRAL_IMAGE_HIGHER_ORDER_STATISTICS
  CHECK(pixel_image_destroy(&target->I_3));
  CHECK(pixel_image_destroy(&target->I_4));
#endif
  FINALLY(integral_image_destroy);
  RETURN();
}

/******************************************************************************/

result integral_image_nullify
  (
  integral_image *target
  )
{
  TRY();

  CHECK_POINTER(target);
  target->original = NULL;
  CHECK(pixel_image_nullify(&target->I_1));
  CHECK(pixel_image_nullify(&target->I_2));
#ifdef INTEGRAL_IMAGE_HIGHER_ORDER_STATISTICS
  CHECK(pixel_image_nullify(&target->I_3));
  CHECK(pixel_image_nullify(&target->I_4));
#endif
  target->width = 0;
  target->height = 0;
  target->step = 0;
  target->stride = 0;

  FINALLY(integral_image_nullify);
  RETURN();
}

/******************************************************************************/

bool integral_image_is_null
(
  integral_image *target
)
{
  if (target != NULL) {
    if (target->original == NULL) {
      return true;
    }
  }
  else {
    return true;
  }
  return false;
}

/******************************************************************************/

result integral_image_clone
  (
  integral_image *target,
  integral_image *source
  )
{
  TRY();

  CHECK_POINTER(target);
  CHECK_POINTER(source);

  /* clone will use the same original image */
  target->original = source->original;
  target->width = source->width;
  target->height = source->height;
  target->step = source->step;
  target->stride = source->stride;

  CHECK(pixel_image_clone(&target->I_1, &source->I_1));
  CHECK(pixel_image_clone(&target->I_2, &source->I_2));
#ifdef INTEGRAL_IMAGE_HIGHER_ORDER_STATISTICS
  CHECK(pixel_image_clone(&target->I_3, &source->I_3));
  CHECK(pixel_image_clone(&target->I_4, &source->I_4));
#endif

  FINALLY(integral_image_clone);
  RETURN();
}

/******************************************************************************/

result integral_image_copy
  (
  integral_image *target,
  integral_image *source
  )
{
  TRY();

  CHECK_POINTER(target);
  CHECK_POINTER(source);
  CHECK_PARAM(target != source);
  CHECK_PARAM(source->width == target->width);
  CHECK_PARAM(source->height == target->height);
  CHECK_PARAM(source->step == target->step);
  CHECK_PARAM(source->stride == target->stride);

  if (target->original != source->original) {
      CHECK(pixel_image_copy(target->original, source->original));
  }
  CHECK(pixel_image_copy(&target->I_1, &source->I_1));
  CHECK(pixel_image_copy(&target->I_2, &source->I_2));
#ifdef INTEGRAL_IMAGE_HIGHER_ORDER_STATISTICS
  CHECK(pixel_image_copy(&target->I_3, &source->I_3));
  CHECK(pixel_image_copy(&target->I_4, &source->I_4));
#endif

  FINALLY(integral_image_copy);
  RETURN();
}

/******************************************************************************/
/* private macros for generating the integral_image_update function           */

#if (IMAGE_ACCESS_METHOD == IMAGE_ACCESS_BY_INDEX)

#define INTEGRAL_IMAGE_UPDATE_DEFINE_VARIABLES(I_1_type, I_2_type)\
    I_1_type *I_1_data;\
    I_2_type *I_2_data;\
    uint32 current_pos

#define INTEGRAL_IMAGE_SET_POS(offset)\
    current_pos = (offset)

#define INTEGRAL_IMAGE_ADVANCE_POS(offset)\
    current_pos += (offset)

#define I_1_GET_VALUE() (I_1_data[current_pos])
#define I_1_SET_VALUE(value) I_1_data[current_pos] = (value)
#define I_1_GET_VALUE_WITH_OFFSET(offset) (I_1_data[current_pos - (offset)])
#define I_2_GET_VALUE() (I_2_data[current_pos])
#define I_2_SET_VALUE(value) I_2_data[current_pos] = (value)
#define I_2_GET_VALUE_WITH_OFFSET(offset) (I_2_data[current_pos - (offset)])

#elif (IMAGE_ACCESS_METHOD == IMAGE_ACCESS_BY_POINTER)

#define INTEGRAL_IMAGE_UPDATE_DEFINE_VARIABLES(I_1_type, I_2_type)\
    I_1_type *I_1_data, *I_1_pos;\
    I_2_type *I_2_data, *I_2_pos

#define INTEGRAL_IMAGE_SET_POS(offset)\
    I_1_pos = I_1_data + (offset);\
    I_2_pos = I_2_data + (offset)

#define INTEGRAL_IMAGE_ADVANCE_POS(offset)\
    I_1_pos += (offset);\
    I_2_pos += (offset)

#define I_1_GET_VALUE() (*I_1_pos)
#define I_1_SET_VALUE(value) *I_1_pos = (value)
#define I_1_GET_VALUE_WITH_OFFSET(offset) (*(I_1_pos - (offset)))
#define I_2_GET_VALUE() (*I_2_pos)
#define I_2_SET_VALUE(value) *I_2_pos = (value)
#define I_2_GET_VALUE_WITH_OFFSET(offset) (*(I_2_pos - (offset)))

#else
#error "Image access method not defined"
#endif

/******************************************************************************/

result integral_image_update
  (
  integral_image *target
  )
{
  TRY();
  pixel_image *source;

  CHECK_POINTER(target);
  CHECK_POINTER(target->original);
  CHECK_POINTER(target->I_1.data);
  CHECK_POINTER(target->I_2.data);

  source = target->original;
  /* TODO: handle multiple channels, and higher powers */
  {
    INTEGRAL_IMAGE_UPDATE_DEFINE_VARIABLES(integral_value, integral_value);
    uint32 intensity, width, height, x, y, h, v, d;
    SINGLE_DISCONTINUOUS_IMAGE_VARIABLES(source, byte);

    /* set the image content to 0's */
    /* the first row and column must contain only 0's */
    /* otherwise the algorithm doesn't work correctly */
    CHECK(pixel_image_clear(&target->I_1));
    CHECK(pixel_image_clear(&target->I_2));

    width = target->width;
    height = target->height;
    I_1_data = (integral_value *)target->I_1.data;
    I_2_data = (integral_value *)target->I_2.data;

    /* horizontal offset for integral images */
    h = target->step;
    /* vertical offset for integral images */
    v = target->stride;
    /* diagonal offset for integral images */
    d = target->stride + target->step;

    /* initialize rest of integral images */
    /* add value of this pixel and integrals from top and left */
    /* subtract integral from top left diagonal */
    {
      INTEGRAL_IMAGE_SET_POS(d);
      for (y = 0; y < height; y++) {
        for (x = width, source_pos = source_rows[y]; x--; source_pos += source_step) {
          intensity = PIXEL_VALUE(source);
          I_1_SET_VALUE((I_1_GET_VALUE_WITH_OFFSET(v) -
                         I_1_GET_VALUE_WITH_OFFSET(d)) +
                         I_1_GET_VALUE_WITH_OFFSET(h) +
                         intensity);
          I_2_SET_VALUE((I_2_GET_VALUE_WITH_OFFSET(v) -
                         I_2_GET_VALUE_WITH_OFFSET(d)) +
                         I_2_GET_VALUE_WITH_OFFSET(h) +
                         pixel_squared[intensity]);
          INTEGRAL_IMAGE_ADVANCE_POS(h);
        }
        /* skip one col to reach the beginning of next row */
        INTEGRAL_IMAGE_ADVANCE_POS(h);
      }
    }
  }

  FINALLY(integral_image_update);
  RETURN();
}

/******************************************************************************/

image_rect integral_image_create_rect
  (
  integral_image *target,
  sint32 x,
  sint32 y,
  sint32 dx,
  sint32 dy,
  uint32 offset
  )
{
  image_rect rect;
  register uint32 width, height, step, stride;

  width = target->width;
  height = target->height;

  rect.valid = 0;
  if (x < 0) {
    dx = dx + x;
    x = 0;
  }
  if (y < 0) {
    dy = dy + y;
    y = 0;
  }
  if ((unsigned)x < width && (unsigned)y < height) {
    if (dx > 0 && dy > 0) {
      if (x + dx > width) dx = width - x;
      if (y + dy > height) dy = height - y;

      step = target->step;
      stride = target->stride;
      rect.valid = 1;
      rect.offset = ((unsigned)y) * stride + ((unsigned)x) * step + offset;
      rect.hstep = ((unsigned)dx) * step;
      rect.vstep = ((unsigned)dy) * stride;
      rect.N = ((unsigned)dx) * ((unsigned)dy);
    }
  }

  return rect;
}


/*
Notes:
-x and y are signed to allow creating regions around any pixel (cx - r, cy - r)
-top left corner of rectangle is in reality one pixel right and up from the
 coordinates of the top left corner of the image region; but since an empty
 row and column is added to top and left, the coordinates are handled
 automatically correctly, and the real width and height of the rectangle can be
 used for dx and dy, there is no need to subtract 1
-this takes into account multi-channel images
*/

integral_value integral_image_calculate_mean
  (
  integral_image *target,
  sint32 x,
  sint32 y,
  sint32 dx,
  sint32 dy,
  uint32 offset
  )
{
  image_rect rect;
  integral_value *iA, sum;

  rect = integral_image_create_rect(target, x, y, dx, dy, offset);
  if (rect.valid == 0) {
    return 0;
  }
  else {
    iA = ((integral_value *)target->I_1.data) + rect.offset;
    sum = *(iA + rect.vstep + rect.hstep) + *iA - *(iA + rect.hstep) - *(iA + rect.vstep);
    return sum / ((integral_value)rect.N);
  }
}

/**
 * Use integral_image to calculate intensity variance within the given area.
 */
integral_value integral_image_calculate_variance
  (
  integral_image *target,
  sint32 x,
  sint32 y,
  sint32 dx,
  sint32 dy,
  uint32 offset
  )
{
  image_rect rect;
  integral_value *iA, *i2A, mean, sum2, var;

  rect = integral_image_create_rect(target, x, y, dx, dy, offset);
  if (rect.valid == 0) {
    return 0;
  }
  else {
    iA = ((integral_value *)target->I_1.data) + rect.offset;
    i2A = ((integral_value *)target->I_2.data) + rect.offset;
    mean = (*(iA + rect.vstep + rect.hstep) + *iA - *(iA + rect.hstep) - *(iA + rect.vstep)) / ((integral_value)rect.N);
    sum2 = *(i2A + rect.vstep + rect.hstep) + *i2A - *(i2A + rect.hstep) - *(i2A + rect.vstep);
    var = (sum2 / ((integral_value)rect.N)) - mean*mean;
    if (var < 0) var = 0;
    return var;
  }
}

/**
 * Use integral_image to calculate intensity statistics within the given area.
 */
void integral_image_calculate_statistics(
  integral_image *target,
  statistics *stat,
  sint32 x,
  sint32 y,
  sint32 dx,
  sint32 dy,
  uint32 offset
  )
{
  image_rect rect;
  integral_value *iA, *i2A, N, sum, sum2, mean, var;

  statistics_init(stat);
  rect = integral_image_create_rect(target, x, y, dx, dy, offset);
  if (rect.valid == 0) {
    return;
  }
  else {
    iA = ((integral_value *)target->I_1.data) + rect.offset;
    i2A = ((integral_value *)target->I_2.data) + rect.offset;
    N = ((integral_value)rect.N);
    sum = *(iA + rect.vstep + rect.hstep) + *iA - *(iA + rect.hstep) - *(iA + rect.vstep);
    sum2 = *(i2A + rect.vstep + rect.hstep) + *i2A - *(i2A + rect.hstep) - *(i2A + rect.vstep);
    mean = sum / N;
    var = (sum2 / N) - mean*mean;
    if (var < 0) var = 0;
    stat->N = N;
    stat->sum = sum;
    stat->sum2 = sum2;
    stat->mean = mean;
    stat->variance = var;
    stat->deviation = sqrt(var);
    return;
  }
}

/******************************************************************************/

result integral_image_threshold_sauvola
(
  integral_image *source,
  pixel_image *target,
  truth_value invert,
  uint32 radius,
  integral_value k,
  truth_value calculate_max,
  integral_value max,
  truth_value use_mean
)
{
  TRY();
  pixel_image temp_mean, temp_dev;
  byte *source_data, *target_data, source_value, t, value1, value2;
  integral_value *mean_data, *dev_data, mean, dev, R, dev_max, dev_sum;
  uint32 x, y, width, height, step, stride, offset, size, pos;
  statistics stat;

  CHECK_POINTER(source);
  CHECK_POINTER(target);
  CHECK_PARAM(source->original->type == p_U8);

  CHECK(pixel_image_clone(target, source->original));

  width = target->width;
  height = target->height;
  step = target->step;
  stride = target->stride;
  offset = target->offset;

  if (IS_TRUE(invert)) {
    value1 = 0;
    value2 = 255;
  }
  else {
    value1 = 255;
    value2 = 0;
  }

  CHECK(pixel_image_create(&temp_mean, p_I, target->format, width, height, step, stride));
  CHECK(pixel_image_create(&temp_dev, p_I, target->format, width, height, step, stride));

  source_data = (byte *)source->original->data;
  target_data = (byte *)target->data;
  mean_data = (integral_value *)temp_mean.data;
  dev_data = (integral_value *)temp_dev.data;
  size = 2 * radius + 1;

  if (IS_FALSE(calculate_max)) {
    R = max;
    for (y = 0; y < height; y++) {
      pos = y * stride;
      for (x = 0; x < width; x++, pos += step) {
        source_value = source_data[pos];
        integral_image_calculate_statistics(source, &stat, x-radius, y-radius, size, size, offset);
        t = (byte)floor(stat.mean * (1.0 + k * ((stat.deviation / R) - 1.0)));
        if (source_value > t) {
          target_data[pos] = value1;
        }
        else {
          target_data[pos] = value2;
        }
      }
    }
  }
  else {
    if (IS_FALSE(use_mean)) {
      dev_max = 0;
      for (y = 0; y < height; y++) {
        pos = y * stride;
        for (x = 0; x < width; x++, pos += step) {
          integral_image_calculate_statistics(source, &stat, x-radius, y-radius, size, size, offset);
          mean_data[pos] = stat.mean;
          dev = stat.deviation;
          dev_data[pos] = dev;
          if (dev > dev_max) dev_max = dev;
        }
      }
      R = dev_max;
    }
    else {
      dev_sum = 0;
      for (y = 0; y < height; y++) {
        pos = y * stride;
        for (x = 0; x < width; x++, pos += step) {
          integral_image_calculate_statistics(source, &stat, x-radius, y-radius, size, size, offset);
          mean_data[pos] = stat.mean;
          dev_sum += dev_data[pos] = stat.deviation;
        }
      }
      R = dev_sum / ((integral_value)(width*height));
    }
    for (y = 0; y < height; y++) {
      pos = y * stride;
      for (x = 0; x < width; x++, pos += step) {
        source_value = source_data[pos];
        mean = mean_data[pos];
        dev = dev_data[pos];
        t = (byte)floor(mean * (1.0 + k * ((dev / R) - 1.0)));
        if (source_value > t) {
          target_data[pos] = value1;
        }
        else {
          target_data[pos] = value2;
        }
      }
    }
  }


  FINALLY(integral_image_threshold_sauvola);
  pixel_image_destroy(&temp_mean);
  pixel_image_destroy(&temp_dev);
  RETURN();
}

/******************************************************************************/

result integral_image_threshold_feng
(
  integral_image *source,
  pixel_image *target,
  truth_value invert,
  uint32 radius1,
  integral_value multiplier,
  truth_value estimate_min,
  integral_value alpha
)
{
  TRY();
  byte *source_data, *target_data, source_value, t, value1, value2;
  integral_value min, mean, dev1, dev2, as, a1, a2, a3, k1, k2, g, asg;
  uint32 x, y, width, height, step, stride, offset, radius2, size1, size2, pos;
  statistics stat;

  CHECK_POINTER(source);
  CHECK_POINTER(target);
  CHECK_PARAM(source->original->type == p_U8);

  CHECK(pixel_image_clone(target, source->original));

  width = target->width;
  height = target->height;
  step = target->step;
  stride = target->stride;
  offset = target->offset;

  g = 2;
  a1 = 0.12;
  k1 = 0.25;
  k2 = 0.04;

  if (IS_TRUE(invert)) {
    value1 = 0;
    value2 = 255;
  }
  else {
    value1 = 255;
    value2 = 0;
  }

  source_data = (byte *)source->original->data;
  target_data = (byte *)target->data;
  size1 = 2 * radius1 + 1;
  radius2 = (uint32)(multiplier * radius1);
  size2 = 2 * radius2 + 1;

  for (y = 0; y < height; y++) {
    pos = y * stride;
    for (x = 0; x < width; x++, pos += step) {
      source_value = source_data[pos];
      integral_image_calculate_statistics(source, &stat, x-radius1, y-radius1, size1, size1, offset);
      mean = stat.mean;
      dev1 = stat.deviation;
      if (IS_TRUE(estimate_min)) {
        min = fmax(0, mean - alpha * dev1);
      }
      else {
        min = pixel_image_find_min_byte(source->original, x-radius1, y-radius1, size1, size1, offset);
      }
      dev2 = sqrt(integral_image_calculate_variance(source, x-radius2, y-radius2, size2, size2, offset));
      as = dev1 / fmax(1,dev2);
      asg = pow(as, g);
      a2 = k1 * asg;
      a3 = k2 * asg;
      t = (byte)floor((1 - a1) * mean + a2 * as * (mean - min) + a3 * min);
      if (source_value > t) {
        target_data[pos] = value1;
      }
      else {
        target_data[pos] = value2;
      }
    }
  }

  FINALLY(integral_image_threshold_feng);
  RETURN();
}

/******************************************************************************/

void integral_image_box_create(
    integral_image_box *target,
    integral_image *source,
    uint32 width,
    uint32 height,
    uint32 dx,
    uint32 dy
    )
{
    target->I_1_data = (I_1_t *)source->I_1.data;
    target->iA = target->I_1_data;
    target->I_2_data = (I_2_t *)source->I_2.data;
    target->i2A = target->I_2_data;
    target->sum = 0;
    target->sumsqr = 0;
    target->offset = 0;
    target->stride = source->stride;
    target->B_inc = width;
    target->C_inc = height * target->stride + width;
    target->D_inc = height * target->stride;
    target->N = width * height;
    target->dx = dx;
    target->dy = dy;
}

/******************************************************************************/

void integral_image_box_resize(
    integral_image_box *target,
    uint32 width,
    uint32 height
    )
{
    target->B_inc = width;
    target->C_inc = height * target->stride + width;
    target->D_inc = height * target->stride;
    target->N = width * height;
}

/******************************************************************************/

void integral_image_box_update(
    integral_image_box *target,
    uint32 x,
    uint32 y
    )
{
    target->offset = (y - target->dy) * target->stride + (x - target->dx);
    target->iA = target->I_1_data + target->offset;
    target->i2A = target->I_2_data + target->offset;
    target->sum =    (I_1_t)(*(target->iA + target->C_inc) + *target->iA - *(target->iA + target->B_inc) - *(target->iA + target->D_inc));
    target->sumsqr = (I_2_t)(*(target->i2A + target->C_inc) + *target->i2A - *(target->i2A + target->B_inc) - *(target->i2A + target->D_inc));
}

/******************************************************************************/

result small_integral_image_create(
    integral_image *target,
    pixel_image *source
    )
{
    TRY();

    CHECK_POINTER(target);
    CHECK_POINTER(source);
    CHECK_POINTER(source->data);
    CHECK_PARAM(source->type == p_U8);

    target->original = source;
    target->width = source->width;
    target->height = source->height;
    target->step = source->step;
    target->I_1.data = NULL;
    target->I_2.data = NULL;

    /* integral image requires one extra row and column at top and left side */
    target->stride = (target->width + 1) * target->step;

    CHECK(pixel_image_create(&target->I_1, p_SI_1, source->format,
            target->width+1, target->height+1, target->step, target->stride));

    CHECK(pixel_image_create(&target->I_2, p_SI_2, source->format,
            target->width+1, target->height+1, target->step, target->stride));

    init_tables();

    FINALLY(integral_image_create);
    RETURN();
}

/******************************************************************************/

void small_integral_image_update_channel(
    integral_image *target,
    uint32 channel
    )
{
    pixel_image *source;

    source = target->original;
    {
        INTEGRAL_IMAGE_UPDATE_DEFINE_VARIABLES(SI_1_t, SI_2_t);
        uint32 intensity, width, height, x, y, h, v, d;
        SINGLE_DISCONTINUOUS_IMAGE_VARIABLES(source, byte);

        width = target->width;
        height = target->height;
        I_1_data = (SI_1_t *)target->I_1.data;
        I_2_data = (SI_2_t *)target->I_2.data;

        /* horizontal offset for integral images */
        h = target->step;
        /* vertical offset for integral images */
        v = target->stride;
        /* diagonal offset for integral images */
        d = target->stride + target->step;

        /* initialize rest of integral images */
        /* add value of this pixel and integrals from top and left */
        /* subtract integral from top left diagonal */
        {
            INTEGRAL_IMAGE_SET_POS(d + channel);
            for (y = 0; y < height; y++) {
                for (x = width, source_pos = source_rows[y] + channel; x--; source_pos += source_step) {
                    intensity = PIXEL_VALUE(source);
                    I_1_SET_VALUE((I_1_GET_VALUE_WITH_OFFSET(v) -
                                   I_1_GET_VALUE_WITH_OFFSET(d)) +
                                   I_1_GET_VALUE_WITH_OFFSET(h) +
                                   intensity);
                    I_2_SET_VALUE((I_2_GET_VALUE_WITH_OFFSET(v) -
                                   I_2_GET_VALUE_WITH_OFFSET(d)) +
                                   I_2_GET_VALUE_WITH_OFFSET(h) +
                                   small_pixel_squared[intensity]);
                    INTEGRAL_IMAGE_ADVANCE_POS(h);
                }
                /* skip one col from the beginning of next row */
                INTEGRAL_IMAGE_ADVANCE_POS(h);
            }
        }
    }
}

/******************************************************************************/

result small_integral_image_update(
    integral_image *target
    )
{
    TRY();
    uint32 i;

    CHECK_POINTER(target);
    CHECK_POINTER(target->original);
    CHECK_POINTER(target->I_1.data);
    CHECK_POINTER(target->I_2.data);
    CHECK_PARAM(target->I_1.type == p_SI_1);
    CHECK_PARAM(target->I_2.type == p_SI_2);

    /* set the image content to 0's */
    /* the first row and column must contain only 0's */
    /* otherwise the algorithm doesn't work correctly */
    CHECK(pixel_image_clear(&target->I_1));
    CHECK(pixel_image_clear(&target->I_2));

    for (i = 0; i < target->original->step; i++) {
        small_integral_image_update_channel(target, i);
    }

    FINALLY(small_integral_image_update);
    RETURN();
}

/******************************************************************************/

void small_integral_image_box_create(
    small_integral_image_box *target,
    integral_image *source,
    uint32 width,
    uint32 height,
    uint32 dx,
    uint32 dy
    )
{
    target->I_1_data = (SI_1_t *)source->I_1.data;
    target->iA = target->I_1_data;
    target->I_2_data = (SI_2_t *)source->I_2.data;
    target->i2A = target->I_2_data;
    target->sum = 0;
    target->sumsqr = 0;
    target->offset = 0;
    target->step = source->step;
    target->stride = source->stride;
    target->dx = dx;
    target->dy = dy;
    target->channel = 0;

    small_integral_image_box_resize(target, width, height);
}

/******************************************************************************/

void small_integral_image_box_resize(
    small_integral_image_box *target,
    uint32 width,
    uint32 height
    )
{
    target->B_inc = width * target->step;
    target->C_inc = height * target->stride + width * target->step;
    target->D_inc = height * target->stride;
    target->N = width * height;
}

/******************************************************************************/

void small_integral_image_box_update(
    small_integral_image_box *target,
    uint32 x,
    uint32 y
    )
{
    target->offset = (y - target->dy) * target->stride + (x - target->dx) * target->step + target->channel;
    target->iA = target->I_1_data + target->offset;
    target->i2A = target->I_2_data + target->offset;
    target->sum =    (SI_1_t)(*(target->iA + target->C_inc) + *target->iA - *(target->iA + target->B_inc) - *(target->iA + target->D_inc));
    target->sumsqr = (SI_2_t)(*(target->i2A + target->C_inc) + *target->i2A - *(target->i2A + target->B_inc) - *(target->i2A + target->D_inc));
}

/******************************************************************************/
