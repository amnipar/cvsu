/**
 * @file cvsu_simple_scene.h
 * @author Matti J. Eskelinen <matti.j.eskelinen@gmail.com>
 * @brief Simple scene geometry handling for the cvsu module.
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

#ifndef CVSU_SIMPLE_SCENE_H
#   define CVSU_SIMPLE_SCENE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "cvsu_basic.h"
#include "cvsu_edges.h"
#include "cvsu_list.h"

/**
 * Stores a boundary between image regions as a list of lines.
 */

typedef struct boundary_t {
    line *first;
    line *last;
    line best_fit;
    uint32 count;
    list lines;
} boundary;

/**
 * Stores an image block's relationship to its neighbors.
 */

typedef struct neighbor_relation_t {
    struct block_t *b;
    long strength;
    long pass_count;
} neighbor_relation;

/**
 * Stores an image block with links to its neighbors.
 */

typedef struct block_t {
    /** block position in grid (col, row) */
    point grid_pos;
    /** block position in pixels (top left corner) */
    point pixel_pos;
    /** width of block in pixels */
    uint16 width;
    /** height of block in pixels */
    uint16 height;
    /** block statistics (intensity mean and deviation) */
    stat_grey stat;
    /** count of horizontal lines (weighted with line strength?) */
    short hlines;
    /** count of vertical lines (weighted with line strength?) */
    short vlines;
    /** Pointer to the relevant section of the vertical edge image */
    char *vedges;
    /** Pointer to the relevant section of the horizontal edge image */
    char *hedges;
    /** Keeps track of processing passes this block has been subject to */
    long pass_count;
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
} block;

/**
 * Stores a simple scene, based on edge images.
 */

typedef struct simple_scene_t {
    edge_image curr_edges;
    edge_image prev_edges;
    uint32 rows;
    uint32 cols;
    uint32 hstep;
    uint32 vstep;
    uint32 hmargin;
    uint32 vmargin;
    uint32 width;
    uint32 height;
    list_item *mid_line;
    list_item *mid_boundary;
    list lines;
    list boundaries;
    list all_blocks;
    list blocks_by_deviation;
    block *blocks;
} simple_scene;

/**
 * Creates a simple scene based on a pixel image.
 * Only initializes the structure, does not update the scene.
 * The same image will be used for updating the scene in future.
 * @see simple_scene_update
 */

result simple_scene_create(
    simple_scene *target,
    pixel_image *source
);

/**
 * Destroys the scene and deallocates the memory.
 */

result simple_scene_destroy(
    simple_scene *target
);

result simple_scene_nullify(
    simple_scene *target
);

/**
 * Updates the scene data structures based on the data in the source image.
 */

result simple_scene_update(
    simple_scene *target
);

/**
 *
 */

result simple_scene_pack_lines_to_array(
    simple_scene *source,
    line *target,
    uint32 size,
    uint32 *count
);

#ifdef __cplusplus
}
#endif

#endif /* CVSU_SIMPLE_SCENE_H */
