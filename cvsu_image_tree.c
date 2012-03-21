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

string image_tree_forest_alloc_name = "image_tree_forest_alloc";
string image_tree_forest_free_name = "image_tree_forest_free";
string image_tree_forest_create_name = "image_tree_forest_create";
string image_tree_forest_reload_name = "image_tree_forest_reload";
string image_tree_forest_destroy_name = "image_tree_forest_destroy";
string image_tree_forest_nullify_name = "image_tree_forest_nullify";
string image_tree_forest_update_prepare_name = "image_tree_forest_update_prepare";
string image_tree_forest_update_name = "image_tree_forest_update";
string image_tree_forest_divide_with_dev_name = "image_tree_forest_divide_with_dev";
string image_tree_forest_read_name = "image_tree_forest_read";
string image_tree_root_update_name = "image_tree_root_update";
string image_tree_update_name = "image_tree_update";
string image_tree_divide_name = "image_tree_divide";
string image_tree_create_neighbor_list_name = "image_tree_create_neighbor_list";
string image_tree_get_direct_neighbor_name = "image_tree_get_direct_neighbor";
string image_tree_get_direct_neighbor_n_name = "image_tree_get_direct_neighbor_n";
string image_tree_get_direct_neighbor_e_name = "image_tree_get_direct_neighbor_e";
string image_tree_get_direct_neighbor_s_name = "image_tree_get_direct_neighbor_s";
string image_tree_get_direct_neighbor_w_name = "image_tree_get_direct_neighbor_w";
string image_tree_add_children_as_immediate_neighbors_name = "image_tree_add_children_as_immediate_neighbors";
string image_tree_find_all_immediate_neighbors_name = "image_tree_find_all_immediate_neighbors";

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
    r = SUCCESS;
    if (ptr != NULL) {
        CHECK(image_tree_forest_destroy(ptr));
        CHECK(memory_deallocate((data_pointer *)&ptr));
    }
    FINALLY(image_tree_forest_free);
}

/******************************************************************************/

result image_tree_forest_create(
    image_tree_forest *target,
    pixel_image *source,
    uint16 tree_width,
    uint16 tree_height
    )
{
    TRY();
    uint32 row, col, pos, size;
    image_tree new_tree, *tree_ptr;
    image_block new_block, *block_ptr;

    CHECK_POINTER(target);
    CHECK_POINTER(source);
    CHECK_PARAM(tree_width <= source->width);
    CHECK_PARAM(tree_height <= source->height);
    CHECK_PARAM(source->type == p_U8);
    CHECK_PARAM(source->format == GREY || source->format == YUV || source->format == RGB);

    if (source->format == RGB) {
        target->original = pixel_image_alloc();
        target->own_original = 1;
        CHECK(pixel_image_create(target->original, p_U8, YUV, source->width, source->height, 3, 3 * source->width));
        CHECK(convert_rgb24_to_yuv24(source, target->original));
    }
    else {
        target->original = source;
        target->own_original = 0;
    }

    target->tree_width = tree_width;
    target->tree_height = tree_height;
    target->cols = (uint16)(source->width / target->tree_width);
    target->rows = (uint16)(source->height / target->tree_height);
    target->dx = (uint16)((source->width - (target->cols * target->tree_width)) / 2);
    target->dy = (uint16)((source->height - (target->rows * target->tree_height)) / 2);
    target->type = b_STAT_COLOR;

    size = target->rows * target->cols;

    /*CHECK(edge_block_image_create(&target->edge_image, 10 * size, 10 * size, 100 * size));*/
    CHECK(memory_allocate((data_pointer *)&target->roots, size, sizeof(image_tree_root)));
    CHECK(list_create(&target->trees, 100 * size, sizeof(image_tree), 10));
    CHECK(list_create(&target->blocks, 100 * size, sizeof(image_block), 10));

    /* create tree roots and their trees and blocks */
    for (row = 0, pos = 0; row < target->rows; row++) {
        for (col = 0; col < target->cols; col++, pos++) {
            new_block.x = (uint16)(target->dx + col * target->tree_width);
            new_block.y = (uint16)(target->dy + row * target->tree_height);
            new_block.w = (uint16)(target->tree_width);
            new_block.h = (uint16)(target->tree_height);
            /*new_block.value = NULL;*/
            new_block.value.mean_i = 0;
            new_block.value.dev_i = 0;
            new_block.value.mean_c1 = 0;
            new_block.value.dev_c1 = 0;
            new_block.value.mean_c2 = 0;
            new_block.value.dev_c2 = 0;
            CHECK(list_append_reveal_data(&target->blocks, (pointer)&new_block, (pointer*)&block_ptr));
            new_tree.block = block_ptr;
            new_tree.root = &target->roots[pos];
            new_tree.parent = NULL;
            new_tree.nw = NULL;
            new_tree.ne = NULL;
            new_tree.sw = NULL;
            new_tree.se = NULL;
            CHECK(list_append_reveal_data(&target->trees, (pointer)&new_tree, (pointer*)&tree_ptr));
            target->roots[pos].forest = target;
            target->roots[pos].tree = tree_ptr;
            target->roots[pos].n = NULL;
            target->roots[pos].e = NULL;
            target->roots[pos].s = NULL;
            target->roots[pos].w = NULL;
            CHECK(pixel_image_create_roi(&target->roots[pos].ROI, target->original, block_ptr->x, block_ptr->y, block_ptr->w, block_ptr->h));
            CHECK(small_integral_image_create(&target->roots[pos].I, &target->roots[pos].ROI));
        }
    }

    target->last_base_block = target->blocks.last.prev;
    target->last_base_tree = target->trees.last.prev;

    /* add neighbors to roots */
    for (row = 0, pos = 0; row < target->rows; row++) {
        for (col = 0; col < target->cols; col++, pos++) {
            /* add neighbor to west */
            if (col > 0) {
                target->roots[pos].w = target->roots[pos - 1].tree;
            }
            /* add neighbor to north */
            if (row > 0) {
                target->roots[pos].n = target->roots[pos - target->cols].tree;
            }
            /* add neighbor to east */
            if (col < (unsigned)(target->cols - 1)) {
                target->roots[pos].e = target->roots[pos + 1].tree;
            }
            /* add neighbor to south */
            if (row < (unsigned)(target->rows - 1)) {
                target->roots[pos].s = target->roots[pos + target->cols].tree;
            }
        }
    }

    FINALLY(image_tree_forest_create);
    RETURN();
}

