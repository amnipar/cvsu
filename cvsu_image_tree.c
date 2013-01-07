/**
 * @file cvsu_image_tree.c
 * @author Matti J. Eskelinen <matti.j.eskelinen@gmail.com>
 * @brief Quad-tree-like hierarchical data structure for images.
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

#include "cvsu_image_tree.h"
#include "cvsu_macros.h"
#include "cvsu_memory.h"

#include <stdlib.h>
/*#include <stdio.h>*/
#include <sys/time.h>
#include <math.h>

double fmin (double __x, double __y);
double fmax (double __x, double __y);

/******************************************************************************/
/* constants for reporting function names in error messages                   */

string image_tree_nullify_name = "image_tree_nullify";
string image_tree_forest_init_name = "image_tree_forest_init";
string image_tree_forest_alloc_name = "image_tree_forest_alloc";
string image_tree_forest_free_name = "image_tree_forest_free";
string image_tree_forest_create_name = "image_tree_forest_create";
string image_tree_forest_reload_name = "image_tree_forest_reload";
string image_tree_forest_destroy_name = "image_tree_forest_destroy";
string image_tree_forest_nullify_name = "image_tree_forest_nullify";
string image_tree_forest_update_prepare_name = "image_tree_forest_update_prepare";
string image_tree_forest_update_name = "image_tree_forest_update";
string image_tree_forest_segment_with_deviation_name = "image_tree_forest_segment_with_deviation";
string image_tree_forest_segment_with_entropy_name = "image_tree_forest_segment_with_entropy";
string image_tree_forest_get_regions_name = "image_tree_forest_get_regions";
string image_tree_forest_draw_image_name = "image_tree_forest_draw_image";
string image_tree_root_update_name = "image_tree_root_update";
string image_tree_update_name = "image_tree_update";
string image_tree_calculate_consistency_name = "image_tree_calculate_consistency";
string image_tree_divide_name = "image_tree_divide";
string image_tree_get_statistics_name = "image_tree_get_statistics";
string image_tree_get_child_statistics_name = "image_tree_get_child_statistics";
string image_tree_divide_with_entropy_name = "image_tree_divide_with_entropy";
string image_tree_create_neighbor_list_name = "image_tree_create_neighbor_list";
string image_tree_get_direct_neighbor_name = "image_tree_get_direct_neighbor";
string image_tree_get_direct_neighbor_n_name = "image_tree_get_direct_neighbor_n";
string image_tree_get_direct_neighbor_e_name = "image_tree_get_direct_neighbor_e";
string image_tree_get_direct_neighbor_s_name = "image_tree_get_direct_neighbor_s";
string image_tree_get_direct_neighbor_w_name = "image_tree_get_direct_neighbor_w";
string image_tree_add_children_as_immediate_neighbors_name = "image_tree_add_children_as_immediate_neighbors";
string image_tree_find_all_immediate_neighbors_name = "image_tree_find_all_immediate_neighbors";

/******************************************************************************/

result image_tree_nullify
(
  image_tree *target
)
{
  TRY();

  CHECK(memory_clear((data_pointer)target, 1, sizeof(image_tree)));
  /*
  target->root = NULL;
  target->parent = NULL;
  target->nw = NULL;
  target->ne = NULL;
  target->sw = NULL;
  target->se = NULL;
  target->n = NULL;
  target->e = NULL;
  target->s = NULL;
  target->w = NULL;
  target->level = 0;
  target->region_info.id = NULL;
  target->region_info.rank = 0;
  target->region_info.x1 = 0;
  target->region_info.y1 = 0;
  target->region_info.x2 = 0;
  target->region_info.y2 = 0;
  */
  FINALLY(image_tree_nullify);
  RETURN();
}

/******************************************************************************/

bool image_tree_is_null
(
  image_tree *target
)
{
  if (target != NULL) {
    if (target->root != NULL) {
      return false;
    }
  }
  return true;
}

/******************************************************************************/
/* private function for initializing tree structure                           */
/* used in create and in reload                                               */

result image_tree_forest_init
(
  image_tree_forest *target,
  uint16 tree_width,
  uint16 tree_height,
  image_block_type type
)
{
  TRY();
  uint32 row, col, pos, size, width, height;
  image_tree new_tree, *tree_ptr;
  image_block new_block, *block_ptr;
  /*printf("init w=%u h=%u t=%d\n", tree_width, tree_height, type);*/

  /* not necessary to check target pointer, calling function should handle that */

  width = target->original->width;
  height = target->original->height;

  CHECK_PARAM(tree_width <= width);
  CHECK_PARAM(tree_height <= height);

  /* for now, support grey and color stat types only */
  CHECK_PARAM(type == b_STAT_GREY || type == b_STAT_COLOR || type == b_STATISTICS);

  if (target->tree_width != tree_width || target->tree_height != tree_height) {
    /*printf("initialize values\n");*/
    /* initialize values */
    target->tree_width = tree_width;
    target->tree_height = tree_height;
    target->cols = (uint16)(width / target->tree_width);
    target->rows = (uint16)(height / target->tree_height);
    target->dx = (uint16)((width - (target->cols * target->tree_width)) / 2);
    target->dy = (uint16)((height - (target->rows * target->tree_height)) / 2);
    size = target->rows * target->cols;

    if (target->roots != NULL) {
      CHECK(memory_deallocate((data_pointer*)&target->roots));
    }
    CHECK(memory_allocate((data_pointer *)&target->roots, size, sizeof(image_tree_root)));

    if (!list_is_null(&target->trees)) {
      CHECK(list_destroy(&target->trees));
    }
    CHECK(list_create(&target->trees, 128 * size, sizeof(image_tree), 1));

    if (!list_is_null(&target->blocks)) {
      CHECK(list_destroy(&target->blocks));
    }
    CHECK(list_create(&target->blocks, 128 * size, sizeof(image_block), 1));
  }
  else {
    size = target->rows * target->cols;
    /* TODO: need to determine also max tree depth -> max number of tree nodes */
  }

  /* source image may need to be (re-)created if it doesn't exist or type has changed */
  /* in create, type is set to b_NONE, so this is done also in the first init */
  if (target->type != type) {
    /*printf("create source image\n");*/
    if (target->source != NULL) {
      CHECK(pixel_image_destroy(target->source));
    }
    else {
      target->source = pixel_image_alloc();
      CHECK_POINTER(target->source);
    }
    if (!list_is_null(&target->values)) {
      CHECK(list_destroy(&target->values));
    }
    /* grey image required for grey statistics */
    if (type == b_STAT_GREY) {
      CHECK(pixel_image_create(target->source, p_U8, GREY, width, height, 1,
                               1 * width));
      CHECK(list_create(&target->values, 128 * size, sizeof(stat_grey), 1));
    }
    else
    /* yuv image required for color statistics */
    /* TODO: need a mechanism to enable using Lab or similar (maybe use a macro?) */
    if (type == b_STAT_COLOR) {
      CHECK(pixel_image_create(target->source, p_U8, YUV, width, height, 3,
                               3 * width));
      CHECK(list_create(&target->values, 128 * size, sizeof(stat_color), 1));
    }
    else
    if (type == b_STATISTICS) {
      CHECK(pixel_image_create(target->source, p_U8, GREY, width, height, 1,
                               1 * width));
      CHECK(list_create(&target->values, 128 * size, sizeof(statistics), 1));
    }
    else {
      /* should never reach here actually */
      ERROR(BAD_PARAM);
    }

    /*printf("create integral image\n");*/
    if (!integral_image_is_null(&target->integral)) {
      CHECK(integral_image_destroy(&target->integral));
    }
    CHECK(integral_image_create(&target->integral, target->source));

    /* image is only created here, content is copied/converted in update stage */
    target->type = type;
  }

  CHECK(list_clear(&target->trees));
  CHECK(list_clear(&target->blocks));
  CHECK(list_clear(&target->values));

  /* create tree roots and their trees and blocks */
  /* TODO: init value only once */
  for (row = 0, pos = 0; row < target->rows; row++) {
    for (col = 0; col < target->cols; col++, pos++) {
      new_block.x = (uint16)(target->dx + col * target->tree_width);
      new_block.y = (uint16)(target->dy + row * target->tree_height);
      new_block.w = (uint16)(target->tree_width);
      new_block.h = (uint16)(target->tree_height);
      if (target->type == b_STAT_GREY) {
        stat_grey new_value, *value_ptr;
        new_value.mean = 0;
        new_value.dev = 0;
        CHECK(list_append_reveal_data(&target->values, (pointer)&new_value, (pointer*)&value_ptr));
        new_block.value = (pointer)value_ptr;
      }
      else
      if (target->type == b_STAT_COLOR) {
        stat_color new_value, *value_ptr;
        new_value.mean_i = 0;
        new_value.dev_i = 0;
        new_value.mean_c1 = 0;
        new_value.dev_c1 = 0;
        new_value.mean_c2 = 0;
        CHECK(list_append_reveal_data(&target->values, (pointer)&new_value, (pointer*)&value_ptr));
        new_block.value = (pointer)value_ptr;
      }
      else
      if (target->type == b_STATISTICS) {
        statistics new_value, *value_ptr;
        new_value.N = 0;
        new_value.sum = 0;
        new_value.sum2 = 0;
        new_value.mean = 0;
        new_value.variance = 0;
        new_value.deviation = 0;
        CHECK(list_append_reveal_data(&target->values, (pointer)&new_value, (pointer*)&value_ptr));
        new_block.value = (pointer)value_ptr;
      }
      CHECK(list_append_reveal_data(&target->blocks, (pointer)&new_block, (pointer*)&block_ptr));

      image_tree_nullify(&new_tree);
      new_tree.root = &target->roots[pos];
      new_tree.block = block_ptr;
      new_tree.level = 1;
      CHECK(list_append_reveal_data(&target->trees, (pointer)&new_tree, (pointer*)&tree_ptr));

      target->roots[pos].forest = target;
      target->roots[pos].tree = tree_ptr;
      /*
      CHECK(pixel_image_create_roi(&target->roots[pos].ROI, target->original, block_ptr->x, block_ptr->y, block_ptr->w, block_ptr->h));
      CHECK(small_integral_image_create(&target->roots[pos].I, &target->roots[pos].ROI));
      */
    }
  }

  target->last_base_block = target->blocks.last.prev;
  target->last_base_tree = target->trees.last.prev;
  target->last_base_value = target->values.last.prev;

  /* add neighbors to roots */
  for (row = 0, pos = 0; row < target->rows; row++) {
    for (col = 0; col < target->cols; col++, pos++) {
      /* add neighbor to west */
      if (col > 0) {
        target->roots[pos].tree->w = target->roots[pos - 1].tree;
      }
      /* add neighbor to north */
      if (row > 0) {
        target->roots[pos].tree->n = target->roots[pos - target->cols].tree;
      }
      /* add neighbor to east */
      if (col < (unsigned)(target->cols - 1)) {
        target->roots[pos].tree->e = target->roots[pos + 1].tree;
      }
      /* add neighbor to south */
      if (row < (unsigned)(target->rows - 1)) {
        target->roots[pos].tree->s = target->roots[pos + target->cols].tree;
      }
    }
  }

  FINALLY(image_tree_forest_init);
  RETURN();
}

