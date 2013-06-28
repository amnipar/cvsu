/**
 * @file cvsu_quad_forest.c
 * @author Matti J. Eskelinen <matti.j.eskelinen@gmail.com>
 * @brief Quad Forest hierarchical data structure for analyzing images.
 *
 * Copyright (c) 2011-2013, Matti Johannes Eskelinen
 * All Rights Reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *   * Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright notice,
 *     this list of conditions and the following disclaimer in the documentation
 *     and/or other materials provided with the distribution.
 *   * Neither the name of the copyright holder nor the names of its
 *     contributors may be used to endorse or promote products derived from this
 *     software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL COPYRIGHT HOLDER BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "cvsu_macros.h"
#include "cvsu_quad_tree.h"
#include "cvsu_quad_forest.h"
#include "cvsu_memory.h"

#include <stdlib.h>
/*#include <stdio.h>*/
#include <sys/time.h>

/******************************************************************************/
/* constants for reporting function names in error messages                   */

string quad_forest_init_name = "quad_forest_init";
string quad_forest_alloc_name = "quad_forest_alloc";
string quad_forest_free_name = "quad_forest_free";
string quad_forest_create_name = "quad_forest_create";
string quad_forest_reload_name = "quad_forest_reload";
string quad_forest_destroy_name = "quad_forest_destroy";
string quad_forest_nullify_name = "quad_forest_nullify";
string quad_forest_update_name = "quad_forest_update";
string quad_forest_get_segments_name = "quad_forest_get_regions";
string quad_forest_get_segment_trees_name = "quad_forest_get_segment_trees";
string quad_forest_get_segment_neighbors_name = "quad_forest_get_segment_neighbors";
string quad_forest_get_segment_mask_name = "quad_forest_get_segment_mask";
string quad_forest_get_segment_boundary_name = "quad_forest_get_segment_boundary";
string quad_forest_get_edge_chain_name = "quad_forest_get_edge_chain";
string quad_forest_get_path_sniffers_name = "quad_forest_get_path_sniffers";
string quad_forest_get_links_name = "quad_forest_get_links";
string quad_forest_draw_trees_name =  "quad_forest_draw_trees";
string quad_forest_highlight_segments_name = "quad_forest_highlight_segments";
string quad_forest_draw_image_name = "quad_forest_draw_image";

/******************************************************************************/
/* quad_forest_status possible values                                         */

const quad_forest_status FOREST_UNINITIALIZED = 0x00;
/* Forest has been initialized, but not yet updated. */
const quad_forest_status FOREST_INITIALIZED = 0x01;
/* Forest has been updated, but no analysis performed. */
const quad_forest_status FOREST_UPDATED = 0x02;
/* Segmentation operation has been performed. */
const quad_forest_status FOREST_SEGMENTED = 0x04;
/* Edge detection operation has been performed. */
const quad_forest_status FOREST_EDGES_DETECTED = 0x08;
/* Parse operation has been performed. */
const quad_forest_status FOREST_PARSED = 0x10;

/******************************************************************************/
/* a private function for initializing quad_forest structure                  */
/* used in create and in reload                                               */

#define ADD_LINK(neighbor,cat)\
  new_link.category = cat;\
  new_link.b.tree = (neighbor);\
  CHECK(list_append_return_pointer(&target->links, (pointer)&new_link,\
        (pointer*)&link));\
  link->a.link = link;\
  link->a.other = &link->b;\
  link->b.link = link;\
  link->b.other = &link->a;\
  head = &link->a;\
  CHECK(list_append(&tree->links, (pointer)&head));\

#define GET_LINK(neighbor,cat)\
  CHECK(quad_tree_find_link(neighbor, tree, &head));\
  if (head == NULL) {\
    ERROR(NOT_FOUND);\
  }\
  CHECK(list_append(&tree->links, (pointer)&head));\

result quad_forest_init
(
  quad_forest *target,
  pixel_image *source,
  uint32 tree_max_size,
  uint32 tree_min_size
)
{
  TRY();
  uint32 row, col, rows, cols, pos, size, width, height;
  integral_value angle;
  quad_tree new_tree, *tree;
  quad_tree_link new_link, *link;
  quad_tree_link_head *head, *n, *ne, *e, *se, *s, *sw, *w, *nw;

  /* not necessary to check target pointer, calling function should handle that */
  width = source->width;
  height = source->height;

  CHECK_PARAM(tree_max_size <= width && tree_max_size <= height);
  CHECK_PARAM(tree_min_size <= tree_max_size);

  if (target->tree_max_size != tree_max_size || target->tree_min_size != tree_min_size) {
    /* initialize values */
    rows = (uint32)(height / tree_max_size);
    cols = (uint32)(width / tree_max_size);
    target->rows = rows;
    target->cols = cols;
    target->tree_max_size = tree_max_size;
    target->tree_min_size = tree_min_size;
    target->dx = (uint32)((width - (cols * tree_max_size)) / 2);
    target->dy = (uint32)((height - (rows * tree_max_size)) / 2);
    target->token = 0;

    size = rows * cols;
    if (target->roots != NULL) {
      CHECK(memory_deallocate((data_pointer*)&target->roots));
    }
    CHECK(memory_allocate((data_pointer *)&target->roots, size, sizeof(quad_tree*)));

    if (!list_is_null(&target->trees)) {
      CHECK(list_destroy(&target->trees));
    }
    CHECK(list_create(&target->trees, 8 * size, sizeof(quad_tree), 1));

    if (!list_is_null(&target->links)) {
      CHECK(list_destroy(&target->links));
    }
    CHECK(list_create(&target->links, 8 * size, sizeof(quad_tree_link), 1));
  }
  else {
    rows = target->rows;
    cols = target->cols;
    size = rows * cols;
    /* TODO: need to determine also max tree depth -> max number of tree nodes */
  }

  if (target->source == NULL) {
    target->source = pixel_image_alloc();
    CHECK_POINTER(target->source);
    CHECK(pixel_image_clone(target->source, source));
    /*p_U8, GREY, width, height, 1, width));*/
  }

  /*printf("create integral image\n");*/
  if (integral_image_is_null(&target->integral)) {
    CHECK(integral_image_create(&target->integral, target->source));
  }

  CHECK(list_clear(&target->trees));

  /* create tree roots and their trees and blocks */
  /* TODO: init value only once */
  quad_tree_nullify(&new_tree);
  new_tree.size = tree_max_size;
  for (row = 0, pos = 0; row < rows; row++) {
    new_tree.y = (uint32)(target->dy + row * tree_max_size);
    new_tree.x = (uint32)(target->dx);
    for (col = 0; col < cols; col++, pos++, new_tree.x += tree_max_size) {
      CHECK(list_append_return_pointer(&target->trees, (pointer)&new_tree, (pointer*)&tree));
      CHECK(list_create(&tree->links, 8, sizeof(quad_tree_link_head*), 1));
      target->roots[pos] = tree;
    }
  }
  target->last_root_tree = target->trees.last.prev;

  n = NULL;
  ne = NULL;
  e = NULL;
  se = NULL;
  s = NULL;
  sw = NULL;
  w = NULL;
  nw = NULL;
  new_link.a.opposite = NULL;
  new_link.a.angle = 0;
  new_link.a.annotation.type = t_UNDEF;
  new_link.a.annotation.count = 0;
  new_link.a.annotation.token = 0;
  new_link.a.annotation.value = NULL;
  new_link.b.opposite = NULL;
  new_link.b.angle = 0;
  new_link.b.annotation.type = t_UNDEF;
  new_link.b.annotation.count = 0;
  new_link.b.annotation.token = 0;
  new_link.b.annotation.value = NULL;
  new_link.distance = 0;
  new_link.annotation.type = t_UNDEF;
  new_link.annotation.count = 0;
  new_link.annotation.token = 0;
  new_link.annotation.value = NULL;
  /* add neighbors to roots */
  /* TODO: create the neighbor links (first-rate 8-neighborhood) */
  /* then implement a simple edge propagation algorithm */
  /* collect links to list and visualize on image (with blue color?) */
  /* then visualize tree values ('height') also (with red color?) */
  for (row = 0, pos = 0; row < rows; row++) {
    for (col = 0; col < cols; col++, pos++) {
      tree = target->roots[pos];
      new_link.a.tree = tree;
      head = NULL;
      /* add neighbor to west */
      if (col > 0) {
        tree->w = target->roots[pos - 1];
        GET_LINK(tree->w, d_N4);
        head->angle = M_PI;
        w = head;
        /* nw neighbor */
        if (row > 0) {
          GET_LINK(target->roots[pos - cols - 1], d_N8);
          head->angle = 3 * M_PI / 4;
          nw = head;
        }
        else {
          nw = NULL;
        }
        /* sw neighbor */
        if (row < (unsigned)(rows - 1)) {
          new_link.distance = sqrt(2);
          ADD_LINK(target->roots[pos + cols - 1], d_N8);
          head->angle = 5 * M_PI / 4;
          sw = head;
        }
        else {
          sw = NULL;
        }
      }
      else {
        w = NULL;
        nw = NULL;
        sw = NULL;
      }
      /* add neighbor to north */
      if (row > 0) {
        tree->n = target->roots[pos - cols];
        GET_LINK(tree->n, d_N4);
        head->angle = M_PI / 2;
        n = head;
      }
      else {
        n = NULL;
      }
      /* add neighbor to east */
      if (col < (unsigned)(cols - 1)) {
        tree->e = target->roots[pos + 1];
        new_link.distance = 1;
        ADD_LINK(tree->e, d_N4);
        head->angle = 0;
        e = head;
        /* ne neighbor */
        if (row > 0) {
          GET_LINK(target->roots[pos - cols + 1], d_N8);
          head->angle = M_PI / 4;
          ne = head;
        }
        else {
          ne = NULL;
        }
        /* se neighbor */
        if (row < (unsigned)(rows - 1)) {
          new_link.distance = sqrt(2);
          ADD_LINK(target->roots[pos + cols + 1], d_N8);
          head->angle = 7 * M_PI / 4;
          se = head;
        }
        else {
          se = NULL;
        }
      }
      else {
        e = NULL;
        ne = NULL;
        se = NULL;
      }
      /* add neighbor to south */
      if (row < (unsigned)(rows - 1)) {
        tree->s = target->roots[pos + cols];
        new_link.distance = 1;
        ADD_LINK(tree->s, d_N4);
        head->angle = 3 * M_PI / 2;
        s = head;
      }
      else {
        s = NULL;
      }

      if (n != NULL && s != NULL) {
        n->opposite = s;
        s->opposite = n;
      }
      if (ne != NULL && sw != NULL) {
        ne->opposite = sw;
        sw->opposite = ne;
      }
      if (e != NULL && w != NULL) {
        e->opposite = w;
        w->opposite = e;
      }
      if (se != NULL && nw != NULL) {
        se->opposite = nw;
        nw->opposite = se;
      }
    }
  }

  quad_forest_set_init(target);

  FINALLY(quad_forest_init);
  RETURN();
}