/******************************************************************************/

result image_tree_forest_reload(
    image_tree_forest *target,
    uint16 tree_width,
    uint16 tree_height
    )
{
    TRY();
    /*printf("w:%d,h:%d\n", tree_width, tree_height);*/
    pixel_image *img;
    uint32 own_original = 0;
    
    CHECK_POINTER(target);
    CHECK_POINTER(target->original);
    
    if (tree_width != target->tree_width || tree_height != target->tree_height) {
        img = target->original;
        if (target->own_original != 0) {
            own_original = 1;
            target->own_original = 0;
        }
        
        CHECK(image_tree_forest_destroy(target));
        CHECK(image_tree_forest_create(target, img, tree_width, tree_height));
        
        if (own_original != 0) {
            target->own_original = 1;
        }
    }
    FINALLY(image_tree_forest_reload);
    if (r != SUCCESS) {
        if (own_original != 0) {
            pixel_image_destroy(img);
            memory_deallocate((data_pointer*)img);
        }
    }
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

    size = target->rows * target->cols;
    for (pos = 0; pos < size; pos++) {
        CHECK(integral_image_destroy(&target->roots[pos].I));
        CHECK(pixel_image_destroy(&target->roots[pos].ROI));
    }
    CHECK(memory_deallocate((data_pointer*)&target->roots));

    if (target->own_original != 0) {
        CHECK(pixel_image_destroy(target->original));
        CHECK(memory_deallocate((data_pointer*)&target->original));
    }

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
    /*CHECK(edge_block_image_nullify(&target->edge_image));*/
    target->own_original = 0;
    target->rows = 0;
    target->cols = 0;
    target->tree_width = 0;
    target->tree_height = 0;
    target->dx = 0;
    target->dy = 0;
    target->type = b_NONE;
    target->last_base_tree = NULL;
    target->last_base_block = NULL;
    target->roots = NULL;

    FINALLY(image_tree_forest_nullify);
    RETURN();
}

/******************************************************************************/

/*
 * prepare stage is separated from update stage, to allow
 * easier parallelization of forest update; each image tree root
 * is completely separate from the rest, so they can be handled in parallel.
 */
result image_tree_forest_update_prepare(
    image_tree_forest *target
    )
{
    TRY();
    uint32 pos, size;
    image_tree *tree;

    CHECK_POINTER(target);

    CHECK(list_remove_rest(&target->blocks, target->last_base_block));
    CHECK(list_remove_rest(&target->trees, target->last_base_tree));

    /* must also set child nodes of root level trees to NULL... */
    /* TODO: find out a better way */
    size = target->rows * target->cols;
    for (pos = 0; pos < size; pos++) {
        tree = target->roots[pos].tree;
        tree->nw = NULL;
        tree->ne = NULL;
        tree->sw = NULL;
        tree->se = NULL;
    }

    FINALLY(image_tree_forest_update_prepare);
    RETURN();
}

/******************************************************************************/

