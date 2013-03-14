/**
 * @file cvsu_segmentation.c
 * @author Matti J. Eskelinen <matti.j.eskelinen@gmail.com>
 * @brief Functions for segmenting images.
 *
 * Copyright (c) 2013, Matti Johannes Eskelinen
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

#include "cvsu_segmentation.h"

/******************************************************************************/

result quad_forest_refresh_segments
(
  quad_forest *target
)
{
  TRY();
  list_item *trees, *end;
  quad_tree *tree;
  quad_forest_segment *parent, *segment;
  uint32 count;

  CHECK_POINTER(target);

  count = 0;
  /* initialize the random number generator for assigning the colors */
  srand(1234);

  trees = target->trees.first.next;
  end = &target->trees.last;
  while (trees != end) {
    tree = (quad_tree *)trees->data;
    if (tree->nw == NULL) {
      segment = &tree->segment;
      if (segment != NULL) {
        parent = quad_tree_segment_find(tree);
        if (parent == segment) {
          segment->color[0] = (byte)(rand() % 256);
          segment->color[1] = (byte)(rand() % 256);
          segment->color[2] = (byte)(rand() % 256);
          count++;
        }
      }
    }
    trees = trees->next;
  }
  target->segments = count;

  FINALLY(quad_forest_refresh_segments);
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
  list_item *trees, *end;
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
  end = &target->trees.last;
  while (trees != end) {
    tree = (quad_tree *)trees->data;
    if (tree->size >= 2 * min_size) {
      if (tree->stat.deviation > threshold) {
        CHECK(quad_tree_divide(target, tree));
      }
      else {
        quad_tree_segment_create(tree);
      }
    }
    else {
      quad_tree_segment_create(tree);
    }
    trees = trees->next;
  }

  /* then, make a union of those neighboring regions that are consistent together */
  /*printf("starting to merge trees\n");*/
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
  /*printf("starting to merge regions\n");*/
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
  CHECK(quad_forest_refresh_segments(target));

  FINALLY(quad_forest_segment_with_deviation);
  RETURN();
}

/******************************************************************************/