/******************************************************************************/

quad_forest *quad_forest_alloc()
{
  TRY();
  quad_forest *forest;

  CHECK(memory_allocate((data_pointer *)&forest, 1, sizeof(quad_forest)));
  CHECK(quad_forest_nullify(forest));

  FINALLY(quad_forest_alloc);
  return forest;
}

/******************************************************************************/

void quad_forest_free
(
  quad_forest *forest
)
{
  TRY();

  r = SUCCESS;
  if (forest != NULL) {
    CHECK(quad_forest_destroy(forest));
    CHECK(memory_deallocate((data_pointer *)&forest));
  }

  FINALLY(quad_forest_free);
}

/******************************************************************************/

result quad_forest_create
(
  quad_forest *target,
  pixel_image *source,
  uint32 tree_max_size,
  uint32 tree_min_size
)
{
  TRY();

  CHECK_POINTER(target);

  CHECK_FALSE(pixel_image_is_null(source));

  CHECK_PARAM(source->type == p_U8);
  CHECK_PARAM(source->format == GREY);

  /* nullify so the values can be checked and set in the init function */
  CHECK(quad_forest_nullify(target));

  target->original = source;

  CHECK(quad_forest_init(target, source, tree_max_size, tree_min_size));
  CHECK(pixel_image_copy(target->source, source));

  FINALLY(quad_forest_create);
  RETURN();
}

/******************************************************************************/

result quad_forest_reload
(
  quad_forest *target,
  uint32 tree_max_size,
  uint32 tree_min_size
)
{
  TRY();

  CHECK_POINTER(target);
  CHECK_POINTER(target->original);

  if (target->tree_max_size != tree_max_size || target->tree_min_size != tree_min_size) {
    quad_forest_init(target, target->original, tree_max_size, tree_min_size);
    CHECK(pixel_image_copy(target->source, target->original));
  }

  FINALLY(quad_forest_reload);
  RETURN();
}

/******************************************************************************/

result quad_forest_destroy
(
  quad_forest *target
)
{
  TRY();
  list_item *items, *end;

  CHECK_POINTER(target);

  /* it is necessary to destroy the trees, as they may contain typed pointers */
  items = target->trees.first.next;
  end = &target->trees.last;
  while (items != end) {
    quad_tree_destroy((quad_tree *)items->data);
    items = items->next;
  }
  CHECK(list_destroy(&target->trees));

  items = target->links.first.next;
  end = &target->links.last;
  while (items != end) {
    quad_tree_link_destroy((quad_tree_link*)items->data);
    items = items->next;
  }

  CHECK(list_destroy(&target->links));
  CHECK(memory_deallocate((data_pointer*)&target->roots));
  CHECK(integral_image_destroy(&target->integral));
  if (target->source != NULL) {
    pixel_image_free(target->source);
  }
  CHECK(quad_forest_nullify(target));

  FINALLY(quad_forest_destroy);
  RETURN();
}

/******************************************************************************/

result quad_forest_nullify
(
  quad_forest *target
)
{
  TRY();

  CHECK_POINTER(target);

  target->status = FOREST_UNINITIALIZED;
  target->original = NULL;
  target->source = NULL;
  CHECK(integral_image_nullify(&target->integral));
  target->rows = 0;
  target->cols = 0;
  target->segments = 0;
  target->tree_max_size = 0;
  target->tree_min_size = 0;
  target->dx = 0;
  target->dy = 0;
  CHECK(list_nullify(&target->trees));
  CHECK(list_nullify(&target->links));
  target->last_root_tree = NULL;
  target->roots = NULL;

  FINALLY(quad_forest_nullify);
  RETURN();
}

/******************************************************************************/

truth_value quad_forest_is_null
(
  quad_forest *target
)
{
  if (target != NULL) {
    if (target->original == NULL && target->source == NULL) {
      return TRUE;
    }
  }
  return FALSE;
}

/******************************************************************************/

void quad_forest_set_init(quad_forest *forest)
{
  forest->status = FOREST_INITIALIZED;
}

void quad_forest_set_update(quad_forest *forest)
{
  forest->status = FOREST_INITIALIZED | FOREST_UPDATED;
}

void quad_forest_set_parse(quad_forest *forest)
{
  forest->status |= FOREST_PARSED;
}

truth_value quad_forest_has_parse(quad_forest *forest)
{
  if ((forest->status & FOREST_PARSED) != 0) {
    return TRUE;
  }
  return FALSE;
}

/******************************************************************************/

