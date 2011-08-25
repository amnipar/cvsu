/**
 * @file cv_scene.c
 * @author Matti Eskelinen (matti dot j dot eskelinen at jyu dot fi)
 * @brief Scene geometry handling for the cv module.
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

#include "cv_scene.h"

/**
 * Initialize a scene object by allocating the necessary contained objects.
 * The starting point is an image, which is used to create an edge image.
 * A brand new image is allocated to store the previous frame.
 */

result create_scene(scene *dst, pixel_image *src)
{
    result r;

    if (dst == NULL) {
        return BAD_POINTER;
    }
    r = create_edge_image(&dst->current_edges, src, 16, 16, 16, 16, 16, 8);
    if (r != SUCCESS) {
        return r;
    }
    r = clone_edge_image(&dst->previous_edges, &dst->current_edges);
    if (r != SUCCESS) {
        return r;
    }
    r = list_allocate(&dst->all_lines, 10000, sizeof(line));
    if (r != SUCCESS) {
        return r;
    }
    r = list_allocate(&dst->boundaries, 1000, sizeof(boundary));
    if (r != SUCCESS) {
        return r;
    }

    return SUCCESS;
}

result destroy_scene(scene *dst)
{
    result r;

    if (dst == NULL) {
        return BAD_POINTER;
    }
    r = destroy_edge_image(&dst->previous_edges);
    if (r != SUCCESS) {
        return r;
    }
    r = destroy_edge_image(&dst->current_edges);
    if (r != SUCCESS) {
        return r;
    }
    r = list_destroy(&dst->boundaries);
    if (r != SUCCESS) {
        return r;
    }
    r = list_destroy(&dst->all_lines);
    if (r != SUCCESS) {
        return r;
    }
    return SUCCESS;
}

result update_scene(scene *dst)
{
    result r;
    long i, width, height, margin, pos, row, rows, col, cols, dx, dy, row_height, col_width;
    char *edge_data;
    line new_line, *l;
    boundary new_boundary, *b;
    list_item *lines, *boundaries, *temp;

    if (dst == NULL) {
        return BAD_POINTER;
    }
    r = calculate_edges(&dst->current_edges);
    if (r != SUCCESS) {
        return r;
    }
    list_clear(&dst->all_lines);
    {
        edge_data = (char *)(*dst).current_edges.vedges.data;
        // always investigate the row below if edge is found, so skip last row
        rows = (*dst).current_edges.vedges.height - 1;
        // look for edges only within the margin
        width = (*dst).current_edges.vedges.width;
        margin = (*dst).current_edges.hmargin;
        cols = width - 2 * margin;
        row_height = (*dst).current_edges.vstep;
        dy = (*dst).current_edges.vmargin + (long)(row_height / 2);
        for (row = 0; row < rows; row++) {
            pos = row * width + margin;
            for (col = margin; col < cols; col++) {
                if (edge_data[pos] != 0) {
                    for (i = -3; i <= 3; i++) {
                        if (edge_data[pos + width + i] != 0) {
                            new_line.start.x = col;
                            new_line.start.y = dy + row * row_height;
                            new_line.end.x = col + i;
                            new_line.end.y = new_line.start.y + row_height;
                            list_append(&dst->all_lines, &new_line);
                        }
                    }
                }
                pos++;
            }
        }
    }
    /* mid_line will point to the last vertical line */
    dst->mid_line = dst->all_lines.last->prev;
    {
        edge_data = (char *)(*dst).current_edges.hedges.data;
        // always investigate the col to the right if edge is found, so skip last row
        cols = (*dst).current_edges.hedges.width - 1;
        // look for edges only within the margin
        width = (*dst).current_edges.hedges.width;
        height = (*dst).current_edges.hedges.height;
        margin = (*dst).current_edges.vmargin;
        rows = height - 2 * margin;
        col_width = (*dst).current_edges.hstep;
        dx = (*dst).current_edges.hmargin + (long)(col_width / 2);
        for (col = 0; col < cols; col++) {
            pos = margin * width + col;
            for (row = margin; row < rows; row++) {
                if (edge_data[pos] != 0) {
                    for (i = -3; i <= 3; i++) {
                        if (edge_data[pos + 1 + i * width] != 0) {
                            new_line.start.x = dx + col * col_width;
                            new_line.start.y = row;
                            new_line.end.x = new_line.start.x + col_width;
                            new_line.end.y = row + i;
                            list_append(&dst->all_lines, &new_line);
                        }
                    }
                }
                pos += width;
            }
        }
    }

    list_clear(&dst->boundaries);
    lines = (*dst).all_lines.first;
    lines = lines->next;
    while (lines != (*dst).all_lines.last && lines != NULL) {
        l = (line *)lines->data;
        boundaries = (*dst).boundaries.first;
        boundaries = boundaries->next;
        b = NULL;
        while (boundaries != (*dst).boundaries.last && boundaries != NULL) {
            b = (boundary *)boundaries->data;
            if (b->last->end.x == l->start.x && b->last->end.y == l->start.y) {
                sublist_append(&b->lines, lines);
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
            sublist_create(&new_boundary.lines, &dst->all_lines);
            sublist_append(&new_boundary.lines, lines);
            list_append(&dst->boundaries, &new_boundary);
        }
        lines = lines->next;
    };

    boundaries = (*dst).boundaries.first;
    boundaries = boundaries->next;
    while (boundaries != (*dst).boundaries.last && boundaries != NULL) {
        b = (boundary *)boundaries->data;
        if (b->count < 3) {
            temp = boundaries->next;
            list_remove_item(&dst->boundaries, boundaries);
            boundaries = temp;
        }
        else {
            boundaries = boundaries->next;
        }
    };

    r = copy_edge_image(&dst->previous_edges, &dst->current_edges);
    if (r != SUCCESS) {
        return r;
    }
    return SUCCESS;
}
