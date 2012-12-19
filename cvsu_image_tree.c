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
string image_tree_forest_divide_with_dev_name = "image_tree_forest_divide_with_dev";
string image_tree_forest_divide_with_entropy_name = "image_tree_forest_divide_with_entropy";
string image_tree_forest_read_name = "image_tree_forest_read";
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

result image_tree_nullify
(
  image_tree *target
)
{
  TRY();
  target->root = NULL;
  target->parent = NULL;
  target->class_id = NULL;
  target->nw = NULL;
  target->ne = NULL;
  target->sw = NULL;
  target->se = NULL;
  target->n = NULL;
  target->e = NULL;
  target->s = NULL;
  target->w = NULL;
  target->level = 0;
  target->class_rank = 0;
  FINALLY(image_tree_nullify);
  RETURN();
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
    CHECK(list_create(&target->trees, 64 * size, sizeof(image_tree), 1));

    if (!list_is_null(&target->blocks)) {
      CHECK(list_destroy(&target->blocks));
    }
    CHECK(list_create(&target->blocks, 64 * size, sizeof(image_block), 1));
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
      CHECK(list_create(&target->values, 64 * size, sizeof(stat_grey), 1));
    }
    else
    /* yuv image required for color statistics */
    /* TODO: need a mechanism to enable using Lab or similar (maybe use a macro?) */
    if (type == b_STAT_COLOR) {
      CHECK(pixel_image_create(target->source, p_U8, YUV, width, height, 3,
                               3 * width));
      CHECK(list_create(&target->values, 64 * size, sizeof(stat_color), 1));
    }
    else
    if (type == b_STATISTICS) {
      CHECK(pixel_image_create(target->source, p_U8, GREY, width, height, 1,
                               1 * width));
      CHECK(list_create(&target->values, 64 * size, sizeof(statistics), 1));
    }
    else {
      /* should never reach here actually */
      ERROR(BAD_PARAM);
    }

    /* image is only created here, content is copied/converted in update stage */
    target->type = type;
  }

  CHECK(list_clear(&target->trees));
  CHECK(list_clear(&target->blocks));
  CHECK(list_clear(&target->values));

  /* create tree roots and their trees and blocks */
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

      new_tree.root = &target->roots[pos];
      new_tree.parent = NULL;
      new_tree.class_id = NULL;
      new_tree.nw = NULL;
      new_tree.ne = NULL;
      new_tree.sw = NULL;
      new_tree.se = NULL;
      new_tree.block = block_ptr;
      new_tree.n = NULL;
      new_tree.e = NULL;
      new_tree.s = NULL;
      new_tree.w = NULL;
      new_tree.level = 1;
      CHECK(list_append_reveal_data(&target->trees, (pointer)&new_tree, (pointer*)&tree_ptr));

      target->roots[pos].forest = target;
      target->roots[pos].tree = tree_ptr;
      CHECK(pixel_image_create_roi(&target->roots[pos].ROI, target->original, block_ptr->x, block_ptr->y, block_ptr->w, block_ptr->h));
      CHECK(small_integral_image_create(&target->roots[pos].I, &target->roots[pos].ROI));
    }
  }result image_tree_forest_divide_with_entropy(
  /** forest to be segmented */
  image_tree_forest *target,
                                                /** the minimum size for the trees in the end result */
                                                uint32 min_size
                                                );


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

    size = target->rows * target->cols;
    for (pos = 0; pos < size; pos++) {
        CHECK(integral_image_destroy(&target->roots[pos].I));
        CHECK(pixel_image_destroy(&target->roots[pos].ROI));
    }
    CHECK(memory_deallocate((data_pointer*)&target->roots));

    CHECK(pixel_image_destroy(target->source));
    CHECK(memory_deallocate((data_pointer*)&target->source));

    /*CHECK(edge_block_image_destroy(&target->edge_image));*/

    CHECK(image_tree_forest_nullify(target));

    FINALLY(image_tree_forest_destroy);
    RETURN();
}