result quad_forest_update
(
  quad_forest *target
)
{
  TRY();
  uint32 row, col, rows, cols, pos, size, step, stride, hstep, vstep, dstep, offset;
  quad_tree *tree;
  integral_image *I;
  statistics *stat;
  integral_value *iA, *i2A, N, sum1, sum2, mean, var;

  size = target->tree_max_size;
  I = &target->integral;
  N = (integral_value)(size * size);
  step = I->step;
  stride = I->stride;
  hstep = size * step;
  vstep = size * stride;
  dstep = hstep + vstep;

  /* create a fresh copy of the source image in case it has changed */
  /* TODO: need to remove original and force giving the source images as param? */
  /*CHECK(pixel_image_copy(target->source, target->original));*/
  CHECK(integral_image_update(&target->integral));
  /* if there are existing child nodes and blocks, remove them */

  CHECK(list_remove_rest(&target->trees, target->last_root_tree));

  rows = target->rows;
  cols = target->cols;

  pos = 0;
  for (row = 0; row < rows; row++) {
    tree = target->roots[pos];
    /* TODO: calculate offset for first row only, then add vstep */
    offset = (tree->y * stride) + (tree->x * step);
    for (col = 0; col < cols; col++, pos++, offset += hstep) {

      iA = ((integral_value *)I->I_1.data) + offset;
      i2A = ((integral_value *)I->I_2.data) + offset;

      sum1 = *(iA + dstep) + *iA - *(iA + hstep) - *(iA + vstep);
      sum2 = *(i2A + dstep) + *i2A - *(i2A + hstep) - *(i2A + vstep);
      mean = sum1 / N;
      var = sum2 / N - mean*mean;
      if (var < 0) var = 0;

      tree = target->roots[pos];
      stat = &tree->stat;

      stat->N = N;
      stat->sum = sum1;
      stat->sum2 = sum2;
      stat->mean = mean;
      stat->variance = var;
      stat->deviation = sqrt(var);

      tree->nw = NULL;
      tree->ne = NULL;
      tree->sw = NULL;
      tree->se = NULL;

      tree->annotation.token = target->token;
    }
  }

  quad_forest_set_update(target);

  FINALLY(quad_forest_update);
  RETURN();
}

/******************************************************************************/

result quad_forest_get_segments
(
  quad_forest *source,
  segment **target
)
{
  TRY();
  list_item *trees, *end;
  quad_tree *tree;
  segment *tree_segment, *parent;
  uint32 count;

  CHECK_POINTER(source);
  CHECK_POINTER(target);

  if (source->segments == 0) {
    TERMINATE(SUCCESS);
  }

  /* collect all segments to the array */
  count = 0;
  trees = source->trees.first.next;
  end = &source->trees.last;
  while (trees != end) {
    tree = (quad_tree *)trees->data;
    if (tree->nw == NULL) {
      tree_segment = quad_tree_get_segment(tree);
      parent = quad_tree_segment_find(tree);
      if (tree_segment != NULL && parent == tree_segment) {
        target[count] = tree_segment;
        count++;
      }
    }
    trees = trees->next;
  }

  FINALLY(quad_forest_get_segments);
  RETURN();
}

/******************************************************************************/
/* a private function for collecting trees that belong to one of the segments */

void quad_forest_collect_trees
(
  quad_tree *tree,
  list *target,
  segment **segments,
  uint32 segment_count
)
{
  /* if the tree has children, recurse */
  if (tree->nw != NULL) {
    quad_forest_collect_trees(tree->nw, target, segments, segment_count);
    quad_forest_collect_trees(tree->ne, target, segments, segment_count);
    quad_forest_collect_trees(tree->sw, target, segments, segment_count);
    quad_forest_collect_trees(tree->se, target, segments, segment_count);
  }
  else {
    uint32 i;
    segment *tree_segment;

    tree_segment = quad_tree_segment_find(tree);

    /* loop through all segments, if the tree belongs to one of them, add to list */
    for (i = 0; i < segment_count; i++) {
      if (tree_segment == segments[i]) {
        list_append(target, &tree);
        break;
      }
    }
  }
}

/******************************************************************************/

result quad_forest_get_segment_trees
(
  list *target,
  quad_forest *forest,
  segment **segments,
  uint32 segment_count
)
{
  TRY();
  uint32 i, x1, y1, x2, y2;
  uint32 pos, col, firstcol, lastcol, row, firstrow, lastrow;

  CHECK_POINTER(forest);
  CHECK_POINTER(target);
  CHECK_POINTER(segments);

  if (segment_count == 0) {
    TERMINATE(SUCCESS);
  }

  CHECK(list_create(target, 100, sizeof(quad_tree*), 1));

  /* find bounding box of the collection */
  x1 = segments[0]->x1;
  y1 = segments[0]->y1;
  x2 = segments[0]->x2;
  y2 = segments[0]->y2;
  for (i = 1; i < segment_count; i++) {
    if (segments[i]->x1 < x1) x1 = segments[i]->x1;
    if (segments[i]->y1 < y1) y1 = segments[i]->y1;
    if (segments[i]->x2 > x2) x2 = segments[i]->x2;
    if (segments[i]->y2 > y2) y2 = segments[i]->y2;
  }

  /* determine which root trees are within the bounding box */
  firstcol = (uint32)((x1 - forest->dx) / forest->tree_max_size);
  lastcol = (uint32)((x2 - forest->dx) / forest->tree_max_size);
  firstrow = (uint32)((y1 - forest->dy) / forest->tree_max_size);
  lastrow = (uint32)((y2 - forest->dy) / forest->tree_max_size);

  /* loop through the root trees */
  for (row = firstrow; row <= lastrow; row++) {
    pos = row * forest->cols + firstcol;
    for (col = firstcol; col <= lastcol; col++) {
      quad_forest_collect_trees(forest->roots[pos], target, segments, segment_count);
      pos++;
    }
  }

  FINALLY(quad_forest_get_segment_trees);
  RETURN();
}

/******************************************************************************/
/* private function for adding neighboring segments to list                   */

void quad_tree_add_neighbor_segments
(
  list *target,
  quad_tree *tree,
  segment **segments,
  uint32 segment_count,
  direction dir
)
{
  /* in the initial run, the direction is set to whole 4-neighborhood */
  if (dir == d_N4) {
    /* recurse to all direct neighbors that are not NULL */
    if (tree->n != NULL) {
      quad_tree_add_neighbor_segments(target, tree->n, segments, segment_count, d_N);
    }
    if (tree->e != NULL) {
      quad_tree_add_neighbor_segments(target, tree->e, segments, segment_count, d_E);
    }
    if (tree->s != NULL) {
      quad_tree_add_neighbor_segments(target, tree->s, segments, segment_count, d_S);
    }
    if (tree->w != NULL) {
      quad_tree_add_neighbor_segments(target, tree->w, segments, segment_count, d_W);
    }
  }
  else {
    /* if the tree does not have children, check the segments and add */
    if (tree->nw == NULL) {
      uint32 i;
      segment *tree_segment;

      tree_segment = quad_tree_segment_find(tree);
      if (tree_segment != NULL) {
        /* loop through all segments, if the tree doesn't belong to any of them, add to list */
        for (i = 0; i < segment_count; i++) {
          if (tree_segment == segments[i]) {
            goto found;
          }
        }
        /* segment was not found, it is a neighboring segment */
        list_insert_unique(target, &tree_segment, &compare_segments);
      }
      found: ;
      /* otherwise the neighbor belongs to same collection of segments, don't add it */
    }
    /* otherwise need to recurse to children in the given direction */
    else {
      switch (dir) {
      case d_N:
        {
          quad_tree_add_neighbor_segments(target, tree->sw, segments, segment_count, d_N);
          quad_tree_add_neighbor_segments(target, tree->se, segments, segment_count, d_N);
        }
        break;
      case d_E:
        {
          quad_tree_add_neighbor_segments(target, tree->nw, segments, segment_count, d_E);
          quad_tree_add_neighbor_segments(target, tree->sw, segments, segment_count, d_E);
        }
        break;
      case d_S:
        {
          quad_tree_add_neighbor_segments(target, tree->nw, segments, segment_count, d_S);
          quad_tree_add_neighbor_segments(target, tree->ne, segments, segment_count, d_S);
        }
        break;
      case d_W:
        {
          quad_tree_add_neighbor_segments(target, tree->ne, segments, segment_count, d_E);
          quad_tree_add_neighbor_segments(target, tree->se, segments, segment_count, d_E);
        }
        break;
      default:
        /* should never end up here */
        return;
      }
    }
  }
}

/******************************************************************************/

result quad_forest_get_segment_neighbors
(
  list *target,
  quad_forest *forest,
  segment **segments,
  uint32 segment_count
)
{
  TRY();
  list tree_list;
  list_item *trees;
  quad_tree *tree;

  CHECK_POINTER(forest);
  CHECK_POINTER(target);
  CHECK_POINTER(segments);

  list_nullify(&tree_list);

  if (segment_count == 0) {
    TERMINATE(SUCCESS);
  }

  CHECK(list_create(target, 100, sizeof(segment*), 1));

  CHECK(quad_forest_get_segment_trees(&tree_list, forest, segments, segment_count));

  trees = tree_list.first.next;
  while (trees != &tree_list.last) {
    tree = *((quad_tree **)trees->data);
    quad_tree_add_neighbor_segments(target, tree, segments, segment_count, d_N4);
    trees = trees->next;
  }

  FINALLY(quad_forest_get_segment_neighbors);
  if (IS_FALSE(list_is_null(&tree_list))) {
    list_destroy(&tree_list);
  }
  RETURN();
}

