/**
 * @file cvsu_simple_scene.c
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

#include "cvsu_config.h"
#include "cvsu_macros.h"
#include "cvsu_simple_scene.h"
#include "cvsu_memory.h"

#include <math.h>

/******************************************************************************/
/* constants for reporting function names in error messages                   */

string simple_scene_create_name = "simple_scene_create";
string simple_scene_destroy_name = "simple_scene_destroy";
string simple_scene_nullify_name = "simple_scene_nullify";
string simple_scene_update_name = "simple_scene_update";
string simple_scene_pack_lines_to_array_name = "simple_scene_pack_lines_to_array";

/******************************************************************************/

result simple_scene_create(
    simple_scene *target,
    pixel_image *source
    )
{
    TRY();
    uint16 row, col, pos;

    CHECK_POINTER(target);
    CHECK_POINTER(source);

    CHECK(edge_image_create(&target->curr_edges, source, 32, 32, 0, 0, 32, 32));
    /*CHECK(edge_image_create(&target->curr_edges, source, 16, 16, 16, 16, 16, 8));*/

    /*edge_image_clone(&target->prev_edges, &target->curr_edges);*/

    CHECK(list_create(&target->lines, 1000, sizeof(line), 10));
    CHECK(list_create(&target->boundaries, 100, sizeof(boundary), 10));

    target->rows = target->curr_edges.height;
    target->cols = target->curr_edges.width;
    target->hstep = target->curr_edges.hstep;
    target->vstep = target->curr_edges.vstep;
    target->hmargin = target->curr_edges.hmargin;
    target->vmargin = target->curr_edges.vmargin;
    target->width = target->curr_edges.vedges.width;
    target->height = target->curr_edges.hedges.height;

    CHECK(memory_allocate((data_pointer *)&target->blocks, target->rows * target->cols, sizeof(block)));
    CHECK(list_create_from_data(&target->all_blocks, (data_pointer)target->blocks, target->rows * target->cols, sizeof(block), 10));
    CHECK(sublist_create(&target->blocks_by_deviation, &target->all_blocks));

    pos = 0;
    for (row = 0; row < target->rows; row++) {
        for (col = 0; col < target->cols; col++) {
            target->blocks[pos].grid_pos.y = (sint16)row;
            target->blocks[pos].grid_pos.x = (sint16)col;
            target->blocks[pos].pixel_pos.x = (sint16)(col * target->hstep + target->hmargin);
            target->blocks[pos].pixel_pos.y = (sint16)(row * target->vstep + target->vmargin);
            target->blocks[pos].width = (uint16)target->hstep;
            target->blocks[pos].height = (uint16)target->vstep;
            target->blocks[pos].stat.mean = 0;
            target->blocks[pos].stat.dev = 0;

            target->blocks[pos].vedges = ((char *)target->curr_edges.vedges.data)
                    + row * target->width + target->hmargin + col * target->hstep;
            target->blocks[pos].hedges = ((char *)target->curr_edges.hedges.data)
                    + (row * target->vstep + target->vmargin) * target->cols + col;

            /* define neighbors above the block */
            if (row > 0) {
                if (col > 0) {
                    target->blocks[pos].neighbor_nw.b = &target->blocks[pos - target->cols - 1];
                }
                else {
                    target->blocks[pos].neighbor_nw.b = NULL;
                }
                target->blocks[pos].neighbor_n.b = &target->blocks[pos - target->cols];
                if (col < target->cols - 1) {
                    target->blocks[pos].neighbor_ne.b = &target->blocks[pos - target->cols + 1];
                }
                else {
                    target->blocks[pos].neighbor_ne.b = NULL;
                }
            }
            else {
                target->blocks[pos].neighbor_nw.b = NULL;
                target->blocks[pos].neighbor_n.b = NULL;
                target->blocks[pos].neighbor_ne.b = NULL;
            }

            /* define the neighbor on the right */
            if (col < target->cols - 1) {
                target->blocks[pos].neighbor_e.b = &target->blocks[pos + 1];
            }
            else {
                target->blocks[pos].neighbor_e.b = NULL;
            }

            /* define neighbors below the block */
            if (row < target->rows - 1) {
                if (col < target->cols - 1) {
                    target->blocks[pos].neighbor_se.b = &target->blocks[pos + target->cols + 1];
                }
                else {
                    target->blocks[pos].neighbor_se.b = NULL;
                }
                target->blocks[pos].neighbor_s.b = &target->blocks[pos + target->cols];
                if (col > 0) {
                    target->blocks[pos].neighbor_sw.b = &target->blocks[pos + target->cols - 1];
                }
                else {
                    target->blocks[pos].neighbor_sw.b = NULL;
                }
            }
            else {
                target->blocks[pos].neighbor_se.b = NULL;
                target->blocks[pos].neighbor_s.b = NULL;
                target->blocks[pos].neighbor_sw.b = NULL;
            }

            /* define the neighbor on the left */
            if (col > 0) {
                target->blocks[pos].neighbor_w.b = &target->blocks[pos - 1];
            }
            else {
                target->blocks[pos].neighbor_w.b = NULL;
            }

            list_append_index(&target->all_blocks, pos);

            pos++;
        }
    }

    FINALLY(simple_scene_create);
    RETURN();
}