/******************************************************************************/

result image_tree_forest_nullify(
    image_tree_forest *target
    )
{
    TRY();

    CHECK_POINTER(target);

    target->original = NULL;
    target->source = NULL;
    /*CHECK(edge_block_image_nullify(&target->edge_image));*/
    target->rows = 0;
    target->cols = 0;
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

  /* must also set child nodes of root level trees to NULL... */
  /* TODO: find out a better way */
  size = target->rows * target->cols;
  for (pos = 0; pos < size; pos++) {
    tree = target->roots[pos].tree;
    tree->class_id = NULL;
    tree->nw = NULL;
    tree->ne = NULL;
    tree->sw = NULL;
    tree->se = NULL;
  }

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
  uint32 pos, size;
  /*printf("update\n");*/

  CHECK(image_tree_forest_update_prepare(target));

  size = target->rows * target->cols;
  /*printf("size = %ul\n", size);*/
  for (pos = 0; pos < size; pos++) {
      CHECK(image_tree_root_update(&target->roots[pos]));
  }

  FINALLY(image_tree_forest_update);
  RETURN();
}

/******************************************************************************/

result image_tree_forest_divide_with_dev
(
  image_tree_forest *target,
  sint16 threshold
)
{
  TRY();
  list_item *trees;
  image_tree *current_tree;

  CHECK_POINTER(target);
  CHECK_PARAM(threshold > 1);

  if (target->type == b_STAT_GREY) {
    trees = target->trees.first.next;
    while (trees != &target->trees.last) {
      current_tree = (image_tree *)trees->data;
      if (((stat_grey *)current_tree->block->value)->dev > threshold) {
        CHECK(image_tree_divide(current_tree));
      }
      trees = trees->next;
    }
  }
  else
  if (target->type == b_STAT_COLOR) {
    trees = target->trees.first.next;
    while (trees != &target->trees.last) {
      current_tree = (image_tree *)trees->data;
      if (((stat_color *)current_tree->block->value)->dev_i > threshold) {
        CHECK(image_tree_divide(current_tree));
      }
      trees = trees->next;
    }
  }
  else
  if (target->type == b_STATISTICS) {
    trees = target->trees.first.next;
    while (trees != &target->trees.last) {
      current_tree = (image_tree *)trees->data;
      if (((statistics *)current_tree->block->value)->deviation > (I_value)threshold) {
        CHECK(image_tree_divide(current_tree));
      }
      trees = trees->next;
    }
  }
  else {
    /* should never come here... */
    ERROR(BAD_TYPE);
  }

  FINALLY(image_tree_forest_divide_with_dev);
  RETURN();
}

/******************************************************************************/

result image_tree_forest_divide_with_entropy
(
  image_tree_forest *target,
  uint32 min_size
)
{
  TRY();
  list_item *trees;
  image_tree *current_tree;

  CHECK_POINTER(target);
  CHECK_PARAM(min_size < target->tree_width && min_size < target->tree_height);

  trees = target->trees.first.next;
  while (trees != &target->trees.last) {
    current_tree = (image_tree *)trees->data;
    CHECK(image_tree_divide_with_entropy(current_tree, min_size));
    trees = trees->next;
  }

  FINALLY(image_tree_forest_divide_with_entropy);
  RETURN();
}

/******************************************************************************/