/******************************************************************************/

result quad_forest_draw_trees
(
  quad_forest *forest,
  pixel_image *target,
  truth_value use_segments
)
{
  TRY();
  list_item *trees, *end;
  quad_tree *tree;
  uint32 count, x, y, width, height, row_step, pos_step;
  integral_value mean, dev;
  byte *target_pos, value0, value1, value2;

  CHECK_POINTER(forest);
  CHECK_POINTER(target);
  CHECK_PARAM(target->type == p_U8);
  CHECK_PARAM(target->format == RGB);

  trees = forest->trees.first.next;
  end = &forest->trees.last;
  count = 0;
  while (trees != end) {
    tree = (quad_tree*)trees->data;
    if (tree->nw == NULL) {
      width = tree->size;
      height = width;
      row_step = target->stride - width * target->step;
      if (target->step <= 3) {
        pos_step = 1;
      }
      else {
        pos_step = target->step - 2;
      }

      if (IS_FALSE(use_segments)) {
        mean = tree->stat.mean;
        if (mean < 0) mean = 0;
        if (mean > 255) mean = 255;
        value1 = value2 = (byte)mean;
        dev = mean + tree->stat.deviation;
        if (dev < 0) dev = 0;
        if (dev > 255) dev = 255;
        value0 = (byte)dev;
      }
      else {
        segment *tree_segment = quad_tree_segment_find(tree);
        if (tree_segment != NULL) {
          value0 = tree_segment->color[0];
          value1 = tree_segment->color[1];
          value2 = tree_segment->color[2];
        }
        else {
          value0 = value1 = value2 = 0;
        }
      }

      target_pos = (byte*)target->data + tree->y * target->stride + tree->x * target->step;
      for (y = height; y--; target_pos += row_step) {
        for (x = width; x--; ) {
          *target_pos = value0;
          target_pos++;
          *target_pos = value1;
          target_pos++;
          *target_pos = value2;
          target_pos += pos_step;
        }
      }
      count++;
    }
    trees = trees->next;
  }

  FINALLY(quad_forest_draw_trees);
  RETURN();
}

/******************************************************************************/
/* private function for drawing recursively the trees into the bitmask        */

void quad_forest_draw_segments
(
  quad_tree *tree,
  pixel_image *target,
  uint32 dx,
  uint32 dy,
  segment **segments,
  uint32 segment_count,
  byte color[4],
  uint32 channels
)
{
  if (tree->nw != NULL) {
    quad_forest_draw_segments(tree->nw, target, dx, dy, segments, segment_count, color, channels);
    quad_forest_draw_segments(tree->ne, target, dx, dy, segments, segment_count, color, channels);
    quad_forest_draw_segments(tree->sw, target, dx, dy, segments, segment_count, color, channels);
    quad_forest_draw_segments(tree->se, target, dx, dy, segments, segment_count, color, channels);
  }
  else {
    uint32 i, x, y, width, height, row_step;
    byte *target_pos;
    segment *tree_segment;

    tree_segment = quad_tree_segment_find(tree);

    /* loop through all segments, if the tree belongs to one of them, draw the pixels */
    if (channels == 1) {
      for (i = 0; i < segment_count; i++) {
        if (tree_segment == segments[i]) {
          width = tree->size;
          height = width;
          row_step = target->stride - width;
          target_pos = (byte*)target->data + (tree->y - dy) * target->stride + (tree->x - dx);
          for (y = 0; y < height; y++, target_pos += row_step) {
            for (x = 0; x < width; x++) {
              *target_pos = color[0];
              target_pos++;
            }
          }
          break;
        }
      }
    }
    else
    if (channels == 3) {
      for (i = 0; i < segment_count; i++) {
        if (tree_segment == segments[i]) {
          width = tree->size;
          height = width;
          row_step = target->stride - width * target->step;
          target_pos = (byte*)target->data + (tree->y - dy) * target->stride + (tree->x - dx) * target->step;
          for (y = 0; y < height; y++, target_pos += row_step) {
            for (x = 0; x < width; x++) {
              *target_pos = color[0];
              target_pos++;
              *target_pos = color[1];
              target_pos++;
              *target_pos = color[2];
              target_pos++;
            }
          }
          break;
        }
      }
    }
  }
}

/******************************************************************************/

result quad_forest_get_segment_mask
(
  quad_forest *forest,
  pixel_image *target,
  segment **segments,
  uint32 segment_count,
  truth_value invert
)
{
  TRY();
  uint32 i, x1, y1, x2, y2, width, height;
  byte value[4];

  CHECK_POINTER(forest);
  CHECK_POINTER(target);
  CHECK_POINTER(segments);

  if (segment_count == 0) {
    TERMINATE(SUCCESS);
  }

  /*
  loop all root trees under the segment bounding box
  recurse to leaf trees
  if parent is one of the segments, draw pixels to image
  */
  /* find bounding box of the collection */
  x1 = segments[0]->x1;
  y1 = segments[0]->y1;
  x2 = segments[0]->x2;
  y2 = segments[0]->y2;
  for (i = 1; i < segment_count; i++) {
    if (segments[i]->x1 < x1) x1 = segments[i]->x1;
    if (segments[i]->y1 < y1) y1 = segments[i]->y1;
    if (segments[i]->x2 > x2) x2 = segments[i]->x2;
    if (segments[i]->y2 > y2) y2 = segments[i]->y2;
  }

  /* create the image */
  width = x2 - x1;
  height = y2 - y1;
  CHECK(pixel_image_create(target, p_U8, GREY, width, height, 1, width));

  if (IS_FALSE(invert)) {
    CHECK(pixel_image_clear(target));
    value[0] = 255;
  }
  else {
    SINGLE_CONTINUOUS_IMAGE_VARIABLES(target, byte);
    FOR_CONTINUOUS_IMAGE(target)
      PIXEL_VALUE(target) = 255;
    value[0] = 0;
  }

  {
    uint32 pos, col, firstcol, lastcol, row, firstrow, lastrow;

    /* determine which root trees are within the bounding box */
    firstcol = (uint32)((x1 - forest->dx) / forest->tree_max_size);
    lastcol = (uint32)((x2 - forest->dx) / forest->tree_max_size);
    firstrow = (uint32)((y1 - forest->dy) / forest->tree_max_size);
    lastrow = (uint32)((y2 - forest->dy) / forest->tree_max_size);

    /* loop through the root trees */
    for (row = firstrow; row <= lastrow; row++) {
      pos = row * forest->cols + firstcol;
      for (col = firstcol; col <= lastcol; col++) {
        quad_forest_draw_segments(forest->roots[pos], target, x1, y1, segments, segment_count, value, 1);
        pos++;
      }
    }
  }

  FINALLY(quad_forest_get_segment_mask);
  RETURN();
}

/******************************************************************************/

#define TRY_SW()\
  if (tree->s != NULL && tree->s->w != NULL) {\
    new_tree = tree->s->w;\
    new_segment = quad_tree_segment_find(new_tree);\
    if (new_segment == tree_segment) {\
      *next_tree = new_tree;\
      *next_dir = d_SW;\
      break;\
    }\
  }

#define TRY_W()\
  if (tree->w != NULL) {\
    new_tree = tree->w;\
    new_segment = quad_tree_segment_find(new_tree);\
    if (new_segment == tree_segment) {\
      *next_tree = new_tree;\
      *next_dir = d_W;\
      break;\
    }\
  }

#define TRY_NW()\
  if (tree->n != NULL && tree->n->w != NULL) {\
    new_tree = tree->n->w;\
    new_segment = quad_tree_segment_find(new_tree);\
    if (new_segment == tree_segment) {\
      *next_tree = new_tree;\
      *next_dir = d_NW;\
      break;\
    }\
  }

