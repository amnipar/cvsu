/**
 * @file cvsu_block.c
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

#include "cvsu_block.h"
#include "cvsu_macros.h"

#include <stdio.h>
#include <math.h>

string block_update_name = "block_update";
string block_divide_name = "block_divide";
string region_init_name = "region_init";
string region_new_block_name = "region_new_block";
string region_add_block_name = "region_add_block";
string region_expand_name = "region_expand";
/*
int compare_blocks_by_deviation(const void *a, const void *b)
{
    return ((const block *)a)->stat.dev - ((const block *)b)->stat.dev;
}
*/
bool compare_points(const point *a, const point *b)
{
    return (a->x == b->x && a->y == b->y);
}

bool compare_lines(const line *a, const line *b)
{
    return (compare_points(&a->start, &b->start) && compare_points(&a->end, &b->end));
}

result block_update(block *target, integral_image *I)
{
    TRY();
    INTEGRAL_IMAGE_1BOX_VARIABLES();
    uint32 offset;
    double mean, dev;

    CHECK_POINTER(target);
    CHECK_POINTER(I);
    INTEGRAL_IMAGE_INIT_1BOX(I, target->height, target->width);
    CHECK_POINTER(I_1_data);
    CHECK_POINTER(I_2_data);

    offset = (target->pos.y) * I->stride + target->pos.x * I->step;
    iA = I_1_data + offset;
    i2A = I_2_data + offset;

    sum = INTEGRAL_IMAGE_SUM();
    sumsqr = INTEGRAL_IMAGE_SUMSQR();
    mean = (double)sum / (double)N;
    dev = (sumsqr / (double)N) - (mean * mean);
    if (dev < 1) dev = 1;

    target->stat.mean = (sint16)((mean < 0) ? 0 : ((mean > 255) ? 255 : mean));
    target->stat.dev = (sint16)sqrt(dev);

    FINALLY(block_update);
    RETURN();
}

result block_divide(block *target, integral_image *I, list *block_list)
{
    TRY();
    block new_block, *child_block;

    new_block.width = target->height / 2;
    new_block.height = target->height / 2;
    new_block.nw = NULL;
    new_block.ne = NULL;
    new_block.sw = NULL;
    new_block.se = NULL;
    /* new blocks always to region 0, so they must be checked? */
    //new_block.region_id = 0;
    new_block.region_ptr = NULL;
    new_block.pass_count = target->pass_count;
    /* nw block */
    new_block.pos = target->pos;
    CHECK(block_update(&new_block, I));
    CHECK(list_append_reveal_data(block_list, (pointer)&new_block, (pointer*)&child_block));
    target->nw = child_block;
    /* ne block */
    new_block.pos.x = target->pos.x + new_block.width;
    new_block.pos.y = target->pos.y;
    CHECK(block_update(&new_block, I));
    CHECK(list_append_reveal_data(block_list, (pointer)&new_block, (pointer*)&child_block));
    target->ne = child_block;
    /* sw block */
    new_block.pos.x = target->pos.x;
    new_block.pos.y = target->pos.y + new_block.height;
    CHECK(block_update(&new_block, I));
    CHECK(list_append_reveal_data(block_list, (pointer)&new_block, (pointer*)&child_block));
    target->sw = child_block;
    /* se block */
    new_block.pos.x = target->pos.x + new_block.width;
    new_block.pos.y = target->pos.y + new_block.height;
    CHECK(block_update(&new_block, I));
    CHECK(list_append_reveal_data(block_list, (pointer)&new_block, (pointer*)&child_block));
    target->se = child_block;

    FINALLY(block_divide);
    RETURN();
}

result region_init(
    region *target,
    grid_item *item,
    list *border_list,
    list *point_list,
    list *block_list
    )
{
    TRY();
    region_border_item new_border;

    CHECK_POINTER(target);
    CHECK_POINTER(item);

    target->block_count = 0;
    target->mean = 0;
    target->dev = 0;
    rect_create(&target->bounding_box, 0, 0, 0, 0);

    //CHECK(sublist_create(&target->lines, line_list));
    CHECK(sublist_create(&target->borders, border_list));
    CHECK(sublist_create(&target->points, point_list));
    CHECK(sublist_create(&target->blocks, block_list));

    new_border.region_ptr = target;
    new_border.grid_ptr = item;
    new_border.dir = d_NULL;

    CHECK(sublist_append(&target->borders, (pointer)&new_border));

    FINALLY(region_init);
    RETURN();
}

