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
 * Stores information about a tree neighbor.
 * Possible purposes:
 * Is the neighbor on same level? (alternative: store level in each tree)
 * Have I checked this neighbor already?
 * What is my relation to this neighbor? What is the strength of our relation?
 * Downside of using a struct is that it uses more memory even if no neighbor.
 * It is a possibility to re-calculate the strength each time it is needed, to
 * allow storing only a pointer to neighbor in the tree.
 * It is also possible to create a 'relation pointer' type that can store
 * information about a relation to another tree, in addition to pointer...
 * These relations would have to be stored in a list within the forest.
 */
/*
typedef struct image_tree_neighbor_t {
    struct image_tree_t *tree;
    real32 strength;
} image_tree_neighbor;
*/

/**
 * Stores a quad tree holding image data.
 */

typedef struct image_tree_t {
    struct image_tree_root_t *root;
    struct image_tree_t *parent;
    struct image_tree_t *class_id;
    
    /* subtrees, NULL if the tree has not beed divided */
    struct image_tree_t *nw;
    struct image_tree_t *ne;
    struct image_tree_t *sw;
    struct image_tree_t *se;

    image_block *block;

    /* cache direct neighbors in each tree as they are determined */
    struct image_tree_t *n;
    struct image_tree_t *e;
    struct image_tree_t *s;
    struct image_tree_t *w;

    /*edge_block *edge;*/
    uint32 level;
    uint32 class_rank;
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
} image_tree_root;

/**
 * Stores a forest of image trees.
 */

typedef struct image_tree_forest_t {
    pixel_image *original;
    pixel_image *source;
    /*edge_block_image edge_image;*/
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
    list values;
    /*list edges;*/
    list_item *last_base_tree;
    list_item *last_base_block;
    list_item *last_base_value;
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
 * Creates an image forest from a pixel image.
 */

result image_tree_forest_create(
    image_tree_forest *target,
    pixel_image *source,
    uint16 tree_width,
    uint16 tree_height,
    image_block_type type
);

/**
 * Reloads the forest using the same image, but possibly different box size.
 * Also used in create function, to create the structures the first time.
 */

result image_tree_forest_reload(
    image_tree_forest *target,
    uint16 tree_width,
    uint16 tree_height,
    image_block_type type
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
* Everything that can be nullified should be able to tell if it's null
*/

bool image_tree_forest_is_null(
  image_tree_forest *target
);

/**
 * Prepares an image forest for the update stage.
 * This is separate from the actual update to allow easier parallelization.
 * All root trees are separate from each other, so arbitrary tree collections
 * can be handled in parallel.
 * In this stage new image content is copied from the source
 * (possibly converting the format) and data structures cleaned for a new
 * iteration.
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

/**
 * Creates and initializes a neighbor list
 */
result image_tree_create_neighbor_list(
    list *target
);

/**
 * Finds the direct neighbor (directly adjacent on the same level) of a tree
 * and returns it in the neighbor reference. If the tree does not have a
 * neighbor on the same level, the direct neighbor on the highest level is
 * returned.
 *
 * 1. Check if the direct neighbor has already been cached.
 * 2. If not, go to parent and check if the neighbor is in the same parent.
 * 3. If not, recursively get the neighbor of the parent.
 * 4. Check if the neighbor of the parent has children; if yes, choose the
 *    correct one as neighbor, if not, return the parent as neighbor
 * 5. It is possible that there is no neighbor (tree is on the edge); in this
 *    case, the neighbor is set to NULL
 *
 * This requires that the neighbors of top level trees have been set properly.
 */

result image_tree_get_direct_neighbor(
    image_tree *tree,
    image_tree **neighbor,
    direction dir
);

result image_tree_get_direct_neighbor_n(
    image_tree *tree,
    image_tree **neighbor
);

result image_tree_get_direct_neighbor_e(
    image_tree *tree,
    image_tree **neighbor
);

result image_tree_get_direct_neighbor_s(
    image_tree *tree,
    image_tree **neighbor
);

result image_tree_get_direct_neighbor_w(
    image_tree *tree,
    image_tree **neighbor
);

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
);

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
);

dir image_tree_dir_i(image_tree *tree);
dir image_tree_dir_c1(image_tree *tree);
dir image_tree_dir_c2(image_tree *tree);

/**
 * Creates an equivalence class for this tree.
 * Part of the Union-Find implementation for image trees.
 */
void image_tree_class_create(image_tree *tree);

/**
 * Creates a union of the two classes these two trees belong to.
 * Part of the Union-Find implementation for image trees.
 */
void image_tree_class_union(image_tree *tree1, image_tree *tree2);

/**
 * Finds the representative item in the class this tree belongs to.
 * Part of the Union-Find implementation for image trees.
 */
image_tree *image_tree_class_find(image_tree *tree);

/**
 * Gets the class label for this tree. Effectively the pointer cast into int.
 * Helper function on top of the Union-Find implementation for image trees.
 */
uint32 image_tree_class_get(image_tree *tree);

#ifdef __cplusplus
}
#endif

#endif /* IMAGE_TREE_H */