#define TRY_N()\
  if (tree->n != NULL) {\
    new_tree = tree->n;\
    new_segment = quad_tree_segment_find(new_tree);\
    if (new_segment == tree_segment) {\
      *next_tree = new_tree;\
      *next_dir = d_N;\
      break;\
    }\
  }

#define TRY_NE()\
  if (tree->n != NULL && tree->n->e != NULL) {\
    new_tree = tree->n->e;\
    new_segment = quad_tree_segment_find(new_tree);\
    if (new_segment == tree_segment) {\
      *next_tree = new_tree;\
      *next_dir = d_NE;\
      break;\
    }\
  }

#define TRY_E()\
  if (tree->e != NULL) {\
    new_tree = tree->e;\
    new_segment = quad_tree_segment_find(new_tree);\
    if (new_segment == tree_segment) {\
      *next_tree = new_tree;\
      *next_dir = d_E;\
      break;\
    }\
  }

#define TRY_SE()\
  if (tree->s != NULL && tree->s->e != NULL) {\
    new_tree = tree->s->e;\
    new_segment = quad_tree_segment_find(new_tree);\
    if (new_segment == tree_segment) {\
      *next_tree = new_tree;\
      *next_dir = d_SE;\
      break;\
    }\
  }

#define TRY_S()\
  if (tree->s != NULL) {\
    new_tree = tree->s;\
    new_segment = quad_tree_segment_find(new_tree);\
    if (new_segment == tree_segment) {\
      *next_tree = new_tree;\
      *next_dir = d_S;\
      break;\
    }\
  }

void get_next
(
  quad_tree *tree,
  segment *tree_segment,
  direction arrival_dir,
  quad_tree **next_tree,
  direction *next_dir
)
{
  quad_tree *new_tree;
  segment *new_segment;

  switch (arrival_dir) {
    case d_NW:
    {
      TRY_SW()
      TRY_W()
      TRY_NW()
      TRY_N()
      TRY_NE()
      TRY_E()
      TRY_SE()
      TRY_S()
      *next_dir = d_UNDEF;
      PRINT0("not found: nw");
      break;
    }
    case d_N:
    {
      TRY_NW()
      TRY_N()
      TRY_NE()
      TRY_E()
      TRY_SE()
      TRY_S()
      TRY_SW()
      TRY_W()
      *next_dir = d_UNDEF;
      PRINT0("not found: w");
      break;
    }
    case d_NE:
    {
      TRY_NW()
      TRY_N()
      TRY_NE()
      TRY_E()
      TRY_SE()
      TRY_S()
      TRY_SW()
      TRY_W()
      *next_dir = d_UNDEF;
      PRINT0("not found: nw");
      break;
    }
    case d_E:
    {
      TRY_NE()
      TRY_E()
      TRY_SE()
      TRY_S()
      TRY_SW()
      TRY_W()
      TRY_NW()
      TRY_N()
      *next_dir = d_UNDEF;
      PRINT0("not found: e");
      break;
    }
    case d_SE:
    {
      TRY_NE()
      TRY_E()
      TRY_SE()
      TRY_S()
      TRY_SW()
      TRY_W()
      TRY_NW()
      TRY_N()
      *next_dir = d_UNDEF;
      PRINT0("not found: se");
      break;
    }
    case d_S:
    {
      TRY_SE()
      TRY_S()
      TRY_SW()
      TRY_W()
      TRY_NW()
      TRY_N()
      TRY_NE()
      TRY_E()
      *next_dir = d_UNDEF;
      PRINT0("not found: s");
      break;
    }
    case d_SW:
    {
      TRY_SE()
      TRY_S()
      TRY_SW()
      TRY_W()
      TRY_NW()
      TRY_N()
      TRY_NE()
      TRY_E()
      *next_dir = d_UNDEF;
      PRINT0("not found: sw");
      break;
    }
    case d_W:
    {
      TRY_SW()
      TRY_W()
      TRY_NW()
      TRY_N()
      TRY_NE()
      TRY_E()
      TRY_SE()
      TRY_S()
      *next_dir = d_UNDEF;
      PRINT0("not found: w");
      break;
    }
    default:
      *next_dir = d_UNDEF;
      PRINT0("wrong direction!");
  }
}

#define POINT_LEFT(tree) {\
  point_b.x = (signed)tree->x;\
  point_b.y = (signed)tree->y + ((sint32)(tree->size / 2));\
  new_line.start = point_a;\
  new_line.end = point_b;\
  list_append(segment_boundary, (pointer)&new_line);\
  point_a = point_b;}

#define POINT_TOP(tree) {\
  point_b.x = (signed)tree->x + ((sint32)(tree->size / 2));\
  point_b.y = (signed)tree->y;\
  new_line.start = point_a;\
  new_line.end = point_b;\
  list_append(segment_boundary, (pointer)&new_line);\
  point_a = point_b;}

#define POINT_RIGHT(tree) {\
  point_b.x = (signed)(tree->x + tree->size);\
  point_b.y = (signed)tree->y + ((sint32)(tree->size / 2));\
  new_line.start = point_a;\
  new_line.end = point_b;\
  list_append(segment_boundary, (pointer)&new_line);\
  point_a = point_b;}

#define POINT_BOTTOM(tree) {\
  point_b.x = (signed)tree->x + ((sint32)(tree->size / 2));\
  point_b.y = (signed)(tree->y + tree->size);\
  new_line.start = point_a;\
  new_line.end = point_b;\
  list_append(segment_boundary, (pointer)&new_line);\
  point_a = point_b;}

