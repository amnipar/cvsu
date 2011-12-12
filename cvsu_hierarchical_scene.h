/**
 * @file cvsu_hierarchical_scene.h
 * @author Matti J. Eskelinen <matti.j.eskelinen@gmail.com>
 * @brief Hierarchical scene geometry handling for the cvsu module.
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

#ifndef CVSU_HIERARCHICAL_SCENE_H
#   define CVSU_HIERARCHICAL_SCENE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "cvsu_basic.h"
#include "cvsu_edges.h"
#include "cvsu_block.h"
#include "cvsu_list.h"

typedef struct hierarchical_scene_t {
    integral_image Int;

    uint32 hstep;
    uint32 vstep;
    uint32 hmargin;
    uint32 vmargin;
    uint32 box_width;
    uint32 box_length;
    uint32 rows;
    uint32 cols;
    uint32 width;
    uint32 height;
    uint32 dx;
    uint32 dy;

    grid_item *grid;

    list blocks_1;
    list blocks_2;
    list blocks_by_deviation;
    list *previous_blocks;
    list *current_blocks;

    pointer_list vedges_1;
    pointer_list vedges_2;
    pointer_list hedges_1;
    pointer_list hedges_2;
    pointer_list *previous_vedges;
    pointer_list *current_vedges;
    pointer_list *previous_hedges;
    pointer_list *current_hedges;

    list region_borders;

    list points_1;
    list points_2;
    list *previous_points;
    list *current_points;
    list lines_1;
    list lines_2;
    list *previous_lines;
    list *current_lines;
    list boundaries_1;
    list boundaries_2;
    list *previous_boundaries;
    list *current_boundaries;
    list regions_1;
    list regions_2;
    list *previous_regions;
    list *current_regions;
} hierarchical_scene;

result hierarchical_scene_create(
    hierarchical_scene *target,
    pixel_image *source
);

result hierarchical_scene_destroy(
    hierarchical_scene *target
);

result hierarchical_scene_nullify(
    hierarchical_scene *target
);

result hierarchical_scene_update(
    hierarchical_scene *target
);

#ifdef __cplusplus
}
#endif

#endif /* CVSU_HIERARCHICAL_SCENE_H */
