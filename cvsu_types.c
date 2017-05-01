/**
 * @file cvsu_types.c
 * @author Matti J. Eskelinen <matti.j.eskelinen@gmail.com>
 * @brief General type definitions for cvsu.
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

#include "cvsu_types.h"
#include "cvsu_memory.h"
#include "cvsu_macros.h"

/******************************************************************************/
/* constants for reporting function names in error messages                   */

string statistics_alloc_name = "statistics_alloc";
string raw_moments_alloc_name = "raw_moments_alloc";

/******************************************************************************/

sint32 signum
(
  integral_value value
)
{
  if (value < -0.000001) return -1;
  if (value > 0.000001) return +1;
  return 0;
}

/******************************************************************************/

typedef real (*pixel_casting_function)
(
  void *data,
  uint32 offset
);

/******************************************************************************/

real cast_none
(
  void *data,
  uint32 offset
)
{
  (void)data;
  (void)offset;
  return 0;
}

/******************************************************************************/

real cast_u8
(
  void *data,
  uint32 offset
)
{
  return (real)*(((byte *)data) + offset);
}

/******************************************************************************/

real cast_s8
(
  void *data,
  uint32 offset
)
{
  return (real)*(((char *)data) + offset);
}

/******************************************************************************/

real cast_u16
(
  void *data,
  uint32 offset
)
{
  return (real)*(((uint16 *)data) + offset);
}

/******************************************************************************/

real cast_s16
(
  void *data,
  uint32 offset
)
{
  return (real)*(((sint16 *)data) + offset);
}

/******************************************************************************/

real cast_u32
(
  void *data,
  uint32 offset
)
{
  return (real)*(((uint32 *)data) + offset);
}

/******************************************************************************/

real cast_s32
(
  void *data,
  uint32 offset
)
{
  return (real)*(((sint32 *)data) + offset);
}

/******************************************************************************/
/*
real cast_u64(void *data, uint32 offset)
{
  return (real)*(((uint64 *)data) + offset);
}
*/
/******************************************************************************/
/*
real cast_s64(void *data, uint32 offset)
{
  return (real)*(((sint64 *)data) + offset);
}
*/
/******************************************************************************/

real cast_f32
(
  void *data,
  uint32 offset
)
{
  return (real)*(((real32 *)data) + offset);
}

/******************************************************************************/

real cast_f64
(
  void *data,
  uint32 offset
)
{
  return (real)*(((real64 *)data) + offset);
}

/******************************************************************************/

pixel_casting_function casts[] = {
  &cast_none,
  &cast_u8,
  &cast_s8,
  &cast_u16,
  &cast_s16,
  &cast_u32,
  &cast_s32,
  /*
  &cast_u64,
  &cast_s64,
  */
  &cast_f32,
  &cast_f64
};

/******************************************************************************/

real cast_pixel_value
(
  void *data,
  pixel_type type,
  uint32 offset
)
{
  return (casts[(uint32)type])(data, offset);
}

/******************************************************************************/

real pixel_value_cache
(
  pixel_value *target,
  void *data,
  pixel_type type,
  uint32 token
)
{
  /*printf("cache i\n");*/
  if (target != NULL && data != NULL) {
    if (target->token != token) {
      target->cache = cast_pixel_value(data, type, target->offset);
      target->token = token;
    }
    /*printf("cache o %.3f\n", target->cache);*/
    return target->cache;
  }
  return 0;
}

/******************************************************************************/

statistics *statistics_alloc
()
{
  TRY();
  statistics *ptr;

  CHECK(memory_allocate((data_pointer*)&ptr, 1, sizeof(statistics)));

  ptr->N = 0;
  ptr->sum = 0;
  ptr->sum2 = 0;
  ptr->mean = 0;
  ptr->variance = 0;
  ptr->deviation = 0;

  FINALLY(statistics_alloc);
  return ptr;
}

/******************************************************************************/

void statistics_free
(
  statistics *ptr
)
{
  if (ptr != NULL) {
    memory_deallocate((data_pointer*)&ptr);
  }
}

