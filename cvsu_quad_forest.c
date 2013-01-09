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

#include "cvsu_quad_forest.h"
#include "cvsu_macros.h"
#include "cvsu_memory.h"

#include <stdlib.h>
/*#include <stdio.h>*/
#include <sys/time.h>
#include <math.h>

double fmin (double __x, double __y);
double fmax (double __x, double __y);

/******************************************************************************/
/* constants for reporting function names in error messages                   */

string quad_tree_nullify_name = "quad_tree_nullify";
string quad_forest_init_name = "quad_forest_init";
string quad_forest_alloc_name = "quad_forest_alloc";
string quad_forest_free_name = "quad_forest_free";
string quad_forest_create_name = "quad_forest_create";
string quad_forest_reload_name = "quad_forest_reload";
string quad_forest_destroy_name = "quad_forest_destroy";
string quad_forest_nullify_name = "quad_forest_nullify";
string quad_forest_update_name = "quad_forest_update";
string quad_forest_segment_with_deviation_name = "quad_forest_segment_with_deviation";
string quad_forest_segment_with_overlap_name = "quad_forest_segment_with_overlap";
string quad_forest_get_segments_name = "quad_forest_get_regions";
string quad_forest_draw_image_name = "quad_forest_draw_image";
string quad_tree_divide_name = "quad_tree_divide";
string quad_tree_get_child_statistics_name = "quad_tree_get_child_statistics";
string quad_tree_divide_with_overlap_name = "quad_tree_divide_with_overlap";
/*
string image_tree_create_neighbor_list_name = "image_tree_create_neighbor_list";
string image_tree_add_children_as_immediate_neighbors_name = "image_tree_add_children_as_immediate_neighbors";
string image_tree_find_all_immediate_neighbors_name = "image_tree_find_all_immediate_neighbors";
*/
/******************************************************************************/

result quad_tree_nullify
(
  quad_tree *target
)
{
  TRY();

  CHECK(memory_clear((data_pointer)target, 1, sizeof(quad_tree)));

  FINALLY(quad_tree_nullify);
  RETURN();
}

/******************************************************************************/

truth_value quad_tree_is_null
(
  quad_tree *target
)
{
  if (target != NULL) {
    /* a tree is NULL if its size has not been set */
    if (target->s != 0) {
      return FALSE;
    }
  }
  return TRUE;
}

/******************************************************************************/
/* a private function for initializing quad_forest structure                  */
/* used in create and in reload                                               */

result quad_forest_init
(
  quad_forest *target,
  uint32 tree_max_size,
  uint32 tree_min_size
)
{
  TRY();
  uint32 row, col, rows, cols, pos, size, width, height;
  quad_tree new_tree, *tree_ptr;

  /* not necessary to check target pointer, calling function should handle that */
  width = target->original->width;
  height = target->original->height;

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

    size = rows * cols;
    if (target->roots != NULL) {
      CHECK(memory_deallocate((data_pointer*)&target->roots));
    }
    CHECK(memory_allocate((data_pointer *)&target->roots, size, sizeof(quad_tree*)));

    if (!list_is_null(&target->trees)) {
      CHECK(list_destroy(&target->trees));
    }
    CHECK(list_create(&target->trees, 32 * size, sizeof(quad_tree), 1));
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
    CHECK(pixel_image_create(target->source, p_U8, GREY, width, height, 1, width));
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
      CHECK(list_append_reveal_data(&target->trees, (pointer)&new_tree, (pointer*)&tree_ptr));
      target->roots[pos] = tree_ptr;
    }
  }
  target->last_root_tree = target->trees.last.prev;

  /* add neighbors to roots */
  for (row = 0, pos = 0; row < rows; row++) {
    for (col = 0; col < cols; col++, pos++) {
      /* add neighbor to west */
      if (col > 0) {
        target->roots[pos]->w = target->roots[pos - 1];
      }
      /* add neighbor to north */
      if (row > 0) {
        target->roots[pos]->n = target->roots[pos - cols];
      }
      /* add neighbor to east */
      if (col < (unsigned)(cols - 1)) {
        target->roots[pos]->e = target->roots[pos + 1];
      }
      /* add neighbor to south */
      if (row < (unsigned)(rows - 1)) {
        target->roots[pos]->s = target->roots[pos + cols];
      }
    }
  }

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

  CHECK(quad_forest_init(target, tree_max_size, tree_min_size));

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
    quad_forest_init(target, tree_max_size, tree_min_size);
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
  uint32 pos, size;

  CHECK_POINTER(target);

  CHECK(list_destroy(&target->trees));

  CHECK(memory_deallocate((data_pointer*)&target->roots));

  CHECK(integral_image_destroy(&target->integral));
  pixel_image_free(target->source);

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
  CHECK(pixel_image_copy(target->source, target->original));
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

      /* TODO: decide where the segments are created */
      quad_tree_segment_create(tree);
      tree->nw = NULL;
      tree->ne = NULL;
      tree->sw = NULL;
      tree->se = NULL;
    }
  }

  FINALLY(quad_forest_update);
  RETURN();
}

/******************************************************************************/

