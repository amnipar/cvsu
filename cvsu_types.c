/**
 * @file cvsu_types.c
 * @author Matti J. Eskelinen <matti.j.eskelinen@gmail.com>
 * @brief Generic type definitions for the cvsu module.
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

#include "cvsu_types.h"

void statistics_init(statistics *stat)
{
  stat->mean = 0;
  stat->variance = 0;
#ifdef INTEGRAL_IMAGE_HIGHER_ORDER_STATISTICS
  stat->skewness = 0;
  stat->kurtosis = 0;
#endif
}

void point_create(point *target, coord x, coord y)
{
    if (target != NULL) {
        target->x = x;
        target->y = y;
    }
}

void point_add(point *target, coord x, coord y)
{
    if (target != NULL) {
        target->x += x;
        target->y += y;
    }
}

void point_subtract(point *target, coord x, coord y)
{
    if (target != NULL) {
        target->x -= x;
        target->y -= y;
    }
}

void line_create(line *target, coord start_x, coord start_y, coord end_x, coord end_y)
{
    if (target != NULL) {
        target->start.x = start_x;
        target->start.y = start_y;
        target->end.x = end_x;
        target->end.y = end_y;
    }
}

void line_create_from_points(line *target, point start, point end)
{
    if (target != NULL) {
        target->start = start;
        target->end = end;
    }
}

void rect_create(rect *target, coord left, coord right, coord top, coord bottom)
{
    if (target != NULL) {
        if (left < right) {
            target->left = left;
            target->right = right;
        }
        else {
            target->left = right;
            target->right = left;
        }
        if (top < bottom) {
            target->top = top;
            target->bottom = bottom;
        }
        else {
            target->top = bottom;
            target->bottom = top;
        }
    }
}

void rect_create_from_points(rect *target, point first, point second)
{
    if (target != NULL) {
        if (first.x < second.x) {
            target->left = first.x;
            target->right = second.x;
        }
        else {
            target->left = second.x;
            target->right = first.x;
        }
        if (first.y < second.y) {
            target->top = first.y;
            target->bottom = second.y;
        }
        else {
            target->top = second.y;
            target->bottom = first.y;
        }
    }
}
