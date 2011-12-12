/**
 * @file cvsu_block.h
 * @author Matti J. Eskelinen <matti.j.eskelinen@gmail.com>
 * @brief Image block handling for the cvsu module.
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

#ifndef CVSU_BLOCK_H
#   define CVSU_BLOCK_H

#ifdef __cplusplus
extern "C" {
#endif

#include "cvsu_basic.h"
#include "cvsu_integral.h"
#include "cvsu_list.h"

#define dev_threshold 10
#define dev_diff 10

#define region_undefined    0
#define region_low_dev      1
#define region_uniform_dev  2

typedef struct boundary_t {
    line *first;
    line *last;
    line best_fit;
    long count;
    list lines;
} boundary;

typedef struct neighbor_relation_t {
    struct grid_item_t *item;
    long strength;
    long pass_count;
} neighbor_relation;

typedef struct block_t {
    point pos;
    uint16 width;
    uint16 height;
    statistics stat;
    struct block_t *nw;
    struct block_t *ne;
    struct block_t *sw;
    struct block_t *se;
    uint16 pass_count;
    struct region_t *region_ptr;
    //uint16 region_id;
} block;

typedef struct grid_item_t {
    /** position in grid (col, row) */
    point pos;
    /** main level block item */
    block *main_block;
    /** count of horizontal lines (weighted with line strength?) */
    uint16 hlines;
    /** strength of vertical motion estimation */
    uint16 vweight;
    /** estimated vertical motion */
    sint16 vmotion;
    /** count of vertical lines (weighted with line strength?) */
    uint16 vlines;
    /** Strength of horizontal motion estimation */
    uint16 hweight;
    /** Estimated horizontal motion. */
    sint16 hmotion;
    /** Pointer to the relevant section of the vertical edge image */
    char *vedges;
    /** Pointer to the relevant section of the horizontal edge image */
    char *hedges;
    /** Keeps track of processing passes this block has been subject to */
    uint16 pass_count;
    /** total strength / salience of block */
    uint16 strength;
    /** horizontal strength for reliability of horizontal motion estimation */
    uint16 hstrength;
    /** vertical strength for reliability of vertical motion estimation */
    uint16 vstrength;

    pointer_list vedge_list;
    pointer_list hedge_list;

    /** Neighboring block in the north-west direction */
    struct neighbor_relation_t neighbor_nw;
    /** Neighboring block in the north direction */
    struct neighbor_relation_t neighbor_n;
    /** Neighboring block in the north-east direction */
    struct neighbor_relation_t neighbor_ne;
    /** Neighboring block in the east direction */
    struct neighbor_relation_t neighbor_e;
    /** Neighboring block in the south-east direction */
    struct neighbor_relation_t neighbor_se;
    /** Neighboring block in the south direction */
    struct neighbor_relation_t neighbor_s;
    /** Neighboring block in the south-west direction */
    struct neighbor_relation_t neighbor_sw;
    /** Neighboring block in the west direction */
    struct neighbor_relation_t neighbor_w;
} grid_item;

typedef struct region_t {
    uint16 id;
    uint16 type;
    float block_count;
    float mean;
    float dev;
    statistics stat;
    rect bounding_box;
    list borders;
    list blocks;
    //list lines;
    list points;
} region;

typedef struct region_border_item_t {
    region *region_ptr;
    grid_item *grid_ptr;
    /* direction from the item that added this item to border */
    /* in which direction this item is the neighbor of the other item... */
    direction dir;
} region_border_item;

int compare_blocks_by_deviation(const void *a, const void *b);

result block_update(block *target, integral_image *I);

result block_divide(block *target, integral_image *I, list *block_list);

/**
 * Initialize the region structure and the lists.
 * Add the given item to the beginning of the border item list.
 * Item's block is not added to the block list yet, and this would complicate
 * the region expansion phase. Instead, the first item is handled in the
 * @see region_expand function.
 */
result region_init(
    region *target,
    grid_item *item,
    list *border_list,
    list *point_list,
    list *block_list
);

/**
 * Expand the region from the given border item.
 * If the item is suitable for adding to the region, its main block is added to
 * the block list, and its neighbors are added to the border item list.
 * Boundary points are added only when the border item chain is ready.
 */
result region_expand(
    region_border_item *item
);

#ifdef __cplusplus
}
#endif

#endif /* CVSU_BLOCK_H */