/* a macro for calculating neighbor difference; neighbor should be statistics pointer */
#define EVALUATE_NEIGHBOR_DEVIATION(neighbor)\
  stat = neighbor;\
  nm = stat->mean;\
  dist = fabs(tm - nm)

result quad_forest_segment_with_deviation
(
  quad_forest *target,
  integral_value threshold,
  integral_value alpha
)
{
  TRY();
  list_item *trees;
  quad_tree *tree, *neighbor, *best_neighbor;
  quad_forest_segment *tree_segment, *neighbor_segment;
  statistics *stat;
  integral_value tm, nm, dist, best_dist;
  uint32 min_size;

  CHECK_POINTER(target);
  CHECK_PARAM(threshold > 0);
  CHECK_PARAM(alpha > 0);

  min_size = target->tree_min_size;

  /* first, divide until all trees are consistent */
  trees = target->trees.first.next;
  while (trees != &target->trees.last) {
    tree = (quad_tree *)trees->data;
    if (tree->size >= 2 * min_size) {
      if (tree->stat.deviation > threshold) {
        CHECK(quad_tree_divide(target, tree));
      }
    }
    trees = trees->next;
  }

  /* then, make a union of those neighboring regions that are consistent together */
  printf("starting to merge trees\n");
  trees = target->trees.first.next;
  while (trees != &target->trees.last) {
    tree = (quad_tree *)trees->data;
    tree_segment = quad_tree_segment_find(tree);
    /* only consider consistent trees (those that have not been divided) */
    if (tree->nw == NULL) {
      stat = &tree->stat;
      tm = stat->mean;

      best_dist = 255;
      best_neighbor = NULL;
      /* neighbor n */
      neighbor = tree->n;
      if (neighbor != NULL && neighbor->nw == NULL) {
        neighbor_segment = quad_tree_segment_find(neighbor);
        if (tree_segment != neighbor_segment) {
          EVALUATE_NEIGHBOR_DEVIATION(&neighbor->stat);
          if (dist < best_dist) {
            best_dist = dist;
            best_neighbor = neighbor;
          }
        }
      }
      /* neighbor e */
      neighbor = tree->e;
      if (neighbor != NULL && neighbor->nw == NULL) {
        neighbor_segment = quad_tree_segment_find(neighbor);
        if (tree_segment != neighbor_segment) {
          EVALUATE_NEIGHBOR_DEVIATION(&neighbor->stat);
          if (dist < best_dist) {
            best_dist = dist;
            best_neighbor = neighbor;
          }
        }
      }
      /* neighbor s */
      neighbor = tree->s;
      if (neighbor != NULL && neighbor->nw == NULL) {
        neighbor_segment = quad_tree_segment_find(neighbor);
        if (tree_segment != neighbor_segment) {
          EVALUATE_NEIGHBOR_DEVIATION(&neighbor->stat);
          if (dist < best_dist) {
            best_dist = dist;
            best_neighbor = neighbor;
          }
        }
      }
      /* neighbor w */
      neighbor = tree->w;
      if (neighbor != NULL && neighbor->nw == NULL) {
        neighbor_segment = quad_tree_segment_find(neighbor);
        if (tree_segment != neighbor_segment) {
          EVALUATE_NEIGHBOR_DEVIATION(&neighbor->stat);
          if (dist < best_dist) {
            best_dist = dist;
            best_neighbor = neighbor;
          }
        }
      }

      if (best_dist < alpha * threshold) {
        quad_tree_segment_union(tree, best_neighbor);
      }
    }
    trees = trees->next;
  }

  /* then, merge those neighboring regions that are consistent together */
  printf("starting to merge regions\n");
  trees = target->trees.first.next;
  while (trees != &target->trees.last) {
    tree = (quad_tree *)trees->data;
    tree_segment = quad_tree_segment_find(tree);
    /* only consider consistent trees (those that have not been divided) */
    if (tree->nw == NULL) {
      stat = &tree_segment->stat;
      tm = stat->mean;

      /* neighbor n */
      neighbor = tree->n;
      if (neighbor != NULL && neighbor->nw == NULL) {
        neighbor_segment = quad_tree_segment_find(neighbor);
        if (tree_segment != neighbor_segment) {
          EVALUATE_NEIGHBOR_DEVIATION(&neighbor_segment->stat);
          if (dist < alpha * threshold) {
            quad_tree_segment_union(tree, neighbor);
          }
        }
      }
      /* neighbor e */
      neighbor = tree->e;
      if (neighbor != NULL && neighbor->nw == NULL) {
        neighbor_segment = quad_tree_segment_find(neighbor);
        if (tree_segment != neighbor_segment) {
          EVALUATE_NEIGHBOR_DEVIATION(&neighbor_segment->stat);
          if (dist < alpha * threshold) {
            quad_tree_segment_union(tree, neighbor);
          }
        }
      }
      /* neighbor s */
      neighbor = tree->s;
      if (neighbor != NULL && neighbor->nw == NULL) {
        neighbor_segment = quad_tree_segment_find(neighbor);
        if (tree_segment != neighbor_segment) {
          EVALUATE_NEIGHBOR_DEVIATION(&neighbor_segment->stat);
          if (dist < alpha * threshold) {
            quad_tree_segment_union(tree, neighbor);
          }
        }
      }
      /* neighbor w */
      neighbor = tree->w;
      if (neighbor != NULL && neighbor->nw == NULL) {
        neighbor_segment = quad_tree_segment_find(neighbor);
        if (tree_segment != neighbor_segment) {
          EVALUATE_NEIGHBOR_DEVIATION(&neighbor_segment->stat);
          if (dist < alpha * threshold) {
            quad_tree_segment_union(tree, neighbor);
          }
        }
      }
    }
    trees = trees->next;
  }

  /* finally, count regions and assign colors */
  {
    quad_forest_segment *parent, *segment;
    uint32 count;

    count = 0;
    /* initialize the random number generator for assigning the colors */
    srand(1234);

    trees = target->trees.first.next;
    while (trees != &target->trees.last) {
      tree = (quad_tree *)trees->data;
      if (tree->nw == NULL) {
        segment = &tree->segment;
        parent = quad_tree_segment_find(tree);
        if (parent == segment) {
          segment->color[0] = (byte)(rand() % 256);
          segment->color[1] = (byte)(rand() % 256);
          segment->color[2] = (byte)(rand() % 256);
          count++;
        }
      }
      trees = trees->next;
    }
    target->segments = count;
    printf("segmentation finished, %lu segments found\n", count);
  }

  FINALLY(quad_forest_segment_with_deviation);
  RETURN();
}

