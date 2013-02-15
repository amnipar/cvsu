/**
 * @file cvsu_integral.c
 * @author Matti J. Eskelinen <matti.j.eskelinen@gmail.com>
 * @brief Integral image types and operations for cvsu.
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
#include "cvsu_integral.h"
#include "cvsu_memory.h"

#include <math.h>

/******************************************************************************/
/* some gcc versions seem to require these definitions to work properly       */
/* remove them if they cause problems with other compilers                    */

#ifdef NEED_MINMAX
#if INTEGRAL_IMAGE_DATA_TYPE == INTEGRAL_IMAGE_USING_FLOAT
float fminf(float __x, float __y);
float fmaxf(float __x, float __y);
#define getmin fminf
#define getmax fmaxf
#elif INTEGRAL_IMAGE_DATA_TYPE == INTEGRAL_IMAGE_USING_DOUBLE
double fmin(double __x, double __y);
double fmax(double __x, double __y);
#define getmin fmin
#define getmax fmax
#else
#error "integral image data type not defined"
#endif
#endif

/******************************************************************************/
/* constants for reporting function names in error messages                   */

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

/******************************************************************************/
/* constants for lookup tables                                                */

truth_value tables_initialized = FALSE;
integral_value pixel_squared[256];
#ifdef INTEGRAL_IMAGE_HIGHER_ORDER_STATISTICS
integral_value pixel_cubed[256];
integral_value pixel_fourth[256];
#endif
SI_2_t small_pixel_squared[256];

/******************************************************************************/

void init_tables()
{
  if (IS_FALSE(tables_initialized)) {
    uint32 i;
    for (i = 0; i < 256; i++) {
      pixel_squared[i] = (integral_value)(i * i);
#ifdef INTEGRAL_IMAGE_HIGHER_ORDER_STATISTICS
      pixel_cubed[i] = (integral_value)(i * i * i);
      pixel_fourth[i] = (integral_value)(i * i * i * i);
#endif
      small_pixel_squared[i] = (SI_2_t)(i * i);
    }
    tables_initialized = TRUE;
  }
}

/******************************************************************************/

integral_image *integral_image_alloc()
{
  TRY();
  integral_image *target;

  CHECK(memory_allocate((data_pointer *)&target, 1, sizeof(integral_image)));
  CHECK(integral_image_nullify(target));

  FINALLY(integral_image_alloc);
  return target;
}

/******************************************************************************/

void integral_image_free
(
  integral_image *target
)
{
  TRY();

  r = SUCCESS;

  if (target != NULL) {
    CHECK(integral_image_destroy(target));
    CHECK(memory_deallocate((data_pointer *)&target));
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

truth_value integral_image_is_null
(
  integral_image *target
)
{
  if (target != NULL) {
    if (target->original == NULL) {
      return TRUE;
    }
  }
  else {
    return TRUE;
  }
  return FALSE;
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
    uint32 width, height, x, y, h, v, d;
    byte intensity;
    SINGLE_DISCONTINUOUS_IMAGE_VARIABLES(source, byte);

    /* set the image content to 0's */
    /* the first row and column must contain only 0's */
    /* otherwise the algorithm doesn't work correctly */
    CHECK(pixel_image_clear(&target->I_1));
    CHECK(pixel_image_clear(&target->I_2));

    width = target->width;
    height = target->height;
    I_1_data = (integral_value*)target->I_1.data;
    I_2_data = (integral_value*)target->I_2.data;

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
                         ((integral_value)intensity));
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
  image_rect irect;
  register uint32 width, height, step, stride;

  width = target->width;
  height = target->height;

  irect.valid = 0;
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
      if (((unsigned)(x + dx)) > width) dx = ((signed)width) - x;
      if (((unsigned)(y + dy)) > height) dy = ((signed)height) - y;

      step = target->step;
      stride = target->stride;
      irect.valid = 1;
      irect.offset = ((unsigned)y) * stride + ((unsigned)x) * step + offset;
      irect.hstep = ((unsigned)dx) * step;
      irect.vstep = ((unsigned)dy) * stride;
      irect.N = ((unsigned)dx) * ((unsigned)dy);
    }
  }

  return irect;
}

/******************************************************************************/

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
  image_rect irect;
  integral_value *iA, sum;

  irect = integral_image_create_rect(target, x, y, dx, dy, offset);
  if (irect.valid == 0) {
    return 0;
  }
  else {
    iA = ((integral_value *)target->I_1.data) + irect.offset;
    sum = *(iA + irect.vstep + irect.hstep) + *iA - *(iA + irect.hstep) - *(iA + irect.vstep);
    return sum / ((integral_value)irect.N);
  }
}

