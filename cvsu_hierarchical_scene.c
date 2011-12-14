/**
 * @file cvsu_hierarchical_scene.c
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

#include "cvsu_config.h"
#include "cvsu_macros.h"
#include "cvsu_memory.h"
#include "cvsu_hierarchical_scene.h"

#include <math.h>

string hierarchical_scene_add_block_name = "hierarchical_scene_add_block";
string hierarchical_scene_create_name = "hierarchical_scene_create";
string hierarchical_scene_destroy_name = "hierarchical_scene_destroy";
string hierarchical_scene_nullify_name = "hierarchical_scene_nullify";
string hierarchical_scene_update_name = "hierarchical_scene_update";

/* private function, do not check pointers */
/*
result hierarchical_scene_add_block(hierarchical_scene *target, block *source)
{
    TRY();

    FINALLY(hierarchical_scene_add_block);
    RETURN();
}
*/

void grid_item_set_block(hierarchical_scene *target, grid_item *item)
{
    block new_block, *grid_block;

    new_block.pos.x = item->pos.x * target->hstep;// + target->hmargin;
    new_block.pos.y = item->pos.y * target->vstep;// + target->vmargin;
    new_block.width = target->hstep;
    new_block.height = target->vstep;
    new_block.stat.mean = 0;
    new_block.stat.dev = 0;
    new_block.nw = NULL;
    new_block.ne = NULL;
    new_block.sw = NULL;
    new_block.se = NULL;
    //new_block.region_id = 0;
    new_block.region_ptr = NULL;
    new_block.pass_count = 0;

    list_append_reveal_data(target->current_blocks, (pointer)&new_block, (pointer*)&grid_block);
    item->main_block = grid_block;
}

result hierarchical_scene_create(
    hierarchical_scene *target,
    pixel_image *source
    )
{
    TRY();
    uint32 row, col, pos;

    CHECK_POINTER(target);

    target->hstep = 32;//16;
    target->vstep = 32;//16;
    target->hmargin = 0;
    target->vmargin = 0;
    target->box_width = 16;
    target->box_length = 8;
    target->width = source->width;
    target->height = source->height;
    target->rows = (long)((target->height/* - 2 * target->vmargin*/) / target->vstep);
    target->cols = (long)((target->width/* - 2 * target->hmargin*/) / target->hstep);
    target->dx = 0;//(long)((target->hstep - target->box_width) / 2);
    target->dy = 0;//(long)((target->vstep - target->box_width) / 2);

    CHECK(integral_image_create(&target->Int, source));
    CHECK(memory_allocate((data_pointer *)&target->grid, target->rows * target->cols, sizeof(grid_item)));

    CHECK(list_create(&target->blocks_1, 100 * target->rows * target->cols, sizeof(block), 30));
    CHECK(list_create(&target->blocks_2, 100 * target->rows * target->cols, sizeof(block), 30));
    CHECK(sublist_create(&target->blocks_by_deviation, &target->blocks_1));
    //CHECK(pointer_list_create(&target->vedges_1, target->rows * target->width, sizeof(edge_elem), 10, 10));
    //CHECK(pointer_list_create(&target->vedges_2, target->rows * target->width, sizeof(edge_elem), 10, 10));
    //CHECK(pointer_list_create(&target->hedges_1, target->height * target->cols, sizeof(edge_elem), 10, 10));
    //CHECK(pointer_list_create(&target->hedges_2, target->height * target->cols, sizeof(edge_elem), 10, 10));

    CHECK(list_create(&target->region_borders, 200 * target->rows * target->cols, sizeof(region_border_item), 100));

    CHECK(list_create(&target->points_1, 100 * target->rows * target->cols, sizeof(point), 10));
    CHECK(list_create(&target->points_2, 100 * target->rows * target->cols, sizeof(point), 10));
    //CHECK(list_create(&target->lines_1, 1000, sizeof(line), 10));
    //CHECK(list_create(&target->lines_2, 1000, sizeof(line), 10));
    //CHECK(list_create(&target->boundaries_1, 100, sizeof(boundary), 10));
    //CHECK(list_create(&target->boundaries_2, 100, sizeof(boundary), 10));
    CHECK(list_create(&target->regions_1, 100, sizeof(region), 10));
    CHECK(list_create(&target->regions_2, 100, sizeof(region), 10));

    target->current_blocks = &target->blocks_2;
    target->previous_blocks = &target->blocks_1;
    //target->current_vedges = &target->vedges_2;
    //target->previous_vedges = &target->vedges_1;
    //target->current_hedges = &target->hedges_2;
    //target->previous_hedges = &target->hedges_1;
    target->current_points = &target->points_2;
    target->previous_points = &target->points_1;
    //target->current_lines = &target->lines_2;
    //target->previous_lines = &target->lines_1;
    //target->current_boundaries = &target->boundaries_2;
    //target->previous_boundaries = &target->boundaries_1;
    target->current_regions = &target->regions_2;
    target->previous_regions = &target->regions_1;

    pos = 0;
    for (row = 0; row < target->rows; row++) {
        for (col = 0; col < target->cols; col++) {
            target->grid[pos].pos.y = row;
            target->grid[pos].pos.x = col;

            grid_item_set_block(target, &target->grid[pos]);

            /* define neighbors above the block */
            if (row > 0) {
                if (col > 0) {
                    target->grid[pos].neighbor_nw.item = &target->grid[pos - target->cols - 1];
                }
                else {
                    target->grid[pos].neighbor_nw.item = NULL;
                    target->grid[pos].neighbor_nw.strength = 0;
                }
                target->grid[pos].neighbor_n.item = &target->grid[pos - target->cols];
                if (col < target->cols - 1) {
                    target->grid[pos].neighbor_ne.item = &target->grid[pos - target->cols + 1];
                }
                else {
                    target->grid[pos].neighbor_ne.item = NULL;
                    target->grid[pos].neighbor_ne.strength = 0;
                }
            }
            else {
                target->grid[pos].neighbor_nw.item = NULL;
                target->grid[pos].neighbor_nw.strength = 0;
                target->grid[pos].neighbor_n.item = NULL;
                target->grid[pos].neighbor_n.strength = 0;
                target->grid[pos].neighbor_ne.item = NULL;
                target->grid[pos].neighbor_ne.strength = 0;
            }

            // define the right neighbor
            if (col < target->cols - 1) {
                target->grid[pos].neighbor_e.item = &target->grid[pos + 1];
            }
            else {
                target->grid[pos].neighbor_e.item = NULL;
                target->grid[pos].neighbor_e.strength = 0;
            }

            // define neighbors below the block
            if (row < target->rows - 1) {
                if (col < target->cols - 1) {
                    target->grid[pos].neighbor_se.item = &target->grid[pos + target->cols + 1];
                }
                else {
                    target->grid[pos].neighbor_se.item = NULL;
                    target->grid[pos].neighbor_se.strength = 0;
                }
                target->grid[pos].neighbor_s.item = &target->grid[pos + target->cols];
                if (col > 0) {
                    target->grid[pos].neighbor_sw.item = &target->grid[pos + target->cols - 1];
                }
                else {
                    target->grid[pos].neighbor_sw.item = NULL;
                    target->grid[pos].neighbor_sw.strength = 0;
                }
            }
            else {
                target->grid[pos].neighbor_se.item = NULL;
                target->grid[pos].neighbor_se.strength = 0;
                target->grid[pos].neighbor_s.item = NULL;
                target->grid[pos].neighbor_s.strength = 0;
                target->grid[pos].neighbor_sw.item = NULL;
                target->grid[pos].neighbor_sw.strength = 0;
            }

            // define the left neighbor
            if (col > 0) {
                target->grid[pos].neighbor_w.item = &target->grid[pos - 1];
            }
            else {
                target->grid[pos].neighbor_w.item = NULL;
                target->grid[pos].neighbor_w.strength = 0;
            }

            pos++;
        }
    }

    FINALLY(hierarchical_scene_create);
    RETURN();
}