/******************************************************************************/

image_tree_forest *image_tree_forest_alloc()
{
  TRY();
  image_tree_forest *ptr;
  CHECK(memory_allocate((data_pointer *)&ptr, 1, sizeof(image_tree_forest)));
  CHECK(image_tree_forest_nullify(ptr));
  FINALLY(image_tree_forest_alloc);
  return ptr;
}

/******************************************************************************/

void image_tree_forest_free(image_tree_forest *ptr)
{
  TRY();
  /*printf("free\n");*/
  r = SUCCESS;
  if (ptr != NULL) {
      CHECK(image_tree_forest_destroy(ptr));
      CHECK(memory_deallocate((data_pointer *)&ptr));
  }
  FINALLY(image_tree_forest_free);
}

/******************************************************************************/

result image_tree_forest_create
(
  image_tree_forest *target,
  pixel_image *source,
  uint16 tree_width,
  uint16 tree_height,
  image_block_type type
)
{
  TRY();
  /*printf("create\n");*/

  CHECK_POINTER(target);
  CHECK_POINTER(source);

  CHECK_PARAM(source->type == p_U8);
  CHECK_PARAM(source->format == GREY || source->format == YUV || source->format == RGB);

  /* set to NULL so the values can be checked and set in the init function */
  CHECK(image_tree_forest_nullify(target));

  target->original = source;

  CHECK(image_tree_forest_init(target, tree_width, tree_height, type));

  FINALLY(image_tree_forest_create);
  RETURN();
}

/******************************************************************************/

result image_tree_forest_reload
(
  image_tree_forest *target,
  uint16 tree_width,
  uint16 tree_height,
  image_block_type type
)
{
  TRY();

  CHECK_POINTER(target);
  CHECK_POINTER(target->original);

  if (target->tree_width != tree_width ||
      target->tree_height != tree_height ||
      target->type != type
      )
  {
    image_tree_forest_init(target, tree_width, tree_height, type);
  }

  FINALLY(image_tree_forest_reload);
  RETURN();
}

/******************************************************************************/

result image_tree_forest_destroy(
    image_tree_forest *target
    )
{
    TRY();
    uint32 pos, size;

    CHECK_POINTER(target);

    CHECK(list_destroy(&target->blocks));
    CHECK(list_destroy(&target->trees));
    CHECK(list_destroy(&target->values));

    /*
    size = target->rows * target->cols;
    for (pos = 0; pos < size; pos++) {
        CHECK(integral_image_destroy(&target->roots[pos].I));
        CHECK(pixel_image_destroy(&target->roots[pos].ROI));
    }
    */
    CHECK(memory_deallocate((data_pointer*)&target->roots));

    CHECK(integral_image_destroy(&target->integral));
    CHECK(pixel_image_destroy(target->source));
    CHECK(memory_deallocate((data_pointer*)&target->source));

    /*CHECK(edge_block_image_destroy(&target->edge_image));*/

    CHECK(image_tree_forest_nullify(target));

    FINALLY(image_tree_forest_destroy);
    RETURN();
}

/******************************************************************************/

result image_tree_forest_nullify
(
  image_tree_forest *target
)
{
  TRY();

  CHECK_POINTER(target);

  target->original = NULL;
  target->source = NULL;
  CHECK(integral_image_nullify(&target->integral));
  /*CHECK(edge_block_image_nullify(&target->edge_image));*/
  target->rows = 0;
  target->cols = 0;
  target->regions = 0;
  target->tree_width = 0;
  target->tree_height = 0;
  target->dx = 0;
  target->dy = 0;
  target->type = b_NONE;
  CHECK(list_nullify(&target->trees));
  CHECK(list_nullify(&target->blocks));
  CHECK(list_nullify(&target->values));
  target->last_base_tree = NULL;
  target->last_base_block = NULL;
  target->last_base_value = NULL;
  target->roots = NULL;

  FINALLY(image_tree_forest_nullify);
  RETURN();
}

/******************************************************************************/

bool image_tree_forest_is_null
(
  image_tree_forest *target
)
{
  if (target != NULL) {
    if (target->original == NULL && target->source == NULL) {
      return true;
    }
  }
  return false;
}

/******************************************************************************/

result image_tree_forest_update_prepare
(
  image_tree_forest *target
)
{
  TRY();
  uint32 pos, size;
  image_tree *tree;

  CHECK_POINTER(target);

  /*printf("update prepare\n");*/

  /* create a fresh copy of the source image in case it has changed */
  /* image format may be converted */
  if (target->type == b_STAT_GREY || target->type == b_STATISTICS) {
    /* grey image can be used as is */
    if (target->original->format == GREY) {
      CHECK(pixel_image_copy(target->source, target->original));
    }
    else
    /* from yuv image, take the y channel */
    if (target->original->format == YUV) {
      CHECK(pick_1_channel_from_3_channels(target->original, target->source, 0));
    }
    else
    /* rgb image must be converted to gray */
    if (target->original->format == RGB) {
      CHECK(convert_rgb24_to_grey8(target->original, target->source));
    }
    /* otherwise indicate error */
    else {
      ERROR(BAD_TYPE);
    }
  }
  else
  if (target->type == b_STAT_COLOR) {
    /* grey image must be converted to yuv */
    if (target->original->format == GREY) {
      CHECK(convert_grey8_to_yuv24(target->original, target->source));
    }
    else
    /* yuv image can be used as is */
    if (target->original->format == YUV) {
      CHECK(pixel_image_copy(target->source, target->original));
    }
    else
    /* rgb image must be converted to yuv */
    if (target->original->format == RGB) {
      CHECK(convert_rgb24_to_yuv24(target->original, target->source));
    }
    /* otherwise indicate error */
    else {
      ERROR(BAD_TYPE);
    }
  }

  /* if there are existing child nodes and blocks, remove them */
  CHECK(list_remove_rest(&target->blocks, target->last_base_block));
  CHECK(list_remove_rest(&target->trees, target->last_base_tree));
  CHECK(list_remove_rest(&target->values, target->last_base_value));

  FINALLY(image_tree_forest_update_prepare);
  RETURN();
}

/******************************************************************************/