bool check_neighbor(grid_item *neighbor, region *region_ptr)
{
    return (neighbor != NULL && neighbor->main_block->region_ptr == region_ptr);
}

bool border_condition_nw(region_border_item *item)
{
    return
       (check_neighbor(item->grid_ptr->neighbor_w.item, item->region_ptr) ||
        //check_neighbor(item->grid_ptr->neighbor_nw.item, item->region_ptr->id)||
        check_neighbor(item->grid_ptr->neighbor_n.item, item->region_ptr));
}

bool border_condition_ne(region_border_item *item)
{
    return
       (check_neighbor(item->grid_ptr->neighbor_n.item, item->region_ptr) ||
        //check_neighbor(item->grid_ptr->neighbor_ne.item, item->region_ptr->id)||
        check_neighbor(item->grid_ptr->neighbor_e.item, item->region_ptr));
}

bool border_condition_sw(region_border_item *item)
{
    return
       (check_neighbor(item->grid_ptr->neighbor_w.item, item->region_ptr) ||
        //check_neighbor(item->grid_ptr->neighbor_sw.item, item->region_ptr->id)||
        check_neighbor(item->grid_ptr->neighbor_s.item, item->region_ptr));
}

bool border_condition_se(region_border_item *item)
{
    return
       (check_neighbor(item->grid_ptr->neighbor_s.item, item->region_ptr) ||
        //check_neighbor(item->grid_ptr->neighbor_se.item, item->region_ptr->id)||
        check_neighbor(item->grid_ptr->neighbor_e.item, item->region_ptr));
}

bool region_condition_dev_low(block *new_block)
{
    return true;
}

bool region_condition_dev_similar(grid_item *item)
{
    sint16 dev_c, dev_n, dev_e, dev_s, dev_w, dev_min, dev_max;
    if (
            (item->main_block->region_ptr == NULL) &&
            check_neighbor(item->neighbor_n.item, 0) &&
            check_neighbor(item->neighbor_e.item, 0) &&
            check_neighbor(item->neighbor_s.item, 0) &&
            check_neighbor(item->neighbor_w.item, 0)) {
        dev_c = item->main_block->stat.dev;
        dev_n = item->neighbor_n.item->main_block->stat.dev;
        dev_e = item->neighbor_e.item->main_block->stat.dev;
        dev_s = item->neighbor_s.item->main_block->stat.dev;
        dev_w = item->neighbor_w.item->main_block->stat.dev;
        dev_min = dev_c - 10;
        dev_max = dev_c + 10;
        if (
                (dev_min < dev_n && dev_n < dev_max) &&
                (dev_min < dev_e && dev_e < dev_max) &&
                (dev_min < dev_s && dev_s < dev_max) &&
                (dev_min < dev_w && dev_w < dev_max)) {
            return true;
        }
        else {
            return false;
        }
    }
    return false;
}

bool is_normal_mean(region *new_region, block *new_block)
{
    sint16 dev, mean;
    dev = (sint16)(new_region->dev / new_region->block_count);
    mean = (sint16)(new_region->mean / new_region->block_count);
    if (dev < 1) dev = 1;
    return (new_block->stat.mean < (mean + 2*dev*dev) &&
            new_block->stat.mean > (mean - 2*dev*dev));// {
    //    return true;
    //}
    /*
    else {
        printf("Dev %d mean1 %d mean2 %d\n", dev, new_block->stat.mean, mean);
        return false;
    }
    */
}

bool is_normal_dev(region *new_region, block *new_block)
{
    return true;
}

