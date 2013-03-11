/**
 * @file cvsu_temporal_forest.c
 * @author Matti J. Eskelinen <matti.j.eskelinen@gmail.com>
 * @brief Temporal Forest structure for analyzing image changes over time.
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

#include "cvsu_temporal_forest.h"
#include "cvsu_macros.h"
#include "cvsu_memory.h"

/******************************************************************************/
/* constants for reporting function names in error messages                   */

string temporal_forest_alloc_name = "temporal_forest_alloc";
string temporal_forest_free_name = "temporal_forest_free";
string temporal_forest_create_name = "temporal_forest_create";
string temporal_forest_destroy_name = "temporal_forest_destroy";
string temporal_forest_nullify_name = "temporal_forest_nullify";
string temporal_forest_update_name = "temporal_forest_update";
string temporal_forest_visualize_name = "temporal_forest_visualize";

/******************************************************************************/

temporal_forest *temporal_forest_alloc()
{
  TRY();
  temporal_forest *forest;

  CHECK(memory_allocate((data_pointer*)&forest, 1, sizeof(temporal_forest)));
  CHECK(temporal_forest_nullify(forest));

  FINALLY(temporal_forest_alloc);
  return forest;
}

/******************************************************************************/

void temporal_forest_free
(
  temporal_forest *forest
)
{
  TRY();

  r = SUCCESS;
  if (forest != NULL) {
    CHECK(temporal_forest_destroy(forest));
    CHECK(memory_deallocate((data_pointer*)&forest));
  }

  FINALLY(temporal_forest_free);
}

/******************************************************************************/

result temporal_forest_create
(
  temporal_forest *target,
  pixel_image *source,
  uint32 max_size,
  uint32 min_size,
  uint32 frame_count,
  uint32 history_count
)
{
  TRY();
  uint32 i;

  CHECK_POINTER(target);
  CHECK_POINTER(source);
  CHECK_TRUE(temporal_forest_is_null(target));
  CHECK_PARAM(min_size > 0);
  CHECK_PARAM(max_size >= min_size);
  CHECK_PARAM(frame_count > 0);

  CHECK(memory_allocate((data_pointer*)&target->forests, frame_count, sizeof(quad_forest)));
  for (i = 0; i < frame_count; i++) {
    CHECK(quad_forest_create(&target->forests[i], source, max_size, min_size));
  }
  target->count = frame_count;
  target->current = -1;
  target->frames = 0;
  CHECK(pixel_image_create(&target->visual, p_U8, RGB, source->width, source->height, source->step, source->stride));
  target->tree_max_size = max_size;
  target->tree_min_size = min_size;
  target->rows = target->forests[0].rows;
  target->cols = target->forests[0].cols;
  target->dx = target->forests[0].dx;
  target->dy = target->forests[0].dy;
  if (history_count > 0) {
    /* create background forest target->background */
  }

  FINALLY(temporal_forest_create);
  RETURN();
}

/******************************************************************************/

result temporal_forest_destroy
(
  temporal_forest *target
)
{
  TRY();

  CHECK(memory_deallocate((data_pointer*)&target->frames));
  CHECK(pixel_image_destroy(&target->visual));
  CHECK(temporal_forest_nullify(target));

  FINALLY(temporal_forest_destroy);
  RETURN();
}

/******************************************************************************/

result temporal_forest_nullify
(
  temporal_forest *target
)
{
  TRY();

  CHECK_POINTER(target);

  CHECK(pixel_image_nullify(&target->visual));
  target->background = NULL;
  target->forests = NULL;
  target->rows = 0;
  target->cols = 0;
  target->tree_max_size = 0;
  target->tree_min_size = 0;
  target->dx = 0;
  target->dy = 0;
  target->count = 0;
  target->current = 0;
  target->frames = 0;

  FINALLY(temporal_forest_nullify);
  RETURN();
}

/******************************************************************************/

truth_value temporal_forest_is_null
(
  temporal_forest *target
)
{
  if (target != NULL) {
    if (target->forests == NULL) {
      return TRUE;
    }
  }
  return FALSE;
}

/******************************************************************************/

result temporal_forest_update
(
  temporal_forest *target,
  pixel_image *source
)
{
  TRY();

  CHECK_POINTER(target);
  CHECK_POINTER(source);

  target->current++;
  if (target->current >= target->count) {
    target->current = 0;
  }
  CHECK(pixel_image_copy(target->forests[0].source, source));
  CHECK(quad_forest_update(&target->forests[0]));
  target->frames++;

  FINALLY(temporal_forest_update);
  RETURN();
}

/******************************************************************************/

result temporal_forest_visualize
(
  temporal_forest *target
)
{
  TRY();
  list_item *trees, *end;
  quad_forest *forest;
  quad_tree *tree;
  uint32 x, y, width, height, stride, row_step;
  byte *target_data, *target_pos, color0, color1, color2;

  CHECK_POINTER(target);
  CHECK(target->current >= 0);

  forest = &target->forests[target->current];
  width = target->visual.width;
  height = target->visual.height;
  stride = target->visual.stride;
  target_data = (byte*)target->visual.data;

  CHECK(pixel_image_clear(&target->visual));

  trees = forest->trees.first.next;
  while (trees != &forest->trees.last) {
    tree = (quad_tree *)trees->data;
    if (tree->nw == NULL) {
      color0 = (byte)tree->stat.mean;
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
    trees = trees->next;
  }

FINALLY(temporal_forest_visualize);
  RETURN();
}

/* end of file                                                                */
/******************************************************************************/