/******************************************************************************/

/* a macro for calculating neighbor overlap; neighbor should be statistics pointer */
#define EVALUATE_NEIGHBOR_OVERLAP(neighbor)\
  stat = neighbor;\
  nm = stat->mean;\
  ns = fmax(alpha, alpha * stat->deviation);\
  x1min = fmax(0, tm - ts);\
  x1max = x1min;\
  x2min = fmin(255, tm + ts);\
  x2max = x2min;\
  x1 = fmax(0, nm - ns);\
  x2 = fmin(255, nm + ns);\
  if (x1 < x1min) x1min = x1; else x1max = x1;\
  if (x2 < x2min) x2min = x2; else x2max = x2;\
  if (x1max > x2min) {\
    I = 0;\
  }\
  else {\
    I = (x2min - x1max);\
    if (I < 1) I = 1;\
  }\
  U = (x2max - x1min);\
  if (U < 1) U = 1;\
  overlap = I / U

result quad_forest_segment_with_overlap
(
  quad_forest *target,
  integral_value alpha,
  integral_value threshold_trees,
  integral_value threshold_segments
)
{
  TRY();
  list_item *trees;
  quad_tree *tree, *neighbor, *best_neighbor;
  quad_forest_segment *tree_segment, *neighbor_segment;
  statistics *stat;
  integral_value tm, ts, nm, ns, x1, x2, x1min, x1max, x2min, x2max, I, U, overlap, best_overlap;

  CHECK_POINTER(target);
  CHECK_PARAM(alpha > 0);
  CHECK_PARAM(threshold_trees > 0);
  CHECK_PARAM(threshold_segments > 0);

  /* first, divide until all trees are consistent */
  printf("starting to divide trees\n");
  trees = target->trees.first.next;
  while (trees != &target->trees.last) {
    tree = (quad_tree *)trees->data;
    CHECK(quad_tree_divide_with_overlap(target, tree, alpha, threshold_trees));
    trees = trees->next;
  }

  /* then, merge each tree with the best neighboring tree that is close enough */
  printf("starting to merge trees\n");
  trees = target->trees.first.next;
  while (trees != &target->trees.last) {
    tree = (quad_tree *)trees->data;
    tree_segment = quad_tree_segment_find(tree);
    /* only consider consistent trees (those that have not been divided) */
    if (tree->nw == NULL) {
      stat = &tree->stat;
      tm = stat->mean;
      ts = fmax(alpha, alpha * stat->deviation);

      best_overlap = 0;
      best_neighbor = NULL;
      /* neighbor n */
      neighbor = tree->n;
      if (neighbor != NULL && neighbor->nw == NULL) {
        neighbor_segment = quad_tree_segment_find(neighbor);
        if (tree_segment != neighbor_segment) {
          EVALUATE_NEIGHBOR_OVERLAP(&neighbor->stat);
          if (overlap > best_overlap) {
            best_overlap = overlap;
            best_neighbor = neighbor;
          }
        }
      }
      /* neighbor e */
      neighbor = tree->e;
      if (neighbor != NULL && neighbor->nw == NULL) {
        neighbor_segment = quad_tree_segment_find(neighbor);
        if (tree_segment != neighbor_segment) {
          EVALUATE_NEIGHBOR_OVERLAP(&neighbor->stat);
          if (overlap > best_overlap) {
            best_overlap = overlap;
            best_neighbor = neighbor;
          }
        }
      }
      /* neighbor s */
      neighbor = tree->s;
      if (neighbor != NULL && neighbor->nw == NULL) {
        neighbor_segment = quad_tree_segment_find(neighbor);
        if (tree_segment != neighbor_segment) {
          EVALUATE_NEIGHBOR_OVERLAP(&neighbor->stat);
          if (overlap > best_overlap) {
            best_overlap = overlap;
            best_neighbor = neighbor;
          }
        }
      }
      /* neighbor w */
      neighbor = tree->w;
      if (neighbor != NULL && neighbor->nw == NULL) {
        neighbor_segment = quad_tree_segment_find(neighbor);
        if (tree_segment != neighbor_segment) {
          EVALUATE_NEIGHBOR_OVERLAP(&neighbor->stat);
          if (overlap > best_overlap) {
            best_overlap = overlap;
            best_neighbor = neighbor;
          }
        }
      }

      if (best_overlap > threshold_trees) {
        quad_tree_segment_union(tree, best_neighbor);
      }
    }
    trees = trees->next;
  }

  /* then, merge those neighboring regions that are consistent together */
  printf("starting to merge regions\n");
  trees = target->trees.first.next;
  while (trees != &target->trees.last) {
    tree = (quad_tree *)trees->data;
    tree_segment = quad_tree_segment_find(tree);
    /* only consider consistent trees (those that have not been divided) */
    if (tree->nw == NULL) {
      stat = &tree_segment->stat;
      tm = stat->mean;
      ts = fmax(alpha, alpha * stat->deviation);

      /* neighbor n */
      neighbor = tree->n;
      if (neighbor != NULL && neighbor->nw == NULL) {
        neighbor_segment = quad_tree_segment_find(neighbor);
        if (tree_segment != neighbor_segment) {
          EVALUATE_NEIGHBOR_OVERLAP(&neighbor_segment->stat);
          if (overlap > threshold_segments) {
            quad_tree_segment_union(tree, neighbor);
          }
        }
      }
      /* neighbor e */
      neighbor = tree->e;
      if (neighbor != NULL && neighbor->nw == NULL) {
        neighbor_segment = quad_tree_segment_find(neighbor);
        if (tree_segment != neighbor_segment) {
          EVALUATE_NEIGHBOR_OVERLAP(&neighbor_segment->stat);
          if (overlap > threshold_segments) {
            quad_tree_segment_union(tree, neighbor);
          }
        }
      }
      /* neighbor s */
      neighbor = tree->s;
      if (neighbor != NULL && neighbor->nw == NULL) {
        neighbor_segment = quad_tree_segment_find(neighbor);
        if (tree_segment != neighbor_segment) {
          EVALUATE_NEIGHBOR_OVERLAP(&neighbor_segment->stat);
          if (overlap > threshold_segments) {
            quad_tree_segment_union(tree, neighbor);
          }
        }
      }
      /* neighbor w */
      neighbor = tree->w;
      if (neighbor != NULL && neighbor->nw == NULL) {
        neighbor_segment = quad_tree_segment_find(neighbor);
        if (tree_segment != neighbor_segment) {
          EVALUATE_NEIGHBOR_OVERLAP(&neighbor_segment->stat);
          if (overlap > threshold_segments) {
            quad_tree_segment_union(tree, neighbor);
          }
        }
      }
    }
    trees = trees->next;
  }

  /* finally, count regions and assign colors */
  {
    quad_forest_segment *parent, *segment;
    uint32 count;

    count = 0;
    /* initialize the random number generator for assigning the colors */
    srand(1234);

    trees = target->trees.first.next;
    while (trees != &target->trees.last) {
      tree = (quad_tree *)trees->data;
      if (tree->nw == NULL) {
        segment = &tree->segment;
        parent = quad_tree_segment_find(tree);
        if (parent == segment) {
          segment->color[0] = (byte)(rand() % 256);
          segment->color[1] = (byte)(rand() % 256);
          segment->color[2] = (byte)(rand() % 256);
          count++;
        }
      }
      trees = trees->next;
    }
    target->segments = count;
    printf("segmentation finished, %lu segments found\n", count);
  }

  FINALLY(quad_forest_segment_with_overlap);
  RETURN();
}