/* a macro for calculating neighbor overlap; neighbor should be statistics pointer */
#define EVALUATE_NEIGHBOR_OVERLAP(neighbor)\
  stat = neighbor;\
  nm = stat->mean;\
  ns = getmax(alpha, alpha * stat->deviation);\
  x1min = getmax(0, tm - ts);\
  x1max = x1min;\
  x2min = getmin(255, tm + ts);\
  x2max = x2min;\
  x1 = getmax(0, nm - ns);\
  x2 = getmin(255, nm + ns);\
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
  list_item *trees, *end;
  quad_tree *tree, *neighbor, *best_neighbor;
  quad_forest_segment *tree_segment, *neighbor_segment;
  statistics *stat;
  integral_value tm, ts, nm, ns, x1, x2, x1min, x1max, x2min, x2max, I, U, overlap, best_overlap;

  CHECK_POINTER(target);
  CHECK_PARAM(alpha > 0);
  CHECK_PARAM(threshold_trees > 0);
  CHECK_PARAM(threshold_segments > 0);

  /* first, divide until all trees are consistent */
  /*printf("starting to divide trees\n");*/
  trees = target->trees.first.next;
  while (trees != &target->trees.last) {
    tree = (quad_tree *)trees->data;
    CHECK(quad_tree_divide_with_overlap(target, tree, alpha, threshold_trees));
    trees = trees->next;
  }

  /* then, merge each tree with the best neighboring tree that is close enough */
  /*printf("starting to merge trees\n");*/
  trees = target->trees.first.next;
  end = &target->trees.last;
  while (trees != end) {
    tree = (quad_tree *)trees->data;
    tree_segment = quad_tree_segment_find(tree);
    /* only consider consistent trees (those that have not been divided) */
    if (tree->nw == NULL) {
      stat = &tree->stat;
      tm = stat->mean;
      ts = getmax(alpha, alpha * stat->deviation);

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
  /*printf("starting to merge regions\n");*/
  trees = target->trees.first.next;
  end = &target->trees.last;
  while (trees != end) {
    tree = (quad_tree *)trees->data;
    tree_segment = quad_tree_segment_find(tree);
    /* only consider consistent trees (those that have not been divided) */
    if (tree->nw == NULL) {
      stat = &tree_segment->stat;
      tm = stat->mean;
      ts = getmax(alpha, alpha * stat->deviation);

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
  CHECK(quad_forest_refresh_segments(target));

  FINALLY(quad_forest_segment_with_overlap);
  RETURN();
}

/******************************************************************************/

result quad_forest_segment_edges
(
  quad_forest *target,
  uint32 detect_rounds,
  integral_value detect_bias,
  direction detect_dir,
  uint32 propagate_rounds,
  integral_value propagate_threshold,
  direction propagate_dir,
  direction merge_dir
)
{
  TRY();
  uint32 i, size, remaining;
  quad_tree *tree, *neighbor;

  CHECK_POINTER(target);
  CHECK_PARAM(detect_rounds > 0);
  CHECK_PARAM(detect_dir == d_H || detect_dir == d_V || detect_dir == d_N4);
  CHECK_PARAM(propagate_dir == d_H || propagate_dir == d_V || propagate_dir == d_N4);
  CHECK_PARAM(merge_dir == d_H || merge_dir == d_V || merge_dir == d_N4);

  CHECK(quad_forest_find_edges(target, detect_rounds, detect_bias, detect_dir));

  size = target->rows * target->cols;

  /* prepare to propagate detected edges */
  for (i = 0; i < size; i++) {
    quad_tree_prime_with_edge(target->roots[i], 10);
  }

  /* propagate in desired direction */
  for (remaining = propagate_rounds; remaining--;) {
    if (propagate_dir == d_H) {
      for (i = 0; i < size; i++) {
        quad_tree_propagate_h(target->roots[i]);
      }
    }
    else
    if (propagate_dir == d_V) {
      for (i = 0; i < size; i++) {
        quad_tree_propagate_v(target->roots[i]);
      }
    }
    else {
      for (i = 0; i < size; i++) {
        quad_tree_propagate_m(target->roots[i]);
      }
    }
    if (remaining > 0) {
      for (i = 0; i < size; i++) {
        quad_tree_prime_with_pool(target->roots[i]);
      }
    }
  }

  /* now trees with pool value higher than threshold have edge */
  for (i = 0; i < size; i++) {
    tree = target->roots[i];
    if (tree->pool > propagate_threshold) {
      tree->edge.has_edge = TRUE;
    }
  }

  /* initialize segments with trees that have edge */
  for (i = 0; i < size; i++) {
    tree = target->roots[i];
    if (IS_TRUE(tree->edge.has_edge)) {
      quad_tree_segment_create(tree);
    }
  }

  /* then merge segments in the desired direction */
  if (merge_dir == d_H) {
    for (i = 0; i < size; i++) {
      tree = target->roots[i];
      if (IS_TRUE(tree->edge.has_edge)) {
        neighbor = tree->w;
        if (neighbor != NULL && IS_TRUE(neighbor->edge.has_edge)) {
          quad_tree_segment_union(tree, neighbor);
        }
        neighbor = tree->e;
        if (neighbor != NULL && IS_TRUE(neighbor->edge.has_edge)) {
          quad_tree_segment_union(tree, neighbor);
        }
      }
    }
  }
  else
  if (merge_dir == d_V) {
    for (i = 0; i < size; i++) {
      tree = target->roots[i];
      if (IS_TRUE(tree->edge.has_edge)) {
        neighbor = tree->n;
        if (neighbor != NULL && IS_TRUE(neighbor->edge.has_edge)) {
          quad_tree_segment_union(tree, neighbor);
        }
        neighbor = tree->s;
        if (neighbor != NULL && IS_TRUE(neighbor->edge.has_edge)) {
          quad_tree_segment_union(tree, neighbor);
        }
      }
    }
  }
  else {
    for (i = 0; i < size; i++) {
      tree = target->roots[i];
      if (IS_TRUE(tree->edge.has_edge)) {
        neighbor = tree->w;
        if (neighbor != NULL && IS_TRUE(neighbor->edge.has_edge)) {
          quad_tree_segment_union(tree, neighbor);
        }
        neighbor = tree->e;
        if (neighbor != NULL && IS_TRUE(neighbor->edge.has_edge)) {
          quad_tree_segment_union(tree, neighbor);
        }
        neighbor = tree->n;
        if (neighbor != NULL && IS_TRUE(neighbor->edge.has_edge)) {
          quad_tree_segment_union(tree, neighbor);
        }
        neighbor = tree->s;
        if (neighbor != NULL && IS_TRUE(neighbor->edge.has_edge)) {
          quad_tree_segment_union(tree, neighbor);
        }
      }
    }
  }

  /* finally, count regions and assign colors */
  quad_forest_refresh_segments(target);

  FINALLY(quad_forest_segment_edges);
  RETURN();
}

/******************************************************************************/

result quad_forest_segment_with_boundaries
(
  quad_forest *forest,
  uint32 rounds,
  integral_value high_bias,
  integral_value low_factor,
  integral_value tree_alpha,
  integral_value segment_alpha,
  truth_value use_hysteresis,
  truth_value use_pruning
)
{
  TRY();
  list_item *trees, *end;
  quad_tree *tree, *neighbor;
  quad_forest_segment *tree_segment, *neighbor_segment;
  integral_value dev, tree_dev, tree_mean, neighbor_dev, neighbor_mean, dist;

  CHECK_POINTER(forest);

  /* first find boundaries (and establish devmean and devdev) */
  if (IS_TRUE(use_hysteresis)) {
    CHECK(quad_forest_find_boundaries_with_hysteresis(forest, rounds, high_bias, low_factor));
  }
  else {
    CHECK(quad_forest_find_boundaries(forest, rounds, high_bias, 3));
  }

  /* then merge consistent non-boundary neighbors */
  trees = forest->trees.first.next;
  end = &forest->trees.last;
  while (trees != end) {
    tree = (quad_tree*)trees->data;
    if (IS_FALSE(tree->segment.has_boundary)) {
      tree_segment = quad_tree_segment_find(tree);
      if (tree_segment == NULL) {
        tree_mean = tree->stat.mean;
        tree_dev = getmax(1, tree->segment.devmean + tree->segment.devdev);

        neighbor = tree->n;
        if (neighbor != NULL && IS_FALSE(neighbor->segment.has_boundary)) {
          neighbor_mean = neighbor->stat.mean;
          neighbor_dev = getmax(1, neighbor->segment.devmean + neighbor->segment.devdev);

          dev = getmin(tree_dev, neighbor_dev);
          dist = fabs(tree_mean - neighbor_mean);
          if (dist < tree_alpha * dev) {
            quad_tree_segment_create(tree);
            quad_tree_segment_create(neighbor);
            quad_tree_segment_union(tree, neighbor);
          }
        }
        neighbor = tree->e;
        if (neighbor != NULL && IS_FALSE(neighbor->segment.has_boundary)) {
          neighbor_mean = neighbor->stat.mean;
          neighbor_dev = getmax(1, neighbor->segment.devmean + neighbor->segment.devdev);

          dev = getmin(tree_dev, neighbor_dev);
          dist = fabs(tree_mean - neighbor_mean);
          if (dist < tree_alpha * dev) {
            quad_tree_segment_create(tree);
            quad_tree_segment_create(neighbor);
            quad_tree_segment_union(tree, neighbor);
          }
        }
        neighbor = tree->s;
        if (neighbor != NULL && IS_FALSE(neighbor->segment.has_boundary)) {
          neighbor_mean = neighbor->stat.mean;
          neighbor_dev = getmax(1, neighbor->segment.devmean + neighbor->segment.devdev);

          dev = getmin(tree_dev, neighbor_dev);
          dist = fabs(tree_mean - neighbor_mean);
          if (dist < tree_alpha * dev) {
            quad_tree_segment_create(tree);
            quad_tree_segment_create(neighbor);
            quad_tree_segment_union(tree, neighbor);
          }
        }
        neighbor = tree->w;
        if (neighbor != NULL && IS_FALSE(neighbor->segment.has_boundary)) {
          neighbor_mean = neighbor->stat.mean;
          neighbor_dev = getmax(1, neighbor->segment.devmean + neighbor->segment.devdev);

          dev = getmin(tree_dev, neighbor_dev);
          dist = fabs(tree_mean - neighbor_mean);
          if (dist < tree_alpha * dev) {
            quad_tree_segment_create(tree);
            quad_tree_segment_create(neighbor);
            quad_tree_segment_union(tree, neighbor);
          }
        }
      }
    }
    trees = trees->next;
  }

  /* then merge consistent segments */
  trees = forest->trees.first.next;
  end = &forest->trees.last;
  while (trees != end) {
    tree = (quad_tree *)trees->data;
    tree_segment = quad_tree_segment_find(tree);
    if (tree->nw == NULL && tree_segment != NULL) {
      tree_mean = tree_segment->stat.mean;
      tree_dev = getmax(1, tree_segment->stat.deviation);

      neighbor = tree->n;
      if (neighbor != NULL && neighbor->nw == NULL) {
        neighbor_segment = quad_tree_segment_find(neighbor);
        if (neighbor_segment != NULL && tree_segment != neighbor_segment) {
          neighbor_mean = neighbor_segment->stat.mean;
          neighbor_dev = getmax(1, neighbor_segment->stat.deviation);
          dev = getmin(tree_dev, neighbor_dev);
          dist = fabs(tree_mean - neighbor_mean);
          if (dist < segment_alpha * dev) {
            quad_tree_segment_union(tree, neighbor);
          }
        }
      }
      neighbor = tree->e;
      if (neighbor != NULL && neighbor->nw == NULL) {
        neighbor_segment = quad_tree_segment_find(neighbor);
        if (neighbor_segment != NULL && tree_segment != neighbor_segment) {
          neighbor_mean = neighbor_segment->stat.mean;
          neighbor_dev = getmax(1, neighbor_segment->stat.deviation);
          dev = getmin(tree_dev, neighbor_dev);
          dist = fabs(tree_mean - neighbor_mean);
          if (dist < segment_alpha * dev) {
            quad_tree_segment_union(tree, neighbor);
          }
        }
      }
      neighbor = tree->s;
      if (neighbor != NULL && neighbor->nw == NULL) {
        neighbor_segment = quad_tree_segment_find(neighbor);
        if (neighbor_segment != NULL && tree_segment != neighbor_segment) {
          neighbor_mean = neighbor_segment->stat.mean;
          neighbor_dev = getmax(1, neighbor_segment->stat.deviation);
          dev = getmin(tree_dev, neighbor_dev);
          dist = fabs(tree_mean - neighbor_mean);
          if (dist < segment_alpha * dev) {
            quad_tree_segment_union(tree, neighbor);
          }
        }
      }
      neighbor = tree->w;
      if (neighbor != NULL && neighbor->nw == NULL) {
        neighbor_segment = quad_tree_segment_find(neighbor);
        if (neighbor_segment != NULL && tree_segment != neighbor_segment) {
          neighbor_mean = neighbor_segment->stat.mean;
          neighbor_dev = getmax(1, neighbor_segment->stat.deviation);
          dev = getmin(tree_dev, neighbor_dev);
          dist = fabs(tree_mean - neighbor_mean);
          if (dist < segment_alpha * dev) {
            quad_tree_segment_union(tree, neighbor);
          }
        }
      }
    }
    trees = trees->next;
  }

  if (IS_TRUE(use_pruning)) {
    CHECK(quad_forest_prune_boundaries(forest));
  }

  /* finally refresh segments and assign colors */
  CHECK(quad_forest_refresh_segments(forest));

  FINALLY(quad_forest_segment_with_boundaries);
  RETURN();
}

/* end of file                                                                */
/******************************************************************************/
