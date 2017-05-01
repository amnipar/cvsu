/**
 * @file cvsu_connected_components.c
 * @author Matti J. Eskelinen <matti.j.eskelinen@gmail.com>
 * @brief Connected components handling for cvsu.
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
#include "cvsu_connected_components.h"

#include <stdlib.h> /* for srand, rand */

/******************************************************************************/
/* constants for reporting function names in error messages                   */

string connected_components_alloc_name = "connected_components_alloc";
string connected_components_free_name = "connected_components_free";
string connected_components_create_name = "connected_components_create";
string connected_components_destroy_name = "connected_components_destroy";
string connected_components_nullify_name = "connected_components_nullify";
string connected_components_update_name = "connected_components_update";
string connected_components_draw_image_name = "connected_components_draw_image";

/******************************************************************************/
/* private functions for handling union-find of regions                       */

region_info *region_find(region_info *region)
{
  if (region != NULL) {
    if (region->id != region && region->id != NULL) {
      region->id = region_find(region->id);
    }
    return region->id;
  }
  return NULL;
}

/******************************************************************************/

void region_union(region_info *region1, region_info *region2)
{
  if (region1 != NULL && region2 != NULL) {
    region_info *id1, *id2;

    id1 = region_find(region1);
    id2 = region_find(region2);
    if (id1 == NULL || id2 == NULL) {
      return;
    }
    /* if the trees are already in the same class, no need for union */
    if (id1 == id2) {
      return;
    }
    /* otherwise set the tree with higher class rank as id of the union */
    else {
      if (id1->rank < id2->rank) {
        id1->id = id2;
        id2->x1 = (id1->x1 < id2->x1) ? id1->x1 : id2->x1;
        id2->y1 = (id1->y1 < id2->y1) ? id1->y1 : id2->y1;
        id2->x2 = (id1->x2 > id2->x2) ? id1->x2 : id2->x2;
        id2->y2 = (id1->y2 > id2->y2) ? id1->y2 : id2->y2;
      }
      else
      if (id1->rank > id2->rank) {
        id2->id = id1;
        id1->x1 = (id1->x1 < id2->x1) ? id1->x1 : id2->x1;
        id1->y1 = (id1->y1 < id2->y1) ? id1->y1 : id2->y1;
        id1->x2 = (id1->x2 > id2->x2) ? id1->x2 : id2->x2;
        id1->y2 = (id1->y2 > id2->y2) ? id1->y2 : id2->y2;
      }
      /* when equal rank trees are combined, the root tree's rank is increased */
      else {
        id2->id = id1;
        id1->rank += 1;
        id1->x1 = (id1->x1 < id2->x1) ? id1->x1 : id2->x1;
        id1->y1 = (id1->y1 < id2->y1) ? id1->y1 : id2->y1;
        id1->x2 = (id1->x2 > id2->x2) ? id1->x2 : id2->x2;
        id1->y2 = (id1->y2 > id2->y2) ? id1->y2 : id2->y2;
      }
    }
  }
}

/******************************************************************************/
/* connected_components public functions                                      */

connected_components *connected_components_alloc()
{
  TRY();
  connected_components *target;

  CHECK(memory_allocate((data_pointer *)&target, 1, sizeof(connected_components)));
  CHECK(connected_components_nullify(target));

  FINALLY(connected_components_alloc);
  return target;
}

/******************************************************************************/

void connected_components_free
(
  connected_components *target
)
{
  TRY();

  r = SUCCESS;

  if (target != NULL) {
    CHECK(connected_components_destroy(target));
    CHECK(memory_deallocate((data_pointer *)&target));
  }

  FINALLY(connected_components_free);
}

/******************************************************************************/

result connected_components_create
(
  connected_components *target,
  pixel_image *source
)
{
  TRY();
  uint32 x, y, width, height;
  region_info *pixel;

  CHECK_POINTER(target);
  CHECK_POINTER(source);
  CHECK_PARAM(source->type == p_U8);

  connected_components_nullify(target);

  width = source->width;
  height = source->height;

  CHECK(memory_allocate((data_pointer *)&target->pixels, width*height, sizeof(region_info)));
  CHECK(memory_clear((data_pointer)target->pixels, width*height, sizeof(region_info)));

  target->original = source;
  target->width = width;
  target->height = height;
  target->channels = source->step;
  
  {
    SINGLE_DISCONTINUOUS_IMAGE_VARIABLES(source, byte);

    for (y = 0, pixel = target->pixels; y < height; y++) {
      for (x = 0, source_pos = source_rows[y]; x < width; x++, source_pos += source_step, pixel++) {
        pixel->id = pixel;
        pixel->x1 = x;
        pixel->y1 = y;
        pixel->x2 = x;
        pixel->y2 = y;
        pixel->value = source_pos;
      }
    }
  }

  FINALLY(connected_components_create);
  RETURN();
}