/******************************************************************************/

result simple_scene_destroy(
    simple_scene *target
    )
{
    TRY();

    CHECK_POINTER(target);

    /*CHECK(edge_image_destroy(&dst->prev_edges));*/
    CHECK(edge_image_destroy(&target->curr_edges));
    CHECK(list_destroy(&target->boundaries));
    CHECK(list_destroy(&target->lines));

    /* list takes ownership of the block array, don't deallocate that */
    CHECK(list_destroy(&target->all_blocks));

    CHECK(simple_scene_nullify(target));

    FINALLY(simple_scene_destroy);
    RETURN();
}

/******************************************************************************/

result simple_scene_nullify(
    simple_scene *target
    )
{
    TRY();

    CHECK_POINTER(target);

    target->rows = 0;
    target->cols = 0;
    target->hstep = 0;
    target->vstep = 0;
    target->hmargin = 0;
    target->vmargin = 0;
    target->width = 0;
    target->height = 0;
    target->mid_line = NULL;
    target->mid_boundary = NULL;
    target->blocks = NULL;

    FINALLY(simple_scene_nullify);
    RETURN();
}

/******************************************************************************/
/* private function for handling block comparison in list iteration.          */

int compare_blocks_by_deviation(const void *a, const void *b)
{
    return ((const block *)a)->stat.dev - ((const block *)b)->stat.dev;
}

/******************************************************************************/