result quad_forest_get_segment_boundary
(
  quad_forest *forest,
  segment *input_segment,
  list *segment_boundary
)
{
  uint32 row, col, pos;
  quad_tree *tree, *next_tree, *end_tree;
  segment *tree_segment;
  direction prev_dir, next_dir;
  point start_point, point_a, point_b;
  line new_line;
  TRY();

  CHECK_POINTER(forest);
  CHECK_POINTER(input_segment);
  CHECK_POINTER(segment_boundary);

  CHECK(list_create(segment_boundary, 100, sizeof(line), 1));

  if (input_segment->x2 - input_segment->x1 > 33 && input_segment->y2 - input_segment->y1 > 32) {
    /* find the tree in center left of bounding box */
    col = (uint32)((input_segment->x1 - forest->dx) / forest->tree_max_size);
    row = (uint32)((((uint32)((input_segment->y1 + input_segment->y2) / 2)) - forest->dy) / forest->tree_max_size);
    pos = row * forest->cols + col;

    tree = forest->roots[pos];
    tree_segment = quad_tree_segment_find(tree);
    while (tree_segment != input_segment) {
      if (tree->e != NULL) {
        tree = tree->e;
        tree_segment = quad_tree_segment_find(tree);
      }
      else {
        /*printf("%lu %lu %lu %lu %lu %lu\n", segment->x1, segment->y1, segment->x2, segment->y2, row, col);*/
        TERMINATE(SUCCESS); /*NOT_FOUND*/
      }
    }

    point_a.x = (signed)tree->x;
    point_a.y = (signed)tree->y + ((sint32)(tree->size / 2));
    start_point = point_a;
    end_tree = tree;
    /* use nw as first dir to get the correct direction for searching */
    prev_dir = d_NE;
    /*get_next(tree, segment, prev_dir, &next_tree, &next_dir);*/
    /*prev_dir = next_dir;*/
    do {
      get_next(tree, input_segment, prev_dir, &next_tree, &next_dir);
      switch (next_dir) {
        case d_NW:
        {
          switch (prev_dir) {
            case d_S:
              POINT_RIGHT(tree);
            case d_SW:
            case d_W:
              POINT_BOTTOM(tree);
              break;
            case d_NW:
            case d_N:
            case d_NE:
              POINT_LEFT(tree);
              break;
            default: ;
              /*PRINT1("incorrect prev_dir with d_NW: %d", prev_dir);*/
          }
        }
        break;
        case d_N:
        {
          switch (prev_dir) {
            case d_S:
              POINT_RIGHT(tree);
            case d_SW:
            case d_W:
              POINT_BOTTOM(tree);
            case d_NW:
            case d_N:
            case d_NE:
              POINT_LEFT(tree);
              break;
            default: ;
              /*PRINT1("incorrect prev_dir with d_N: %d", prev_dir);*/
              /* TODO: maybe add d_E creating a sharp corner? */
          }
        }
        break;
        case d_NE:
        {
          switch (prev_dir) {
            case d_W:
              POINT_BOTTOM(tree);
            case d_NW:
            case d_N:
            case d_NE:
              POINT_LEFT(tree);
              break;
            case d_E:
            case d_SE:
              POINT_TOP(tree);
              break;
            default: ;
              /*PRINT1("incorrect prev_dir with d_NE: %d", prev_dir);*/
          }
        }
        break;
        case d_E:
        {
          switch (prev_dir) {
            case d_W:
              POINT_BOTTOM(tree);
            case d_NW:
            case d_N:
              POINT_LEFT(tree);
            case d_NE:
            case d_E:
            case d_SE:
              POINT_TOP(tree);
              break;
            default: ;
              /*PRINT1("incorrect prev_dir with d_E: %d", prev_dir);*/
              /* maybe add d_S creating a sharp corner? */
          }
        }
        break;
        case d_SE:
        {
          switch (prev_dir) {
            case d_N:
              POINT_LEFT(tree);
            case d_NE:
            case d_E:
            case d_SE:
              POINT_TOP(tree);
              break;
            case d_S:
            case d_SW:
              POINT_RIGHT(tree);
              break;
            default: ;
              /*PRINT1("incorrect prev_dir with d_SE: %d", prev_dir);*/
          }
        }
        break;
        case d_S:
        {
          switch (prev_dir) {
            case d_N:
              POINT_LEFT(tree);
            case d_NE:
            case d_E:
              POINT_TOP(tree);
            case d_SE:
            case d_S:
            case d_SW:
              POINT_RIGHT(tree);
              break;
            default: ;
              /*PRINT1("incorrect prev_dir with d_S: %d", prev_dir);*/
          }
        }
        break;
        case d_SW:
        {
          switch (prev_dir) {
            case d_E:
              POINT_TOP(tree);
            case d_SE:
            case d_S:
            case d_SW:
              POINT_RIGHT(tree);
              break;
            case d_W:
            case d_NW:
              POINT_BOTTOM(tree);
              break;
            default: ;
              /*PRINT1("incorrect prev_dir with d_SW: %d", prev_dir);*/
          }
        }
        break;
        case d_W:
        {
          switch (prev_dir) {
            case d_E:
              POINT_TOP(tree);
            case d_SE:
            case d_S:
              POINT_RIGHT(tree);
            case d_SW:
            case d_W:
            case d_NW:
              POINT_BOTTOM(tree);
              break;
            default: ;
              /*PRINT1("incorrect prev_dir with d_W: %d", prev_dir);*/
          }
        }
        break;
        default: ;
          /*PRINT1("incorrect next_dir: %d", next_dir);*/
      }
      if (next_dir == d_UNDEF) break;
      tree = next_tree;
      prev_dir = next_dir;
    } while (tree != end_tree);

    new_line.start = point_a;
    new_line.end = start_point;
    list_append(segment_boundary, (pointer)&new_line);
  }
  FINALLY(quad_forest_get_segment_boundary);
  RETURN();
}

/******************************************************************************/