/******************************************************************************/

void statistics_init
(
  statistics *stat
)
{
  stat->mean = 0;
  stat->variance = 0;
#ifdef INTEGRAL_IMAGE_HIGHER_ORDER_STATISTICS
  stat->skewness = 0;
  stat->kurtosis = 0;
#endif
}

/******************************************************************************/

raw_moments *raw_moments_alloc
()
{
  TRY();
  raw_moments *ptr;

  CHECK(memory_allocate((data_pointer*)&ptr, 1, sizeof(raw_moments)));

  ptr->m00 = 0;
  ptr->m10 = 0;
  ptr->m01 = 0;
  ptr->m11 = 0;
  ptr->m20 = 0;
  ptr->m02 = 0;

  FINALLY(raw_moments_alloc);
  return ptr;
}

/******************************************************************************/

void raw_moments_free
(
  raw_moments *ptr
)
{
  if (ptr != NULL) {
    memory_deallocate((data_pointer*)&ptr);
  }
}

/******************************************************************************/

void point_create
(
  point *target,
  coord x,
  coord y
)
{
  if (target != NULL) {
    target->x = x;
    target->y = y;
  }
}

/******************************************************************************/

void point_add
(
  point *target,
  coord x,
  coord y
)
{
  if (target != NULL) {
    target->x += x;
    target->y += y;
  }
}

/******************************************************************************/

void point_subtract
(
  point *target,
  coord x,
  coord y
)
{
  if (target != NULL) {
    target->x -= x;
    target->y -= y;
  }
}

/******************************************************************************/

void line_create
(
  line *target,
  coord start_x,
  coord start_y,
  coord end_x,
  coord end_y
)
{
  if (target != NULL) {
    target->start.x = start_x;
    target->start.y = start_y;
    target->end.x = end_x;
    target->end.y = end_y;
  }
}

/******************************************************************************/

void line_create_from_points
(
  line *target,
  point start,
  point end
)
{
  if (target != NULL) {
    target->start = start;
    target->end = end;
  }
}

/******************************************************************************/

void rect_create
(
  rect *target,
  coord left,
  coord right,
  coord top,
  coord bottom
)
{
  if (target != NULL) {
    if (left < right) {
      target->left = left;
      target->right = right;
    }
    else {
      target->left = right;
      target->right = left;
    }
    if (top < bottom) {
      target->top = top;
      target->bottom = bottom;
    }
    else {
      target->top = bottom;
      target->bottom = top;
    }
  }
}

/******************************************************************************/

void rect_create_from_points
(
  rect *target,
  point first,
  point second
)
{
  if (target != NULL) {
    if (first.x < second.x) {
      target->left = first.x;
      target->right = second.x;
    }
    else {
      target->left = second.x;
      target->right = first.x;
    }
    if (first.y < second.y) {
      target->top = first.y;
      target->bottom = second.y;
    }
    else {
      target->top = second.y;
      target->bottom = first.y;
    }
  }
}

/******************************************************************************/
/* utility functions for angles                                               */

/**
 * Calculates the combined angle; this means the total amount of rotation, if
 * first rotate by angle1, then rotate more by angle2.
 * Note: positive rotation direction is _counterclockwise_.
 * Takes into account crossing the origin (0 / 2PI).
 */
integral_value angle_plus_angle
(
  integral_value angle1,
  integral_value angle2
)
{
  (void)angle1;
  (void)angle2;
  return 0;
}

/**
 * Calculates the angle distance; this means the amount of rotation needed to
 * go from angle2 to angle1, by _shortest path_.
 * Note: positive rotation direction is _counterclockwise_.
 * Takes into account crossing the origin (0 / 2PI).
 */
integral_value angle_minus_angle
(
  integral_value angle1,
  integral_value angle2
)
{
  integral_value diff = angle1 - angle2;
  if (diff > M_PI) {
    return diff - M_2PI;
  }
  if (diff < -M_PI) {
    return diff + M_2PI;
  }
  return diff;
}

/* end of file                                                                */
/******************************************************************************/