result simple_scene_update(
    simple_scene *target
    )
{
    TRY();
    char *edge_data;
    integral_image_box box;
    sint32 i, match, delta;
    uint32 x, y, row, col, pos;
    uint32 width, height, rows, cols, margin, row_height, col_width, dx, dy;
    double mean, dev;

    CHECK_POINTER(target);

    CHECK(edge_image_update(&target->curr_edges));

    /*for (i = 0; i < 100; i++) {
        CHECK(integral_image_update(&target->curr_edges.I));
    }*/

    integral_image_box_create(&box, &target->curr_edges.I, target->vstep, target->hstep, 0, 0);

    CHECK(list_clear(&target->blocks_by_deviation));
    pos = 0;
    for (row = 0; row < target->rows; row++) {
        x = target->hmargin;
        y = row * target->vstep + target->vmargin;

        for (col = 0; col < target->cols; col++, pos++, /*iA*/x += target->hstep /*,i2A += target->vstep*/) {
            integral_image_box_update(&box, x, y);

            mean = ((double)box.sum / (double)box.N);
            dev = (double)((box.sumsqr / (double)box.N) - (mean * mean));
            if (dev < 1) dev = 1;

            target->blocks[pos].stat.mean = (sint16)((mean < 0) ? 0 : ((mean > 255) ? 255 : mean));
            target->blocks[pos].stat.dev = (sint16)sqrt(dev);

            CHECK(list_insert_sorted_index(&target->blocks_by_deviation, (unsigned)pos, &compare_blocks_by_deviation));
        }
    }

    CHECK(list_clear(&target->lines));
    {
        line new_line;
        sint16 value1, value2, diff1, diff2;

        pos = 0;
        for (row = 0; row < target->rows; row++) {
            for (col = 0; col < target->cols; col++) {
                target->blocks[pos].pass_count = 0;
                pos++;
            }
        }
/*
        pos = 0;
        for (row = 0; row < target->rows; row++) {
            for (col = 0; col < target->cols; col++) {
                if (dst->blocks[pos].check_count < 1) {
                    for (i = 0; i < dst->hstep; i++) {
                        if (dst->blocks[pos].vedges[i] != 0) {

                        }
                    }
                    dst->blocks[pos].check_count++;
                }
                pos++;
            }
        }
*/
        edge_data = (char *)target->curr_edges.vedges.data;
        /* always investigate the row below if edge is found, so skip last row */
        rows = target->curr_edges.vedges.height - 1;
        /* look for edges only within the margin */
        width = target->curr_edges.vedges.width;
        margin = target->curr_edges.hmargin;
        cols = width - (2 * margin);
        row_height = target->curr_edges.vstep;
        delta = (sint32)(target->vstep / 2);
        dy = target->curr_edges.vmargin + (uint32)(row_height / 2);
        for (row = 0; row < rows; row++) {
            pos = row * width + margin;
            for (col = margin; col < cols; col++) {
                match = delta + 1;
                value1 = edge_data[pos];
                if (value1 != 0) {
                    /* check points with allowed distance from found point */
                    for (i = -delta; i <= delta; i++) {
                        value2 = edge_data[(signed)(pos + width) + i];
                        /* check for same sign of edges */
                        if ((value1 > 0 && value2 > 0) || (value1 < 0 && value2 < 0)) {
                            /* use absolute value of diff */
                            diff1 = (sint16)(value2 - value1);
                            if (diff1 < 0) diff1 = (sint16)(-diff1);
                            /* if no match yet, check that diff is smaller than 4 */
                            if (match > delta) {
                                if (diff1 < 5) {
                                    match = i;
                                    diff2 = diff1;
                                }
                            }
                            /* if previous match is found, check for smaller diff */
                            else {
                                if (diff1 <= diff2 && (match * match) > (i * i)) {
                                    match = i;
                                    diff2 = diff1;
                                }
                            }
                        }
                    }
                    if (match <= delta) {
                        new_line.start.x = (uint16)col;
                        new_line.start.y = (uint16)(dy + row * row_height);
                        new_line.end.x = (uint16)((signed)col + match);
                        new_line.end.y = (uint16)(new_line.start.y + row_height);
                        CHECK(list_append(&target->lines, &new_line));
                    }
                }
                pos++;
            }
        }
    }
    /* mid_line will point to the last vertical line */
    target->mid_line = target->lines.last.prev;
    {
        line new_line;
        sint16 value1, value2, diff1, diff2;

        edge_data = (char *)target->curr_edges.hedges.data;
        /* always investigate the col to the right if edge is found, so skip last row */
        cols = target->curr_edges.hedges.width - 1;
        /* look for edges only within the margin */
        width = target->curr_edges.hedges.width;
        height = target->curr_edges.hedges.height;
        margin = target->curr_edges.vmargin;
        rows = height - 2 * margin;
        col_width = target->curr_edges.hstep;
        delta = (long)(target->hstep / 2);
        dx = target->curr_edges.hmargin + (uint32)(col_width / 2);
        for (col = 0; col < cols; col++) {
            pos = margin * width + col;
            for (row = margin; row < rows; row++) {
                value1 = edge_data[pos];
                if (value1 != 0) {
                    match = delta + 1;
                    /* check points with allowed distance from found point */
                    for (i = -delta; i <= delta; i++) {
                        value2 = edge_data[(signed)(pos + 1) + i * (signed)width];
                        /* check for same sign of edges */
                        if ((value1 > 0 && value2 > 0) || (value1 < 0 && value2 < 0)) {
                            /* use absolute value of diff */
                            diff1 = (sint16)(value2 - value1);
                            if (diff1 < 0) diff1 = (sint16)(-diff1);
                            /* if no match yet, check that diff is smaller than 3 */
                            if (match > delta) {
                                if (diff1 < 5) {
                                    match = i;
                                    diff2 = diff1;
                                }
                            }
                            /* if previous match is found, check for smaller diff */
                            else {
                                if (diff1 <= diff2 && (match * match) > (i * i)) {
                                    match = i;
                                    diff2 = diff1;
                                }
                            }
                        }
                    }
                    if (match <= delta) {
                        new_line.start.x = (uint16)(dx + col * col_width);
                        new_line.start.y = (uint16)(row);
                        new_line.end.x = (uint16)(new_line.start.x + col_width);
                        new_line.end.y = (uint16)((signed)row + match);
                        CHECK(list_append(&target->lines, &new_line));
                    }
                }
                pos += width;
            }
        }
    }

    CHECK(list_clear(&target->boundaries));
    {

        line *l;
        boundary new_boundary, *b;
        list_item *lines, *boundaries, *temp;

        lines = &target->lines.first;
        lines = lines->next;
        while (lines != &target->lines.last && lines != NULL) {
            l = (line *)lines->data;
            boundaries = &target->boundaries.first;
            boundaries = boundaries->next;
            b = NULL;
            while (boundaries != &target->boundaries.last && boundaries != NULL) {
                b = (boundary *)boundaries->data;
                if (b->last->end.x == l->start.x && b->last->end.y == l->start.y) {
                    CHECK(list_append(&b->lines, lines->data));
                    b->last = l;
                    b->best_fit.end = l->end;
                    b->count++;
                    break;
                }
                b = NULL;
                boundaries = boundaries->next;
            };
            if (b == NULL) {
                new_boundary.first = l;
                new_boundary.last = l;
                new_boundary.best_fit.start = l->start;
                new_boundary.best_fit.end = l->end;
                new_boundary.count = 1;
                CHECK(sublist_create(&new_boundary.lines, &target->lines));
                CHECK(list_append(&new_boundary.lines, lines->data));
                CHECK(list_append(&target->boundaries, &new_boundary));
            }
            lines = lines->next;
        };

        boundaries = &target->boundaries.first;
        boundaries = boundaries->next;
        while (boundaries != &target->boundaries.last && boundaries != NULL) {
            b = (boundary *)boundaries->data;
            if (b->count < 3) {
                temp = boundaries->next;
                CHECK(list_remove_item(&target->boundaries, boundaries));
                boundaries = temp;
            }
            else {
                boundaries = boundaries->next;
            }
        };
    }

    /*CHECK(edge_image_copy(&dst->prev_edges, &dst->curr_edges));*/

    FINALLY(simple_scene_update);
    RETURN();
}

/******************************************************************************/

result simple_scene_pack_lines_to_array(
    simple_scene *source,
    line *target,
    uint32 size,
    uint32 *count
    )
{
    TRY();
    list_item *boundaries;
    boundary *b;
    uint32 i;

    CHECK_POINTER(source);
    CHECK_POINTER(target);

    i = 0;
    boundaries = &source->boundaries.first;
    boundaries = boundaries->next;
    while (boundaries != &source->boundaries.last) {
        if (boundaries == NULL) {
            ERROR(NOT_FOUND);
        }
        if (i < size) {
            b = (boundary *)boundaries->data;
            target[i] = b->best_fit;
        }
        else {
            ERROR(BAD_SIZE);
        }
        i++;
        boundaries = boundaries->next;
    };

    FINALLY(simple_scene_pack_lines_to_array);
    *count = i;
    RETURN();
}

/******************************************************************************/