result image_tree_forest_update(
    image_tree_forest *target
    )
{
    TRY();
    uint32 pos, size;

    CHECK(image_tree_forest_update_prepare(target));

    size = target->rows * target->cols;
    for (pos = 0; pos < size; pos++) {
        CHECK(image_tree_root_update(&target->roots[pos]));
    }

    FINALLY(image_tree_forest_update);
    RETURN();
}

/******************************************************************************/

result image_tree_forest_divide_with_dev(
    image_tree_forest *target,
    sint16 threshold
    )
{
    TRY();
    list_item *trees;
    image_tree *current_tree;

    CHECK_POINTER(target);
    CHECK_PARAM(threshold > 1);

    trees = target->trees.first.next;
    while (trees != &target->trees.last) {
        current_tree = (image_tree *)trees->data;
        if (current_tree->block->value.dev_i > threshold) {
            CHECK(image_tree_divide(current_tree));
        }
        trees = trees->next;
    }

    FINALLY(image_tree_forest_divide_with_dev);
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
        /*CHECK(convert_rgb24_to_grey8(rgb_image, grey_image));*/
    }
    else {
        /*printf("Error: image type not supported\n");*/
        ERROR(BAD_PARAM);
    }

    /*printf("Successfully read image of size (%lu x %lu)\n", width, height);*/

    CHECK(image_tree_forest_create(target, new_image, tree_width, tree_height));

    /*printf("Created image forest with trees (%u x %u)\n", tree_width, tree_height);*/
    /*CHECK(image_tree_forest_update(target));*/

    FINALLY(image_tree_forest_read);

    if (target->own_original != 0) {
        pixel_image_free(new_image);
    }
    /*
    if (r != SUCCESS) {
        pixel_image_destroy(grey_image);
        memory_deallocate((data_pointer*)&grey_image);
    }
    target->own_original = 1;
    */
    RETURN();
}

/******************************************************************************/