result quad_forest_get_links
(
  quad_forest *forest,
  list *links,
  link_visualization_mode mode
)
{
  TRY();
  list_item *items, *end;
  quad_tree *tree;
  quad_tree_link *link;
  uint32 size;
  /*integral_value d;*/
  weighted_line new_line;
  colored_line color_line;

  CHECK_POINTER(forest);

  /* caller should provide a list - should start using typed pointers so could */
  /* check that the list has the correct type */
  /* CHECK(list_create(links, forest->links.count * 2, sizeof(weighted_line), 1)); */

  if (mode == v_LINK_DISTANCE) {
    items = forest->links.first.next;
    end = &forest->links.last;
    while (items != end) {
      link = (quad_tree_link*)items->data;
      tree = link->a.tree;
      size = (uint32)(tree->size / 2);
      new_line.start.x = (signed)(tree->x + size);
      new_line.start.y = (signed)(tree->y + size);
      tree = link->b.tree;
      size = (uint32)(tree->size / 2);
      new_line.end.x = (signed)(tree->x + size);
      new_line.end.y = (signed)(tree->y + size);
      new_line.weight = 1 / link->distance;
      CHECK(list_append(links, (pointer)&new_line));
      items = items->next;
    }
  }
  else
  if (mode == v_LINK_ANGLE_COST) {
    /*
    boundary_strength *bstrength;
    items = forest->links.first.next;
    end = &forest->links.last;
    while (items != end) {
      link = (quad_tree_link*)items->data;
      bstrength = has_boundary_strength(&link->annotation, forest->token);
      if (bstrength != NULL) {
        tree = link->a.tree;
        size = (uint32)(tree->size / 2);
        new_line.start.x = (signed)(tree->x + size);
        new_line.start.y = (signed)(tree->y + size);
        tree = link->b.tree;
        size = (uint32)(tree->size / 2);
        new_line.end.x = (signed)(tree->x + size);
        new_line.end.y = (signed)(tree->y + size);
        new_line.weight = (1 - bstrength->angle_score);
        CHECK(list_append(links, (pointer)&new_line));
      }
      items = items->next;
    }
    */
  }
  else
  if (mode == v_LINK_SIMILARITY) {
    /*
    segment_strength *sstrength;
    items = forest->links.first.next;
    end = &forest->links.last;
    while (items != end) {
      link = (quad_tree_link*)items->data;
      sstrength = has_segment_strength(&link->annotation, forest->token);
      if (sstrength != NULL) {
        tree = link->a.tree;
        size = (uint32)(tree->size / 2);
        new_line.start.x = (signed)(tree->x + size);
        new_line.start.y = (signed)(tree->y + size);
        tree = link->b.tree;
        size = (uint32)(tree->size / 2);
        new_line.end.x = (signed)(tree->x + size);
        new_line.end.y = (signed)(tree->y + size);
        new_line.weight = sstrength->overlap;
        CHECK(list_append(links, (pointer)&new_line));
      }
      items = items->next;
    }
    */
  }
  else
  if (mode == v_LINK_MEASURE) {
    quad_tree_link_head *head;
    link_measure *lmeasure;
    integral_value radius, str, max_str;
    uint32 score;
    sint32 x, y, dx, dy;
    /*
    max_str = 0;
    items = forest->links.first.next;
    end = &forest->links.last;
    while (items != end) {
      link = (quad_tree_link*)items->data;
      head = &link->a;
      lmeasure = has_link_measure(&head->annotation, forest->token);
      if (lmeasure != NULL &&
          (lmeasure->category == bl_TOWARDS || lmeasure->category == bl_AGAINST)) {
        str = lmeasure->angle_score * lmeasure->straightness_score;
        if (str > max_str) max_str = str;
      }
      items = items->next;
    }
    */
    items = forest->links.first.next;
    end = &forest->links.last;
    while (items != end) {
      link = (quad_tree_link*)items->data;
      head = &link->a;
      lmeasure = has_link_measure(&head->annotation, forest->token);
      if (lmeasure != NULL) {
        tree = head->tree;
        radius = ((integral_value)tree->size) / 2.0;
        x = getlround((integral_value)tree->x + radius);
        y = getlround((integral_value)tree->y + radius);
        dx = getlround(cos(head->angle) * radius);
        dy = getlround(sin(head->angle) * radius);

        color_line.start.x = x;
        color_line.start.y = y;
        color_line.end.x = x + dx;
        color_line.end.y = y - dy;
        if (IS_PARALLEL(lmeasure->category)) {
          /*score = (uint32)(lmeasure->parallel_score * 255.0);*/
          /*score = (uint32)((1 - lmeasure->strength_score) * 255.0);*/
          score = (uint32)((1 - lmeasure->angle_score) * 255.0);
          if (score > 255) score = 255;
          color_line.color[0] = 0;
          color_line.color[1] = (byte)score;
          color_line.color[2] = (byte)score;
          CHECK(list_append(links, (pointer)&color_line));
        }
        else {
          /*score = (uint32)(lmeasure->perpendicular_score * 255.0);*/
          score = (uint32)(lmeasure->strength_score * 255.0);
          if (score > 255) score = 255;
          color_line.color[0] = (byte)score;
          color_line.color[1] = (byte)score;
          color_line.color[2] = 0;
          /*CHECK(list_append(links, (pointer)&color_line));*/
        }
      }
      head = &link->b;
      lmeasure = has_link_measure(&head->annotation, forest->token);
      if (lmeasure != NULL) {
        tree = head->tree;
        radius = ((integral_value)tree->size) / 2.0;
        x = getlround((integral_value)tree->x + radius);
        y = getlround((integral_value)tree->y + radius);
        dx = getlround(cos(head->angle) * radius);
        dy = getlround(sin(head->angle) * radius);

        color_line.start.x = x;
        color_line.start.y = y;
        color_line.end.x = x + dx;
        color_line.end.y = y - dy;
        if (IS_PARALLEL(lmeasure->category)) {
          /*score = (uint32)(lmeasure->parallel_score * 255.0);*/
          /*score = (uint32)((1 - lmeasure->strength_score) * 255.0);*/
          score = (uint32)((1 - lmeasure->angle_score) * 255.0);
          if (score > 255) score = 255;
          color_line.color[0] = 0;
          color_line.color[1] = (byte)score;
          color_line.color[2] = (byte)score;
          CHECK(list_append(links, (pointer)&color_line));
        }
        else {
          /*score = (uint32)((lmeasure->perpendicular_score) * 255.0);*/
          score = (uint32)((lmeasure->strength_score) * 255.0);
          if (score > 255) score = 255;
          color_line.color[0] = (byte)score;
          color_line.color[1] = (byte)score;
          color_line.color[2] = 0;
          /*CHECK(list_append(links, (pointer)&color_line));*/
        }
      }
      items = items->next;
    }
  }
  else
  if (mode == v_LINK_STRENGTH) {
    /*
    quad_tree_link_head *head;
    boundary_potential *bstrength;
    integral_value radius, max_strength;
    sint32 x, y, dx, dy;

    max_strength = 0;
    items = forest->links.first.next;
    end = &forest->links.last;
    while (items != end) {
      link = (quad_tree_link*)items->data;
      head = &link->a;
      bstrength = has_boundary_potential(&head->annotation, forest->token);
      if (bstrength != NULL) {
        if (bstrength->strength_score > max_strength) {
          max_strength = bstrength->strength_score;
        }
      }
      head = &link->b;
      bstrength = has_boundary_potential(&head->annotation, forest->token);
      if (bstrength != NULL) {
        if (bstrength->strength_score > max_strength) {
          max_strength = bstrength->strength_score;
        }
      }
      items = items->next;
    }
    items = forest->links.first.next;
    end = &forest->links.last;
    while (items != end) {
      link = (quad_tree_link*)items->data;
      head = &link->a;
      bstrength = has_boundary_potential(&head->annotation, forest->token);
      if (bstrength != NULL) {
        tree = head->tree;
        radius = ((integral_value)tree->size) / 2.0;
        x = getlround((integral_value)tree->x + radius);
        y = getlround((integral_value)tree->y + radius);
        dx = getlround(cos(head->angle) * radius);
        dy = getlround(sin(head->angle) * radius);

        new_line.start.x = x;
        new_line.start.y = y;
        new_line.end.x = x + dx;
        new_line.end.y = y - dy;
        new_line.weight = bstrength->strength_score / max_strength;
        if (new_line.weight < 0) {
          new_line.weight = 0;
        }

        CHECK(list_append(links, (pointer)&new_line));
      }
      head = &link->b;
      bstrength = has_boundary_potential(&head->annotation, forest->token);
      if (bstrength != NULL) {
        tree = head->tree;
        radius = ((integral_value)tree->size) / 2.0;
        x = getlround((integral_value)tree->x + radius);
        y = getlround((integral_value)tree->y + radius);
        dx = getlround(cos(head->angle) * radius);
        dy = getlround(sin(head->angle) * radius);

        new_line.start.x = x;
        new_line.start.y = y;
        new_line.end.x = x + dx;
        new_line.end.y = y - dy;
        new_line.weight = bstrength->strength_score / max_strength;
        if (new_line.weight < 0) {
          new_line.weight = 0;
        }

        CHECK(list_append(links, (pointer)&new_line));
      }
      items = items->next;
    }
   */
  }
  else
  if (mode == v_LINK_EDGE) {
    quad_tree_link_head *head;
    edge_links *elinks;
    integral_value radius;
    sint32 x, y, dx, dy;

    items = forest->trees.first.next;
    end = &forest->trees.last;
    while (items != end) {
      tree = (quad_tree*)items->data;
      elinks = has_edge_links(&tree->annotation, forest->token);
      if (elinks != NULL) {
        if (elinks->towards != NULL) {
          head = elinks->towards;
          radius = ((integral_value)tree->size) / 2.0;
          x = getlround((integral_value)tree->x + radius);
          y = getlround((integral_value)tree->y + radius);
          dx = getlround(cos(head->angle) * radius);
          dy = getlround(sin(head->angle) * radius);

          new_line.start.x = x;
          new_line.start.y = y;
          new_line.end.x = x + dx;
          new_line.end.y = y - dy;
          new_line.weight = 1;
          CHECK(list_append(links, (pointer)&new_line));
        }
        if (elinks->against != NULL) {
          head = elinks->against;
          radius = ((integral_value)tree->size) / 2.0;
          x = getlround((integral_value)tree->x + radius);
          y = getlround((integral_value)tree->y + radius);
          dx = getlround(cos(head->angle) * radius);
          dy = getlround(sin(head->angle) * radius);

          new_line.start.x = x;
          new_line.start.y = y;
          new_line.end.x = x + dx;
          new_line.end.y = y - dy;
          new_line.weight = 0.75;
          CHECK(list_append(links, (pointer)&new_line));
        }
        /*
        if (elinks->other != NULL) {
          head = elinks->other;
          radius = ((integral_value)tree->size) / 2.0;
          x = getlround((integral_value)tree->x + radius);
          y = getlround((integral_value)tree->y + radius);
          dx = getlround(cos(head->angle) * radius);
          dy = getlround(sin(head->angle) * radius);

          new_line.start.x = x;
          new_line.start.y = y;
          new_line.end.x = x + dx;
          new_line.end.y = y - dy;
          new_line.weight = 0.5;
          CHECK(list_append(links, (pointer)&new_line));
        }
        */
      }
      items = items->next;
    }
  }
  else
  if (mode == v_LINK_STRAIGHT) {
    quad_tree_link_head *head;
    edge_links *elinks;
    edge_response *eresp;
    integral_value angle, radius;
    sint32 x, y, dx, dy;

    items = forest->trees.first.next;
    end = &forest->trees.last;
    while (items != end) {
      tree = (quad_tree*)items->data;
      elinks = has_edge_links(&tree->annotation, forest->token);
      if (elinks != NULL) {
        radius = ((integral_value)tree->size) / 2.0;
        x = getlround((integral_value)tree->x + radius);
        y = getlround((integral_value)tree->y + radius);
        new_line.start.x = x;
        new_line.start.y = y;

        eresp = has_edge_response(&tree->annotation, forest->token);
        if (eresp != NULL) {
          angle = eresp->ang - M_PI_2;
          if (angle < 0) angle += 2 * M_PI;
          dx = getlround(cos(angle) * radius);
          dy = getlround(sin(angle) * radius);
          new_line.end.x = x + dx;
          new_line.end.y = y - dy;
          new_line.weight = 0;
          CHECK(list_append(links, (pointer)&new_line));
          angle -= M_PI;
          if (angle < 0) angle += 2 * M_PI;
          dx = getlround(cos(angle) * radius);
          dy = getlround(sin(angle) * radius);
          new_line.end.x = x + dx;
          new_line.end.y = y - dy;
          new_line.weight = 0;
          /*CHECK(list_append(links, (pointer)&new_line));*/
        }
        angle = elinks->own_angle;
        dx = getlround(cos(angle) * radius);
        dy = getlround(sin(angle) * radius);
        new_line.end.x = x + dx;
        new_line.end.y = y - dy;
        new_line.weight = elinks->straightness;
        /*CHECK(list_append(links, (pointer)&new_line));*/
        angle -= M_PI;
        if (angle < 0) angle += 2 * M_PI;
        dx = getlround(cos(angle) * radius);
        dy = getlround(sin(angle) * radius);
        new_line.end.x = x + dx;
        new_line.end.y = y - dy;
        new_line.weight = elinks->straightness;
        /*CHECK(list_append(links, (pointer)&new_line));*/

        angle = elinks->towards_angle;
        dx = getlround(cos(angle) * radius);
        dy = getlround(sin(angle) * radius);
        new_line.end.x = x + dx;
        new_line.end.y = y - dy;
        new_line.weight = elinks->towards_consistency;
        CHECK(list_append(links, (pointer)&new_line));

        angle = elinks->against_angle;
        angle -= M_PI;
        if (angle < 0) angle += 2 * M_PI;
        dx = getlround(cos(angle) * radius);
        dy = getlround(sin(angle) * radius);
        new_line.end.x = x + dx;
        new_line.end.y = y - dy;
        new_line.weight = elinks->against_consistency;
        CHECK(list_append(links, (pointer)&new_line));
      }
      items = items->next;
    }
  }
    /*
    new_line.start.x = tree->x + (uint32)(tree->size / 2);
    new_line.start.y = tree->y + (uint32)(tree->size / 2);
    d = (integral_value)(tree->size / 2);
    new_line.end.x = new_line.start.x + (uint32)(d * cos(link->a.angle));
    new_line.end.y = new_line.start.y - (uint32)(d * sin(link->a.angle));

    new_line.weight = 1;
    CHECK(list_append(links, (pointer)&new_line));

    tree = link->b.tree;
    new_line.start.x = tree->x + (uint32)(tree->size / 2);
    new_line.start.y = tree->y + (uint32)(tree->size / 2);
    d = (integral_value)(tree->size / 2);
    new_line.end.x = new_line.start.x + (uint32)(d * cos(link->b.angle));
    new_line.end.y = new_line.start.y - (uint32)(d * sin(link->b.angle));

    new_line.weight = 0.5;
    CHECK(list_append(links, (pointer)&new_line));
    */

/*
  items = forest->trees.first.next;
  end = &forest->trees.last;
  while (items != end) {
    tree = (quad_tree*)items->data;
    if (tree->context.token != 0) {
      new_line.start.x = tree->x + (uint32)(tree->size / 2);
      new_line.start.y = tree->y + (uint32)(tree->size / 2);
      d = (integral_value)(tree->size);
      new_line.end.x = new_line.start.x + (uint32)(d * cos(tree->edge.ang));
      new_line.end.y = new_line.start.y - (uint32)(d * sin(tree->edge.ang));
      if (tree->stat.deviation < getmax(1, tree->segment.devmean)) {
        new_line.weight = 0;
      }
      else {
        new_line.weight = 1;
      }
      CHECK(list_append(links, (pointer)&new_line));
    }
    items = items->next;
  }
*/
  FINALLY(quad_forest_get_links);
  RETURN();
}