result image_tree_forest_update
(
  image_tree_forest *target
)
{
  TRY();
  uint32 row, col, rows, cols, pos, step, stride, hstep, vstep, dstep, offset;
  image_tree *tree;
  integral_image *I;
  statistics *stat;
  I_value *iA, *i2A, N, sum1, sum2, mean, var;

  /*printf("update\n");*/

  I = &target->integral;
  N = (I_value)(target->tree_width * target->tree_height);
  step = I->step;
  stride = I->stride;
  hstep = target->tree_width * step;
  vstep = target->tree_height * stride;
  dstep = hstep + vstep;

  CHECK(image_tree_forest_update_prepare(target));
  CHECK(integral_image_update(&target->integral));

  rows = target->rows;
  cols = target->cols;
  pos = 0;
  for (row = 0; row < rows; row++) {
    tree = target->roots[pos].tree;
    /* TODO: calculate offset for first row only, then add vstep */
    offset = (tree->block->y * stride) + (tree->block->x * step);
    for (col = 0; col < cols; col++, pos++, offset += hstep) {

      iA = ((I_value *)I->I_1.data) + offset;
      i2A = ((I_value *)I->I_2.data) + offset;

      sum1 = *(iA + dstep) + *iA - *(iA + hstep) - *(iA + vstep);
      sum2 = *(i2A + dstep) + *i2A - *(i2A + hstep) - *(i2A + vstep);
      mean = sum1 / N;
      var = sum2 / N - mean*mean;
      if (var < 0) var = 0;

      tree = target->roots[pos].tree;
      stat = (statistics *)tree->block->value;

      stat->N = N;
      stat->sum = sum1;
      stat->sum2 = sum2;
      stat->mean = mean;
      stat->variance = var;
      stat->deviation = sqrt(var);

      image_tree_class_create(tree);
      tree->nw = NULL;
      tree->ne = NULL;
      tree->sw = NULL;
      tree->se = NULL;
    }
  }

  FINALLY(image_tree_forest_update);
  RETURN();
}

/******************************************************************************/

/* a macro for calculating neighbor difference; neighbor should be statistics pointer */
#define EVALUATE_NEIGHBOR_DEVIATION(neighbor)\
  stat = (statistics *)(neighbor);\
  nm = stat->mean;\
  dist = fabs(tm - nm)

result image_tree_forest_segment_with_deviation
(
  image_tree_forest *target,
  I_value threshold,
  uint32 min_size,
  I_value alpha
)
{
  TRY();
  list_item *trees;
  image_tree *tree, *neighbor, *best_neighbor;
  forest_region_info *tree_id, *neighbor_id;
  statistics *stat;
  I_value tm, nm, dist, best_dist;

  CHECK_POINTER(target);
  CHECK_PARAM(threshold > 0);
  CHECK_PARAM(min_size <= target->tree_width && min_size <= target->tree_height);
  CHECK_PARAM(alpha > 0);

  /* first, divide until all trees are consistent */
  trees = target->trees.first.next;
  while (trees != &target->trees.last) {
    tree = (image_tree *)trees->data;
    if (tree->block->w >= 2 * min_size && tree->block->h >= 2 * min_size) {
      if (((statistics *)tree->block->value)->deviation > threshold) {
        CHECK(image_tree_divide(tree));
      }
    }
    trees = trees->next;
  }

  /* then, make a union of those neighboring regions that are consistent together */
  printf("starting to merge trees\n");
  trees = target->trees.first.next;
  while (trees != &target->trees.last) {
    tree = (image_tree *)trees->data;
    tree_id = image_tree_class_find(&tree->region_info);
    /* only consider consistent trees (those that have not been divided) */
    if (tree->nw == NULL) {
      stat = (statistics *)tree->block->value;
      tm = stat->mean;

      best_dist = 255;
      best_neighbor = NULL;
      /* neighbor n */
      neighbor = tree->n;
      if (neighbor != NULL && neighbor->nw == NULL) {
        neighbor_id = image_tree_class_find(&neighbor->region_info);
        if (tree_id != neighbor_id) {
          EVALUATE_NEIGHBOR_DEVIATION(neighbor->block->value);
          if (dist < best_dist) {
            best_dist = dist;
            best_neighbor = neighbor;
          }
        }
      }
      /* neighbor e */
      neighbor = tree->e;
      if (neighbor != NULL && neighbor->nw == NULL) {
        neighbor_id = image_tree_class_find(&neighbor->region_info);
        if (tree_id != neighbor_id) {
          EVALUATE_NEIGHBOR_DEVIATION(neighbor->block->value);
          if (dist < best_dist) {
            best_dist = dist;
            best_neighbor = neighbor;
          }
        }
      }
      /* neighbor s */
      neighbor = tree->s;
      if (neighbor != NULL && neighbor->nw == NULL) {
        neighbor_id = image_tree_class_find(&neighbor->region_info);
        if (tree_id != neighbor_id) {
          EVALUATE_NEIGHBOR_DEVIATION(neighbor->block->value);
          if (dist < best_dist) {
            best_dist = dist;
            best_neighbor = neighbor;
          }
        }
      }
      /* neighbor w */
      neighbor = tree->w;
      if (neighbor != NULL && neighbor->nw == NULL) {
        neighbor_id = image_tree_class_find(&neighbor->region_info);
        if (tree_id != neighbor_id) {
          EVALUATE_NEIGHBOR_DEVIATION(neighbor->block->value);
          if (dist < best_dist) {
            best_dist = dist;
            best_neighbor = neighbor;
          }
        }
      }

      if (best_dist < alpha * threshold) {
        image_tree_class_union(tree, best_neighbor);
      }
    }
    trees = trees->next;
  }

  /* then, merge those neighboring regions that are consistent together */
  printf("starting to merge regions\n");
  trees = target->trees.first.next;
  while (trees != &target->trees.last) {
    tree = (image_tree *)trees->data;
    tree_id = image_tree_class_find(&tree->region_info);
    /* only consider consistent trees (those that have not been divided) */
    if (tree->nw == NULL) {
      stat = (statistics *)&tree_id->stat;
      tm = stat->mean;

      /* neighbor n */
      neighbor = tree->n;
      if (neighbor != NULL && neighbor->nw == NULL) {
        neighbor_id = image_tree_class_find(&neighbor->region_info);
        if (tree_id != neighbor_id) {
          EVALUATE_NEIGHBOR_DEVIATION(&neighbor_id->stat);
          if (dist < alpha * threshold) {
            image_tree_class_union(tree, neighbor);
          }
        }
      }
      /* neighbor e */
      neighbor = tree->e;
      if (neighbor != NULL && neighbor->nw == NULL) {
        neighbor_id = image_tree_class_find(&neighbor->region_info);
        if (tree_id != neighbor_id) {
          EVALUATE_NEIGHBOR_DEVIATION(&neighbor_id->stat);
          if (dist < alpha * threshold) {
            image_tree_class_union(tree, neighbor);
          }
        }
      }
      /* neighbor s */
      neighbor = tree->s;
      if (neighbor != NULL && neighbor->nw == NULL) {
        neighbor_id = image_tree_class_find(&neighbor->region_info);
        if (tree_id != neighbor_id) {
          EVALUATE_NEIGHBOR_DEVIATION(&neighbor_id->stat);
          if (dist < alpha * threshold) {
            image_tree_class_union(tree, neighbor);
          }
        }
      }
      /* neighbor w */
      neighbor = tree->w;
      if (neighbor != NULL && neighbor->nw == NULL) {
        neighbor_id = image_tree_class_find(&neighbor->region_info);
        if (tree_id != neighbor_id) {
          EVALUATE_NEIGHBOR_DEVIATION(&neighbor_id->stat);
          if (dist < alpha * threshold) {
            image_tree_class_union(tree, neighbor);
          }
        }
      }
    }
    trees = trees->next;
  }

  /* finally, count regions and assign colors */
  {
    forest_region_info *id, *region;
    uint32 count;

    count = 0;
    /* initialize the random number generator for assigning the colors */
    srand(1234);

    trees = target->trees.first.next;
    while (trees != &target->trees.last) {
      tree = (image_tree *)trees->data;
      if (tree->nw == NULL) {
        region = &tree->region_info;
        id = image_tree_class_find(region);
        if (id == region) {
          region->color[0] = (byte)(rand() % 256);
          region->color[1] = (byte)(rand() % 256);
          region->color[2] = (byte)(rand() % 256);
          count++;
        }
      }
      trees = trees->next;
    }
    target->regions = count;
    printf("segmentation finished, %lu regions found\n", count);
  }

  FINALLY(image_tree_forest_segment_with_deviation);
  RETURN();
}

/******************************************************************************/

/* a macro for calculating neighbor entropy; neighbor should be statistics pointer */
#define EVALUATE_NEIGHBOR_ENTROPY(neighbor)\
  stat = (statistics *)(neighbor);\
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
  entropy = I / U