/******************************************************************************/

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
  image_rect irect;
  integral_value *iA, *i2A, mean, sum2, var;

  irect = integral_image_create_rect(target, x, y, dx, dy, offset);
  if (irect.valid == 0) {
    return 0;
  }
  else {
    iA = ((integral_value *)target->I_1.data) + irect.offset;
    i2A = ((integral_value *)target->I_2.data) + irect.offset;
    mean = (*(iA + irect.vstep + irect.hstep) + *iA - *(iA + irect.hstep) - *(iA + irect.vstep)) / ((integral_value)irect.N);
    sum2 = *(i2A + irect.vstep + irect.hstep) + *i2A - *(i2A + irect.hstep) - *(i2A + irect.vstep);
    var = (sum2 / ((integral_value)irect.N)) - mean*mean;
    if (var < 0) var = 0;
    return var;
  }
}

/******************************************************************************/

void integral_image_calculate_statistics
(
  integral_image *target,
  statistics *stat,
  sint32 x,
  sint32 y,
  sint32 dx,
  sint32 dy,
  uint32 offset
)
{
  image_rect irect;
  integral_value *iA, *i2A, N, sum, sum2, mean, var;

  statistics_init(stat);
  irect = integral_image_create_rect(target, x, y, dx, dy, offset);
  if (irect.valid == 0) {
    return;
  }
  else {
    iA = ((integral_value *)target->I_1.data) + irect.offset;
    i2A = ((integral_value *)target->I_2.data) + irect.offset;
    N = ((integral_value)irect.N);
    sum = *(iA + irect.vstep + irect.hstep) + *iA - *(iA + irect.hstep) - *(iA + irect.vstep);
    sum2 = *(i2A + irect.vstep + irect.hstep) + *i2A - *(i2A + irect.hstep) - *(i2A + irect.vstep);
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
  sint32 radius,
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
  sint32 x, y, width, height, step, stride, size, pos;
  uint32 offset;
  statistics stat;

  CHECK_POINTER(source);
  CHECK_POINTER(target);
  CHECK_PARAM(source->original->type == p_U8);

  CHECK(pixel_image_clone(target, source->original));

  width = ((signed)target->width);
  height = ((signed)target->height);
  step = ((signed)target->step);
  stride = ((signed)target->stride);
  offset = target->offset;

  if (IS_TRUE(invert)) {
    value1 = 0;
    value2 = 255;
  }
  else {
    value1 = 255;
    value2 = 0;
  }

  CHECK(pixel_image_create(&temp_mean, p_I, target->format, ((unsigned)width),
                           ((unsigned)height), ((unsigned)step),
                           ((unsigned)stride)));
  CHECK(pixel_image_create(&temp_dev, p_I, target->format, ((unsigned)width),
                           ((unsigned)height), ((unsigned)step),
                           ((unsigned)stride)));

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
        integral_image_calculate_statistics(source, &stat, x-radius, y-radius,
                                            size, size, offset);
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
          integral_image_calculate_statistics(source, &stat, x-radius, y-radius,
                                              size, size, offset);
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
          integral_image_calculate_statistics(source, &stat, x-radius, y-radius,
                                              size, size, offset);
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
  sint32 radius1,
  integral_value multiplier,
  truth_value estimate_min,
  integral_value alpha
)
{
  TRY();
  byte *source_data, *target_data, source_value, t, value1, value2;
  integral_value min, mean, dev1, dev2, as, a1, a2, a3, k1, k2, g, asg;
  sint32 x, y, width, height, step, stride, radius2, size1, size2, pos;
  uint32 offset;
  statistics stat;

  CHECK_POINTER(source);
  CHECK_POINTER(target);
  CHECK_PARAM(source->original->type == p_U8);

  CHECK(pixel_image_clone(target, source->original));

  width = ((signed)target->width);
  height = ((signed)target->height);
  step = ((signed)target->step);
  stride = ((signed)target->stride);
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
  size1 = 2 * ((signed)radius1) + 1;
  radius2 = (sint32)(multiplier * ((integral_value)radius1));
  size2 = 2 * radius2 + 1;

  for (y = 0; y < height; y++) {
    pos = y * stride;
    for (x = 0; x < width; x++, pos += step) {
      source_value = source_data[pos];
      integral_image_calculate_statistics(source, &stat, x-radius1,y-radius1,
                                          size1, size1, offset);
      mean = stat.mean;
      dev1 = stat.deviation;
      if (IS_TRUE(estimate_min)) {
        min = fmax(0, mean - alpha * dev1);
      }
      else {
        min = pixel_image_find_min_byte(source->original, x-radius1, y-radius1,
                                        size1, size1, offset);
      }
      dev2 = sqrt(integral_image_calculate_variance(source, x-radius2, y-radius2,
                                                    size2, size2, offset));
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

result small_integral_image_create
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

void small_integral_image_update_channel
(
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

result small_integral_image_update
(
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

/* end of file                                                                */
/******************************************************************************/