/******************************************************************************/

result quad_forest_highlight_segments
(
  quad_forest *forest,
  pixel_image *target,
  segment **segments,
  uint32 segment_count,
  byte color[4]
)
{
  TRY();
  uint32 i, x1, y1, x2, y2;

  CHECK_POINTER(forest);
  CHECK_POINTER(target);
  CHECK_POINTER(segments);

  if (segment_count == 0) {
    TERMINATE(SUCCESS);
  }

  /* find bounding box of the collection */
  x1 = segments[0]->x1;
  y1 = segments[0]->y1;
  x2 = segments[0]->x2;
  y2 = segments[0]->y2;
  for (i = 1; i < segment_count; i++) {
    if (segments[i]->x1 < x1) x1 = segments[i]->x1;
    if (segments[i]->y1 < y1) y1 = segments[i]->y1;
    if (segments[i]->x2 > x2) x2 = segments[i]->x2;
    if (segments[i]->y2 > y2) y2 = segments[i]->y2;
  }

  {
    uint32 pos, col, firstcol, lastcol, row, firstrow, lastrow;

    /* determine which root trees are within the bounding box */
    firstcol = (uint32)((x1 - forest->dx) / forest->tree_max_size);
    lastcol = (uint32)((x2 - forest->dx) / forest->tree_max_size);
    firstrow = (uint32)((y1 - forest->dy) / forest->tree_max_size);
    lastrow = (uint32)((y2 - forest->dy) / forest->tree_max_size);

    /* loop through the root trees */
    for (row = firstrow; row <= lastrow; row++) {
      pos = row * forest->cols + firstcol;
      for (col = firstcol; col <= lastcol; col++) {
        /*printf("(%lu %lu) ", row, col);*/
        quad_forest_draw_segments(forest->roots[pos], target, 0, 0, segments, segment_count, color, 3);
        pos++;
      }
    }
  }

  FINALLY(quad_forest_highlight_segments);
  RETURN();
}

/******************************************************************************/

result quad_forest_draw_image
(
  quad_forest *forest,
  pixel_image *target,
  truth_value use_segments,
  truth_value use_colors
)
{
  TRY();
  list_item *trees;
  quad_tree *tree;
  segment *parent;
  uint32 x, y, width, height, stride, row_step;
  byte *target_data, *target_pos, color0, color1, color2;

  CHECK_POINTER(forest);
  CHECK_POINTER(forest->source);
  CHECK_POINTER(target);

  width = forest->source->width;
  height = forest->source->height;

  CHECK(pixel_image_create(target, p_U8, RGB, width, height, 3, 3 * width));
  CHECK(pixel_image_clear(target));

  stride = target->stride;
  target_data = (byte*)target->data;

  /* draw using tree mean value */
  if (IS_FALSE(use_segments)) {
    statistics *stat;
    trees = forest->trees.first.next;
    while (trees != &forest->trees.last) {
      tree = (quad_tree *)trees->data;
      if (tree->nw == NULL) {
        stat = &tree->stat;
        /* TODO: maybe could create only a grayscale image..? */
        color0 = (byte)stat->mean;
        width = tree->size;
        height = width;
        row_step = stride - 3 * width;
        target_pos = target_data + tree->y * stride + tree->x * 3;
        for (y = 0; y < height; y++, target_pos += row_step) {
          for (x = 0; x < width; x++) {
            *target_pos = color0;
            target_pos++;
            *target_pos = color0;
            target_pos++;
            *target_pos = color0;
            target_pos++;
          }
        }
      }
      trees = trees->next;
    }
  }
  else {
    /* draw using region mean value */
    if (IS_FALSE(use_colors)) {
      trees = forest->trees.first.next;
      while (trees != &forest->trees.last) {
        tree = (quad_tree *)trees->data;
        if (tree->nw == NULL) {
          parent = quad_tree_segment_find(tree);
          if (parent != NULL) {
            color0 = (byte)parent->stat.mean;
            color1 = color0;
            color2 = color0;
            width = tree->size;
            height = width;
            row_step = stride - 3 * width;
            target_pos = target_data + tree->y * stride + tree->x * 3;
            for (y = 0; y < height; y++, target_pos += row_step) {
              for (x = 0; x < width; x++) {
                *target_pos = color0;
                target_pos++;
                *target_pos = color1;
                target_pos++;
                *target_pos = color2;
                target_pos++;
              }
            }
          }
        }
        trees = trees->next;
      }
    }
    /* draw using region color */
    else {
      trees = forest->trees.first.next;
      while (trees != &forest->trees.last) {
        tree = (quad_tree *)trees->data;
        if (tree->nw == NULL) {
          parent = quad_tree_segment_find(tree);
          if (parent != NULL) {
            color0 = parent->color[0];
            color1 = parent->color[1];
            color2 = parent->color[2];
            width = tree->size;
            height = width;
            row_step = stride - 3 * width;
            target_pos = target_data + tree->y * stride + tree->x * 3;
            for (y = 0; y < height; y++, target_pos += row_step) {
              for (x = 0; x < width; x++) {
                *target_pos = color0;
                target_pos++;
                *target_pos = color1;
                target_pos++;
                *target_pos = color2;
                target_pos++;
              }
            }
          }
        }
        trees = trees->next;
      }
    }
  }

  FINALLY(quad_forest_draw_image);
  RETURN();
}

/* end of file                                                                */
/******************************************************************************/