result image_tree_forest_segment_with_entropy
(
  image_tree_forest *target,
  uint32 min_size,
  I_value alpha,
  I_value diff_tree,
  I_value diff_region
)
{
  TRY();
  list_item *trees;
  image_tree *tree, *neighbor, *best_neighbor;
  forest_region_info *tree_id, *neighbor_id;
  statistics *stat;
  I_value tm, ts, nm, ns, x1, x2, x1min, x1max, x2min, x2max, I, U, entropy, best_entropy;

  CHECK_POINTER(target);
  CHECK_PARAM(min_size <= target->tree_width && min_size <= target->tree_height);
  CHECK_PARAM(alpha > 0);
  CHECK_PARAM(diff_tree > 0);
  CHECK_PARAM(diff_region > 0);

  /* first, divide until all trees are consistent */
  printf("starting to divide trees\n");
  trees = target->trees.first.next;
  while (trees != &target->trees.last) {
    tree = (image_tree *)trees->data;
    CHECK(image_tree_divide_with_entropy(tree, min_size));
    trees = trees->next;
  }

  /* then, merge each tree with the best neighboring tree that is close enough */
  printf("starting to merge trees\n");
  trees = target->trees.first.next;
  while (trees != &target->trees.last) {
    tree = (image_tree *)trees->data;
    tree_id = image_tree_class_find(&tree->region_info);
    /* only consider consistent trees (those that have not been divided) */
    if (tree->nw == NULL) {
      stat = (statistics *)tree->block->value;
      tm = stat->mean;
      ts = fmax(alpha, alpha * stat->deviation);

      best_entropy = 0;
      best_neighbor = NULL;
      /* neighbor n */
      neighbor = tree->n;
      if (neighbor != NULL && neighbor->nw == NULL) {
        neighbor_id = image_tree_class_find(&neighbor->region_info);
        if (tree_id != neighbor_id) {
          EVALUATE_NEIGHBOR_ENTROPY(neighbor->block->value);
          if (entropy > best_entropy) {
            best_entropy = entropy;
            best_neighbor = neighbor;
          }
        }
      }
      /* neighbor e */
      neighbor = tree->e;
      if (neighbor != NULL && neighbor->nw == NULL) {
        neighbor_id = image_tree_class_find(&neighbor->region_info);
        if (tree_id != neighbor_id) {
          EVALUATE_NEIGHBOR_ENTROPY(neighbor->block->value);
          if (entropy > best_entropy) {
            best_entropy = entropy;
            best_neighbor = neighbor;
          }
        }
      }
      /* neighbor s */
      neighbor = tree->s;
      if (neighbor != NULL && neighbor->nw == NULL) {
        neighbor_id = image_tree_class_find(&neighbor->region_info);
        if (tree_id != neighbor_id) {
          EVALUATE_NEIGHBOR_ENTROPY(neighbor->block->value);
          if (entropy > best_entropy) {
            best_entropy = entropy;
            best_neighbor = neighbor;
          }
        }
      }
      /* neighbor w */
      neighbor = tree->w;
      if (neighbor != NULL && neighbor->nw == NULL) {
        neighbor_id = image_tree_class_find(&neighbor->region_info);
        if (tree_id != neighbor_id) {
          EVALUATE_NEIGHBOR_ENTROPY(neighbor->block->value);
          if (entropy > best_entropy) {
            best_entropy = entropy;
            best_neighbor = neighbor;
          }
        }
      }

      if (best_entropy > diff_tree) {
        image_tree_class_union(tree, best_neighbor);
      }
    }
    trees = trees->next;
  }

  /* then, merge those neighboring regions that are consistent together */
  printf("starting to merge regions\n");
  trees = target->trees.first.next;
  while (trees != &target->trees.last) {
    tree = (image_tree *)trees->data;
    tree_id = image_tree_class_find(&tree->region_info);
    /* only consider consistent trees (those that have not been divided) */
    if (tree->nw == NULL) {
      stat = (statistics *)&tree_id->stat;
      tm = stat->mean;
      ts = fmax(alpha, alpha * stat->deviation);

      /* neighbor n */
      neighbor = tree->n;
      if (neighbor != NULL && neighbor->nw == NULL) {
        neighbor_id = image_tree_class_find(&neighbor->region_info);
        if (tree_id != neighbor_id) {
          EVALUATE_NEIGHBOR_ENTROPY(&neighbor_id->stat);
          if (entropy > diff_region) {
            image_tree_class_union(tree, neighbor);
          }
        }
      }
      /* neighbor e */
      neighbor = tree->e;
      if (neighbor != NULL && neighbor->nw == NULL) {
        neighbor_id = image_tree_class_find(&neighbor->region_info);
        if (tree_id != neighbor_id) {
          EVALUATE_NEIGHBOR_ENTROPY(&neighbor_id->stat);
          if (entropy > diff_region) {
            image_tree_class_union(tree, neighbor);
          }
        }
      }
      /* neighbor s */
      neighbor = tree->s;
      if (neighbor != NULL && neighbor->nw == NULL) {
        neighbor_id = image_tree_class_find(&neighbor->region_info);
        if (tree_id != neighbor_id) {
          EVALUATE_NEIGHBOR_ENTROPY(&neighbor_id->stat);
          if (entropy > diff_region) {
            image_tree_class_union(tree, neighbor);
          }
        }
      }
      /* neighbor w */
      neighbor = tree->w;
      if (neighbor != NULL && neighbor->nw == NULL) {
        neighbor_id = image_tree_class_find(&neighbor->region_info);
        if (tree_id != neighbor_id) {
          EVALUATE_NEIGHBOR_ENTROPY(&neighbor_id->stat);
          if (entropy > diff_region) {
            image_tree_class_union(tree, neighbor);
          }
        }
      }
    }
    trees = trees->next;
  }

  /* finally, count regions and assign colors */
  {
    forest_region_info *id, *region;
    uint32 count;

    count = 0;
    /* initialize the random number generator for assigning the colors */
    srand(1234);

    trees = target->trees.first.next;
    while (trees != &target->trees.last) {
      tree = (image_tree *)trees->data;
      if (tree->nw == NULL) {
        region = &tree->region_info;
        id = image_tree_class_find(region);
        if (id == region) {
          region->color[0] = (byte)(rand() % 256);
          region->color[1] = (byte)(rand() % 256);
          region->color[2] = (byte)(rand() % 256);
          count++;
        }
      }
      trees = trees->next;
    }
    target->regions = count;
    printf("segmentation finished, %lu regions found\n", count);
  }
  FINALLY(image_tree_forest_segment_with_entropy);
  RETURN();
}

/******************************************************************************/

result image_tree_forest_get_regions
(
  image_tree_forest *source,
  forest_region_info **target
)
{
  TRY();
  list_item *trees;
  image_tree *tree;
  forest_region_info *region, *id;
  uint32 count;

  CHECK_POINTER(source);
  CHECK_POINTER(target);

  /* collect all regions to array and generate colors */
  count = 0;
  trees = source->trees.first.next;
  while (trees != &source->trees.last) {
    tree = (image_tree *)trees->data;
    if (tree->nw == NULL) {
      region = &tree->region_info;
      id = image_tree_class_find(region);
      if (id == region) {
        target[count] = region;
        count++;
      }
    }
    trees = trees->next;
  }

  FINALLY(image_tree_forest_get_regions);
  RETURN();
}

/******************************************************************************/