result image_tree_forest_read(
    image_tree_forest *target,
    string source,
    uint16 tree_width,
    uint16 tree_height
    )
{
    TRY();
    FILE* ifh;
    pixel_image *new_image;
    char type, endl;
    int read_result;
    uint32 read_size, width, height, maxval;
    image_block_type btype;

    CHECK_POINTER(target);
    CHECK_POINTER(source);

    new_image = NULL;

    /*printf("Starting to read image '%s'...\n", source);*/
    ifh = fopen(source, "rb");
    if (ifh == NULL) {
        /*printf("Error: opening file failed\n");*/
        ERROR(INPUT_ERROR);
    }
    /* read the image header - supposed to be PPM image */
    read_result = fscanf(ifh, "P%c %lu %lu %lu%c", &type, &width, &height, &maxval, &endl);
    if (read_result < 5) {
        /*printf("Error: reading image header failed\n");*/
        ERROR(INPUT_ERROR);
    }
    /* only support PPM (P6) and PGM (P5) images at this stage... */
    if (type == '5') {
        new_image = pixel_image_alloc();
        /*CHECK(memory_allocate((data_pointer *)&new_image, 1, sizeof(pixel_image)));*/
        CHECK(pixel_image_create(new_image, p_U8, GREY, width, height, 1, width));
        read_size = fread(new_image->data, sizeof(byte), new_image->size, ifh);
        if (read_size != new_image->size) {
            /*printf("Reading image data failed");*/
            ERROR(INPUT_ERROR);
        }
        btype = b_STAT_GREY;
    }
    else
    if (type == '6') {
        new_image = pixel_image_alloc();
        /*CHECK(memory_allocate((data_pointer *)&new_image, 1, sizeof(pixel_image)));*/
        CHECK(pixel_image_create(new_image, p_U8, RGB, width, height, 3, 3 * width));
        /*CHECK(memory_allocate((data_pointer *)&grey_image, 1, sizeof(pixel_image)));*/
        /*CHECK(pixel_image_create(grey_image, p_U8, GREY, width, height, 1, width));*/
        read_size = fread(new_image->data, sizeof(byte), new_image->size, ifh);
        if (read_size != new_image->size) {
            /*printf("Reading image data failed");*/
            ERROR(INPUT_ERROR);
        }
        btype = b_STAT_COLOR;
        /*CHECK(convert_rgb24_to_grey8(rgb_image, grey_image));*/
    }
    else {
        /*printf("Error: image type not supported\n");*/
        ERROR(BAD_PARAM);
    }

    /*printf("Successfully read image of size (%lu x %lu)\n", width, height);*/

    CHECK(image_tree_forest_create(target, new_image, tree_width, tree_height, btype));

    /*printf("Created image forest with trees (%u x %u)\n", tree_width, tree_height);*/
    CHECK(image_tree_forest_update(target));

    FINALLY(image_tree_forest_read);
    /* NOTE: original image is deleted, cannot reload or update after this method */
    pixel_image_free(new_image);
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
  image_tree_forest *forest;
  image_block new_block, *child_block;
  image_tree new_tree, *child_tree;
  image_block_type type;

  CHECK_POINTER(target);
  if (target->nw == NULL && target->ne == NULL && target->sw == NULL && target->se == NULL) {
    if (target->block->w > 1 && target->block->h > 1) {
      forest = target->root->forest;
      type = forest->type;
      new_tree.root = target->root;
      new_tree.parent = target;
      new_tree.class_id = NULL;
      new_tree.nw = NULL;
      new_tree.ne = NULL;
      new_tree.sw = NULL;
      new_tree.se = NULL;
      new_tree.n = NULL;
      new_tree.e = NULL;
      new_tree.s = NULL;
      new_tree.w = NULL;
      new_tree.level = target->level + 1;
      new_tree.class_rank = 0;
      new_block.w = (uint16)(target->block->w / 2);
      new_block.h = (uint16)(target->block->h / 2);

      small_integral_image_box_resize(&target->root->box, new_block.w, new_block.h);

      /* nw block */
      new_block.x = target->block->x;
      new_block.y = target->block->y;
      if (type == b_STAT_GREY) {
        stat_grey new_value, *value_ptr;
        new_value.mean = 0;
        new_value.dev = 0;
        CHECK(list_append_reveal_data(&forest->values, (pointer)&new_value, (pointer*)&value_ptr));
        new_block.value = (pointer)value_ptr;
      }
      else
      if (type == b_STAT_COLOR) {
        stat_color new_value, *value_ptr;
        new_value.mean_i = 0;
        new_value.dev_i = 0;
        new_value.mean_c1 = 0;
        new_value.dev_c1 = 0;
        new_value.mean_c2 = 0;
        CHECK(list_append_reveal_data(&forest->values, (pointer)&new_value, (pointer*)&value_ptr));
        new_block.value = (pointer)value_ptr;
      }
      else
      if (type == b_STATISTICS) {
        statistics new_value, *value_ptr;
        new_value.N = 0;
        new_value.sum = 0;
        new_value.sum2 = 0;
        new_value.mean = 0;
        new_value.variance = 0;
        new_value.deviation = 0;
        CHECK(list_append_reveal_data(&forest->values, (pointer)&new_value, (pointer*)&value_ptr));
        new_block.value = (pointer)value_ptr;
      }
      CHECK(list_append_reveal_data(&forest->blocks, (pointer)&new_block, (pointer*)&child_block));
      new_tree.block = child_block;
      CHECK(list_append_reveal_data(&forest->trees, (pointer)&new_tree, (pointer*)&child_tree));
      target->nw = child_tree;

      CHECK(image_tree_update(child_tree));
      /* ne block */
      new_block.x = (uint16)(target->block->x + new_block.w);
      if (type == b_STAT_GREY) {
        stat_grey new_value, *value_ptr;
        new_value.mean = 0;
        new_value.dev = 0;
        CHECK(list_append_reveal_data(&forest->values, (pointer)&new_value, (pointer*)&value_ptr));
        new_block.value = (pointer)value_ptr;
      }
      else
      if (type == b_STAT_COLOR) {
        stat_color new_value, *value_ptr;
        new_value.mean_i = 0;
        new_value.dev_i = 0;
        new_value.mean_c1 = 0;
        new_value.dev_c1 = 0;
        new_value.mean_c2 = 0;
        CHECK(list_append_reveal_data(&forest->values, (pointer)&new_value, (pointer*)&value_ptr));
        new_block.value = (pointer)value_ptr;
      }
      else
      if (type == b_STATISTICS) {
        statistics new_value, *value_ptr;
        new_value.N = 0;
        new_value.sum = 0;
        new_value.sum2 = 0;
        new_value.mean = 0;
        new_value.variance = 0;
        new_value.deviation = 0;
        CHECK(list_append_reveal_data(&forest->values, (pointer)&new_value, (pointer*)&value_ptr));
        new_block.value = (pointer)value_ptr;
      }
      CHECK(list_append_reveal_data(&forest->blocks, (pointer)&new_block, (pointer*)&child_block));
      new_tree.block = child_block;
      CHECK(list_append_reveal_data(&forest->trees, (pointer)&new_tree, (pointer*)&child_tree));
      target->ne = child_tree;

      CHECK(image_tree_update(child_tree));

      /* se block */
      new_block.y = (uint16)(target->block->y + new_block.h);
      if (type == b_STAT_GREY) {
        stat_grey new_value, *value_ptr;
        new_value.mean = 0;
        new_value.dev = 0;
        CHECK(list_append_reveal_data(&forest->values, (pointer)&new_value, (pointer*)&value_ptr));
        new_block.value = (pointer)value_ptr;
      }
      else
      if (type == b_STAT_COLOR) {
        stat_color new_value, *value_ptr;
        new_value.mean_i = 0;
        new_value.dev_i = 0;
        new_value.mean_c1 = 0;
        new_value.dev_c1 = 0;
        new_value.mean_c2 = 0;
        CHECK(list_append_reveal_data(&forest->values, (pointer)&new_value, (pointer*)&value_ptr));
        new_block.value = (pointer)value_ptr;
      }
      else
      if (type == b_STATISTICS) {
        statistics new_value, *value_ptr;
        new_value.N = 0;
        new_value.sum = 0;
        new_value.sum2 = 0;
        new_value.mean = 0;
        new_value.variance = 0;
        new_value.deviation = 0;
        CHECK(list_append_reveal_data(&forest->values, (pointer)&new_value, (pointer*)&value_ptr));
        new_block.value = (pointer)value_ptr;
      }
      CHECK(list_append_reveal_data(&forest->blocks, (pointer)&new_block, (pointer*)&child_block));
      new_tree.block = child_block;
      CHECK(list_append_reveal_data(&forest->trees, (pointer)&new_tree, (pointer*)&child_tree));
      target->se = child_tree;

      CHECK(image_tree_update(child_tree));

      /* sw block */
      new_block.x = target->block->x;
      if (type == b_STAT_GREY) {
        stat_grey new_value, *value_ptr;
        new_value.mean = 0;
        new_value.dev = 0;
        CHECK(list_append_reveal_data(&forest->values, (pointer)&new_value, (pointer*)&value_ptr));
        new_block.value = (pointer)value_ptr;
      }
      else
      if (type == b_STAT_COLOR) {
        stat_color new_value, *value_ptr;
        new_value.mean_i = 0;
        new_value.dev_i = 0;
        new_value.mean_c1 = 0;
        new_value.dev_c1 = 0;
        new_value.mean_c2 = 0;
        CHECK(list_append_reveal_data(&forest->values, (pointer)&new_value, (pointer*)&value_ptr));
        new_block.value = (pointer)value_ptr;
      }
      else
      if (type == b_STATISTICS) {
        statistics new_value, *value_ptr;
        new_value.N = 0;
        new_value.sum = 0;
        new_value.sum2 = 0;
        new_value.mean = 0;
        new_value.variance = 0;
        new_value.deviation = 0;
        CHECK(list_append_reveal_data(&forest->values, (pointer)&new_value, (pointer*)&value_ptr));
        new_block.value = (pointer)value_ptr;
      }
      CHECK(list_append_reveal_data(&forest->blocks, (pointer)&new_block, (pointer*)&child_block));
      new_tree.block = child_block;
      CHECK(list_append_reveal_data(&forest->trees, (pointer)&new_tree, (pointer*)&child_tree));
      target->sw = child_tree;

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

      CHECK(image_tree_update(child_tree));
    }
  }

  FINALLY(image_tree_divide);
  RETURN();
}

