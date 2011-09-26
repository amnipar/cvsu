/**
 * @file cvsu_scene.h
 * @author Matti Eskelinen (matti dot j dot eskelinen at jyu dot fi)
 * @brief Scene geometry handling for the cvsu module.
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

#ifndef CVSU_SCENE_H
#   define CVSU_SCENE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "cvsu_basic.h"
#include "cvsu_edges.h"
#include "cvsu_list.h"

typedef struct boundary_t {
    line *first;
    line *last;
    line best_fit;
    long count;
    list lines;
} boundary;

typedef struct neighbor_relation_t {
    struct block_t *b;
    long strength;
    long pass_count;
} neighbor_relation;

typedef struct block_t {
    /** row of block position in grid */
    short row;
    /** column of block position in grid */
    short col;
    /** y cordinate of block position (top left corner) */
    short pos_x;
    /** x coordinate of block position (top left corner) */
    short pos_y;
    /** width of block in pixels */
    short width;
    /** height of block in pixels */
    short height;
    /** mean of intensity values */
    short mean;
    /** deviation of intensity values */
    short dev;
    double fdev;
    /** count of horizontal lines (weighted with line strength?) */
    short hlines;
    /** strength of vertical motion estimation */
    short vweight;
    /** estimated vertical motion */
    short vmotion;
    /** count of vertical lines (weighted with line strength?) */
    short vlines;
    /** Strength of horizontal motion estimation */
    short hweight;
    /** Estimated horizontal motion. */
    short hmotion;
    /** total strength / salience of block */
    short strength;
    /** horizontal strength for reliability of horizontal motion estimation */
    short hstrength;
    /** vertical strength for reliability of vertical motion estimation */
    short vstrength;
    /** Number of times this block has been checked in this iteration */
    short check_count;
    /** Pointer to the relevant section of the vertical edge image */
    char *vedges;
    /** Pointer to the relevant section of the horizontal edge image */
    char *hedges;
    /** Keeps track of processing passes this block has been subject to */
    long pass_count;

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
} block;

typedef struct scene_t {
    edge_image previous_edges;
    edge_image current_edges;
    list_item *mid_line;
    list_item *mid_boundary;
    long rows;
    long cols;
    long hstep;
    long vstep;
    long hmargin;
    long vmargin;
    long width;
    long height;
    list all_lines;
    list boundaries;
    list all_blocks;
    list blocks_by_deviation;
    block *blocks;
} scene;

typedef struct simple_scene_t {

} simple_scene;

typedef struct hierarchical_scene_t {
    integral_image Int;
    list blocks_1;
    list blocks_2;
    list blocks_by_deviation;
    pointer_list vedges_1;
    pointer_list vedges_2;
    pointer_list hedges_1;
    pointer_list hedges_2;
    list lines_1;
    list lines_2;
    list boundaries_1;
    list boundaries_2;

    long hstep;
    long vstep;
    long hmargin;
    long vmargin;
    long box_width;
    long box_length;
    long rows;
    long cols;
    long width;
    long height;
    long dx;
    long dy;

    block *block_array_1;
    block *block_array_2;

    block *previous_block_array;
    block *current_block_array;
    list *previous_blocks;
    list *current_blocks;
    pointer_list *previous_vedges;
    pointer_list *current_vedges;
    pointer_list *previous_hedges;
    pointer_list *current_hedges;
    list *previous_lines;
    list *current_lines;
    list *previous_boundaries;
    list *current_boundaries;
} hierarchical_scene;

/*void trace_hline(block *b, long i); */

result create_scene(scene *dst, pixel_image *src);
result destroy_scene(scene *dst);
result update_scene(scene *dst);

result create_hierarchical_scene(hierarchical_scene *dst, pixel_image *src);
result destroy_hierarchical_scene(hierarchical_scene *dst);
result update_hierarchical_scene(hierarchical_scene *dst);

#ifdef __cplusplus
}
#endif

#endif /* CVSU_SCENE_H */