result image_tree_forest_draw_image
(
  image_tree_forest *forest,
  pixel_image *target,
  uint32 use_regions,
  uint32 use_colors
)
{
  TRY();
  list_item *trees;
  image_tree *tree;
  forest_region_info *id;
  uint32 x, y, width, height, stride, row_step;
  byte *target_data, *target_pos, color0, color1, color2;

  CHECK_POINTER(forest);
  CHECK_POINTER(forest->source);
  CHECK_POINTER(target);

  width = forest->source->width;
  height = forest->source->height;

  CHECK(pixel_image_create(target, p_U8, RGB, width, height, 3, 3 * width));

  stride = target->stride;
  target_data = (byte*)target->data;

  /* draw using tree mean value */
  if (use_regions == 0) {
    statistics *stat;
    trees = forest->trees.first.next;
    while (trees != &forest->trees.last) {
      tree = (image_tree *)trees->data;
      if (tree->nw == NULL) {
        stat = (statistics *)tree->block->value;
        /* TODO: maybe could create only a grayscale image..? */
        color0 = (byte)stat->mean;
        color1 = color0;
        color2 = color0;
        width = tree->block->w;
        height = tree->block->h;
        row_step = stride - 3 * width;
        target_pos = target_data + tree->block->y * stride + tree->block->x * 3;
        for (y = 0; y < height; y++, target_pos += row_step) {
          for (x = 0; x < width; x++) {
            *target_pos = color0;
            target_pos++;
            *target_pos = color1;
            target_pos++;
            *target_pos = color2;
            *target_pos++;
          }
        }

      }
      trees = trees->next;
    }
  }
  else {
    /* draw using region mean value */
    if (use_colors == 0) {
      trees = forest->trees.first.next;
      while (trees != &forest->trees.last) {
        tree = (image_tree *)trees->data;
        if (tree->nw == NULL) {
          id = image_tree_class_find(&tree->region_info);
          if (id != NULL) {
            color0 = (byte)id->stat.mean;
            color1 = color0;
            color2 = color0;
            width = tree->block->w;
            height = tree->block->h;
            row_step = stride - 3 * width;
            target_pos = target_data + tree->block->y * stride + tree->block->x * 3;
            for (y = 0; y < height; y++, target_pos += row_step) {
              for (x = 0; x < width; x++) {
                *target_pos = color0;
                target_pos++;
                *target_pos = color1;
                target_pos++;
                *target_pos = color2;
                *target_pos++;
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
        tree = (image_tree *)trees->data;
        if (tree->nw == NULL) {
          id = image_tree_class_find(&tree->region_info);
          if (id != NULL) {
            color0 = id->color[0];
            color1 = id->color[1];
            color2 = id->color[2];
            width = tree->block->w;
            height = tree->block->h;
            row_step = stride - 3 * width;
            target_pos = target_data + tree->block->y * stride + tree->block->x * 3;
            for (y = 0; y < height; y++, target_pos += row_step) {
              for (x = 0; x < width; x++) {
                *target_pos = color0;
                target_pos++;
                *target_pos = color1;
                target_pos++;
                *target_pos = color2;
                *target_pos++;
              }
            }
          }
        }
        trees = trees->next;
      }
    }
  }

  FINALLY(image_tree_forest_draw_image);
  RETURN();
}

/******************************************************************************/

result image_tree_root_update
(
  image_tree_root *target
)
{
  TRY();
  /*uint32 i;*/

  CHECK_POINTER(target);
  /*for (i = 0; i < 100; i++) {*/
      CHECK(small_integral_image_update(&target->I));
  /*}*/

  small_integral_image_box_create(&target->box, &target->I, target->ROI.width, target->ROI.height, target->ROI.dx, target->ROI.dy);
  CHECK(image_tree_update(target->tree));

  FINALLY(image_tree_root_update);
    RETURN();
}

/******************************************************************************/

/**
 * Update image block statistics using the integral image.
 * Assumes the box is of correct size, meaning that
 * @see small_integral_image_box_create has been called previously.
 * Will call @see small_integral_image_box_update to move the box and to
 * calculate the integrals. Updates the statistics based on the integrals.
 */

result image_tree_update
(
  image_tree *tree
)
{
  TRY();
  I_value mean, dev;
  /*uint32 mean, dev;*/
  small_integral_image_box *box;
  image_block *block;
  image_block_type type;
  /*struct timeval now;*/

  CHECK_POINTER(tree);

  block = tree->block;
  box = &tree->root->box;
  type = tree->root->forest->type;

  box->channel = 0;
  small_integral_image_box_update(box, block->x, block->y);

  mean = ((I_value)box->sum / (I_value)box->N);
  dev = (((I_value)box->sumsqr / (I_value)box->N) - (mean * mean));

  if (dev < 0) dev = 0;
  if (type == b_STAT_GREY) {
    ((stat_grey *)block->value)->mean = (sint16)((mean < 0) ? 0 : ((mean > 255) ? 255 : mean));
    ((stat_grey *)block->value)->dev = (sint16)sqrt(dev);
  }
  else
  if (type == b_STAT_COLOR) {
    stat_color *value = (stat_color *)block->value;

    value->mean_i = (sint16)((mean < 0) ? 0 : ((mean > 255) ? 255 : mean));
    value->dev_i = (sint16)sqrt(dev);

    box->channel = 1;
    small_integral_image_box_update(box, block->x, block->y);
    mean = ((I_value)box->sum / (I_value)box->N);
    dev = (((I_value)box->sumsqr / (I_value)box->N) - (mean * mean));
    if (dev < 1) dev = 1;
    value->mean_c1 = (sint16)((mean < 0) ? 0 : ((mean > 255) ? 255 : mean));
    value->dev_c1 = (sint16)sqrt(dev);

    box->channel = 2;
    small_integral_image_box_update(box, block->x, block->y);
    mean = ((I_value)box->sum / (I_value)box->N);
    dev = (((I_value)box->sumsqr / (I_value)box->N) - (mean * mean));
    if (dev < 1) dev = 1;
    value->mean_c2 = (sint16)((mean < 0) ? 0 : ((mean > 255) ? 255 : mean));
    value->dev_c2 = (sint16)sqrt(dev);
  }
  else
  if (type == b_STATISTICS) {
    statistics *value = (statistics *)block->value;

    value->N = (I_value)box->N;
    value->sum = (I_value)box->sum;
    value->sum2 = (I_value)box->sumsqr;
    value->mean = ((mean < 0) ? 0 : ((mean > 255) ? 255 : mean));
    value->variance = dev;
    value->deviation = sqrt(dev);
    /*printf("%.3f,%.3f ", value->mean, value->deviation);*/
  }
  else {
    /* should never get here... */
    ERROR(BAD_TYPE);
  }

  /*gettimeofday(&now, NULL);*/
  /*printf("%d.%d update tree (%d,%d)=%d\n", now.tv_sec, now.tv_usec, block->x, block->y, block->value.mean);*/

  FINALLY(image_tree_update);
  RETURN();
}

/******************************************************************************/

result image_tree_calculate_consistency
  (
  image_tree *tree,
  I_value *rows,
  I_value *cols
  /*consistency *target*/
  )
{
  TRY();
  uint32 row, col, width, height, offset, step, rinc, cinc;
  SI_2_t *A, *B, *C, *D, N, sum, sum2, mean, dev;
  small_integral_image_box *box;

  CHECK_POINTER(tree);
  CHECK_POINTER(rows);
  CHECK_POINTER(cols);

  box = &tree->root->box;
  width = tree->block->w;
  height = tree->block->h;
  step = box->step;
  offset = (tree->block->y - box->dy) * box->stride + (tree->block->x - box->dx) * box->step + box->channel;
  rinc = width * box->step;
  cinc = height * box->stride;

  A = box->I_2_data + offset;
  B = A + 2 * step;
  D = A + cinc;
  C = D + 2 * step;
  for (col = 0; col < width; col++) {
    A += step;
    B += step;
    C += step;
    D += step;
  }

  FINALLY(image_tree_calculate_consistency);
  RETURN();
}

/******************************************************************************/

/* private function for caching beighbors, to be used only when it is known
 * that the tree exists, and its children exist */
void image_tree_cache_neighbors
(
  image_tree *target
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

/*
 * TODO: each image tree root should have its own sublist of blocks, for
 * allowing parallel division of each root.
 */

result image_tree_divide
(
  image_tree *target
)
{
  TRY();
  uint16 w, h;

  CHECK_POINTER(target);

  w = target->block->w;
  h = target->block->h;
  if (w > 1 && h > 1) {
    /*printf("tree %u %u\n", target->block->x, target->block->y);*/
    if (!image_tree_has_children(target)) {
      image_tree_forest *forest;
      image_block blocks[4];
      statistics values[4];
      uint32 i;

      w = (uint16)(w / 2);
      h = (uint16)(h / 2);

      /*printf("creating child trees with size %u,%u\n",w,h);*/

      forest = target->root->forest;

      CHECK(image_tree_get_child_statistics(target, values, blocks));
      {
        image_tree new_tree, *child_tree;
        image_block *child_block;
        statistics *child_value;

        CHECK(image_tree_nullify(&new_tree));

        new_tree.root = target->root;
        new_tree.parent = target;
        new_tree.level = target->level+1;

        /* nw child block */
        CHECK(list_append_reveal_data(&forest->values, (pointer)&values[0], (pointer*)&child_value));
        blocks[0].value = (pointer)child_value;
        CHECK(list_append_reveal_data(&forest->blocks, (pointer)&blocks[0], (pointer*)&child_block));
        new_tree.block = child_block;
        CHECK(list_append_reveal_data(&forest->trees, (pointer)&new_tree, (pointer*)&child_tree));
        image_tree_class_create(child_tree);
        target->nw = child_tree;

        /* ne child block */
        CHECK(list_append_reveal_data(&forest->values, (pointer)&values[1], (pointer*)&child_value));
        blocks[1].value = (pointer)child_value;
        CHECK(list_append_reveal_data(&forest->blocks, (pointer)&blocks[1], (pointer*)&child_block));
        new_tree.block = child_block;
        CHECK(list_append_reveal_data(&forest->trees, (pointer)&new_tree, (pointer*)&child_tree));
        image_tree_class_create(child_tree);
        target->ne = child_tree;

        /* sw child block */
        CHECK(list_append_reveal_data(&forest->values, (pointer)&values[2], (pointer*)&child_value));
        blocks[2].value = (pointer)child_value;
        CHECK(list_append_reveal_data(&forest->blocks, (pointer)&blocks[2], (pointer*)&child_block));
        new_tree.block = child_block;
        CHECK(list_append_reveal_data(&forest->trees, (pointer)&new_tree, (pointer*)&child_tree));
        image_tree_class_create(child_tree);
        target->sw = child_tree;

        /* se child block */
        CHECK(list_append_reveal_data(&forest->values, (pointer)&values[3], (pointer*)&child_value));
        blocks[3].value = (pointer)child_value;
        CHECK(list_append_reveal_data(&forest->blocks, (pointer)&blocks[3], (pointer*)&child_block));
        new_tree.block = child_block;
        CHECK(list_append_reveal_data(&forest->trees, (pointer)&new_tree, (pointer*)&child_tree));
        image_tree_class_create(child_tree);
        target->se = child_tree;

        image_tree_cache_neighbors(target);
      }
    }
  }

  FINALLY(image_tree_divide);
  RETURN();
}

/******************************************************************************/

bool image_tree_has_children
(
  image_tree *tree
)
{
  /* there should be no case in which some children would not be set */
  /* the pointers are nullified at init; therefore, check only one */
  if (tree->nw != NULL) {
    return true;
  }
  return false;
}

/******************************************************************************/

result image_tree_get_child_statistics
(
  image_tree *source,
  statistics *target,
  image_block *blocks
)
{
  TRY();

  CHECK_POINTER(source);
  CHECK_POINTER(target);

  /* if the tree has already been divided, return the values from the children */
  if (image_tree_has_children(source)) {
    /* nw child block */
    CHECK(memory_copy((void*)&target[0], (void*)source->nw->block->value, 1, sizeof(statistics)));
    /* ne child block */
    CHECK(memory_copy((void*)&target[1], (void*)source->ne->block->value, 1, sizeof(statistics)));
    /* sw child block */
    CHECK(memory_copy((void*)&target[2], (void*)source->sw->block->value, 1, sizeof(statistics)));
    /* se child block */
    CHECK(memory_copy((void*)&target[3], (void*)source->se->block->value, 1, sizeof(statistics)));
  }
  /* otherwise have to calculate */
  else {
    if (source->block->w > 1 && source->block->h > 1) {
      uint16 x, y, w, h;
      uint32 step, stride, offset;
      statistics *stat;

      w = (uint16)(source->block->w / 2);
      h = (uint16)(source->block->h / 2);

      if (blocks != NULL) {
        blocks[0].w = blocks[1].w = blocks[2].w = blocks[3].w = w;
        blocks[0].h = blocks[1].h = blocks[2].h = blocks[3].h = h;
      }

      /* if the new width is 1, no need to calculate, use the pixel values */
      if (w == 1 && h == 1) {
        pixel_image *original;
        void *data;
        pixel_type type;
        I_value mean;

        original = source->root->forest->source;
        data = original->data;
        type = original->type;
        step = original->step;
        stride = original->stride;

        /* nw child block */

        x = source->block->x;
        y = source->block->y;
        offset = y * stride + x * step;
        mean = cast_pixel_value(original->data, type, offset);

        stat = &target[0];
        stat->N = 1;
        stat->sum = mean;
        stat->sum2 = mean*mean;
        stat->mean = mean;
        stat->variance = 0;
        stat->deviation = 0;

        if (blocks != NULL) {
          blocks[0].x = x;
          blocks[0].y = y;
        }

        /* ne child block */

        x = x + 1;
        offset = offset + step;
        mean = cast_pixel_value(original->data, type, offset);

        stat = &target[1];
        stat->N = 1;
        stat->sum = mean;
        stat->sum2 = mean*mean;
        stat->mean = mean;
        stat->variance = 0;
        stat->deviation = 0;

        if (blocks != NULL) {
          blocks[1].x = x;
          blocks[1].y = y;
        }

        /* se child block */

        y = y + 1;
        offset = offset + stride;
        mean = cast_pixel_value(original->data, type, offset);

        stat = &target[3];
        stat->N = 1;
        stat->sum = mean;
        stat->sum2 = mean*mean;
        stat->mean = mean;
        stat->variance = 0;
        stat->deviation = 0;

        if (blocks != NULL) {
          blocks[3].x = x;
          blocks[3].y = y;
        }

        /* sw child block */

        x = x - 1;
        offset = offset - step;
        mean = cast_pixel_value(original->data, type, offset);

        stat = &target[2];
        stat->N = 1;
        stat->sum = mean;
        stat->sum2 = mean*mean;
        stat->mean = mean;
        stat->variance = 0;
        stat->deviation = 0;

        if (blocks != NULL) {
          blocks[2].x = x;
          blocks[2].y = y;
        }
      }
      else {
        uint32 hstep, vstep, dstep;
        integral_image *I;
        I_value *iA, *i2A, N, sum1, sum2, mean, var;

        I = &source->root->forest->integral;
        N = (I_value)(w * h);
        step = I->step;
        stride = I->stride;
        hstep = w * step;
        vstep = h * stride;
        dstep = hstep + vstep;

        /* nw child block */

        x = source->block->x;
        y = source->block->y;
        offset = y * stride + x * step;

        iA = ((I_value *)I->I_1.data) + offset;
        i2A = ((I_value *)I->I_2.data) + offset;

        sum1 = *(iA + dstep) + *iA - *(iA + hstep) - *(iA + vstep);
        sum2 = *(i2A + dstep) + *i2A - *(i2A + hstep) - *(i2A + vstep);
        mean = sum1 / N;
        var = sum2 / N - mean*mean;
        if (var < 0) var = 0;

        stat = &target[0];
        stat->N = N;
        stat->sum = sum1;
        stat->sum2 = sum2;
        stat->mean = mean;
        stat->variance = var;
        stat->deviation = sqrt(var);

        if (blocks != NULL) {
          blocks[0].x = x;
          blocks[0].y = y;
        }

        /* ne child block */

        x = x + w;
        iA = iA + hstep;
        i2A = i2A + hstep;

        sum1 = *(iA + dstep) + *iA - *(iA + hstep) - *(iA + vstep);
        sum2 = *(i2A + dstep) + *i2A - *(i2A + hstep) - *(i2A + vstep);
        mean = sum1 / N;
        var = sum2 / N - mean*mean;
        if (var < 0) var = 0;

        stat = &target[1];
        stat->N = N;
        stat->sum = sum1;
        stat->sum2 = sum2;
        stat->mean = mean;
        stat->variance = var;
        stat->deviation = sqrt(var);

        if (blocks != NULL) {
          blocks[1].x = x;
          blocks[1].y = y;
        }

        /* se child block */

        y = y + h;
        iA = iA + vstep;
        i2A = i2A + vstep;

        sum1 = *(iA + dstep) + *iA - *(iA + hstep) - *(iA + vstep);
        sum2 = *(i2A + dstep) + *i2A - *(i2A + hstep) - *(i2A + vstep);
        mean = sum1 / N;
        var = sum2 / N - mean*mean;
        if (var < 0) var = 0;

        stat = &target[3];
        stat->N = N;
        stat->sum = sum1;
        stat->sum2 = sum2;
        stat->mean = mean;
        stat->variance = var;
        stat->deviation = sqrt(var);

        if (blocks != NULL) {
          blocks[3].x = x;
          blocks[3].y = y;
        }

        /* sw child block */

        x = x - w;
        iA = iA - hstep;
        i2A = i2A - hstep;

        sum1 = *(iA + dstep) + *iA - *(iA + hstep) - *(iA + vstep);
        sum2 = *(i2A + dstep) + *i2A - *(i2A + hstep) - *(i2A + vstep);
        mean = sum1 / N;
        var = sum2 / N - mean*mean;
        if (var < 0) var = 0;

        stat = &target[2];
        stat->N = N;
        stat->sum = sum1;
        stat->sum2 = sum2;
        stat->mean = mean;
        stat->variance = var;
        stat->deviation = sqrt(var);

        if (blocks != NULL) {
          blocks[2].x = x;
          blocks[2].y = y;
        }
      }
    }
  }

  FINALLY(image_tree_get_child_statistics);
  RETURN();
}

/******************************************************************************/

result image_tree_divide_with_entropy
(
  image_tree *target,
  uint32 min_size
)
{
  TRY();

  CHECK_POINTER(target);
  CHECK_PARAM(min_size > 0);

  /*printf("tree %u %u\n", target->block->x, target->block->y);*/
  if (target->nw == NULL) {
    uint16 w, h;

    w = target->block->w;
    h = target->block->h;
    if (w >= min_size*2 && h >= min_size*2) {
      w = (uint16)(w / 2);
      h = (uint16)(h / 2);
      /*printf("creating child trees with size %u,%u\n",w,h);*/
      image_tree_forest *forest;
      image_block blocks[4];
      statistics values[4];
      uint32 i;
      I_value m, s, x1, x2, x1min, x1max, x2min, x2max, I, U, entropy;

      forest = target->root->forest;

      CHECK(image_tree_get_child_statistics(target, values, blocks));

      m = values[0].mean;
      s = fmax(3, 3 * values[0].deviation);
      x1min = fmax(0, m - s);
      x1max = x1min;
      x2min = fmin(255, m + s);
      x2max = x2min;
      for (i = 1; i < 4; i++) {
        m = values[i].mean;
        s = fmax(3, 3 * values[i].deviation);
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

      /*printf("%.3f-%.3f, %.3f-%.3f, %.3f, %.3f\n", x1min,x2max,x1max,x2min,I,U);*/
      /* let us define this entropy measure as intersection divided by union */
      entropy = I / U;

      /* if union is more than double the intersection, we have high 'entropy' */
      if (entropy < 0.5) {
        image_tree new_tree, *child_tree;
        image_block *child_block;
        statistics *child_value;

        CHECK(image_tree_nullify(&new_tree));

        new_tree.root = target->root;
        new_tree.parent = target;
        new_tree.level = target->level+1;

        /* nw child block */
        CHECK(list_append_reveal_data(&forest->values, (pointer)&values[0], (pointer*)&child_value));
        blocks[0].value = (pointer)child_value;
        CHECK(list_append_reveal_data(&forest->blocks, (pointer)&blocks[0], (pointer*)&child_block));
        new_tree.block = child_block;
        CHECK(list_append_reveal_data(&forest->trees, (pointer)&new_tree, (pointer*)&child_tree));
        image_tree_class_create(child_tree);
        target->nw = child_tree;

        /* ne child block */
        CHECK(list_append_reveal_data(&forest->values, (pointer)&values[1], (pointer*)&child_value));
        blocks[1].value = (pointer)child_value;
        CHECK(list_append_reveal_data(&forest->blocks, (pointer)&blocks[1], (pointer*)&child_block));
        new_tree.block = child_block;
        CHECK(list_append_reveal_data(&forest->trees, (pointer)&new_tree, (pointer*)&child_tree));
        image_tree_class_create(child_tree);
        target->ne = child_tree;

        /* sw child block */
        CHECK(list_append_reveal_data(&forest->values, (pointer)&values[2], (pointer*)&child_value));
        blocks[2].value = (pointer)child_value;
        CHECK(list_append_reveal_data(&forest->blocks, (pointer)&blocks[2], (pointer*)&child_block));
        new_tree.block = child_block;
        CHECK(list_append_reveal_data(&forest->trees, (pointer)&new_tree, (pointer*)&child_tree));
        image_tree_class_create(child_tree);
        target->sw = child_tree;

        /* se child block */
        CHECK(list_append_reveal_data(&forest->values, (pointer)&values[3], (pointer*)&child_value));
        blocks[3].value = (pointer)child_value;
        CHECK(list_append_reveal_data(&forest->blocks, (pointer)&blocks[3], (pointer*)&child_block));
        new_tree.block = child_block;
        CHECK(list_append_reveal_data(&forest->trees, (pointer)&new_tree, (pointer*)&child_tree));
        image_tree_class_create(child_tree);
        target->se = child_tree;

        image_tree_cache_neighbors(target);
      }
    }
  }

  FINALLY(image_tree_divide_with_entropy);
  RETURN();
}

/******************************************************************************/

result image_tree_create_neighbor_list(
    list *target
    )
{
    TRY();
    /*printf("create neighbor list\n");*/

    CHECK_POINTER(target);

    CHECK(list_create(target, 100, sizeof(image_tree*), 1));

    FINALLY(image_tree_create_neighbor_list);
    RETURN();
}

/******************************************************************************/

result image_tree_get_direct_neighbor(
    image_tree *tree,
    image_tree **neighbor,
    direction ndir
    )
{
    TRY();

    switch (ndir) {
        case d_N:
            CHECK(image_tree_get_direct_neighbor_n(tree, neighbor));
            break;
        case d_E:
            CHECK(image_tree_get_direct_neighbor_e(tree, neighbor));
            break;
        case d_S:
            CHECK(image_tree_get_direct_neighbor_s(tree, neighbor));
            break;
        case d_W:
            CHECK(image_tree_get_direct_neighbor_w(tree, neighbor));
            break;
        default:
            ERROR(BAD_PARAM);
    }

    FINALLY(image_tree_get_direct_neighbor);
    RETURN();
}

/******************************************************************************/

result image_tree_get_direct_neighbor_n(
    image_tree *tree,
    image_tree **neighbor
    )
{
    TRY();
    image_tree *parent_neighbor;

    CHECK_POINTER(tree);
    CHECK_POINTER(neighbor);

    *neighbor = NULL;

    /* check if the neighbor has been cached in the tree already */
    if (tree->n != NULL) {
        *neighbor = tree->n;
    }
    else {
        /* either parent or neighbor should be set */
        /* if not, the tree is on the edge */
        if (tree->parent == NULL) {
            *neighbor = NULL;
        }
        else {
            /* neighbor n of sw is nw */
            if (tree->parent->sw == tree) {
                *neighbor = tree->parent->nw;
            }
            else
            /* neighbor n of se is ne */
            if (tree->parent->se == tree) {
                *neighbor = tree->parent->ne;
            }
            else {
                /* neighbor not on the same parent, get neighbor of parent */
                CHECK(image_tree_get_direct_neighbor_n(tree->parent, &parent_neighbor));
                if (parent_neighbor != NULL) {
                    if (tree->parent->nw == tree) {
                        /* neighbor of nw is parent's neighbor's sw */
                        if (parent_neighbor->sw != NULL) {
                            *neighbor = parent_neighbor->sw;
                        }
                        /* if no children, use parent's neighbor itself */
                        else {
                            *neighbor = parent_neighbor;
                        }
                    }
                    else
                    if (tree->parent->ne == tree) {
                        /* neighbor of ne is parent's neighbor's se */
                        if (parent_neighbor->se != NULL) {
                            *neighbor = parent_neighbor->se;
                        }
                        /* if no children, use parent's neighbor itself */
                        else {
                            *neighbor = parent_neighbor;
                        }
                    }
                    else {
                        /* something wrong with tree structure... */
                        PRINT0("FATAL: tree structure incorrect");
                        ERROR(FATAL);
                    }
                }
                else {
                    /* if parent neighbor is not found, tree is at edge */
                    *neighbor = NULL;
                }
            }
            /* cache the found neighbor to speed up future queries */
            tree->n = *neighbor;
        }
    }

    FINALLY(image_tree_get_direct_neighbor_n);
    RETURN();
}

/******************************************************************************/

result image_tree_get_direct_neighbor_e(
    image_tree *tree,
    image_tree **neighbor
    )
{
    TRY();
    image_tree *parent_neighbor;

    CHECK_POINTER(tree);
    CHECK_POINTER(neighbor);

    *neighbor = NULL;

    /* check if the neighbor has been cached in the tree already */
    if (tree->e != NULL) {
        *neighbor = tree->e;
    }
    else {
        /* either parent or neighbor should be set */
        /* if not, the tree is on the edge */
        if (tree->parent == NULL) {
            *neighbor = NULL;
        }
        else {
            /* neighbor e of nw is ne */
            if (tree->parent->nw == tree) {
                *neighbor = tree->parent->ne;
            }
            else
            /* neighbor e of sw is se */
            if (tree->parent->sw == tree) {
                *neighbor = tree->parent->se;
            }
            else {
                /* neighbor not on the same parent, get neighbor of parent */
                CHECK(image_tree_get_direct_neighbor_e(tree->parent, &parent_neighbor));
                if (parent_neighbor != NULL) {
                    if (tree->parent->ne == tree) {
                        /* neighbor of ne is parent's neighbor's nw */
                        if (parent_neighbor->nw != NULL) {
                            *neighbor = parent_neighbor->nw;
                        }
                        /* if no children, use parent's neighbor itself */
                        else {
                            *neighbor = parent_neighbor;
                        }
                    }
                    else
                    if (tree->parent->se == tree) {
                        /* neighbor of se is parent's neighbor's sw */
                        if (parent_neighbor->sw != NULL) {
                            *neighbor = parent_neighbor->sw;
                        }
                        /* if no children, use parent's neighbor itself */
                        else {
                            *neighbor = parent_neighbor;
                        }
                    }
                    else {
                        /* something wrong with tree structure... */
                        PRINT0("FATAL: tree structure incorrect");
                        ERROR(FATAL);
                    }
                }
                else {
                    /* if parent neighbor is not found, tree is at edge */
                    *neighbor = NULL;
                }
            }
            /* cache the found neighbor to speed up future queries */
            tree->e = *neighbor;
        }
    }

    FINALLY(image_tree_get_direct_neighbor_e);
    RETURN();
}

/******************************************************************************/

result image_tree_get_direct_neighbor_s(
    image_tree *tree,
    image_tree **neighbor
    )
{
    TRY();
    image_tree *parent_neighbor;

    CHECK_POINTER(tree);
    CHECK_POINTER(neighbor);

    *neighbor = NULL;

    /* check if the neighbor has been cached in the tree already */
    if (tree->s != NULL) {
        *neighbor = tree->s;
    }
    else {
        /* either parent or neighbor should be set */
        /* if not, the tree is on the edge */
        if (tree->parent == NULL) {
            *neighbor = NULL;
        }
        else {
            /* neighbor s of nw is sw */
            if (tree->parent->nw == tree) {
                *neighbor = tree->parent->sw;
            }
            else
            /* neighbor s of ne is se */
            if (tree->parent->ne == tree) {
                *neighbor = tree->parent->se;
            }
            else {
                /* neighbor not on the same parent, get neighbor of parent */
                CHECK(image_tree_get_direct_neighbor_s(tree->parent, &parent_neighbor));
                if (parent_neighbor != NULL) {
                    if (tree->parent->sw == tree) {
                        /* neighbor of sw is parent's neighbor's nw */
                        if (parent_neighbor->nw != NULL) {
                            *neighbor = parent_neighbor->nw;
                        }
                        /* if no children, use parent's neighbor itself */
                        else {
                            *neighbor = parent_neighbor;
                        }
                    }
                    else
                    if (tree->parent->se == tree) {
                        /* neighbor of se is parent's neighbor's ne */
                        if (parent_neighbor->ne != NULL) {
                            *neighbor = parent_neighbor->ne;
                        }
                        /* if no children, use parent's neighbor itself */
                        else {
                            *neighbor = parent_neighbor;
                        }
                    }
                    else {
                        /* something wrong with tree structure... */
                        PRINT0("FATAL: tree structure incorrect");
                        ERROR(FATAL);
                    }
                }
                else {
                    /* if parent neighbor is not found, tree is at edge */
                    *neighbor = NULL;
                }
            }
            /* cache the found neighbor to speed up future queries */
            tree->s = *neighbor;
        }
    }

    FINALLY(image_tree_get_direct_neighbor_s);
    RETURN();
}

/******************************************************************************/

result image_tree_get_direct_neighbor_w(
    image_tree *tree,
    image_tree **neighbor
    )
{
    TRY();
    image_tree *parent_neighbor;

    CHECK_POINTER(tree);
    CHECK_POINTER(neighbor);

    *neighbor = NULL;

    /* check if the neighbor has been cached in the tree already */
    if (tree->w != NULL) {
        *neighbor = tree->w;
    }
    else {
        /* either parent or neighbor should be set */
        /* if not, the tree is on the edge */
        if (tree->parent == NULL) {
            *neighbor = NULL;
        }
        else {
            /* neighbor w of ne is nw */
            if (tree->parent->ne == tree) {
                *neighbor = tree->parent->nw;
            }
            else
            /* neighbor w of se is sw */
            if (tree->parent->se == tree) {
                *neighbor = tree->parent->sw;
            }
            else {
                /* neighbor not on the same parent, get neighbor of parent */
                CHECK(image_tree_get_direct_neighbor_w(tree->parent, &parent_neighbor));
                if (parent_neighbor != NULL) {
                    if (tree->parent->nw == tree) {
                        /* neighbor of nw is parent's neighbor's ne */
                        if (parent_neighbor->ne != NULL) {
                            *neighbor = parent_neighbor->ne;
                        }
                        /* if no children, use parent's neighbor itself */
                        else {
                            *neighbor = parent_neighbor;
                        }
                    }
                    else
                    if (tree->parent->sw == tree) {
                        /* neighbor of sw is parent's neighbor's se */
                        if (parent_neighbor->se != NULL) {
                            *neighbor = parent_neighbor->se;
                        }
                        /* if no children, use parent's neighbor itself */
                        else {
                            *neighbor = parent_neighbor;
                        }
                    }
                    else {
                        /* something wrong with tree structure... */
                        PRINT0("FATAL: tree structure incorrect");
                        ERROR(FATAL);
                    }
                }
                else {
                    /* if parent neighbor is not found, tree is at edge */
                    *neighbor = NULL;
                }
            }
            /* cache the found neighbor to speed up future queries */
            tree->w = *neighbor;
        }
    }

    FINALLY(image_tree_get_direct_neighbor_w);
    RETURN();
}

/******************************************************************************/

/**
 * Recursive function for adding child trees from the highest level as
 * immediate neighbors to another tree
 *
 * 1. If tree has no childen, add it to list and return
 * 2. If tree has chilren, call recursively for the two children in the proper
 *    direction
 */

result image_tree_add_children_as_immediate_neighbors(
    list *target,
    image_tree *tree,
    direction dir
    )
{
    TRY();

    FINALLY(image_tree_add_children_as_immediate_neighbors);
    RETURN();
}

/******************************************************************************/

/**
 * Finds all immediate neighbors (directly adjacent neighbors on the highest
 * level) of a tree in the given direction and stores them in the list.
 *
 * 1. Find the direct neighbor; if it has children, call recursively for the
 *    two children adjacent to this tree
 *
 * 2. Peek the item at the top of stack; if it has children, pop it and push the
 *    two children adjacent to this tree into the stack; if not, add it to the
 *    end of the list
 * 3.
 */
result image_tree_find_all_immediate_neighbors
  (
  list *target,
  image_tree *tree
  )
{
  TRY();
  image_tree *new_neighbor;
  image_tree **temp_ptr;

  CHECK_POINTER(target);
  CHECK_POINTER(tree);

  CHECK(image_tree_get_direct_neighbor_n(tree, &new_neighbor));
  if (new_neighbor != NULL) {
    CHECK(list_append(target, (pointer)&new_neighbor));
  }
  CHECK(image_tree_get_direct_neighbor_e(tree, &new_neighbor));
  if (new_neighbor != NULL) {
    CHECK(list_append(target, (pointer)&new_neighbor));
  }
  CHECK(image_tree_get_direct_neighbor_s(tree, &new_neighbor));
  if (new_neighbor != NULL) {
    CHECK(list_append(target, (pointer)&new_neighbor));
  }
  CHECK(image_tree_get_direct_neighbor_w(tree, &new_neighbor));
  if (new_neighbor != NULL) {
    CHECK(list_append(target, (pointer)&new_neighbor));
  }

  FINALLY(image_tree_find_all_immediate_neighbors);
  RETURN();
}

/******************************************************************************/

void image_tree_class_create(image_tree *tree)
{
  forest_region_info *region;
  if (tree != NULL) {
    region = &tree->region_info;
    /* proceed only if the tree doesn't have its region info initialized yet */
    /*if (region->id == NULL) {*/
      /* one-tree class is it's own id, and has the rank of 0 */
      region->id = region;
      region->rank = 0;
      region->x1 = tree->block->x;
      region->y1 = tree->block->y;
      region->x2 = region->x1 + tree->block->w;
      region->y2 = region->y1 + tree->block->h;
      memory_copy((data_pointer)&region->stat, (data_pointer)tree->block->value, 1, sizeof(statistics));
    /*}*/
  }
}

/******************************************************************************/

void image_tree_class_union(image_tree *tree1, image_tree *tree2)
{
  if (tree1 != NULL && tree2 != NULL) {
    forest_region_info *id1, *id2;

    id1 = image_tree_class_find(&tree1->region_info);
    id2 = image_tree_class_find(&tree2->region_info);
    if (id1 == NULL || id2 == NULL) {
      return;
    }
    /* if the trees are already in the same class, no need for union */
    if (id1 == id2) {
      return;
    }
    /* otherwise set the tree with higher class rank as id of the union */
    else {
      statistics *stat;
      I_value N, mean, variance;
      if (id1->rank < id2->rank) {
        id1->id = id2;
        id2->x1 = (id1->x1 < id2->x1) ? id1->x1 : id2->x1;
        id2->y1 = (id1->y1 < id2->y1) ? id1->y1 : id2->y1;
        id2->x2 = (id1->x2 > id2->x2) ? id1->x2 : id2->x2;
        id2->y2 = (id1->y2 > id2->y2) ? id1->y2 : id2->y2;
        stat = &id2->stat;
        N = (stat->N += id1->stat.N);
        stat->sum += id1->stat.sum;
        stat->sum2 += id1->stat.sum2;
        mean = stat->mean = stat->sum / N;
        variance = stat->sum2 / N - mean*mean;
        if (variance < 0) variance = 0;
        stat->variance = variance;
        stat->deviation = sqrt(variance);
      }
      else {
        id2->id = id1;
        /* when equal rank trees are combined, the root tree's rank is increased */
        if (id1->rank == id2->rank) {
          id1->rank += 1;
        }
        id1->x1 = (id1->x1 < id2->x1) ? id1->x1 : id2->x1;
        id1->y1 = (id1->y1 < id2->y1) ? id1->y1 : id2->y1;
        id1->x2 = (id1->x2 > id2->x2) ? id1->x2 : id2->x2;
        id1->y2 = (id1->y2 > id2->y2) ? id1->y2 : id2->y2;
        stat = &id1->stat;
        N = (stat->N += id2->stat.N);
        stat->sum += id2->stat.sum;
        stat->sum2 += id2->stat.sum2;
        mean = stat->mean = stat->sum / N;
        variance = stat->sum2 / N - mean*mean;
        if (variance < 0) variance = 0;
        stat->variance = variance;
        stat->deviation = sqrt(variance);
      }
    }
  }
}

/******************************************************************************/

forest_region_info *image_tree_class_find(forest_region_info *region)
{
  if (region != NULL) {
    if (region->id != NULL && region->id != region) {
      region->id = image_tree_class_find(region->id);
    }
    return region->id;
  }
  return NULL;
}

/******************************************************************************/

uint32 image_tree_class_get(image_tree *tree)
{
  return (uint32)image_tree_class_find(&tree->region_info);
}

/******************************************************************************/

bool image_tree_is_class_parent
(
  image_tree *tree
)
{
  if (tree != NULL) {
    forest_region_info *region, *id;
    region = &tree->region_info;
    id = image_tree_class_find(region);
    if (id == region) {
      return true;
    }
  }
  return false;
}

/* end of file                                                                */
/******************************************************************************/