result image_tree_root_update(
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

result image_tree_update(
    image_tree *tree
    )
{
    TRY();
    real64 mean, dev;
    /*uint32 mean, dev;*/
    small_integral_image_box *box;
    image_block *block;
    /*struct timeval now;*/

    CHECK_POINTER(tree);

    block = tree->block;
    box = &tree->root->box;

    box->channel = 0;
    small_integral_image_box_update(box, block->x, block->y);
    mean = ((real64)box->sum / (real64)box->N);
    dev = (((real64)box->sumsqr / (real64)box->N) - (mean * mean));
    if (dev < 1) dev = 1;
    block->value.mean_i = (sint16)((mean < 0) ? 0 : ((mean > 255) ? 255 : mean));
    block->value.dev_i = (sint16)sqrt(dev);

    if (box->step == 3) {
        box->channel = 1;
        small_integral_image_box_update(box, block->x, block->y);
        mean = ((real64)box->sum / (real64)box->N);
        dev = (((real64)box->sumsqr / (real64)box->N) - (mean * mean));
        if (dev < 1) dev = 1;
        block->value.mean_c1 = (sint16)((mean < 0) ? 0 : ((mean > 255) ? 255 : mean));
        block->value.dev_c1 = (sint16)sqrt(dev);

        box->channel = 2;
        small_integral_image_box_update(box, block->x, block->y);
        mean = ((real64)box->sum / (real64)box->N);
        dev = (((real64)box->sumsqr / (real64)box->N) - (mean * mean));
        if (dev < 1) dev = 1;
        block->value.mean_c2 = (sint16)((mean < 0) ? 0 : ((mean > 255) ? 255 : mean));
        block->value.dev_c2 = (sint16)sqrt(dev);
    }

    /*gettimeofday(&now, NULL);*/
    /*printf("%d.%d update tree (%d,%d)=%d\n", now.tv_sec, now.tv_usec, block->x, block->y, block->value.mean);*/

    FINALLY(image_tree_update);
    RETURN();
}

/******************************************************************************/

/*
 * TODO: each image tree root should have its own sublist of blocks, for
 * allowing paraller division of each root.
 */

result image_tree_divide(
    image_tree *target
    )
{
    TRY();
    image_block new_block, *child_block;
    image_tree new_tree, *child_tree;

    CHECK_POINTER(target);
    if (target->nw == NULL && target->ne == NULL && target->sw == NULL && target->se == NULL) {
        if (target->block->w > 1 && target->block->h > 1) {
            new_tree.root = target->root;
            new_tree.parent = target;
            new_tree.nw = NULL;
            new_tree.ne = NULL;
            new_tree.sw = NULL;
            new_tree.se = NULL;
            new_block.w = (uint16)(target->block->w / 2);
            new_block.h = (uint16)(target->block->h / 2);

            small_integral_image_box_resize(&target->root->box, new_block.w, new_block.h);

            /* nw block */
            new_block.x = target->block->x;
            new_block.y = target->block->y;
            CHECK(list_append_reveal_data(&target->root->forest->blocks, (pointer)&new_block, (pointer*)&child_block));
            new_tree.block = child_block;
            CHECK(list_append_reveal_data(&target->root->forest->trees, (pointer)&new_tree, (pointer*)&child_tree));
            target->nw = child_tree;

            CHECK(image_tree_update(child_tree));

            /* ne block */
            new_block.x = (uint16)(target->block->x + new_block.w);
            CHECK(list_append_reveal_data(&target->root->forest->blocks, (pointer)&new_block, (pointer*)&child_block));
            new_tree.block = child_block;
            CHECK(list_append_reveal_data(&target->root->forest->trees, (pointer)&new_tree, (pointer*)&child_tree));
            target->ne = child_tree;

            CHECK(image_tree_update(child_tree));

            /* se block */
            new_block.y = (uint16)(target->block->y + new_block.h);
            CHECK(list_append_reveal_data(&target->root->forest->blocks, (pointer)&new_block, (pointer*)&child_block));
            new_tree.block = child_block;
            CHECK(list_append_reveal_data(&target->root->forest->trees, (pointer)&new_tree, (pointer*)&child_tree));
            target->se = child_tree;

            CHECK(image_tree_update(child_tree));

            /* sw block */
            new_block.x = target->block->x;
            CHECK(list_append_reveal_data(&target->root->forest->blocks, (pointer)&new_block, (pointer*)&child_block));
            new_tree.block = child_block;
            CHECK(list_append_reveal_data(&target->root->forest->trees, (pointer)&new_tree, (pointer*)&child_tree));
            target->sw = child_tree;

            CHECK(image_tree_update(child_tree));
        }
    }

    FINALLY(image_tree_divide);
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
    dir result;
    sint16 mean_nw, mean_ne, mean_sw, mean_se;

    result.h = 0;
    result.v = 0;

    if (tree == NULL) {
        return result;
    }

    if (tree->nw == NULL || tree->ne == NULL || tree->sw == NULL || tree->se == NULL) {
        return result;
    }

    mean_nw = tree->nw->block->value.mean_i;
    mean_ne = tree->ne->block->value.mean_i;
    mean_sw = tree->sw->block->value.mean_i;
    mean_se = tree->se->block->value.mean_i;

    result.v = (mean_nw + mean_sw) - (mean_ne + mean_se);
    result.h = (mean_nw + mean_ne) - (mean_sw + mean_se);

    return result;
}

dir image_tree_dir_c1(image_tree *tree)
{
    dir result;
    sint16 mean_nw, mean_ne, mean_sw, mean_se;

    result.h = 0;
    result.v = 0;

    if (tree == NULL) {
        return result;
    }

    if (tree->nw == NULL || tree->ne == NULL || tree->sw == NULL || tree->se == NULL) {
        return result;
    }

    mean_nw = tree->nw->block->value.mean_c1;
    mean_ne = tree->ne->block->value.mean_c1;
    mean_sw = tree->sw->block->value.mean_c1;
    mean_se = tree->se->block->value.mean_c1;

    result.v = (mean_nw + mean_sw) - (mean_ne + mean_se);
    result.h = (mean_nw + mean_ne) - (mean_sw + mean_se);

    return result;
}

dir image_tree_dir_c2(image_tree *tree)
{
    dir result;
    sint16 mean_nw, mean_ne, mean_sw, mean_se;

    result.h = 0;
    result.v = 0;

    if (tree == NULL) {
        return result;
    }

    if (tree->nw == NULL || tree->ne == NULL || tree->sw == NULL || tree->se == NULL) {
        return result;
    }

    mean_nw = tree->nw->block->value.mean_c2;
    mean_ne = tree->ne->block->value.mean_c2;
    mean_sw = tree->sw->block->value.mean_c2;
    mean_se = tree->se->block->value.mean_c2;

    result.v = (mean_nw + mean_sw) - (mean_ne + mean_se);
    result.h = (mean_nw + mean_ne) - (mean_sw + mean_se);

    return result;
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
    
    CHECK_POINTER(target);
    
    CHECK(list_create(target, 100, sizeof(image_tree*), 1));
    
    FINALLY(image_tree_create_neighbor_list);
    RETURN();
}

/******************************************************************************/

result image_tree_get_direct_neighbor(
    image_tree *tree,
    image_tree **neighbor,
    direction dir
    )
{
    TRY();
    
    switch (dir) {
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
result image_tree_find_all_immediate_neighbors(
    list *target,
    image_tree *tree
    )
{
    TRY();
    image_tree *new_neighbor;
    
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
