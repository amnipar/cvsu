/**
 * @file cvsu_image_tree.h
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

#ifndef IMAGE_TREE_H
#   define IMAGE_TREE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "cvsu_basic.h"
#include "cvsu_integral.h"
#include "cvsu_edges.h"
#include "cvsu_list.h"

/**
 * Stores a quad tree holding image data.
 */

typedef struct image_tree_t {
    struct image_tree_root_t *root;
    struct image_tree_t *parent;
    struct image_tree_t *nw;
    struct image_tree_t *ne;
    struct image_tree_t *sw;
    struct image_tree_t *se;
    image_block *block;
    /*edge_block *edge;*/
} image_tree;

/**
 * Stores a root of a tree contained within a forest.
 */

typedef struct image_tree_root_t {
    pixel_image ROI;
    integral_image I;
    small_integral_image_box box;
    struct image_tree_forest_t *forest;
    struct image_tree_t *tree;
    struct image_tree_t *n;
    struct image_tree_t *e;
    struct image_tree_t *s;
    struct image_tree_t *w;
} image_tree_root;

/**
 * Stores a forest of image trees.
 */

typedef struct image_tree_forest_t {
    pixel_image *original;
    /*pixel_image *i;*/
    /*pixel_image *c1;*/
    /*pixel_image *c2;*/
    /*edge_block_image edge_image;*/
    uint32 own_original;
    uint32 rows;
    uint32 cols;
    uint32 tree_width;
    uint32 tree_height;
    uint32 dx;
    uint32 dy;
    image_block_type type;
    /* also add max levels by looking at how many times tree can be divided? */
    list trees;
    list blocks;
    /*list edges;*/
    list_item *last_base_tree;
    list_item *last_base_block;
    image_tree_root *roots;
} image_tree_forest;

/**
 * Allocates an image forest structure.
 */

image_tree_forest *image_tree_forest_alloc();

/**
 * Frees an image forest structure allocated with @see image_tree_forest_alloc
 */

void image_tree_forest_free(image_tree_forest *ptr);

/**
 * Creates an image forest from and image.
 */

result image_tree_forest_create(
    image_tree_forest *target,
    pixel_image *source,
    uint16 tree_width,
    uint16 tree_height
);

/**
 * Destroys an image forest and deallocates all data.
 */

result image_tree_forest_destroy(
    image_tree_forest *target
);

/**
 * Sets image forest to null. Does not deallocate data.
 */

result image_tree_forest_nullify(
    image_tree_forest *target
);

/**
 * Prepares an image forest for the update stage.
 */

result image_tree_forest_update_prepare(
    image_tree_forest *target
);

/**
 * Updates an image forest
 */

result image_tree_forest_update(
    image_tree_forest *target
);

/**
 * Divides all trees in the forest that have dev smaller than threshold.
 */

result image_tree_forest_divide_with_dev(
    image_tree_forest *target,
    sint16 threshold
);

/**
 * Reads an image from file and creates an image forest from it.
 * The resulting image will be owned by the forest and deallocated.
 */

result image_tree_forest_read(
    image_tree_forest *target,
    string source,
    uint16 tree_width,
    uint16 tree_height
);

/**
 * Updates an image tree root.
 */

result image_tree_root_update(
    image_tree_root *target
);

/**
 * Updates an image tree.
 */

result image_tree_update(
    image_tree *tree
);

/**
 * Divides an image tree in four smaller trees, if width is greater than 1.
 */

result image_tree_divide(
    image_tree *target
);

dir image_tree_dir_i(image_tree *tree);
dir image_tree_dir_c1(image_tree *tree);
dir image_tree_dir_c2(image_tree *tree);

#ifdef __cplusplus
}
#endif

#endif /* IMAGE_TREE_H */