result hierarchical_scene_destroy(
    hierarchical_scene *target
    )
{
    TRY();

    CHECK_POINTER(target);

    CHECK(integral_image_destroy(&target->Int));
    CHECK(memory_deallocate((data_pointer*)&target->grid));
    CHECK(list_destroy(&target->regions_1));
    CHECK(list_destroy(&target->regions_2));
    CHECK(list_destroy(&target->region_borders));
    //CHECK(list_destroy(&target->boundaries_1));
    //CHECK(list_destroy(&target->boundaries_2));
    CHECK(list_destroy(&target->points_1));
    CHECK(list_destroy(&target->points_2));
    //CHECK(list_destroy(&target->lines_1));
    //CHECK(list_destroy(&target->lines_2));
    CHECK(list_destroy(&target->blocks_1));
    CHECK(list_destroy(&target->blocks_2));
    //CHECK(pointer_list_destroy(&target->vedges_1));
    //CHECK(pointer_list_destroy(&target->vedges_2));
    //CHECK(pointer_list_destroy(&target->hedges_1));
    //CHECK(pointer_list_destroy(&target->hedges_2));

    FINALLY(hierarchical_scene_destroy);
    RETURN();
}

result hierarchical_scene_nullify(
    hierarchical_scene *target
    )
{
    TRY();

    CHECK_POINTER(target);

    target->hstep = 0;
    target->vstep = 0;
    target->hmargin = 0;
    target->vmargin = 0;
    target->box_width = 0;
    target->box_length = 0;
    target->rows = 0;
    target->cols = 0;
    target->width = 0;
    target->height = 0;
    target->dx = 0;
    target->dy = 0;
    target->grid = NULL;
    target->previous_blocks = NULL;
    target->current_blocks = NULL;

    FINALLY(hierarchical_scene_nullify);
    RETURN();
}