result region_new_block(region_border_item *item, block *new_block)
{
    TRY();

    CHECK(list_append(&item->region_ptr->blocks, (pointer)new_block));
    new_block->region_ptr = item->region_ptr;
    item->region_ptr->block_count += 1;
    item->region_ptr->dev += new_block->stat.dev;
    item->region_ptr->mean += new_block->stat.mean;
    item->region_ptr->stat.dev = (sint16)(item->region_ptr->dev / item->region_ptr->block_count);
    item->region_ptr->stat.mean = (sint16)(item->region_ptr->mean / item->region_ptr->block_count);

    FINALLY(region_new_block);
    RETURN();
}

result region_add_block(region_border_item *item, block *new_block)
{
    TRY();
    bool nw_added, ne_added, sw_added, se_added;

    r = SUCCESS;
    nw_added = ne_added = sw_added = se_added = false;

    if (border_condition_nw(item)) {
        if (new_block->nw != NULL &&
            (new_block->nw->stat.dev <= dev_threshold || is_normal_mean(item->region_ptr, new_block->nw)) &&
            new_block->nw->region_ptr == NULL) {
            CHECK(region_new_block(item, new_block->nw));
            nw_added = true;
        }
        /*
        else {
            if (new_block->nw != NULL && new_block->nw->region_id == 0) {
                printf("Dev %d\n", new_block->nw->stat.dev);
            }
        }
        */
    }
    if (border_condition_ne(item)) {
        if (new_block->ne != NULL &&
            (new_block->ne->stat.dev <= dev_threshold || is_normal_mean(item->region_ptr, new_block->ne)) &&
            new_block->ne->region_ptr == NULL) {
            CHECK(region_new_block(item, new_block->ne));
            ne_added = true;
        }
        /*
        else {
            if (new_block->ne != NULL && new_block->ne->region_id == 0) {
                printf("Dev %d\n", new_block->ne->stat.dev);
            }
        }
        */
    }
    if (border_condition_sw(item)) {
        if (new_block->sw != NULL &&
            (new_block->sw->stat.dev <= dev_threshold || is_normal_mean(item->region_ptr, new_block->sw)) &&
            new_block->sw->region_ptr == NULL) {
            CHECK(region_new_block(item, new_block->sw));
            sw_added = true;
        }
        /*
        else {
            if (new_block->sw != NULL && new_block->sw->region_id == 0) {
                printf("Dev %d\n", new_block->sw->stat.dev);
            }
        }
        */
    }
    if (border_condition_se(item)) {
        if (new_block->se != NULL &&
            (new_block->se->stat.dev <= dev_threshold || is_normal_mean(item->region_ptr, new_block->se)) &&
            new_block->se->region_ptr == NULL) {
            CHECK(region_new_block(item, new_block->se));
            se_added = true;
        }
        /*
        else {
            if (new_block->se != NULL && new_block->se->region_id == 0) {
                printf("Dev %d\n", new_block->se->stat.dev);
            }
        }
        */
    }

    if (sw_added && nw_added && ne_added && !se_added) {
        CHECK(region_add_block(item, new_block->se));
    }
    else
    if (nw_added && ne_added && se_added && !sw_added) {
        CHECK(region_add_block(item, new_block->sw));
    }
    else
    if (ne_added && se_added && sw_added && !nw_added) {
        CHECK(region_add_block(item, new_block->nw));
    }
    else
    if (se_added && sw_added && nw_added &&!ne_added) {
        CHECK(region_add_block(item, new_block->ne));
    }
    else
    if (sw_added && nw_added && !ne_added && !se_added) {
        CHECK(region_add_block(item, new_block->ne));
        CHECK(region_add_block(item, new_block->se));
    }
    else
    if (nw_added && ne_added && !se_added && !sw_added) {
        CHECK(region_add_block(item, new_block->se));
        CHECK(region_add_block(item, new_block->sw));
    }
    else
    if (ne_added && se_added && !sw_added && !nw_added) {
        CHECK(region_add_block(item, new_block->sw));
        CHECK(region_add_block(item, new_block->nw));
    }
    else
    if (se_added && sw_added && !nw_added && !ne_added) {
        CHECK(region_add_block(item, new_block->se));
        CHECK(region_add_block(item, new_block->sw));
    }

    FINALLY(region_add_block);
    RETURN();
}