/******************************************************************************/

result quad_forest_get_segments
(
  quad_forest *source,
  quad_forest_segment **target
)
{
  TRY();
  list_item *trees;
  quad_tree *tree;
  quad_forest_segment *segment, *parent;
  uint32 count;

  CHECK_POINTER(source);
  CHECK_POINTER(target);

  /* collect all segments to the array */
  count = 0;
  trees = source->trees.first.next;
  while (trees != &source->trees.last) {
    tree = (quad_tree *)trees->data;
    if (tree->nw == NULL) {
      segment = &tree->segment;
      parent = quad_tree_segment_find(tree);
      if (parent == segment) {
        target[count] = segment;
        count++;
      }
    }
    trees = trees->next;
  }

  FINALLY(quad_forest_get_segments);
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
  quad_forest_segment *parent;
  uint32 x, y, width, height, stride, row_step;
  byte *target_data, *target_pos, color0, color1, color2;

  CHECK_POINTER(forest);
  CHECK_POINTER(forest->source);
  CHECK_POINTER(target);

  width = forest->source->width;
  height = forest->source->height;

  CHECK(pixel_image_create(target, p_U8, RGB, width, height, 3, 3 * width));

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
            *target_pos++;
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
                *target_pos++;
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
                *target_pos++;
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

/******************************************************************************/
/* private function for caching beighbors, to be used only when it is known  */
/* that the tree exists, and its children exist                              */

void quad_tree_cache_neighbors
(
  quad_tree *target
)
{
  target->nw->e = target->ne;
  target->nw->s = target->sw;
  target->ne->w = target->nw;
  target->ne->s = target->se;
  target->sw->e = target->se;
  target->sw->n = target->nw;
  target->se->w = target->sw;
  target->se->n = target->ne;
  if (target->n != NULL) {
    if (target->n->sw != NULL) {
      target->nw->n = target->n->sw;
      target->n->sw->s = target->nw;
    }
    else {
      target->nw->n = target->n;
    }
    if (target->n->se != NULL) {
      target->ne->n = target->n->se;
      target->n->se->s = target->ne;
    }
    else {
      target->ne->n = target->n;
    }
  }
  if (target->e != NULL) {
    if (target->e->nw != NULL) {
      target->ne->e = target->e->nw;
      target->e->nw->w = target->ne;
    }
    else {
      target->ne->e = target->e;
    }
    if (target->e->sw != NULL) {
      target->se->e = target->e->sw;
      target->e->sw->w = target->se;
    }
    else {
      target->se->e = target->e;
    }
  }
  if (target->s != NULL) {
    if (target->s->nw != NULL) {
      target->sw->s = target->s->nw;
      target->s->nw->n = target->sw;
    }
    else {
      target->sw->s = target->s;
    }
    if (target->s->ne != NULL) {
      target->se->s = target->s->ne;
      target->s->ne->n = target->se;
    }
    else {
      target->se->s = target->s;
    }
  }
  if (target->w != NULL) {
    if (target->w->ne != NULL) {
      target->nw->w = target->w->ne;
      target->w->ne->e = target->nw;
    }
    else {
      target->nw->w = target->w;
    }
    if (target->w->se != NULL) {
      target->sw->w = target->w->se;
      target->w->se->e = target->sw;
    }
    else {
      target->sw->w = target->w;
    }
  }
}

/******************************************************************************/
/* TODO: make a new generic function quad_tree_divide_with_criterion */
/* criterion function probably requires child list as parameter */

result quad_tree_divide
(
  quad_forest *forest,
  quad_tree *target
)
{
  TRY();

  CHECK_POINTER(target);

  if (target->size >= forest->tree_min_size * 2) {
    if (target->nw == NULL) {
      quad_tree children[4];

      CHECK(quad_tree_nullify(&children[0]));
      CHECK(quad_tree_nullify(&children[1]));
      CHECK(quad_tree_nullify(&children[2]));
      CHECK(quad_tree_nullify(&children[3]));

      CHECK(quad_tree_get_child_statistics(forest, target, children));
      {
        quad_tree *child_tree;
        uint32 level;

        level = target->level + 1;

        /* nw child block */
        CHECK(list_append_reveal_data(&forest->trees, (pointer)&children[0], (pointer*)&child_tree));
        quad_tree_segment_create(child_tree);
        child_tree->level = level;
        child_tree->parent = target;
        target->nw = child_tree;

        /* ne child block */
        CHECK(list_append_reveal_data(&forest->trees, (pointer)&children[1], (pointer*)&child_tree));
        quad_tree_segment_create(child_tree);
        child_tree->level = level;
        child_tree->parent = target;
        target->ne = child_tree;

        /* sw child block */
        CHECK(list_append_reveal_data(&forest->trees, (pointer)&children[2], (pointer*)&child_tree));
        quad_tree_segment_create(child_tree);
        child_tree->level = level;
        child_tree->parent = target;
        target->sw = child_tree;

        /* se child block */
        CHECK(list_append_reveal_data(&forest->trees, (pointer)&children[3], (pointer*)&child_tree));
        quad_tree_segment_create(child_tree);
        child_tree->level = level;
        child_tree->parent = target;
        target->se = child_tree;

        quad_tree_cache_neighbors(target);
      }
    }
  }

  FINALLY(quad_tree_divide);
  RETURN();
}

/******************************************************************************/

truth_value quad_tree_has_children
(
  quad_tree *tree
)
{
  /* there should be no case in which some children would not be set */
  /* the pointers are nullified at init; therefore, check only one */
  if (tree->nw != NULL) {
    return TRUE;
  }
  return FALSE;
}

/******************************************************************************/

result quad_tree_get_child_statistics
(
  quad_forest *forest,
  quad_tree *source,
  quad_tree *target
)
{
  TRY();

  CHECK_POINTER(source);
  CHECK_POINTER(target);

  /* if the tree has already been divided, return the values from the children */
  if (source->nw != NULL) {
    /* nw child block */
    CHECK(memory_copy((void*)&target[0], (void*)source->nw, 1, sizeof(quad_tree)));
    /* ne child block */
    CHECK(memory_copy((void*)&target[1], (void*)source->ne, 1, sizeof(quad_tree)));
    /* sw child block */
    CHECK(memory_copy((void*)&target[2], (void*)source->sw, 1, sizeof(quad_tree)));
    /* se child block */
    CHECK(memory_copy((void*)&target[3], (void*)source->se, 1, sizeof(quad_tree)));
  }
  /* otherwise have to calculate */
  else {
    uint32 x, y, size;
    uint32 step, stride, offset;
    statistics *stat;

    size = (uint32)(source->size / 2);

    target[0].size = target[1].size = target[2].size = target[3].size = size;

    /* if the new width is 1 or 0, no need to calculate, use the pixel values */
    /* size 0 should not happen unless someone tries to divide tree with size 1 */
    if (size < 2) {
      pixel_image *original;
      void *data;
      pixel_type type;
      integral_value mean;

      original = forest->source;
      data = original->data;
      type = original->type;
      step = original->step;
      stride = original->stride;

      /* nw child block */
      x = source->x;
      y = source->y;
      offset = y * stride + x * step;
      mean = cast_pixel_value(original->data, type, offset);

      stat = &target[0].stat;
      stat->N = 1;
      stat->sum = mean;
      stat->sum2 = mean*mean;
      stat->mean = mean;
      stat->variance = 0;
      stat->deviation = 0;

      target[0].x = x;
      target[0].y = y;

      /* ne child block */
      x = x + size;
      offset = offset + size * step;
      mean = cast_pixel_value(original->data, type, offset);

      stat = &target[1].stat;
      stat->N = 1;
      stat->sum = mean;
      stat->sum2 = mean*mean;
      stat->mean = mean;
      stat->variance = 0;
      stat->deviation = 0;

      target[1].x = x;
      target[1].y = y;

      /* se child block */
      y = y + size;
      offset = offset + size * stride;
      mean = cast_pixel_value(original->data, type, offset);

      stat = &target[3].stat;
      stat->N = 1;
      stat->sum = mean;
      stat->sum2 = mean*mean;
      stat->mean = mean;
      stat->variance = 0;
      stat->deviation = 0;

      target[3].x = x;
      target[3].y = y;

      /* sw child block */
      x = x - size;
      offset = offset - size * step;
      mean = cast_pixel_value(original->data, type, offset);

      stat = &target[2].stat;
      stat->N = 1;
      stat->sum = mean;
      stat->sum2 = mean*mean;
      stat->mean = mean;
      stat->variance = 0;
      stat->deviation = 0;

      target[2].x = x;
      target[2].y = y;
    }
    else {
      uint32 hstep, vstep, dstep;
      integral_image *I;
      integral_value *iA, *i2A, N, sum1, sum2, mean, var;

      I = &forest->integral;
      N = (integral_value)(size * size);
      step = I->step;
      stride = I->stride;
      hstep = size * step;
      vstep = size * stride;
      dstep = hstep + vstep;

      /* nw child block */
      x = source->x;
      y = source->y;
      offset = y * stride + x * step;

      iA = ((integral_value *)I->I_1.data) + offset;
      i2A = ((integral_value *)I->I_2.data) + offset;

      sum1 = *(iA + dstep) + *iA - *(iA + hstep) - *(iA + vstep);
      sum2 = *(i2A + dstep) + *i2A - *(i2A + hstep) - *(i2A + vstep);
      mean = sum1 / N;
      var = sum2 / N - mean*mean;
      if (var < 0) var = 0;

      stat = &target[0].stat;
      stat->N = N;
      stat->sum = sum1;
      stat->sum2 = sum2;
      stat->mean = mean;
      stat->variance = var;
      stat->deviation = sqrt(var);

      target[0].x = x;
      target[0].y = y;

      /* ne child block */
      x = x + size;
      iA = iA + hstep;
      i2A = i2A + hstep;

      sum1 = *(iA + dstep) + *iA - *(iA + hstep) - *(iA + vstep);
      sum2 = *(i2A + dstep) + *i2A - *(i2A + hstep) - *(i2A + vstep);
      mean = sum1 / N;
      var = sum2 / N - mean*mean;
      if (var < 0) var = 0;

      stat = &target[1].stat;
      stat->N = N;
      stat->sum = sum1;
      stat->sum2 = sum2;
      stat->mean = mean;
      stat->variance = var;
      stat->deviation = sqrt(var);

      target[1].x = x;
      target[1].y = y;

      /* se child block */
      y = y + size;
      iA = iA + vstep;
      i2A = i2A + vstep;

      sum1 = *(iA + dstep) + *iA - *(iA + hstep) - *(iA + vstep);
      sum2 = *(i2A + dstep) + *i2A - *(i2A + hstep) - *(i2A + vstep);
      mean = sum1 / N;
      var = sum2 / N - mean*mean;
      if (var < 0) var = 0;

      stat = &target[3].stat;
      stat->N = N;
      stat->sum = sum1;
      stat->sum2 = sum2;
      stat->mean = mean;
      stat->variance = var;
      stat->deviation = sqrt(var);

      target[3].x = x;
      target[3].y = y;

      /* sw child block */
      x = x - size;
      iA = iA - hstep;
      i2A = i2A - hstep;

      sum1 = *(iA + dstep) + *iA - *(iA + hstep) - *(iA + vstep);
      sum2 = *(i2A + dstep) + *i2A - *(i2A + hstep) - *(i2A + vstep);
      mean = sum1 / N;
      var = sum2 / N - mean*mean;
      if (var < 0) var = 0;

      stat = &target[2].stat;
      stat->N = N;
      stat->sum = sum1;
      stat->sum2 = sum2;
      stat->mean = mean;
      stat->variance = var;
      stat->deviation = sqrt(var);

      target[2].x = x;
      target[2].y = y;
    }
  }

  FINALLY(quad_tree_get_child_statistics);
  RETURN();
}

/******************************************************************************/

result quad_tree_divide_with_overlap
(
  quad_forest *forest,
  quad_tree *target,
  integral_value alpha,
  integral_value overlap_threshold
)
{
  TRY();

  CHECK_POINTER(forest);
  CHECK_POINTER(target);

  if (target->nw == NULL) {
    if (target->size >= forest->tree_min_size * 2) {
      quad_tree children[4];
      uint32 i;
      statistics *stat;
      integral_value m, s, x1, x2, x1min, x1max, x2min, x2max, I, U, overlap;

      CHECK(quad_tree_nullify(&children[0]));
      CHECK(quad_tree_nullify(&children[1]));
      CHECK(quad_tree_nullify(&children[2]));
      CHECK(quad_tree_nullify(&children[3]));

      CHECK(quad_tree_get_child_statistics(forest, target, children));

      stat = &children[0].stat;
      m = stat->mean;
      s = fmax(alpha, alpha * stat->deviation);
      x1min = fmax(0, m - s);
      x1max = x1min;
      x2min = fmin(255, m + s);
      x2max = x2min;
      for (i = 1; i < 4; i++) {
        stat = &children[i].stat;
        m = stat->mean;
        s = fmax(alpha, alpha * stat->deviation);
        x1 = fmax(0, m - s);
        x2 = fmin(255, m + s);
        if (x1 < x1min) x1min = x1;
        else if (x1 > x1max) x1max = x1;
        if (x2 < x2min) x2min = x2;
        else if (x2 > x2max) x2max = x2;
      }

      /* it is possible that intersection is negative, this means an empty set */
      if (x1max > x2min) {
        I = 0;
      }
      else {
        I = (x2min - x1max);
        if (I < 1) I = 1;
      }
      U = (x2max - x1min);
      if (U < 1) U = 1;

      /* let us define this entropy measure as intersection divided by union */
      overlap = I / U;

      /* if union is more than double the intersection, we have high 'entropy' */
      if (overlap < overlap_threshold) {
        quad_tree *child_tree;
        uint32 level;

        level = target->level + 1;

        /* nw child block */
        CHECK(list_append_reveal_data(&forest->trees, (pointer)&children[0], (pointer*)&child_tree));
        quad_tree_segment_create(child_tree);
        child_tree->parent = target;
        child_tree->level = level;
        target->nw = child_tree;

        /* ne child block */
        CHECK(list_append_reveal_data(&forest->trees, (pointer)&children[1], (pointer*)&child_tree));
        quad_tree_segment_create(child_tree);
        child_tree->parent = target;
        child_tree->level = level;
        target->ne = child_tree;

        /* sw child block */
        CHECK(list_append_reveal_data(&forest->trees, (pointer)&children[2], (pointer*)&child_tree));
        quad_tree_segment_create(child_tree);
        child_tree->parent = target;
        child_tree->level = level;
        target->sw = child_tree;

        /* se child block */
        CHECK(list_append_reveal_data(&forest->trees, (pointer)&children[3], (pointer*)&child_tree));
        quad_tree_segment_create(child_tree);
        child_tree->parent = target;
        child_tree->level = level;
        target->se = child_tree;

        quad_tree_cache_neighbors(target);
      }
    }
  }

  FINALLY(quad_tree_divide_with_overlap);
  RETURN();
}

/*******************************************************************************

result image_tree_create_neighbor_list(
    list *target
    )
{
    TRY();

    CHECK_POINTER(target);

    CHECK(list_create(target, 100, sizeof(image_tree*), 1));

    FINALLY(image_tree_create_neighbor_list);
    RETURN();
}

 * Recursive function for adding child trees from the highest level as
 * immediate neighbors to another tree
 *
 * 1. If tree has no childen, add it to list and return
 * 2. If tree has chilren, call recursively for the two children in the proper
 *    direction
 *

result image_tree_add_children_as_immediate_neighbors(
    list *target,
    image_tree *tree,
    direction dir
    )
{
    TRY();

    FINALLY(image_tree_add_children_as_immediate_neighbors);
    RETURN();
}

********************************************************************************

 * Finds all immediate neighbors (directly adjacent neighbors on the highest
 * level) of a tree in the given direction and stores them in the list.
 *
 * 1. Find the direct neighbor; if it has children, call recursively for the
 *    two children adjacent to this tree
 *
 * 2. Peek the item at the top of stack; if it has children, pop it and push the
 *    two children adjacent to this tree into the stack; if not, add it to the
 *    end of the list
 * 3.
 *
result image_tree_find_all_immediate_neighbors
  (
  list *target,
  image_tree *tree
  )
{
  TRY();
  image_tree *new_neighbor;
  image_tree **temp_ptr;

  CHECK_POINTER(target);
  CHECK_POINTER(tree);

  CHECK(image_tree_get_direct_neighbor_n(tree, &new_neighbor));
  if (new_neighbor != NULL) {
    CHECK(list_append(target, (pointer)&new_neighbor));
  }
  CHECK(image_tree_get_direct_neighbor_e(tree, &new_neighbor));
  if (new_neighbor != NULL) {
    CHECK(list_append(target, (pointer)&new_neighbor));
  }
  CHECK(image_tree_get_direct_neighbor_s(tree, &new_neighbor));
  if (new_neighbor != NULL) {
    CHECK(list_append(target, (pointer)&new_neighbor));
  }
  CHECK(image_tree_get_direct_neighbor_w(tree, &new_neighbor));
  if (new_neighbor != NULL) {
    CHECK(list_append(target, (pointer)&new_neighbor));
  }

  FINALLY(image_tree_find_all_immediate_neighbors);
  RETURN();
}

*******************************************************************************/

void quad_tree_segment_create
(
  quad_tree *tree
)
{
  quad_forest_segment *segment;
  if (tree != NULL) {
    segment = &tree->segment;
    /* proceed only if the tree doesn't have its region info initialized yet */
    /*if (region->id == NULL) {*/
      /* one-tree class is it's own id, and has the rank of 0 */
      segment->parent = segment;
      segment->rank = 0;
      segment->x1 = tree->x;
      segment->y1 = tree->y;
      segment->x2 = tree->x + tree->size;
      segment->y2 = tree->y + tree->size;
      memory_copy((data_pointer)&segment->stat, (data_pointer)&tree->stat, 1, sizeof(statistics));
    /*}*/
  }
}

/******************************************************************************/

void quad_tree_segment_union
(
  quad_tree *tree1,
  quad_tree *tree2
)
{
  if (tree1 != NULL && tree2 != NULL) {
    quad_forest_segment *segment1, *segment2;

    segment1 = quad_tree_segment_find(tree1);
    segment2 = quad_tree_segment_find(tree2);
    if (segment1 == NULL || segment2 == NULL) {
      return;
    }
    /* if the trees are already in the same class, no need for union */
    if (segment1 == segment2) {
      return;
    }
    /* otherwise set the tree with higher class rank as id of the union */
    else {
      statistics *stat;
      integral_value N, mean, variance;
      if (segment1->rank < segment2->rank) {
        segment1->parent = segment2;
        segment2->x1 = (segment1->x1 < segment2->x1) ? segment1->x1 : segment2->x1;
        segment2->y1 = (segment1->y1 < segment2->y1) ? segment1->y1 : segment2->y1;
        segment2->x2 = (segment1->x2 > segment2->x2) ? segment1->x2 : segment2->x2;
        segment2->y2 = (segment1->y2 > segment2->y2) ? segment1->y2 : segment2->y2;
        stat = &segment2->stat;
        N = (stat->N += segment1->stat.N);
        stat->sum += segment1->stat.sum;
        stat->sum2 += segment1->stat.sum2;
        mean = stat->mean = stat->sum / N;
        variance = stat->sum2 / N - mean*mean;
        if (variance < 0) variance = 0;
        stat->variance = variance;
        stat->deviation = sqrt(variance);
      }
      else {
        segment2->parent = segment1;
        /* when equal rank trees are combined, the root tree's rank is increased */
        if (segment1->rank == segment2->rank) {
          segment1->rank += 1;
        }
        segment1->x1 = (segment1->x1 < segment2->x1) ? segment1->x1 : segment2->x1;
        segment1->y1 = (segment1->y1 < segment2->y1) ? segment1->y1 : segment2->y1;
        segment1->x2 = (segment1->x2 > segment2->x2) ? segment1->x2 : segment2->x2;
        segment1->y2 = (segment1->y2 > segment2->y2) ? segment1->y2 : segment2->y2;
        stat = &segment1->stat;
        N = (stat->N += segment2->stat.N);
        stat->sum += segment2->stat.sum;
        stat->sum2 += segment2->stat.sum2;
        mean = stat->mean = stat->sum / N;
        variance = stat->sum2 / N - mean*mean;
        if (variance < 0) variance = 0;
        stat->variance = variance;
        stat->deviation = sqrt(variance);
      }
    }
  }
}

/******************************************************************************/
/* a private function that takes a segment instead of a tree                  */
/* allows using quad_tree in the public interface                             */

quad_forest_segment *segment_find
(
  quad_forest_segment *segment
)
{
  if (segment != NULL) {
    if (segment->parent != NULL && segment->parent != segment) {
      segment->parent = segment_find(segment->parent);
    }
    return segment->parent;
  }
  return NULL;
}

/******************************************************************************/

quad_forest_segment *quad_tree_segment_find
(
  quad_tree *tree
)
{
  if (tree != NULL) {
    return segment_find(&tree->segment);
  }
  return NULL;
}

/******************************************************************************/

uint32 quad_tree_segment_get
(
  quad_tree *tree
)
{
  return (uint32)quad_tree_segment_find(tree);
}

/******************************************************************************/

truth_value quad_tree_is_segment_parent
(
  quad_tree *tree
)
{
  if (tree != NULL) {
    if (quad_tree_segment_find(tree) == &tree->segment) {
      return TRUE;
    }
  }
  return FALSE;
}

/* end of file                                                                */
/******************************************************************************/
