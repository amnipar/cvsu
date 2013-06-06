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

#include "cvsu_macros.h"
#include "cvsu_segmentation.h"

#include <stdlib.h> /* for rand, srand */

/******************************************************************************/
/* constants for reporting function names in error messages                   */

string quad_forest_refresh_segments_name = "quad_forest_refresh_segments";
string quad_forest_segment_with_deviation_name = "quad_forest_segment_with_deviation";
string quad_forest_segment_with_overlap_name = "quad_forest_segment_with_overlap";
string quad_forest_segment_edges_name = "quad_forest_segment_edges";
string quad_forest_segment_with_boundaries_name = "quad_forest_segment_with_boundaries";

/******************************************************************************/

result quad_forest_refresh_segments
(
  quad_forest *target
)
{
  TRY();
  list_item *trees, *end;
  quad_tree *tree;
  segment *parent, *tree_segment;
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
      tree_segment = quad_tree_get_segment(tree);
      if (tree_segment != NULL) {
        parent = quad_tree_segment_find(tree);
        if (parent == tree_segment) {
          tree_segment->color[0] = (byte)(rand() % 256);
          tree_segment->color[1] = (byte)(rand() % 256);
          tree_segment->color[2] = (byte)(rand() % 256);
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
  segment *tree_segment, *neighbor_segment;
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
        CHECK(quad_tree_ensure_segment(tree, NULL));
      }
    }
    else {
      CHECK(quad_tree_ensure_segment(tree, NULL));
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
  segment *tree_segment, *neighbor_segment;
  statistics *stat;
  integral_value tm, ts, nm, ns, x1, x2, x1min, x1max, x2min, x2max, I, U, overlap, best_overlap;

  CHECK_POINTER(target);
  CHECK_PARAM(alpha > 0);
  CHECK_PARAM(threshold_trees > 0);
  CHECK_PARAM(threshold_segments > 0);

  /* first, divide until all trees are consistent */
  /*PRINT0("starting to divide trees\n");*/
  trees = target->trees.first.next;
  while (trees != &target->trees.last) {
    tree = (quad_tree *)trees->data;
    CHECK(quad_tree_divide_with_overlap(target, tree, alpha, threshold_trees));
    trees = trees->next;
  }

  /* then, merge each tree with the best neighboring tree that is close enough */
  /*PRINT0("starting to merge trees\n");*/
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
  /*PRINT0("starting to merge regions\n");*/
  trees = target->trees.first.next;
  end = &target->trees.last;
  while (trees != end) {
    tree = (quad_tree *)trees->data;
    tree_segment = quad_tree_segment_find(tree);
    /* only consider consistent trees (those that have not been divided) */
    if (tree->nw == NULL && tree_segment != NULL) {
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
  /*PRINT0("refreshing segments\n");*/
  /* finally, count regions and assign colors */
  CHECK(quad_forest_refresh_segments(target));
  
  FINALLY(quad_forest_segment_with_overlap);
  RETURN();
}

/* end of file                                                                */
/******************************************************************************/