result region_expand(
    region_border_item *item
    )
{
    TRY();
    region_border_item new_item;
    grid_item *neighbor;
    /*
    line *l;
    list_item *lines, *line_n, *line_e, *line_s, *line_w, *start;
    point nw, ne, sw, se;
    line n, e, s, w, new_line;
    bool border_n, border_e, border_s, border_w;
    */
    CHECK_POINTER(item);
    CHECK_POINTER(item->grid_ptr);
    CHECK_POINTER(item->region_ptr);

    new_item.dir = d_NULL;
    new_item.region_ptr = item->region_ptr;

    /* is the main block eligible for including into the region */
    if (item->grid_ptr->main_block->stat.dev <= dev_threshold) {
        /* if the main block hasn't been added to region, add it */
        if (item->grid_ptr->main_block->region_ptr == NULL) {
            CHECK(region_new_block(item, item->grid_ptr->main_block));

            neighbor = item->grid_ptr->neighbor_n.item;
            if (neighbor != NULL && neighbor->main_block->region_ptr == NULL) {
                new_item.grid_ptr = neighbor;
                CHECK(sublist_append(&item->region_ptr->borders, (pointer)&new_item));
            }
            /*
            neighbor = item->grid_ptr->neighbor_ne.item;
            if (neighbor != NULL && neighbor->main_block->region_id == 0) {
                new_item.grid_ptr = neighbor;
                CHECK(sublist_append(&item->region_ptr->borders, (pointer)&new_item));
            }
            */
            neighbor = item->grid_ptr->neighbor_e.item;
            if (neighbor != NULL && neighbor->main_block->region_ptr == NULL) {
                new_item.grid_ptr = neighbor;
                CHECK(sublist_append(&item->region_ptr->borders, (pointer)&new_item));
            }
            /*
            neighbor = item->grid_ptr->neighbor_se.item;
            if (neighbor != NULL && neighbor->main_block->region_id == 0) {
                new_item.grid_ptr = neighbor;
                CHECK(sublist_append(&item->region_ptr->borders, (pointer)&new_item));
            }
            */
            neighbor = item->grid_ptr->neighbor_s.item;
            if (neighbor != NULL && neighbor->main_block->region_ptr == NULL) {
                new_item.grid_ptr = neighbor;
                CHECK(sublist_append(&item->region_ptr->borders, (pointer)&new_item));
            }
            /*
            neighbor = item->grid_ptr->neighbor_sw.item;
            if (neighbor != NULL && neighbor->main_block->region_id == 0) {
                new_item.grid_ptr = neighbor;
                CHECK(sublist_append(&item->region_ptr->borders, (pointer)&new_item));
            }
            */
            neighbor = item->grid_ptr->neighbor_w.item;
            if (neighbor != NULL && neighbor->main_block->region_ptr == NULL) {
                new_item.grid_ptr = neighbor;
                CHECK(sublist_append(&item->region_ptr->borders, (pointer)&new_item));
            }
            /*
            neighbor = item->grid_ptr->neighbor_nw.item;
            if (neighbor != NULL && neighbor->main_block->region_id == 0) {
                new_item.grid_ptr = neighbor;
                CHECK(sublist_append(&item->region_ptr->borders, (pointer)&new_item));
            }
            */
        }
    }
    else {
        //item->grid_ptr->main_block->region_id = 1;
        CHECK(region_add_block(item, item->grid_ptr->main_block));
    }

    //if (item->dir == d_NULL && item->region_ptr->block_count )

    /* start from top left corner, add points in CLOCKWISE direction */
    //new_point = item->main_block->pos;
    //CHECK(sublist_append(&(*target)->points, (pointer)&new_point));
    /* go right */
    //point_add(&new_point, item->main_block->width, 0);
    //CHECK(sublist_append(&(*target)->points, (pointer)&new_point));
    /* go down */
    //point_add(&new_point, 0, item->main_block->height);
    //CHECK(sublist_append(&(*target)->points, (pointer)&new_point));
    /* go back left */
    //point_subtract(&new_point, item->main_block->width, 0);
    //CHECK(sublist_append(&(*target)->points, (pointer)&new_point));
/*
    CHECK(list_append(&(*target)->blocks, item->main_block));


    if (item == NULL) {
        TERMINATE(SUCCESS);
    }
    if (item->main_block->stat.dev > 8) {
        TERMINATE(SUCCESS);
    }
    if (item->main_block->region_id != 0) {
        TERMINATE(SUCCESS);
    }

    CHECK(list_append(&target->blocks, item->main_block));
    item->main_block->region_id = target->id;
    target->block_count++;
    //target->stat.mean +=

    nw.x = item->main_block->pos.x;
    nw.y = item->main_block->pos.y;
    ne.x = item->main_block->pos.x + item->main_block->width;
    ne.y = item->main_block->pos.y;
    sw.x = item->main_block->pos.x;
    sw.y = item->main_block->pos.y + item->main_block->height;
    se.x = item->main_block->pos.x + item->main_block->width;
    se.y = item->main_block->pos.y + item->main_block->height;
    n.start = ne;
    n.end = nw;
    line_n = NULL;
    e.start = se;
    e.end = ne;
    line_e = NULL;
    s.start = sw;
    s.end = se;
    line_s = NULL;
    w.start = nw;
    w.end = sw;
    line_w = NULL;
    border_n = false;
    border_e = false;
    border_s = false;
    border_w = false;

    lines = target->lines.first.next;
    while (lines != &target->lines.last) {
        l = (line *)lines->data;
        if (compare_lines(l, &n)) {
            border_n = true;
            line_n = lines;
        }
        else
        if (compare_lines(l, &e)) {
            border_e = true;
            line_e = lines;
        }
        else
        if (compare_lines(l, &s)) {
            border_s = true;
            line_s = lines;
        }
        else
        if (compare_lines(l, &w)) {
            border_w = true;
            line_w = lines;
        }
        lines = lines->next;
    }

    if (border_n && border_e && border_s && border_w) {
        CHECK(list_remove_item(&target->lines, line_n));
        CHECK(list_remove_item(&target->lines, line_e));
        CHECK(list_remove_item(&target->lines, line_s));
        CHECK(list_remove_item(&target->lines, line_w));
    }
    else
    if (border_w && border_n && border_e) {
        start = line_e->prev;
        CHECK(list_remove_between(&target->lines, start, line_w->next));
        new_line.start = se;
        new_line.end = sw;
        CHECK(sublist_insert_at(&target->lines, start, (pointer)&new_line));
    }
    else
    if (border_n && border_e && border_s) {
        start = line_s->prev;
        CHECK(list_remove_between(&target->lines, start, line_n->next));
        new_line.start = sw;
        new_line.end = nw;
        CHECK(sublist_insert_at(&target->lines, start, (pointer)&new_line));
    }
    else
    if (border_e && border_s && border_w) {
        start = line_w->prev;
        CHECK(list_remove_between(&target->lines, start, line_e->next));
        new_line.start = nw;
        new_line.end = ne;
        CHECK(sublist_insert_at(&target->lines, start, (pointer)&new_line));
    }
    else
    if (border_s && border_w && border_n) {
        start = line_n->prev;
        CHECK(list_remove_between(&target->lines, start, line_s->next));
        new_line.start = ne;
        new_line.end = se;
        CHECK(sublist_insert_at(&target->lines, start, (pointer)&new_line));
    }
    else
    if (border_n && border_e) {
        start = line_e->prev;
        CHECK(list_remove_between(&target->lines, start, line_n->next));
        new_line.start = sw;
        new_line.end = nw;
        CHECK(sublist_insert_at(&target->lines, start, (pointer)&new_line));
        new_line.start = se;
        new_line.end = sw;
        CHECK(sublist_insert_at(&target->lines, start, (pointer)&new_line));
    }
    else
    if (border_e && border_s) {
        start = line_s->prev;
        CHECK(list_remove_between(&target->lines, start, line_e->next));
        new_line.start = nw;
        new_line.end = ne;
        CHECK(sublist_insert_at(&target->lines, start, (pointer)&new_line));
        new_line.start = sw;
        new_line.end = nw;
        CHECK(sublist_insert_at(&target->lines, start, (pointer)&new_line));
    }
    else
    if (border_s && border_w) {
        start = line_w->prev;
        CHECK(list_remove_between(&target->lines, start, line_s->next));
        new_line.start = ne;
        new_line.end = se;
        CHECK(sublist_insert_at(&target->lines, start, (pointer)&new_line));
        new_line.start = nw;
        new_line.end = ne;
        CHECK(sublist_insert_at(&target->lines, start, (pointer)&new_line));
    }
    else
    if (border_w && border_n) {
        start = line_n->prev;
        CHECK(list_remove_between(&target->lines, start, line_w->next));
        new_line.start = se;
        new_line.end = sw;
        CHECK(sublist_insert_at(&target->lines, start, (pointer)&new_line));
        new_line.start = ne;
        new_line.end = se;
        CHECK(sublist_insert_at(&target->lines, start, (pointer)&new_line));
    }
    else
    if (border_n) {
        start = line_n->prev;
        CHECK(list_remove_item(&target->lines, line_n));
        new_line.start = sw;
        new_line.end = nw;
        CHECK(sublist_insert_at(&target->lines, start, (pointer)&new_line));
        new_line.start = se;
        new_line.end = sw;
        CHECK(sublist_insert_at(&target->lines, start, (pointer)&new_line));
        new_line.start = ne;
        new_line.end = se;
        CHECK(sublist_insert_at(&target->lines, start, (pointer)&new_line));
    }
    else
    if (border_e) {
        start = line_e->prev;
        CHECK(list_remove_item(&target->lines, line_e));
        new_line.start = nw;
        new_line.end = ne;
        CHECK(sublist_insert_at(&target->lines, start, (pointer)&new_line));
        new_line.start = sw;
        new_line.end = nw;
        CHECK(sublist_insert_at(&target->lines, start, (pointer)&new_line));
        new_line.start = se;
        new_line.end = sw;
        CHECK(sublist_insert_at(&target->lines, start, (pointer)&new_line));
    }
    else
    if (border_s) {
        start = line_s->prev;
        CHECK(list_remove_item(&target->lines, line_s));
        new_line.start = ne;
        new_line.end = se;
        CHECK(sublist_insert_at(&target->lines, start, (pointer)&new_line));
        new_line.start = nw;
        new_line.end = ne;
        CHECK(sublist_insert_at(&target->lines, start, (pointer)&new_line));
        new_line.start = sw;
        new_line.end = nw;
        CHECK(sublist_insert_at(&target->lines, start, (pointer)&new_line));
    }
    else
    if (border_w) {
        start = line_w->prev;
        CHECK(list_remove_item(&target->lines, line_w));
        new_line.start = se;
        new_line.end = sw;
        CHECK(sublist_insert_at(&target->lines, start, (pointer)&new_line));
        new_line.start = ne;
        new_line.end = se;
        CHECK(sublist_insert_at(&target->lines, start, (pointer)&new_line));
        new_line.start = nw;
        new_line.end = ne;
        CHECK(sublist_insert_at(&target->lines, start, (pointer)&new_line));
    }

    neighbor = item->neighbor_n.item;
    if (neighbor != NULL && neighbor->main_block->stat.dev <= 8 && neighbor->main_block->region_id == 0) {
        CHECK(region_expand(target, item->neighbor_n.item, d_N));
    }
    neighbor = item->neighbor_e.item;
    if (neighbor != NULL && neighbor->main_block->stat.dev <= 8 && neighbor->main_block->region_id == 0) {
        CHECK(region_expand(target, item->neighbor_e.item, d_E));
    }
    neighbor = item->neighbor_s.item;
    if (neighbor != NULL && neighbor->main_block->stat.dev <= 8 && neighbor->main_block->region_id == 0) {
        CHECK(region_expand(target, item->neighbor_s.item, d_S));
    }
    neighbor = item->neighbor_w.item;
    if (neighbor != NULL && neighbor->main_block->stat.dev <= 8 && neighbor->main_block->region_id == 0) {
        CHECK(region_expand(target, item->neighbor_w.item, d_W));
    }
*/
    FINALLY(region_expand);
    RETURN();
}