long rate_neighbor(grid_item *current, neighbor_relation *relation)
{
    grid_item *neighbor;
    double diff;/*, dev, dist;*/
    neighbor = relation->item;
    if (neighbor != NULL) {
        diff = current->main_block->stat.mean - neighbor->main_block->stat.mean;
        if (diff < 0) diff = -diff;
        //dev = current->fdev + neighbor->fdev;
        //if (dev < 1) dev = 1;
        //dist = ((double)diff / dev);
        if (diff < current->main_block->stat.dev) {
            relation->strength = 255;
        }
        else {
            relation->strength = 0;
        }
        //relation->strength = (long)(255 * dist);
        return relation->strength;
    }
    else {
        return 0;
    }
}

result hierarchical_scene_update(
    hierarchical_scene *target
    )
{
    TRY();
    uint32 pos, size;
    uint16 region_id;
    list *temp;
    //long strength;
    block *current_block;//, *child_block, new_block;
    region new_region, *region_ptr;
    region_border_item *current_border;
    //line new_line, *l;
    //boundary new_boundary, *b;
    list_item *blocks, *borders;//, *lines, *boundaries, *temp, *block_item;

    temp = target->previous_blocks;
    target->previous_blocks = target->current_blocks;
    target->current_blocks = temp;
    list_clear(target->current_blocks);
    size = target->rows * target->cols;
    for (pos = 0; pos < size; pos++) {
        grid_item_set_block(target, &target->grid[pos]);
    }

    CHECK_POINTER(target);
    CHECK(integral_image_update(&target->Int));

    //list_clear(&target->blocks_by_deviation);
    blocks = target->current_blocks->first.next;
    while (blocks != &target->current_blocks->last) {
        current_block = (block *)blocks->data;
        CHECK(block_update(current_block, &target->Int));
        if (current_block->stat.dev > dev_threshold) {
            CHECK(block_divide(current_block, &target->Int, target->current_blocks));
        }
        current_block->pass_count++;
        //CHECK(list_insert_sorted(&target->blocks_by_deviation, current_block, &compare_blocks_by_deviation));
        blocks = blocks->next;
    }

    temp = target->previous_regions;
    target->previous_regions = target->current_regions;
    target->current_regions = temp;
    list_clear(target->current_regions);
    list_clear(&target->region_borders);
    temp = target->previous_points;
    target->previous_points = target->current_points;
    target->current_points = temp;
    list_clear(target->current_points);
    region_id = 1;
    for (pos = 0; pos < size; pos++) {
        if (target->grid[pos].main_block->stat.dev <= dev_threshold && target->grid[pos].main_block->region_ptr == NULL) {
            region_id++;
            new_region.id = region_id;
            CHECK(list_append_reveal_data(target->current_regions, (pointer)&new_region, (pointer*)&region_ptr));
            CHECK(region_init(region_ptr, &target->grid[pos], &target->region_borders, target->current_points, target->current_blocks));
            borders = region_ptr->borders.first.next;
            while (borders != &region_ptr->borders.last) {
                current_border = (region_border_item *)borders->data;
                region_expand(current_border);
                borders = borders->next;
                CHECK(list_remove_item(&region_ptr->borders, borders->prev));
            }
            //region_expand(new_region, target->grid[pos].neighbor_n.item, d_N);
            //region_expand(new_region, target->grid[pos].neighbor_e.item, d_E);
            //region_expand(new_region, target->grid[pos].neighbor_s.item, d_S);
            //region_expand(new_region, target->grid[pos].neighbor_w.item, d_W);
        }
        //else
        //    if ()
    }

    /*
    block_item = target->blocks_by_deviation.first.next;
    while (block_item != &target->blocks_by_deviation.last) {
        current_block = (block *)block_item->data;
        strength = 0;
        strength += check_neighbor(current_block, &current_block->neighbor_nw);
        strength += check_neighbor(current_block, &current_block->neighbor_n);
        strength += check_neighbor(current_block, &current_block->neighbor_ne);
        strength += check_neighbor(current_block, &current_block->neighbor_e);
        strength += check_neighbor(current_block, &current_block->neighbor_se);
        strength += check_neighbor(current_block, &current_block->neighbor_s);
        strength += check_neighbor(current_block, &current_block->neighbor_sw);
        strength += check_neighbor(current_block, &current_block->neighbor_w);
        strength /= 8;
        current_block->strength = strength;
        block_item = block_item->next;
    }
    */

    //pointer_sublist_create(&target->grid[pos].vedge_list, &target->vedges_1,
    //        row * target->width + target->hmargin + col * target->hstep, target->hstep);
    //pointer_sublist_create(&target->grid[pos].hedge_list, &target->hedges_1,
    //        (row * target->vstep + target->vmargin) * target->cols + col, target->vstep);

    FINALLY(hierarchical_scene_update);
    RETURN();
}