/******************************************************************************/

result connected_components_destroy
(
  connected_components *target
)
{
  TRY();

  CHECK_POINTER(target);

  if (target->pixels != NULL) {
      CHECK(memory_deallocate((data_pointer*)&target->pixels));
  }
  if (target->regions != NULL) {
      CHECK(memory_deallocate((data_pointer*)&target->regions));
  }
  connected_components_nullify(target);

  FINALLY(connected_components_destroy);
  RETURN();
}

/******************************************************************************/

result connected_components_nullify
(
  connected_components *target
)
{
  TRY();

  CHECK_POINTER(target);

  target->original = NULL;
  target->width = 0;
  target->height = 0;
  target->channels = 0;
  target->pixels = NULL;
  target->regions = NULL;
  target->count = 0;

  FINALLY(connected_components_nullify);
  RETURN();
}

/******************************************************************************/

truth_value connected_components_is_null
(
  connected_components *target
)
{
  if (target != NULL) {
    if (target->original != NULL && target->pixels != NULL) {
      return FALSE;
    }
  }
  return TRUE;
}

/******************************************************************************/

#define COMPARE_REGIONS()\
  is_equal = 1;\
  for (i = 0; i < channels; i++) {\
    if (pixel->value[i] != neighbor->value[i]) {\
      pixel->is_border = 1;\
      neighbor->is_border = 1;\
      is_equal = 0;\
      break;\
    }\
  }\
  if (is_equal != 0) {\
    region_union(pixel, neighbor);\
  }

result connected_components_update
(
  connected_components *target
)
{
  TRY();
  uint32 x, y, width, height, i, channels, count;
  truth_value is_equal;
  region_info *pixel, *neighbor, *id, **region;

  CHECK_POINTER(target);

  width = target->width;
  height = target->height;
  channels = target->channels;

  /* first, handle first row comparing only to left */
  y = 0;
  pixel = target->pixels;
  neighbor = pixel++;
  for (x = 1; x < width; x++, pixel++) {
    COMPARE_REGIONS();
    neighbor = pixel;
  }
  for (y = 1; y < height; y++) {
    /* for the first item of each row, compare only to top */
    neighbor = pixel - width;
    COMPARE_REGIONS();
    neighbor = pixel++;
    for (x = 1; x < width; x++, pixel++) {
      /* first compare to left*/
      COMPARE_REGIONS();
      neighbor = pixel - width;
      /* then compare to top */
      COMPARE_REGIONS();
      neighbor = pixel;
    }
  }

  /* count the regions and set up colors */
  count = 0;
  /* initialize the random number generator for generating the colors */
  srand(1234);
  for (i = width*height, pixel = target->pixels; i--; pixel++) {
    id = region_find(pixel);
    if (id == pixel) {
      count++;
      pixel->color[0] = (byte)(rand() % 256);
      pixel->color[1] = (byte)(rand() % 256);
      pixel->color[2] = (byte)(rand() % 256);
    }
  }

  CHECK(memory_allocate((void*)&target->regions, count, sizeof(region_info*)));
  target->count = count;

  /* finally, collect the parent regions to a list */
  for (i = width*height, pixel = target->pixels, region = target->regions; i--; pixel++) {
    id = region_find(pixel);
    if (id == pixel) {
      *region = pixel;
      region++;
    }
  }

  FINALLY(connected_components_update);
  RETURN();
}

/******************************************************************************/

result connected_components_draw_image
(
  connected_components *source,
  pixel_image *target
)
{
  TRY();
  uint32 width, height, i;
  region_info *pixel, *region;
  byte *target_pos;

  CHECK_POINTER(source);
  CHECK_POINTER(source->pixels);
  CHECK_POINTER(target);

  width = source->width;
  height = source->height;
/*
  CHECK(pixel_image_create(target, p_U8, RGB, width, height, 3, 3 * width));
*/
  target_pos = (byte *)target->data;
  for (i = width*height, pixel = source->pixels; i--; pixel++) {
    region = region_find(pixel);
    *target_pos = region->color[0];
    target_pos++;
    *target_pos = region->color[1];
    target_pos++;
    *target_pos = region->color[2];
    target_pos++;
  }

  FINALLY(connected_components_draw_image);
  RETURN();
}

/* end of file                                                                */
/******************************************************************************/