/******************************************************************************/

result image_tree_get_statistics
(
  image_tree *source,
  statistics *target
)
{
  TRY();

  CHECK_POINTER(source);
  CHECK_POINTER(target);

  if (source->root->forest->type == b_STATISTICS) {
    CHECK(memory_copy((void*)target, (void*)source->block->value, 1, sizeof(statistics)));
  }
  else {
    I_value mean, var;
    small_integral_image_box *box;

    box = &source->root->box;

    small_integral_image_box_resize(box, source->block->w, source->block->h);
    box->channel = 0;
    small_integral_image_box_update(box, source->block->x, source->block->y);
    mean = ((I_value)box->sum / (I_value)box->N);
    var = (((I_value)box->sumsqr / (I_value)box->N) - (mean * mean));
    if (var < 0) var = 0;

    target->N = (I_value)box->N;
    target->sum = (I_value)box->sum;
    target->sum2 = (I_value)box->sumsqr;
    target->mean = ((mean < 0) ? 0 : ((mean > 255) ? 255 : mean));
    target->variance = var;
    target->deviation = sqrt(var);
  }

  FINALLY(image_tree_get_statistics);
  RETURN();
}

result image_tree_get_child_statistics
(
  image_tree *source,
  statistics *target
)
{
  TRY();
  uint16 x, y, w, h;
  I_value mean, var;
  small_integral_image_box *box;
  statistics *stat;

  CHECK_POINTER(source);
  CHECK_POINTER(target);

  /* if the tree has already been divided, return the values from the children */
  if (source->root->forest->type == b_STATISTICS && source->nw != NULL && source->ne != NULL && source->sw != NULL && source->se != NULL) {
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
        box = &source->root->box;

        w = (uint16)(source->block->w / 2);
        h = (uint16)(source->block->h / 2);

        /* if the new width is 1, no need to calculate, use the pixel values */
        if (w == 1 && h == 1) {

          /* nw child block */
          x = source->block->x;
          y = source->block->y;

          stat = &target[0];
          stat->N = 1;
          stat->sum = (I_value)box->sum;
          stat->sum2 = (I_value)box->sumsqr;
          stat->mean = ((mean < 0) ? 0 : ((mean > 255) ? 255 : mean));
          stat->variance = var;
          stat->deviation = sqrt(var);

          /* ne child block */
          x = x + w;

          small_integral_image_box_update(box, x, y);
          mean = ((I_value)box->sum / (I_value)box->N);
          var = (((I_value)box->sumsqr / (I_value)box->N) - (mean * mean));
          if (var < 0) var = 0;

          stat = &target[1];
          stat->N = (I_value)box->N;
          stat->sum = (I_value)box->sum;
          stat->sum2 = (I_value)box->sumsqr;
          stat->mean = ((mean < 0) ? 0 : ((mean > 255) ? 255 : mean));
          stat->variance = var;
          stat->deviation = sqrt(var);

          /* se child block */
          y = y + h;

          small_integral_image_box_update(box, x, y);
          mean = ((I_value)box->sum / (I_value)box->N);
          var = (((I_value)box->sumsqr / (I_value)box->N) - (mean * mean));
          if (var < 0) var = 0;

          stat = &target[3];
          stat->N = (I_value)box->N;
          stat->sum = (I_value)box->sum;
          stat->sum2 = (I_value)box->sumsqr;
          stat->mean = ((mean < 0) ? 0 : ((mean > 255) ? 255 : mean));
          stat->variance = var;
          stat->deviation = sqrt(var);

          /* sw child block */
          x = x - w;

          small_integral_image_box_update(box, x, y);
          mean = ((I_value)box->sum / (I_value)box->N);
          var = (((I_value)box->sumsqr / (I_value)box->N) - (mean * mean));
          if (var < 0) var = 0;

          stat = &target[2];
          stat->N = (I_value)box->N;
          stat->sum = (I_value)box->sum;
          stat->sum2 = (I_value)box->sumsqr;
          stat->mean = ((mean < 0) ? 0 : ((mean > 255) ? 255 : mean));
          stat->variance = var;
          stat->deviation = sqrt(var);
        }
        else {
        small_integral_image_box_resize(box, w, h);
        box->channel = 0;

        /* nw child block */
        x = source->block->x;
        y = source->block->y;

        small_integral_image_box_update(box, x, y);
        mean = ((I_value)box->sum / (I_value)box->N);
        var = (((I_value)box->sumsqr / (I_value)box->N) - (mean * mean));
        if (var < 0) var = 0;

        stat = &target[0];
        stat->N = (I_value)box->N;
        stat->sum = (I_value)box->sum;
        stat->sum2 = (I_value)box->sumsqr;
        stat->mean = ((mean < 0) ? 0 : ((mean > 255) ? 255 : mean));
        stat->variance = var;
        stat->deviation = sqrt(var);

        /* ne child block */
        x = x + w;

        small_integral_image_box_update(box, x, y);
        mean = ((I_value)box->sum / (I_value)box->N);
        var = (((I_value)box->sumsqr / (I_value)box->N) - (mean * mean));
        if (var < 0) var = 0;

        stat = &target[1];
        stat->N = (I_value)box->N;
        stat->sum = (I_value)box->sum;
        stat->sum2 = (I_value)box->sumsqr;
        stat->mean = ((mean < 0) ? 0 : ((mean > 255) ? 255 : mean));
        stat->variance = var;
        stat->deviation = sqrt(var);

        /* se child block */
        y = y + h;

        small_integral_image_box_update(box, x, y);
        mean = ((I_value)box->sum / (I_value)box->N);
        var = (((I_value)box->sumsqr / (I_value)box->N) - (mean * mean));
        if (var < 0) var = 0;

        stat = &target[3];
        stat->N = (I_value)box->N;
        stat->sum = (I_value)box->sum;
        stat->sum2 = (I_value)box->sumsqr;
        stat->mean = ((mean < 0) ? 0 : ((mean > 255) ? 255 : mean));
        stat->variance = var;
        stat->deviation = sqrt(var);

        /* sw child block */
        x = x - w;

        small_integral_image_box_update(box, x, y);
        mean = ((I_value)box->sum / (I_value)box->N);
        var = (((I_value)box->sumsqr / (I_value)box->N) - (mean * mean));
        if (var < 0) var = 0;

        stat = &target[2];
        stat->N = (I_value)box->N;
        stat->sum = (I_value)box->sum;
        stat->sum2 = (I_value)box->sumsqr;
        stat->mean = ((mean < 0) ? 0 : ((mean > 255) ? 255 : mean));
        stat->variance = var;
        stat->deviation = sqrt(var);
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
  image_tree_forest *forest;
  image_tree nw_tree, ne_tree, sw_tree, se_tree, *child_tree;
  image_block nw_block, ne_block, sw_block, se_block, *child_block;
  statistics value, values[4], *child_value;
  I_value zscores[4];
  image_block_type type;
  uint16 i, x, y, w, h;

  CHECK_POINTER(target);

  CHECK(image_tree_nullify(&nw_tree));
  CHECK(image_tree_nullify(&ne_tree));
  CHECK(image_tree_nullify(&sw_tree));
  CHECK(image_tree_nullify(&se_tree));

  w = (uint16)(source->block->w / 2);
  h = (uint16)(source->block->h / 2);

  if (target->nw == NULL && target->ne == NULL && target->sw == NULL && target->se == NULL) {
    value = *((statistics *)target->block->value);
    image_tree_get_child_statistics(target, values);
    for (i = 0; i < 4; i++) {
      zscores[i] =
    }

    /* nw child block */
    x = source->block->x;
    y = source->block->y;

  }

  FINALLY(image_tree_divide_with_entropy);
  RETURN();
}

/******************************************************************************/

sint16 signum(sint16 v)
{
    if (v < 0) {
        return -1;
    }
    if (v > 0) {
        return 1;
    }
    return 0;
}

dir image_tree_dir_i(image_tree *tree)
{
  dir res;
  sint16 mean_nw, mean_ne, mean_sw, mean_se;

  res.h = 0;
  res.v = 0;

  if (tree == NULL) {
    return res;
  }

  if (tree->nw == NULL || tree->ne == NULL || tree->sw == NULL || tree->se == NULL) {
    return res;
  }

  if (tree->root->forest->type != b_STAT_COLOR) {
    return res;
  }

  mean_nw = ((stat_color *)tree->nw->block->value)->mean_i;
  mean_ne = ((stat_color *)tree->ne->block->value)->mean_i;
  mean_sw = ((stat_color *)tree->sw->block->value)->mean_i;
  mean_se = ((stat_color *)tree->se->block->value)->mean_i;

  res.v = (sint16)((mean_nw + mean_sw) - (mean_ne + mean_se));
  res.h = (sint16)((mean_nw + mean_ne) - (mean_sw + mean_se));

  return res;
}

dir image_tree_dir_c1(image_tree *tree)
{
  dir res;
  sint16 mean_nw, mean_ne, mean_sw, mean_se;

  res.h = 0;
  res.v = 0;

  if (tree == NULL) {
    return res;
  }

  if (tree->nw == NULL || tree->ne == NULL || tree->sw == NULL || tree->se == NULL) {
    return res;
  }

  if (tree->root->forest->type != b_STAT_COLOR) {
    return res;
  }

  mean_nw = ((stat_color *)tree->nw->block->value)->mean_c1;
  mean_ne = ((stat_color *)tree->ne->block->value)->mean_c1;
  mean_sw = ((stat_color *)tree->sw->block->value)->mean_c1;
  mean_se = ((stat_color *)tree->se->block->value)->mean_c1;

  res.v = (sint16)((mean_nw + mean_sw) - (mean_ne + mean_se));
  res.h = (sint16)((mean_nw + mean_ne) - (mean_sw + mean_se));

  return res;
}

dir image_tree_dir_c2(image_tree *tree)
{
  dir res;
  sint16 mean_nw, mean_ne, mean_sw, mean_se;

  res.h = 0;
  res.v = 0;

  if (tree == NULL) {
    return res;
  }

  if (tree->nw == NULL || tree->ne == NULL || tree->sw == NULL || tree->se == NULL) {
    return res;
  }

  if (tree->root->forest->type != b_STAT_COLOR) {
    return res;
  }

  mean_nw = ((stat_color *)tree->nw->block->value)->mean_c2;
  mean_ne = ((stat_color *)tree->ne->block->value)->mean_c2;
  mean_sw = ((stat_color *)tree->sw->block->value)->mean_c2;
  mean_se = ((stat_color *)tree->se->block->value)->mean_c2;

  res.v = (sint16)((mean_nw + mean_sw) - (mean_ne + mean_se));
  res.h = (sint16)((mean_nw + mean_ne) - (mean_sw + mean_se));

  return res;
}
    /*
    mag_v = (mean_nw + mean_sw) - (mean_ne + mean_se);
    if (mag_v > block->value.dev) {
        block->value.dir_v = mag_v;
    }
    else
    if (mag_v < -block->value.dev) {
        block->value.dir_v = mag_v;
    }
    else {
        block->value.dir_v = 0;
    }

    mag_h = (mean_nw + mean_ne) - (mean_sw + mean_se);
    if (mag_h > block->value.dev) {
        block->value.dir_h = mag_h;
    }
    else
    if (mag_h < -block->value.dev) {
        block->value.dir_h = mag_h;
    }
    else {
        block->value.dir_h = 0;
    }

    if (block->value.dir_h != 0 && block->value.dir_v != 0) {
        if (2 * signum(block->value.dir_h) * block->value.dir_h < block->value.dir_v) {
            block->value.dir_h = 0;
        }
        if (2 * signum(block->value.dir_v) * block->value.dir_v < block->value.dir_h) {
            block->value.dir_v = 0;
        }
    }
    */

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
  if (tree != NULL) {
    /* if tree already has a class, don't reset it */
    if (tree->class_id == NULL) {
      /* one-tree class is it's own id, and has the rank of 0 */
      tree->class_id = tree;
      tree->class_rank = 0;
    }
  }
}

/******************************************************************************/

void image_tree_class_union(image_tree *tree1, image_tree *tree2)
{
  if (tree1 != NULL && tree2 != NULL) {
    image_tree *id1, *id2;

    id1 = image_tree_class_find(tree1);
    id2 = image_tree_class_find(tree2);
    if (id1 == NULL || id2 == NULL) {
      return;
    }
    /* if the trees are already in the same class, no need for union */
    if (id1 == id2) {
      return;
    }
    /* otherwise set the tree with higher class rank as id of the union */
    else {
      if (id1->class_rank < id2->class_rank) {
        id1->class_id = id2;
      }
      else
      if (id1->class_rank > id2->class_rank) {
        id2->class_id = id1;
      }
      /* when equal rank trees are combined, the root tree's rank is increased */
      else {
        id2->class_id = id1;
        id1->class_rank += 1;
      }
    }
  }
}

/******************************************************************************/

image_tree *image_tree_class_find(image_tree *tree)
{
  if (tree != NULL) {
    if (tree->class_id != tree && tree->class_id != NULL) {
      tree->class_id = image_tree_class_find(tree->class_id);
    }
    return tree->class_id;
  }
  return NULL;
}

/******************************************************************************/

uint32 image_tree_class_get(image_tree *tree)
{
  return (uint32)image_tree_class_find(tree);
}

/******************************************************************************/
