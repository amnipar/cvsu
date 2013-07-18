/**
 * @file cvsu_parsing.c
 * @author Matti J. Eskelinen <matti.j.eskelinen@gmail.com>
 * @brief Functions for parsing images.
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
#include "cvsu_parsing.h"
#include "cvsu_annotation.h"
#include "cvsu_opencv.h"

#include <stdlib.h> /* for rand */

/******************************************************************************/
/* constants for reporting function names in error messages                   */

string prime_stat_accumulator_name = "prime_stat_accumulator";
string prop_stat_accumulator_name = "prop_stat_accumulator";
string acc_stat_accumulator_name = "acc_stat_accumulator";
string run_context_operation_name = "run_context_operation";
string quad_forest_calculate_accumulated_stats_name = "quad_forest_calculate_accumulated_stats";
string quad_forest_visualize_accumulated_stats_name = "quad_forest_visualize_accumulated_stats";
string quad_forest_calculate_neighborhood_stats_name = "quad_forest_calculate_neighborhood_stats";
string quad_forest_visualize_neighborhood_stats_name = "quad_forest_visualize_neighborhood_stats";
string quad_tree_ensure_edge_stats_name = "quad_tree_ensure_edge_stats";
string quad_forest_calculate_edge_stats_name = "quad_forest_calculate_edge_stats";
string quad_tree_link_head_ensure_measure_name = "quad_tree_link_head_ensure_measure";
string quad_tree_ensure_edge_profiles_name = "quad_tree_ensure_edge_profiles";
string quad_tree_ensure_edge_links_name = "quad_tree_ensure_edge_links";
string quad_tree_ensure_ridge_potential_name = "quad_tree_ensure_ridge_potential";
string consider_edge_links_name = "consider_edge_links";
string prune_edge_links_name = "prune_edge_links";
string extend_edge_links_name = "extend_edge_links";
string quad_forest_parse_name = "quad_forest_parse";
string quad_forest_visualize_parse_result_name = "quad_forest_visualize_parse_result";

/******************************************************************************/

result prime_stat_accumulator
(
  quad_forest *forest,
  quad_tree *tree,
  list *collection
)
{
  TRY();
  stat_accumulator *acc;

  CHECK_POINTER(tree);

  CHECK(ensure_stat_accumulator(&tree->context, &acc));
  if (acc->round == 0) {
    integral_value mean, dev;

    mean = tree->stat.mean;
    dev = tree->stat.deviation;

    acc->mean_acc1 = mean / 2;
    acc->mean_pool1 = acc->mean_acc1;
    acc->mean_acc2 = mean * acc->mean_acc1;
    acc->mean_pool2 = acc->mean_acc2;

    acc->dev_acc1 = dev / 2;
    acc->dev_pool1 = acc->dev_acc1;
    acc->dev_acc2 = dev * acc->dev_acc1;
    acc->dev_pool2 = acc->dev_acc2;

    acc->round = 1;
  }
  else {
    acc->mean_acc1 = acc->mean_pool1 / 2;
    acc->mean_pool1 = acc->mean_acc1;
    acc->mean_acc2 = acc->mean_pool2 / 2;
    acc->mean_pool2 = acc->mean_acc2;

    acc->dev_acc1 = acc->dev_pool1 / 2;
    acc->dev_pool1 = acc->dev_acc1;
    acc->dev_acc2 = acc->dev_pool2 / 2;
    acc->dev_pool2 = acc->dev_acc2;

    acc->round++;
  }

  FINALLY(prime_stat_accumulator);
  (void)forest;
  (void)collection;
  RETURN();
}

/******************************************************************************/

#define NEIGHBOR_PROP_STAT(neighbor)\
if ((neighbor) != NULL) {\
    neighbor_acc = has_stat_accumulator(&(neighbor)->context);\
    CHECK_POINTER(neighbor_acc);\
    neighbor_acc->mean_pool1 += mean_pool1;\
    neighbor_acc->mean_pool2 += mean_pool2;\
    neighbor_acc->dev_pool1 += dev_pool1;\
    neighbor_acc->dev_pool2 += dev_pool2;\
  }\
  else {\
    tree_acc->mean_pool1 += mean_pool1;\
    tree_acc->mean_pool2 += mean_pool2;\
    tree_acc->dev_pool1 += dev_pool1;\
    tree_acc->dev_pool2 += dev_pool2;\
  }

result prop_stat_accumulator
(
  quad_forest *forest,
  quad_tree *tree,
  list *collection
)
{
  TRY();
  stat_accumulator *tree_acc, *neighbor_acc;
  integral_value mean_pool1, mean_pool2, dev_pool1, dev_pool2;

  CHECK_POINTER(tree);
  tree_acc = has_stat_accumulator(&tree->context);
  CHECK_POINTER(tree_acc);

  mean_pool1 = tree_acc->mean_acc1 / 4;
  mean_pool2 = tree_acc->mean_acc2 / 4;
  dev_pool1 = tree_acc->dev_acc1 / 4;
  dev_pool2 = tree_acc->dev_acc2 / 4;

  /* TODO: use neighbor links and link categorization instead */
  /* neighbor n */
  NEIGHBOR_PROP_STAT(tree->n);
  NEIGHBOR_PROP_STAT(tree->e);
  NEIGHBOR_PROP_STAT(tree->s);
  NEIGHBOR_PROP_STAT(tree->w);

  FINALLY(prop_stat_accumulator);
  (void)forest;
  (void)collection;
  RETURN();
}

/******************************************************************************/

result acc_stat_accumulator
(
  quad_forest *forest,
  quad_tree *tree,
  list *collection
)
{
  TRY();
  stat_accumulator *acc;
  accumulated_stat *astat;
  integral_value mean, dev;

  CHECK_POINTER(tree);
  acc = has_stat_accumulator(&tree->context);
  CHECK_POINTER(acc);
  CHECK(ensure_accumulated_stat(&tree->annotation, &astat));

  mean = acc->mean_pool1;
  dev = acc->mean_pool2;
  dev -= mean*mean;
  if (dev < 0) dev = 0; else dev = sqrt(dev);
  astat->meanmean = mean;
  astat->meandev = dev;

  mean = acc->dev_pool1;
  dev = acc->dev_pool2;
  dev -= mean*mean;
  if (dev < 0) dev = 0; else dev = sqrt(dev);
  astat->devmean = mean;
  astat->devdev = dev;

  astat->strength = 0;

  FINALLY(acc_stat_accumulator);
  (void)forest;
  (void)collection;
  RETURN();
}

/******************************************************************************/

result run_context_operation
(
  quad_forest *forest,
  list *input_trees,
  list *output_trees,
  context_operation prime_operation,
  context_operation propagate_operation,
  context_operation accumulate_operation,
  uint32 rounds,
  truth_value needs_list
)
{
  TRY();
  uint32 remaining;
  list_item *trees, *end;

  /* if a new list needs to be generated, how to do it? */
  /* new list for each round, use output list for accumulate round? */
  if (IS_TRUE(needs_list)) {

  }
  else {
    for (remaining = rounds; remaining--;) {
      trees = input_trees->first.next;
      end = &input_trees->last;
      while (trees != end) {
        CHECK(prime_operation(forest, (quad_tree*)trees->data, NULL));
        trees = trees->next;
      }
      trees = input_trees->first.next;
      end = &input_trees->last;
      while (trees != end) {
        CHECK(propagate_operation(forest, (quad_tree*)trees->data, NULL));
        trees = trees->next;
      }
    }
    trees = input_trees->first.next;
    end = &input_trees->last;
    while (trees != end) {
      CHECK(accumulate_operation(forest, (quad_tree*)trees->data, NULL));
      trees = trees->next;
    }
  }

  FINALLY(run_context_operation);
  (void)output_trees;
  RETURN();
}

/******************************************************************************/

result quad_forest_calculate_accumulated_stats
(
  quad_forest *forest,
  uint32 rounds
)
{
  TRY();
  list_item *trees, *end;
  quad_tree *tree;
  accumulated_stat *astat;
  integral_value maxmeandev, maxdevdev, meandev, devdev;

  CHECK_POINTER(forest);

  CHECK(run_context_operation(forest, &forest->trees, NULL,
                              prime_stat_accumulator,
                              prop_stat_accumulator,
                              acc_stat_accumulator,
                              rounds, FALSE));

  maxmeandev = 0;
  maxdevdev = 0;
  trees = forest->trees.first.next;
  end = &forest->trees.last;
  while (trees != end) {
    tree = (quad_tree*)trees->data;
    if (tree->nw == NULL) {
      astat = has_accumulated_stat(&tree->annotation);
      if (astat != NULL) {
        if (astat->meandev > maxmeandev) {
          maxmeandev = astat->meandev;
        }
        if (astat->devdev > maxdevdev) {
          maxdevdev = astat->devdev;
        }
      }
    }
    trees = trees->next;
  }

  trees = forest->trees.first.next;
  end = &forest->trees.last;
  while (trees != end) {
    tree = (quad_tree*)trees->data;
    if (tree->nw == NULL) {
      astat = has_accumulated_stat(&tree->annotation);
      if (astat != NULL) {
        meandev = astat->meandev / maxmeandev;
        astat->meandev = meandev;
        devdev = astat->devdev / maxdevdev;
        astat->devdev = devdev;
        astat->strength = (0.5 * meandev) + (0.5 * devdev);
      }
    }
    trees = trees->next;
  }

  FINALLY(quad_forest_calculate_accumulated_stats);
  RETURN();
}

/******************************************************************************/

result quad_forest_visualize_accumulated_stats
(
  quad_forest *forest,
  pixel_image *target
)
{
  TRY();
  list_item *trees, *end;
  quad_tree *tree;
  accumulated_stat *astat;
  uint32 x, y, width, height, stride, row_step;
  byte *target_data, *target_pos, color, color0, color1, color2;

  CHECK_POINTER(forest);
  CHECK_POINTER(target);

  width = target->width;
  height = target->height;
  stride = target->stride;
  target_data = (byte*)target->data;

  CHECK(pixel_image_clear(target));

  trees = forest->trees.first.next;
  end = &forest->trees.last;
  while (trees != end) {
    tree = (quad_tree*)trees->data;
    if (tree->nw == NULL) {
      astat = has_accumulated_stat(&tree->annotation);
      if (astat != NULL) {
        color = (byte)(255 * astat->strength);
        color0 = (byte)(255 * astat->strength * astat->meandev);
        color1 = color;
        color2 = (byte)(255 * astat->strength * astat->devdev);
      }
      width = tree->size;
      height = width;
      row_step = stride - 3 * width;
      target_pos = target_data + tree->y * stride + 3 * tree->x;
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
    trees = trees->next;
  }

  FINALLY(quad_forest_visualize_accumulated_stats);
  RETURN();
}

/******************************************************************************/

result quad_forest_calculate_neighborhood_stats
(
  quad_forest *forest,
  truth_value with_overlap
)
{
  TRY();
  list_item *trees, *endtrees, *links, *endlinks;
  typed_pointer *tptr;
  neighborhood_stat *nstat;
  quad_tree_link_head *head;
  quad_tree *tree, *neighbor;
  integral_value weight, count, wcount, strength;
  integral_value mean, mean_sum1, mean_sum2, mean_wsum1;
  integral_value dev, dev_sum1, dev_sum2, dev_wsum1, dev_min, dev_max;
  integral_value mean_mean, mean_dev, max_mean_dev, dev_mean, dev_dev, max_dev_dev;
  integral_value overlap, min_overlap, max_overlap, scale;

  CHECK_POINTER(forest);

  if (IS_FALSE(with_overlap)) {
    max_mean_dev = 0;
    max_dev_dev = 0;
    trees = forest->trees.first.next;
    endtrees = &forest->trees.last;
    while (trees != endtrees) {
      tree = (quad_tree*)trees->data;
      CHECK(ensure_has(&tree->annotation, t_neighborhood_stat, &tptr));
      nstat = (neighborhood_stat*)tptr->value;
      if (tptr->token != forest->token) {
        tptr->token = forest->token;
        mean = tree->stat.mean;
        mean_sum1 = mean;
        mean_sum2 = mean * mean;

        dev = tree->stat.deviation;
        dev_sum1 = dev;
        dev_sum2 = dev * dev;

        count = 1;
        dev_min = dev;
        dev_max = dev;
        links = tree->links.first.next;
        endlinks = &tree->links.last;
        while (links != endlinks) {
          head = *((quad_tree_link_head**)links->data);
          if (head->link->category != d_N6) {
            neighbor = head->other->tree;

            mean = neighbor->stat.mean;
            mean_sum1 += mean;
            mean_sum2 += (mean * mean);

            dev = neighbor->stat.deviation;
            dev_sum1 += dev;
            dev_sum2 += (dev * dev);
            if (dev > dev_max) {
              dev_max = dev;
            }
            if (dev < dev_min) {
              dev_min = dev;
            }

            count += 1;
          }
          links = links->next;
        }

        mean_mean = mean_sum1 / count;
        mean_dev = mean_sum2 / count - mean_mean * mean_mean;
        mean_dev = mean_dev < 0 ? 0 : sqrt(mean_dev);
        dev_mean = dev_sum1 / count;
        dev_dev = dev_sum2 / count - dev_mean * dev_mean;
        dev_dev = dev_dev < 0 ? 0 : sqrt(dev_dev);

        nstat->mean_mean = mean_mean;
        nstat->mean_dev = mean_dev;
        if (mean_dev > max_mean_dev) {
          max_mean_dev = mean_dev;
        }
        nstat->dev_mean = dev_mean;
        nstat->dev_dev = dev_dev;
        if (dev_dev > max_dev_dev) {
          max_dev_dev = dev_dev;
        }
        strength = (tree->stat.deviation - dev_min) / getmax(1,(dev_max - dev_min));
        /*
        if (strength > 1) {
          strength = 1;
        }
        */
        nstat->strength = strength;
        nstat->overlap = 0;
        nstat->mean_ridge_score = 0;
        nstat->mean_ledge_score = 0;
        nstat->dev_ridge_score = 0;
        nstat->dev_ledge_score = 0;
      }
      trees = trees->next;
    }
    /* calculate strength */
    /*weight = 0.5;*/
    /*
    trees = forest->trees.first.next;
    endtrees = &forest->trees.last;
    while (trees != endtrees) {
      tree = (quad_tree*)trees->data;
      nstat = has_neighborhood_stat(&tree->annotation);
      if (nstat != NULL) {
        mean_dev = nstat->mean_dev / max_mean_dev;
        dev_dev = nstat->dev_dev / max_dev_dev;
        nstat->strength *= mean_dev;
        nstat->strength = getmax(mean_dev, dev_dev);
      }
      trees = trees->next;
    }
    */
    /*nstat->strength = (weight * mean_dev) + (weight * dev_dev);*/
  }
  else {
    min_overlap = 1;
    max_overlap = 0;
    max_mean_dev = 0;
    max_dev_dev = 0;
    trees = forest->trees.first.next;
    endtrees = &forest->trees.last;
    while (trees != endtrees) {
      tree = (quad_tree*)trees->data;
      CHECK(ensure_has(&tree->annotation, t_neighborhood_stat, &tptr));
      nstat = (neighborhood_stat*)tptr->value;
      if (tptr->token != forest->token) {
        weight = -4;

        mean = tree->stat.mean;
        mean_sum1 = mean;
        mean_sum2 = mean * mean;
        mean_wsum1 = weight * mean;

        dev = tree->stat.deviation;
        dev_sum1 = dev;
        dev_sum2 = dev * dev;
        dev_wsum1 = weight * dev;

        if (dev > dev_max) {
          dev_max = dev;
        }

        count = 1;
        wcount = weight;

        links = tree->links.first.next;
        endlinks = &tree->links.last;
        while (links != endlinks) {
          head = *((quad_tree_link_head**)links->data);
          if (head->link->category != d_N6) {
            neighbor = head->other->tree;
            weight = 2 / head->link->distance;

            mean = neighbor->stat.mean;
            mean_sum1 += mean;
            mean_sum2 += mean * mean;
            mean_wsum1 += weight * mean;

            dev = neighbor->stat.deviation;
            dev_sum1 += dev;
            dev_sum2 += dev * dev;
            dev_wsum1 += weight * dev;

            count += 1;
            wcount += weight;
          }
          links = links->next;
        }

        /* calculate overlap */
        {
          integral_value x1, x2, x1min, x1max, x2min, x2max, I, U;

          mean_mean = mean_wsum1 / wcount;
          dev_mean = dev_wsum1 / wcount;
          dev_mean = getmax(1, dev_mean);

          x1min = getmax(0, mean_mean - dev_mean);
          x1max = x1min;
          x2min = getmin(mean_mean + dev_mean, 255);
          x2max = x2min;

          mean = tree->stat.mean;
          dev = getmax(1, tree->stat.deviation);
          x1 = getmax(0, mean - dev);
          if (x1 < x1min) x1min = x1; else x1max = x1;
          x2 = getmin(mean + dev, 255);
          if (x2 < x2min) x2min = x2; else x2max = x2;
          if (x1max - x2min > 0.001) {
            I = 0;
          }
          else {
            I = (x2min - x1max);
            if (I < 1) I = 1;
          }
          U = (x2max - x1min);
          if (U < 1) U = 1;

          overlap = I / U;
          nstat->overlap = overlap;
          if (overlap < min_overlap) min_overlap = overlap;
          if (overlap > max_overlap) max_overlap = overlap;
        }

        mean_mean = mean_sum1 / count;
        mean_dev = mean_sum2 / count - mean_mean * mean_mean;
        mean_dev = mean_dev < 0 ? 0 : sqrt(mean_dev);
        dev_mean = dev_sum1 / count;
        dev_dev = dev_sum2 / count - dev_mean * dev_mean;
        dev_dev = dev_dev < 0 ? 0 : sqrt(dev_dev);

        nstat->mean_mean = mean_mean;
        nstat->mean_dev = mean_dev;
        if (mean_dev > max_mean_dev) {
          max_mean_dev = mean_dev;
        }
        nstat->dev_mean = dev_mean;
        nstat->dev_dev = dev_dev;
        if (dev_dev > max_dev_dev) {
          max_dev_dev = dev_dev;
        }

        strength = (dev_max - dev) / dev_dev;
        if (strength > 1) {
          strength = 1;
        }
        nstat->strength = 1 - strength;
        nstat->mean_ridge_score = 0;
        nstat->mean_ledge_score = 0;
        nstat->dev_ridge_score = 0;
        nstat->dev_ledge_score = 0;

        tptr->token = forest->token;
      }
      trees = trees->next;
    }
    /* scale overlap and calculate strength */
    /*weight = 0.333333;*/
    scale = max_overlap - min_overlap;
    trees = forest->trees.first.next;
    endtrees = &forest->trees.last;
    while (trees != endtrees) {
      tree = (quad_tree*)trees->data;
      nstat = has_neighborhood_stat(&tree->annotation);
      if (nstat != NULL) {
        overlap = (nstat->overlap - min_overlap) / scale;
        mean_dev = nstat->mean_dev / max_mean_dev;
        dev_dev = nstat->dev_dev / max_dev_dev;
        nstat->overlap = overlap;
        nstat->strength *= overlap;
        /*nstat->strength =
            (weight * mean_dev) + (weight * dev_dev) + (weight * overlap);*/
      }
      trees = trees->next;
    }
  }

  /* */
  /*
  scale = max_overlap - min_overlap;
  trees = forest->trees.first.next;
  endtrees = &forest->trees.last;
  while (trees != endtrees) {
    tree = (quad_tree*)trees->data;
    nstat = has_neighborhood_stat(&tree->annotation);
    if (nstat != NULL) {
      nstat->overlap = (nstat->overlap - min_overlap) / scale;
    }
    trees = trees->next;
  }
  */

  FINALLY(quad_forest_calculate_neighborhood_stats);
  RETURN();
}

/******************************************************************************/

result quad_forest_visualize_neighborhood_stats
(
  quad_forest *forest,
  pixel_image *target,
  stat_visualization_mode smode
)
{
  TRY();
  list_item *trees, *end;
  quad_tree *tree;
  neighborhood_stat *nstat;
  uint32 x, y, width, height, stride, row_step;
  byte *target_data, *target_pos, color0, color1, color2;

  CHECK_POINTER(forest);
  CHECK_POINTER(target);
  CHECK_PARAM(target->type == p_U8);
  CHECK_PARAM(target->format == RGB);

  width = target->width;
  height = target->height;
  stride = target->stride;
  target_data = (byte*)target->data;

  CHECK(pixel_image_clear(target));

  switch (smode) {
    case v_STAT:
      {
        integral_value mean, min_mean, max_mean, dev, min_dev, max_dev, mean_scale, dev_scale;

        min_mean = 255;
        max_mean = 0;
        min_dev = 128;
        max_dev = 0;
        trees = forest->trees.first.next;
        end = &forest->trees.last;
        while (trees != end) {
          tree = (quad_tree*)trees->data;
          if (tree->nw == NULL) {
            mean = tree->stat.mean;
            if (mean < min_mean) min_mean = mean;
            if (mean > max_mean) max_mean = mean;
            dev = tree->stat.deviation;
            if (dev < min_dev) min_dev = dev;
            if (dev > max_dev) max_dev = dev;
          }
          trees = trees->next;
        }

        mean_scale = max_mean - min_mean;
        dev_scale = max_dev - min_dev;
        trees = forest->trees.first.next;
        end = &forest->trees.last;
        while (trees != end) {
          tree = (quad_tree*)trees->data;
          if (tree->nw == NULL) {
            mean = (tree->stat.mean - min_mean) / mean_scale;
            dev = (tree->stat.deviation - min_dev) / dev_scale;

            color0 = (byte)(255 * mean);
            color1 = 0;
            color2 = (byte)(255 * dev);

            width = tree->size;
            height = width;
            row_step = stride - 3 * width;
            target_pos = target_data + tree->y * stride + 3 * tree->x;
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
          trees = trees->next;
        }
      }
      break;
    case v_NSTAT:
      {
        integral_value mean, min_mean, max_mean, dev, min_dev, max_dev, mean_scale, dev_scale;

        min_mean = 128;
        max_mean = 0;
        min_dev = 128;
        max_dev = 0;
        trees = forest->trees.first.next;
        end = &forest->trees.last;
        while (trees != end) {
          tree = (quad_tree*)trees->data;
          if (tree->nw == NULL) {
            nstat = has_neighborhood_stat(&tree->annotation);
            if (nstat != NULL) {
              mean = nstat->mean_dev;
              if (mean < min_mean) min_mean = mean;
              if (mean > max_mean) max_mean = mean;
              dev = nstat->dev_dev;
              if (dev < min_dev) min_dev = dev;
              if (dev > max_dev) max_dev = dev;
            }
          }
          trees = trees->next;
        }

        mean_scale = max_mean - min_mean;
        dev_scale = max_dev - min_dev;
        trees = forest->trees.first.next;
        end = &forest->trees.last;
        while (trees != end) {
          tree = (quad_tree*)trees->data;
          if (tree->nw == NULL) {
            nstat = has_neighborhood_stat(&tree->annotation);
            if (nstat != NULL) {
              mean = (nstat->mean_dev - min_mean) / mean_scale;
              dev = (nstat->dev_dev - min_dev) / dev_scale;

              color0 = (byte)(255 * mean);
              color1 = 0;
              color2 = (byte)(255 * dev);

              width = tree->size;
              height = width;
              row_step = stride - 3 * width;
              target_pos = target_data + tree->y * stride + 3 * tree->x;
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
      break;
    case v_OVERLAP:
      {
        integral_value overlap;

        trees = forest->trees.first.next;
        end = &forest->trees.last;
        while (trees != end) {
          tree = (quad_tree*)trees->data;
          if (tree->nw == NULL) {
            nstat = has_neighborhood_stat(&tree->annotation);
            if (nstat != NULL) {
              overlap = nstat->overlap;
              color0 = (byte)(255 * overlap);
              color1 = 0;
              color2 = (byte)(255 * (1 - overlap));
            }
            width = tree->size;
            height = width;
            row_step = stride - 3 * width;
            target_pos = target_data + tree->y * stride + 3 * tree->x;
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
          trees = trees->next;
        }
      }
      break;
    case v_STRENGTH:
      {
        integral_value strength, min_strength, max_strength;

        min_strength = 1;
        max_strength = 0;
        trees = forest->trees.first.next;
        end = &forest->trees.last;
        while (trees != end) {
          tree = (quad_tree*)trees->data;
          if (tree->nw == NULL) {
            nstat = has_neighborhood_stat(&tree->annotation);
            if (nstat != NULL) {
              strength = fabs(nstat->strength);
              if (strength > max_strength) max_strength = strength;
            }
          }
          trees = trees->next;
        }
        trees = forest->trees.first.next;
        end = &forest->trees.last;
        while (trees != end) {
          tree = (quad_tree*)trees->data;
          if (tree->nw == NULL) {
            nstat = has_neighborhood_stat(&tree->annotation);
            if (nstat != NULL) {
              strength = fabs(nstat->strength) / max_strength;
              color0 = (byte)(255 * strength);
              color1 = 0;
              color2 = (byte)(255 * (1 - strength));
            }
            width = tree->size;
            height = width;
            row_step = stride - 3 * width;
            target_pos = target_data + tree->y * stride + 3 * tree->x;
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
          trees = trees->next;
        }
      }
      break;
    case v_SCORE:
      {
        integral_value score, total_score;

        trees = forest->trees.first.next;
        end = &forest->trees.last;
        while (trees != end) {
          tree = (quad_tree*)trees->data;
          if (tree->nw == NULL) {
            nstat = has_neighborhood_stat(&tree->annotation);
            if (nstat != NULL) {
              score = nstat->mean_ledge_score;
              if (score < 1) score = 0;
              else score = (getmin(3.0, score) - 1) / 2;
              color0 = (byte)(255 * score);
              total_score = score;
              score = fabs(nstat->mag_ridge_score);
              if (score < 0.5) score = 0;
              else score = (getmin(1.5, score) - 0.5);
              color1 = (byte)(255 * score);
              total_score += score;
              score = fabs(nstat->dev_ridge_score);
              if (score < 0.5) score = 0;
              else score = (getmin(1.5, score) - 0.5);
              color2 = (byte)(255 * score);
              total_score += (2 * score);
              total_score /= 4;

              color0 = (byte)(255 * total_score);
              color1 = color0;
              color2 = color0;

            }
            else {
              color0 = 0;
              color1 = 0;
              color2 = 0;
            }
            width = tree->size;
            height = width;
            row_step = stride - 3 * width;
            target_pos = target_data + tree->y * stride + 3 * tree->x;
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
          trees = trees->next;
        }
      }
      break;
    default:
      ERROR(BAD_PARAM);
  }

  FINALLY(quad_forest_visualize_neighborhood_stats);
  RETURN();
}

/******************************************************************************/

result quad_tree_ensure_edge_stats
(
  quad_forest *forest,
  quad_tree *tree,
  neighborhood_stat **stat
)
{
  TRY();
  integral_value count, mean, mean_sum1, mean_sum2, dev, dev_sum1, dev_sum2;
  integral_value mean_mean, mean_dev, dev_mean, dev_dev;
  integral_value tree_mean, tree_dev, angle1, angle2, weight;
  integral_value left_weight, left_mean_sum, left_dev_sum, left_mag_sum;
  integral_value right_weight, right_mean_sum, right_dev_sum, right_mag_sum;
  integral_value towards_weight, towards_mean_sum, towards_dev_sum, towards_mag_sum;
  integral_value against_weight, against_mean_sum, against_dev_sum, against_mag_sum;
  integral_value score, adjust, counter_score, counter_adjust;
  typed_pointer *tptr;
  neighborhood_stat *nstat;
  edge_response *eresp1, *eresp2;
  link_measure *lmeasure;
  list_item *links, *endlinks;
  quad_tree_link_head *head;
  quad_tree *neighbor;

  CHECK_POINTER(forest);
  CHECK_POINTER(tree);

  CHECK(ensure_has(&tree->annotation, t_neighborhood_stat, &tptr));
  nstat = (neighborhood_stat*)tptr->value;
  if (tptr->token != forest->token) {
    tptr->token = forest->token;
    CHECK(quad_tree_ensure_edge_response(forest, tree, &eresp1, TRUE));

    left_weight = 0;
    left_mean_sum = 0;
    left_dev_sum = 0;
    left_mag_sum = 0;
    right_weight = 0;
    right_mean_sum = 0;
    right_dev_sum = 0;
    right_mag_sum = 0;
    towards_weight = 0;
    towards_mean_sum = 0;
    towards_dev_sum = 0;
    towards_mag_sum = 0;
    against_weight = 0;
    against_mean_sum = 0;
    against_dev_sum = 0;
    against_mag_sum = 0;

    tree_mean = tree->stat.mean;
    tree_dev = tree->stat.deviation;

    mean_sum1 = tree_mean;
    mean_sum2 = tree_mean*tree_mean;
    count = 1;

    links = tree->links.first.next;
    endlinks = &tree->links.last;
    while (links != endlinks) {
      head = *((quad_tree_link_head**)links->data);
      neighbor = head->other->tree;
      CHECK(quad_tree_ensure_edge_response(forest, neighbor, &eresp2, TRUE));
      count += 1;

      mean = neighbor->stat.mean;
      mean_sum1 += mean;
      mean_sum2 += (mean*mean);

      dev = neighbor->stat.deviation;
      dev_sum1 += dev;
      dev_sum2 += (dev*dev);

      CHECK(ensure_has(&head->annotation, t_link_measure, &tptr));
      lmeasure = (link_measure*)tptr->value;
      if (tptr->token != forest->token) {
        tptr->token = forest->token;
        angle1 = eresp1->ang - M_PI_2;
        if (angle1 < 0) angle1 += M_2PI;
        angle2 = angle_minus_angle(head->angle, angle1);
        /* against score will be from 0 to 1 */
        /* smaller than 0.5 for towards, larger than 0.5 for against */
        lmeasure->against_score = fabs(angle2 / M_PI);
        if (angle2 < -M_PI_2) {
          angle2 = -M_PI - angle2;
        }
        else
        if (angle2 > M_PI_2) {
          angle2 = M_PI - angle2;
        }
        /* angle score will be from -1 to 1 */
        /* negative for right, positive for left */
        lmeasure->angle_score = angle2 / M_PI_2;
      }

      if (lmeasure->against_score < 0.5) {
        lmeasure->category = bl_TOWARDS;
        weight = 1 - fabs(lmeasure->angle_score);
        if (weight > 0.33) {
          towards_weight += weight;
          towards_mean_sum += (weight * mean);
          towards_dev_sum += (weight * dev);
          towards_mag_sum += (weight * eresp2->mag);
        }
      }
      else {
        lmeasure->category = bl_AGAINST;
        weight = 1 - fabs(lmeasure->angle_score);
        if (weight > 0.33) {
          against_weight += weight;
          against_mean_sum += (weight * mean);
          against_dev_sum += (weight * dev);
          against_mag_sum += (weight * eresp2->mag);
        }
      }
      if (lmeasure->angle_score > 0) {
        weight = fabs(lmeasure->angle_score);
        if (weight > 0.5) {
          lmeasure->category = bl_LEFT;
          left_weight += weight;
          left_mean_sum += (weight * mean);
          left_dev_sum += (weight * dev);
          left_mag_sum += (weight * eresp2->mag);
        }
      }
      else {
        weight = fabs(lmeasure->angle_score);
        if (weight > 0.5) {
          lmeasure->category = bl_LEFT;
          right_weight += weight;
          right_mean_sum += (weight * mean);
          right_dev_sum += (weight * dev);
          right_mag_sum += (weight * eresp2->mag);
        }
      }
      links = links->next;
    }

    mean_mean = mean_sum1 / count;
    mean_dev = mean_sum2 / count - mean_mean * mean_mean;
    mean_dev = mean_dev < 0 ? 0 : sqrt(mean_dev);
    dev_mean = dev_sum1 / count;
    dev_dev = dev_sum2 / count - dev_mean * dev_mean;
    dev_dev = dev_dev < 0 ? 0 : sqrt(dev_dev);

    nstat->mean_mean = mean_mean;
    nstat->mean_dev = mean_dev;
    nstat->dev_mean = dev_mean;
    nstat->dev_dev = dev_dev;

    left_mean_sum /= left_weight;
    left_dev_sum /= left_weight;
    left_mag_sum /= left_weight;
    right_mean_sum /= right_weight;
    right_dev_sum /= right_weight;
    right_mag_sum /= right_weight;
    towards_mean_sum /= towards_weight;
    towards_dev_sum /= towards_weight;
    towards_mag_sum /= towards_weight;
    against_mean_sum /= against_weight;
    against_dev_sum /= against_weight;
    against_mag_sum /= against_weight;

    /* for mean scores, use mean_dev to scale */
    if (mean_dev < 4) mean_dev = 4;

    /* calculate mean ledge score */
    score = (left_mean_sum - right_mean_sum) / mean_dev;
    counter_score = fabs((towards_mean_sum - against_mean_sum) / mean_dev);
    if (score < 0) {
      score += counter_score;
      if (score > 0) score = 0;
    }
    else {
      score -= counter_score;
      if (score < 0) score = 0;
    }
    nstat->mean_ledge_score = score;

    /* calculate mean ridge score */
    adjust = score;
    score = ((2 * tree_mean) - (left_mean_sum + right_mean_sum)) / mean_dev;
    counter_adjust = counter_score;
    counter_score = ((2 * tree_mean) - (towards_mean_sum + against_mean_sum)) / mean_dev;
    if (counter_score < 0) {
      counter_score += counter_adjust;
      if (counter_score > 0) counter_score = 0;
    }
    else {
      counter_score -= counter_adjust;
      if (counter_score < 0) counter_score = 0;
    }
    if (score < 0) {
      score += fabs(counter_score);
      score += fabs(adjust);
      if (score > 0) score = 0;
    }
    else {
      score -= fabs(counter_score);
      score -= fabs(adjust);
      if (score < 0) score = 0;
    }
    nstat->mean_ridge_score = score;

    /* for dev scores, use dev_dev to scale */
    if (dev_dev < 1) dev_dev = 1;

    /* calculate dev ledge score */
    score = (left_dev_sum - right_dev_sum) / dev_dev;
    counter_score = fabs((towards_dev_sum - against_dev_sum) / dev_dev);
    if (score < 0) {
      score += counter_score;
      if (score > 0) score = 0;
    }
    else {
      score -= counter_score;
      if (score < 0) score = 0;
    }
    nstat->dev_ledge_score = score;

    /* calculate dev ridge score */
    adjust = score;
    score = ((2 * tree_dev) - (left_dev_sum + right_dev_sum)) / dev_dev;
    if (score < 0) {
      /* remove negative dev ridge scores */
      score = 0;
    }
    else {
      counter_adjust = counter_score;
      counter_score = ((2 * tree_dev) - (towards_dev_sum + against_dev_sum)) / dev_dev;
      if (counter_score < 0) {
        counter_score += counter_adjust;
        if (counter_score > 0) counter_score = 0;
      }
      else {
        counter_score -= counter_adjust;
        if (counter_score < 0) counter_score = 0;
      }
      score -= fabs(counter_score);
      score -= fabs(adjust);
      if (score < 0) score = 0;
    }
    nstat->dev_ridge_score = score;

    /* calculate mag ridge score */
    score = (2 * eresp1->mag) - (left_mag_sum + right_mag_sum);
    if (score < 0) {
      score = 0;
    }
    else {
      counter_score = (2 * eresp1->mag) - (towards_mag_sum + against_mag_sum);
      if (counter_score < 0) counter_score = 0;
      score -= counter_score;
      if (score < 0) score = 0;
    }
    nstat->mag_ridge_score = score;

    /* boundary and segment scores are reset here and adjusted later */
    nstat->boundary_score = 0;
    nstat->segment_score = 0;
  }

  if (stat != NULL) {
    *stat = nstat;
  }

  FINALLY(quad_tree_ensure_edge_stats);
  RETURN();
}

/******************************************************************************/


result quad_forest_calculate_edge_stats
(
  quad_forest *forest
)
{
  TRY();
  list_item *trees, *endtrees, *links, *endlinks;
  quad_tree *tree1, *tree2;
  quad_tree_link *link;
  quad_tree_link_head *head1, *head2;
  neighborhood_stat *nstat1, *nstat2;
  edge_response *eresp1, *eresp2;
  link_measure *lmeasure, *lmeasure1, *lmeasure2;
  typed_pointer *tptr;
  integral_value mean_ledge_dist, dev_ridge_dist, mag_ridge_dist;
  /* mean_ridge_dist, dev_ledge_dist */
  integral_value dir_cost, prof_cost, lscore1, dscore1, mscore1, score2;
  integral_value parallel_1, parallel_2, perpendicular_1, perpendicular_2;
  integral_value parallel_count, segment_score, boundary_score;

  /*CHECK(quad_forest_calculate_neighborhood_stats(forest, FALSE));*/

  trees = forest->trees.first.next;
  endtrees = &forest->trees.last;
  while (trees != endtrees) {
    tree1 = (quad_tree*)trees->data;
    CHECK(quad_tree_ensure_edge_stats(forest, tree1, &nstat1));
    CHECK(expect_edge_response(&eresp1, &tree1->annotation));

    /* get the mean ledge score for the node */
    lscore1 = nstat1->mean_ledge_score;
    if (lscore1 > 3) lscore1 = 1;
    else
    if (lscore1 < 1) lscore1 = 0;
    else lscore1 = (lscore1 - 1) / 2;
    /* get the dev ridge score for the node */
    dscore1 = nstat1->dev_ridge_score;
    if (dscore1 > 1.5) dscore1 = 1;
    else if (dscore1 < 0.5) dscore1 = 0;
    else dscore1 = (dscore1 - 0.5);
    /* get the mag ridge score for the node */
    mscore1 = nstat1->mag_ridge_score;
    if (mscore1 > 1.5) mscore1 = 1;
    else if (mscore1 < 0.5) mscore1 = 0;
    else mscore1 = (mscore1 - 0.5);

    boundary_score = (lscore1 + 2 * dscore1 + mscore1) / 4;
    segment_score = 1 - boundary_score;

    links = tree1->links.first.next;
    endlinks = &tree1->links.last;
    while (links != endlinks) {
      head1 = *((quad_tree_link_head**)links->data);
      CHECK(expect_link_measure(&head1->annotation, &lmeasure1, forest->token));

      head2 = head1->other;
      tree2 = head2->tree;
      CHECK(quad_tree_ensure_edge_stats(forest, tree2, &nstat2));
      CHECK(expect_link_measure(&head2->annotation, &lmeasure2, forest->token));
      CHECK(expect_edge_response(&eresp2, &tree2->annotation));

      link = head1->link;
      CHECK(ensure_has(&link->annotation, t_link_measure, &tptr));
      lmeasure = (link_measure*)tptr->value;
      if (tptr->token != forest->token) {
        tptr->token = forest->token;

        /* calculate mean ledge score for the link */
        score2 = nstat2->mean_ledge_score;
        if (score2 > 3) score2 = 1;
        else if (score2 < 1) score2 = 0;
        else score2 = (score2 - 1) / 2;
        if (signum(lscore1) == 0 || signum(score2) == 0) {
          mean_ledge_dist = 1;
        }
        else {
          mean_ledge_dist = fabs(lscore1 - score2);
        }

        /* calculate the dev ridge score for the link */
        score2 = nstat2->dev_ridge_score;
        if (score2 > 1.5) score2 = 1;
        else if (score2 < 0.5) score2 = 0;
        else score2 = (score2 - 0.5);
        if (signum(dscore1) == 0 || signum(score2) == 0) {
          dev_ridge_dist = 1;
        }
        else {
          dev_ridge_dist = fabs(dscore1 - score2);
        }

        score2 = nstat2->mag_ridge_score;
        if (score2 > 1.5) score2 = 1;
        else if (score2 < 0.5) score2 = 0;
        else score2 = (score2 - 0.5);
        if (signum(mscore1) == 0 || signum(score2) == 0) {
          mag_ridge_dist = 1;
        }
        else {
          mag_ridge_dist = fabs(mscore1 - score2);
        }

        /* calculate the profile cost */
        prof_cost = (mean_ledge_dist + mag_ridge_dist + 2 * dev_ridge_dist) / 4;
        lmeasure1->profile_score = prof_cost;
        lmeasure2->profile_score = prof_cost;

        /* link head 1 direction cost and scores */
        dir_cost = fabs(lmeasure1->angle_score);
        parallel_1 = (1 - prof_cost) * (1 - dir_cost);
        lmeasure1->parallel_score = parallel_1;
        perpendicular_1 = prof_cost * dir_cost;
        lmeasure1->perpendicular_score = perpendicular_1;

        /* link head 2 direction cost and scores */
        dir_cost = fabs(lmeasure2->angle_score);
        parallel_2 = (1 - prof_cost) * (1 - dir_cost);
        perpendicular_2 = prof_cost * dir_cost;
        lmeasure2->parallel_score = parallel_2;
        lmeasure2->perpendicular_score = perpendicular_2;

        /* this node thinks this is a parallel link */
        if (parallel_1 > perpendicular_1) {
          /* also the opposing node thinks this is a parallel link */
          if (parallel_2 > perpendicular_2) {
            lmeasure->category = bl_PARALLEL;
            lmeasure->parallel_score = getmax(parallel_1, parallel_2);
            lmeasure->perpendicular_score = getmin(perpendicular_1, perpendicular_2);

            parallel_count += 1;
            if (parallel_1 > parallel_2) {
              if (lmeasure1->against_score < 0.5) {
                lmeasure1->category = bl_TOWARDS;
                lmeasure2->category = bl_AGAINST;
              }
              else {
                lmeasure1->category = bl_AGAINST;
                lmeasure2->category = bl_TOWARDS;
              }
            }
            else {
              if (lmeasure2->against_score < 0.5) {
                lmeasure2->category = bl_TOWARDS;
                lmeasure1->category = bl_AGAINST;
              }
              else {
                lmeasure2->category = bl_AGAINST;
                lmeasure1->category = bl_TOWARDS;
              }
            }
          }
          /* the opposing node things this is a perpendicular link */
          else {
            /* which belief is stronger? */
            if (parallel_1 > perpendicular_2) {
              lmeasure->category = bl_PARALLEL;
              lmeasure->parallel_score = getmax(parallel_1, parallel_2);
              lmeasure->perpendicular_score = getmin(perpendicular_1, perpendicular_2);

              parallel_count += 1;
              if (parallel_1 > parallel_2) {
                if (lmeasure1->against_score < 0.5) {
                  lmeasure1->category = bl_TOWARDS;
                  lmeasure2->category = bl_AGAINST;
                }
                else {
                  lmeasure1->category = bl_AGAINST;
                  lmeasure2->category = bl_TOWARDS;
                }
              }
              else {
                if (lmeasure2->against_score < 0.5) {
                  lmeasure2->category = bl_TOWARDS;
                  lmeasure1->category = bl_AGAINST;
                }
                else {
                  lmeasure2->category = bl_AGAINST;
                  lmeasure1->category = bl_TOWARDS;
                }
              }
            }
            else {
              lmeasure->category = bl_PERPENDICULAR;
              lmeasure->parallel_score = getmin(parallel_1, parallel_2);
              lmeasure->perpendicular_score = getmax(perpendicular_1, perpendicular_2);
              /* boundaries want to have perpendicular nodes far from this node */

              if (perpendicular_1 > perpendicular_2) {
                if (lmeasure1->angle_score < 0) {
                  lmeasure1->category = bl_RIGHT;
                  lmeasure2->category = bl_LEFT;
                }
                else {
                  lmeasure1->category = bl_LEFT;
                  lmeasure2->category = bl_RIGHT;
                }
              }
              else {
                if (lmeasure2->angle_score < 0) {
                  lmeasure2->category = bl_RIGHT;
                  lmeasure1->category = bl_LEFT;
                }
                else {
                  lmeasure2->category = bl_LEFT;
                  lmeasure1->category = bl_RIGHT;
                }
              }
            }
          }
        }
        /* this node thinks this is a perpendicular link */
        else {
          /* the opposing node things this is a parallel link */
          if (parallel_2 > perpendicular_2) {
            /* which belief is stronger? */
            if (parallel_2 > perpendicular_1) {
              lmeasure->category = bl_PARALLEL;
              lmeasure->parallel_score = getmax(parallel_1, parallel_2);
              lmeasure->perpendicular_score = getmin(perpendicular_1, perpendicular_2);

              parallel_count += 1;
              if (parallel_1 > parallel_2) {
                if (lmeasure1->against_score < 0.5) {
                  lmeasure1->category = bl_TOWARDS;
                  lmeasure2->category = bl_AGAINST;
                }
                else {
                  lmeasure1->category = bl_AGAINST;
                  lmeasure2->category = bl_TOWARDS;
                }
              }
              else {
                if (lmeasure2->against_score < 0.5) {
                  lmeasure2->category = bl_TOWARDS;
                  lmeasure1->category = bl_AGAINST;
                }
                else {
                  lmeasure2->category = bl_AGAINST;
                  lmeasure1->category = bl_TOWARDS;
                }
              }
            }
            else {
              lmeasure->category = bl_PERPENDICULAR;
              lmeasure->parallel_score = getmin(parallel_1, parallel_2);
              lmeasure->perpendicular_score = getmax(perpendicular_1, perpendicular_2);

              if (perpendicular_1 > perpendicular_2) {
                if (lmeasure1->angle_score < 0) {
                  lmeasure1->category = bl_RIGHT;
                  lmeasure2->category = bl_LEFT;
                }
                else {
                  lmeasure1->category = bl_LEFT;
                  lmeasure2->category = bl_RIGHT;
                }
              }
              else {
                if (lmeasure2->angle_score < 0) {
                  lmeasure2->category = bl_RIGHT;
                  lmeasure1->category = bl_LEFT;
                }
                else {
                  lmeasure2->category = bl_LEFT;
                  lmeasure1->category = bl_RIGHT;
                }
              }
            }
          }
          /* also the opposing node thinks this is a perpendicular link */
          else {
            lmeasure->category = bl_PERPENDICULAR;
            lmeasure->parallel_score = getmin(parallel_1, parallel_2);
            lmeasure->perpendicular_score = getmax(perpendicular_1, perpendicular_2);

            if (perpendicular_1 > perpendicular_2) {
              if (lmeasure1->angle_score < 0) {
                lmeasure1->category = bl_RIGHT;
                lmeasure2->category = bl_LEFT;
              }
              else {
                lmeasure1->category = bl_LEFT;
                lmeasure2->category = bl_RIGHT;
              }
            }
            else {
              if (lmeasure2->angle_score < 0) {
                lmeasure2->category = bl_RIGHT;
                lmeasure1->category = bl_LEFT;
              }
              else {
                lmeasure2->category = bl_LEFT;
                lmeasure1->category = bl_RIGHT;
              }
            }
          }
        }
      }
      links = links->next;
    }
    nstat1->segment_score += segment_score;
    nstat1->boundary_score += boundary_score;
    nstat1->dir_confusion = parallel_count / 2;

    trees = trees->next;
  }

  FINALLY(quad_forest_calculate_edge_stats);
  RETURN();
}

/******************************************************************************/
/* finds the best option for the other link for trees that have already one   */

result extend_edge_links
(
  quad_forest *forest,
  quad_tree *tree,
  link_category category,
  quad_tree_link_head **found_head
)
{
  TRY();
  quad_tree_link_head *head, *best_head;
  list_item *links, *endlinks;
  link_measure *lmeasure;
  integral_value profile, best_profile;

  CHECK_POINTER(forest);
  CHECK_POINTER(tree);
  CHECK_PARAM(category == bl_TOWARDS || category == bl_AGAINST);

  best_profile = 0;
  best_head = NULL;
  links = tree->links.first.next;
  endlinks = &tree->links.last;
  while (links != endlinks) {
    head = *((quad_tree_link_head**)links->data);
    CHECK(expect_link_measure(&head->annotation, &lmeasure, forest->token));
    if ((category == bl_TOWARDS && lmeasure->against_score < 0.4) ||
        (category == bl_AGAINST && lmeasure->against_score > 0.6))
    {
      profile = lmeasure->parallel_score;
      if (profile > best_profile) {
        best_profile = profile;
        best_head = head;
      }
    }
    links = links->next;
  }

  if (found_head != NULL) {
    *found_head = best_head;
  }

  FINALLY(extend_edge_links);
  RETURN();
}

/******************************************************************************/

result quad_forest_parse
(
  quad_forest *forest,
  uint32 rounds,
  truth_value use_dev
)
{
  TRY();
  list edgelist, boundarylist;

  CHECK_POINTER(forest);
  CHECK_PARAM(rounds > 0);

  CHECK(list_create(&edgelist, 1000, sizeof(quad_tree*), 1));
  CHECK(list_create(&boundarylist, 1000, sizeof(boundary*), 1));

  CHECK(quad_forest_calculate_edge_stats(forest));

  /* in the first round, find nodes with strong outgoing links */
  {
    list_item *trees, *endtrees, *links, *endlinks;
    quad_tree *tree1, *tree2;
    quad_tree_link_head *head1, *best_against, *best_towards;
    neighborhood_stat *nstat1, *nstat2;
    link_measure *lmeasure1;
    typed_pointer *tptr;
    edge_links *elinks_tree;
    edge_response *eresp1;
    integral_value own_angle, score, tscore, ascore;
    integral_value towards_score, against_score, boundary_score;

    trees = forest->trees.first.next;
    endtrees = &forest->trees.last;
    while (trees != endtrees) {
      tree1 = (quad_tree*)trees->data;
      CHECK(expect_neighborhood_stat(&nstat1, &tree1->annotation));

      best_towards = NULL;
      best_against = NULL;
      towards_score = 0;
      against_score = 0;
      boundary_score = 0;
      tscore = 0;
      ascore = 0;
      links = tree1->links.first.next;
      endlinks = &tree1->links.last;
      while (links != endlinks) {
        head1 = *((quad_tree_link_head**)links->data);
        tree2 = head1->other->tree;
        CHECK(expect_link_measure(&head1->annotation, &lmeasure1, forest->token));
        CHECK(expect_neighborhood_stat(&nstat2, &tree2->annotation));

        if (IS_PERPENDICULAR(lmeasure1->category)) {
          score = nstat2->boundary_score;
          if (score > boundary_score) {
            boundary_score = score;
          }
        }
        else
        if (lmeasure1->category == bl_TOWARDS) {
          score = lmeasure1->parallel_score;
          if (score > towards_score) {
            best_towards = head1;
            towards_score = score;
            /* if the best node changes, keep in store the boundary score */
            /* this is to prevent parallel boundary chains */
            if (tscore > boundary_score) {
              boundary_score = tscore;
            }
            tscore = nstat2->boundary_score;
          }
        }
        else
        if (lmeasure1->category == bl_AGAINST) {
          score = lmeasure1->parallel_score;
          if (score > against_score) {
            best_against = head1;
            against_score = score;
            if (ascore > boundary_score) {
              boundary_score = ascore;
            }
            ascore = nstat2->boundary_score;
          }
        }
        links = links->next;
      }

      /* only add edge links if this has higher score than perpendicular neighbors */
      score = nstat1->boundary_score;
      if (score > boundary_score) {
        /* at least one of the links must be readily available */
        if (best_towards != NULL || best_against != NULL) {
          CHECK(ensure_has(&tree1->annotation, t_edge_links, &tptr));
          elinks_tree = (edge_links*)tptr->value;
          if (tptr->token != forest->token) {
            tptr->token = forest->token;

            CHECK(expect_edge_response(&eresp1, &tree1->annotation));
            own_angle = eresp1->ang - M_PI_2;
            if (own_angle < 0) own_angle += M_2PI;
            elinks_tree->own_angle = own_angle;

            /* if the other link is missing, find the best local candidate */
            if (best_towards == NULL) {
              CHECK(extend_edge_links(forest, tree1, bl_TOWARDS, &best_towards));
            }
            else
            if (best_against == NULL) {
              CHECK(extend_edge_links(forest, tree1, bl_AGAINST, &best_against));
            }
            elinks_tree->towards = best_towards;
            elinks_tree->against = best_against;
          }
          CHECK(list_append(&edgelist, &tree1));
        }
      }
      trees = trees->next;
    }
  }

  /* in the second round, check the incoming links and extract undisputed frags */
  {
    list_item *edges, *endedges, *links, *endlinks;
    quad_tree *tree1, *tree2, *tree3;
    quad_tree_link_head *head1, *head2, *best_towards, *best_against;
    boundary *boundary1, *boundary2, *boundary3;
    edge_links *elinks_tree, *elinks_towards, *elinks_against, *elinks2, *elinks3;
    edge_response *eresp2;
    link_measure *lmeasure2;
    typed_pointer *tptr;
    truth_value has_towards, has_against, towards_has_edge, against_has_edge;
    uint32 towards_count, against_count;
    integral_value score, towards_score, against_score, own_angle, angle_diff;

    edges = edgelist.first.next;
    endedges = &edgelist.last;
    while (edges != endedges) {
      tree1 = *((quad_tree**)edges->data);
      CHECK(expect_edge_links(&tree1->annotation, &elinks_tree, forest->token));

      /* in this round, these refer to incoming, not outgoing links */
      towards_has_edge = FALSE;
      towards_count = 0;
      towards_score = 0;
      best_towards = NULL;
      against_has_edge = FALSE;
      against_count = 0;
      against_score = 0;
      best_against = NULL;

      links = tree1->links.first.next;
      endlinks = &tree1->links.last;
      while (links != endlinks) {
        head1 = *((quad_tree_link_head**)links->data);
        head2 = head1->other;
        tree2 = head2->tree;
        elinks2 = has_edge_links(&tree2->annotation, forest->token);
        if (elinks2 != NULL) {
          if (head1 == elinks_tree->towards) {
            towards_has_edge = TRUE;
          }
          else
          if (head1 == elinks_tree->against) {
            against_has_edge = TRUE;
          }
          CHECK(expect_link_measure(&head2->annotation, &lmeasure2, forest->token));
          if (elinks2->towards == head2) {
            towards_count += 1;
            score = lmeasure2->parallel_score;
            if (score > towards_score) {
              towards_score = score;
              best_towards = head1;
            }
          }
          else
          if (elinks2->against == head2) {
            against_count += 1;
            score = lmeasure2->parallel_score;
            if (score > against_score) {
              against_score = score;
              best_against = head1;
            }
          }
        }
        links = links->next;
      }

      has_towards = FALSE;
      if (best_against != NULL) {
        if (against_count == 1) {
          /* if outgoing link matches the incoming link, may connect */
          if (elinks_tree->towards == best_against) {
            has_towards = TRUE;
          }
          else
          /* if outgoing link is missing entirely, may connect to incoming */
          if (elinks_tree->towards == NULL) {
            elinks_tree->towards = best_against;
            has_towards = TRUE;
          }
          else
          /* otherwise, if there is no edge in outgoing node, may connect */
          if (IS_FALSE(towards_has_edge)) {
            elinks_tree->towards = best_against;
            has_towards = TRUE;
          }
        }
        else
        /* if there are multiple incoming links, better link to best one */
        if (against_count > 1) {
          if (elinks_tree->towards != best_against) {
            elinks_tree->towards = best_against;
          }
        }
      }

      has_against = FALSE;
      if (best_towards != NULL) {
        if (towards_count == 1) {
          /* if outgoing link matches the incoming link, may connect */
          if (elinks_tree->against == best_towards) {
            has_against = TRUE;
          }
          else
          /* if outgoing link is missing entirely, may connect to incoming */
          if (elinks_tree->against == NULL) {
            elinks_tree->against = best_towards;
            has_against = TRUE;
          }
          /* otherwise, if there is no edge in outgoing node, may connect */
          if (IS_FALSE(against_has_edge)) {
            elinks_tree->against = best_towards;
            has_against = TRUE;
          }
        }
        else
        /* if there are multiple incoming links, better link to best one */
        if (towards_count > 1) {
          if (elinks_tree->against != best_towards) {
            elinks_tree->against = best_towards;
          }
        }
      }

      /* if has links to both nodes, create a fragment */
      if (IS_TRUE(has_towards) && IS_TRUE(has_against)) {
        CHECK(quad_tree_ensure_boundary(tree1, &boundary1));
        boundary_init(boundary1, elinks_tree);
        CHECK(list_append(&boundarylist, &boundary1));

        tree2 = elinks_tree->towards->other->tree;
        CHECK(quad_tree_ensure_boundary(tree2, &boundary2));
        boundary1->next = boundary2;
        boundary2->prev = boundary1;
        if (boundary2->length < 1) {
          CHECK(expect_edge_links(&tree2->annotation, &elinks_towards, forest->token));
          boundary_init(boundary2, elinks_towards);
          CHECK(list_append(&boundarylist, &boundary2));
        }

        tree3 = elinks_tree->against->other->tree;
        CHECK(quad_tree_ensure_boundary(tree3, &boundary3));
        boundary3->next = boundary1;
        boundary1->prev = boundary3;
        if (boundary3->length < 1) {
          CHECK(expect_edge_links(&tree3->annotation, &elinks_against, forest->token));
          boundary_init(boundary3, elinks_against);
          CHECK(list_append(&boundarylist, &boundary3));
        }
      }
      /* otherwise, if one of the links is there, check the other linked node */
      else {
        if (IS_TRUE(has_towards)) {
          if (elinks_tree->against != NULL) {
            tree2 = elinks_tree->against->other->tree;
            elinks_against = has_edge_links(&tree2->annotation, forest->token);
            /* if the node doesn't have edge links, may connect if strong enough */
            /* or if has exactly one other neighboring, similar edge node */
            if (elinks_against == NULL) {
              against_count = 0;
              best_against = NULL;
              links = tree2->links.first.next;
              endlinks = &tree2->links.last;
              while (links != endlinks) {
                head1 = *((quad_tree_link_head**)links->data);
                if (head1 != elinks_tree->against->other) {
                  tree3 = head1->other->tree;
                  elinks3 = has_edge_links(&tree3->annotation, forest->token);
                  if (elinks3 != NULL) {
                    if (elinks3->towards != NULL && elinks3->towards->other == head1) {
                      against_count += 1;
                      best_against = head1;
                    }
                    else {
                      angle_diff = angle_minus_angle(elinks_tree->own_angle, elinks3->own_angle);
                      if (fabs(angle_diff) < M_PI_3) {
                        against_count += 1;
                        best_against = head1;
                      }
                    }
                  }
                }
                links = links->next;
              }
              if (against_count == 1) {
                /* TODO: should check that the tree3 node is similar */
                /* tree3 = best_against->other->tree; */
                CHECK(ensure_has(&tree2->annotation, t_edge_links, &tptr));
                elinks_against = (edge_links*)tptr->value;
                if (tptr->token != forest->token) {
                  tptr->token = forest->token;
                }
                has_against = TRUE;
                CHECK(expect_edge_response(&eresp2, &tree2->annotation));
                own_angle = eresp2->ang - M_PI_2;
                if (own_angle < 0) own_angle += M_2PI;
                elinks_against->own_angle = own_angle;
                elinks_against->towards = elinks_tree->against->other;
                elinks_against->against = best_against;

                CHECK(quad_tree_ensure_boundary(tree1, &boundary1));
                boundary_init(boundary1, elinks_tree);
                CHECK(list_append(&boundarylist, &boundary1));

                CHECK(quad_tree_ensure_boundary(tree2, &boundary2));
                boundary_init(boundary2, elinks_against);
                boundary2->next = boundary1;
                boundary1->prev = boundary2;
                CHECK(list_append(&boundarylist, &boundary2));

                tree3 = best_against->other->tree;
                CHECK(quad_tree_ensure_boundary(tree3, &boundary3));
                boundary3->next = boundary2;
                boundary2->prev = boundary3;
                if (boundary3->length < 1) {
                  CHECK(expect_edge_links(&tree3->annotation, &elinks3, forest->token));
                  boundary_init(boundary3, elinks3);
                  CHECK(list_append(&boundarylist, &boundary3));
                }
              }
            }
          }
          
          /* extended or not, link the found towards node */
          CHECK(quad_tree_ensure_boundary(tree1, &boundary1));
          if (boundary1->length < 1) {
            boundary_init(boundary1, elinks_tree);
            CHECK(list_append(&boundarylist, &boundary1));
          }

          tree2 = elinks_tree->towards->other->tree;
          CHECK(quad_tree_ensure_boundary(tree2, &boundary2));
          boundary1->next = boundary2;
          boundary2->prev = boundary1;
          if (boundary2->length < 1) {
            CHECK(expect_edge_links(&tree2->annotation, &elinks2, forest->token));
            boundary_init(boundary2, elinks2);
            CHECK(list_append(&boundarylist, &boundary2));
          }
        }
        if (IS_TRUE(has_against)) {
          if (elinks_tree->towards != NULL) {
            tree2 = elinks_tree->towards->other->tree;
            elinks_towards = has_edge_links(&tree2->annotation, forest->token);
            /* if the node doesn't have edge links, may connect if strong enough */
            /* or if has exactly one other neighboring, similar edge node */
            if (elinks_towards == NULL) {
              towards_count = 0;
              best_towards = NULL;
              links = tree2->links.first.next;
              endlinks = &tree2->links.last;
              while (links != endlinks) {
                head1 = *((quad_tree_link_head**)links->data);
                if (head1 != elinks_tree->towards->other) {
                  tree3 = head1->other->tree;
                  elinks3 = has_edge_links(&tree3->annotation, forest->token);
                  if (elinks3 != NULL) {
                    if (elinks3->against != NULL && elinks3->against->other == head1) {
                      towards_count += 1;
                      best_towards = head1;
                    }
                    else {
                      angle_diff = angle_minus_angle(elinks_tree->own_angle, elinks3->own_angle);
                      if (fabs(angle_diff) < M_PI_3) {
                        towards_count += 1;
                        best_towards = head1;
                      }
                    }
                  }
                }
                links = links->next;
              }
              if (towards_count == 1) {
                /* TODO: should check that the tree3 node is similar */
                /* tree3 = best_ltowards->other->tree; */
                CHECK(ensure_has(&tree2->annotation, t_edge_links, &tptr));
                elinks_towards = (edge_links*)tptr->value;
                if (tptr->token != forest->token) {
                  tptr->token = forest->token;
                }
                has_towards = TRUE;
                CHECK(expect_edge_response(&eresp2, &tree2->annotation));
                own_angle = eresp2->ang - M_PI_2;
                if (own_angle < 0) own_angle += M_2PI;
                elinks_towards->own_angle = own_angle;
                elinks_towards->against = elinks_tree->towards->other;
                elinks_towards->towards = best_towards;

                CHECK(quad_tree_ensure_boundary(tree1, &boundary1));
                boundary_init(boundary1, elinks_tree);
                CHECK(list_append(&boundarylist, &boundary1));

                CHECK(quad_tree_ensure_boundary(tree2, &boundary2));
                boundary_init(boundary2, elinks_towards);
                boundary1->next = boundary2;
                boundary2->prev = boundary1;
                CHECK(list_append(&boundarylist, &boundary2));

                tree3 = best_towards->other->tree;
                CHECK(quad_tree_ensure_boundary(tree3, &boundary3));
                boundary2->next = boundary3;
                boundary3->prev = boundary2;
                if (boundary3->length < 1) {
                  CHECK(expect_edge_links(&tree3->annotation, &elinks3, forest->token));
                  boundary_init(boundary3, elinks3);
                  CHECK(list_append(&boundarylist, &boundary3));
                }
              }
            }
          }
          /* extended or not, link the found against node */
          CHECK(quad_tree_ensure_boundary(tree1, &boundary1));
          if (boundary1->length < 1) {
            boundary_init(boundary1, elinks_tree);
            CHECK(list_append(&boundarylist, &boundary1));
          }

          tree2 = elinks_tree->against->other->tree;
          CHECK(quad_tree_ensure_boundary(tree2, &boundary2));
          boundary2->next = boundary1;
          boundary1->prev = boundary2;
          if (boundary2->length < 1) {
            CHECK(expect_edge_links(&tree2->annotation, &elinks2, forest->token));
            boundary_init(boundary2, elinks2);
            CHECK(list_append(&boundarylist, &boundary2));
          }
        }
      }
      edges = edges->next;
    }
  }

  /* in the third round, analyze and merge boundary fragments */
  {
    list_item *boundaries, *endboundaries;
    boundary *boundary1, *boundary2, *boundary3, *parent1, *parent2;
    integral_value angle, angle1, angle2, angle3, curvature;
    integral_value angle2a, angle2b, angle3a, angle3b;
    integral_value diffs[7], diff, diff_sum, diff_mean, diff_diff, diff_dev, diff_count;
    integral_value next_d1, next_d2, prev_d1, prev_d2, d1, d2;
    uint32 i, count;

    boundaries = boundarylist.first.next;
    endboundaries = &boundarylist.last;
    while (boundaries != endboundaries) {
      boundary1 = *((boundary**)boundaries->data);
      angle1 = boundary1->angle;

      boundary2 = boundary1->next;
      if (boundary2 != NULL) {
        angle2 = boundary2->angle;
        angle2 = angle_minus_angle(angle2, angle1);
        angle2a = atan2(
          (integral_value)((signed)boundary1->tree->y - (signed)boundary2->tree->y),
          (integral_value)((signed)boundary2->tree->x - (signed)boundary1->tree->x));
        if (angle2a < 0) angle2a += M_2PI;
        angle2b = angle_minus_angle(angle2a, angle1);
      }
      else {
        angle2 = 0;
        angle2a = -10;
        angle2b = 0;
      }
      boundary3 = boundary1->prev;
      if (boundary3 != NULL) {
        angle3 = boundary3->angle;
        angle3 = angle_minus_angle(angle3, angle1);
        angle3a = atan2(
          (integral_value)((signed)boundary3->tree->y - (signed)boundary1->tree->y),
          (integral_value)((signed)boundary1->tree->x - (signed)boundary3->tree->x));
        if (angle3a < 0) angle3a += M_2PI;
        angle3b = angle_minus_angle(angle3a, angle1);
      }
      else {
        angle3 = 0;
        angle3a = -10;
        angle2b = 0;
      }
      if (angle2a < -4) {
        angle2a = angle3a;
      }
      if (angle3a < -4) {
        angle3a = angle2a;
      }
      angle = (angle2b + angle2 + angle3b + angle3) / 4;
      angle1 = angle1 + angle;
      angle2 = angle2a + (angle_minus_angle(angle3a, angle2a) / 2);
      angle3 = fabs(angle_minus_angle(angle2, angle1));
      if (angle3 > 0.2) {
        boundary1->smoothed_angle = angle2;
      }
      else {
        boundary1->smoothed_angle = angle1;
      }
      
      boundaries = boundaries->next;
    }
    
    boundaries = boundarylist.first.next;
    endboundaries = &boundarylist.last;
    while (boundaries != endboundaries) {
      boundary1 = *((boundary**)boundaries->data);
      
      angle1 = boundary1->smoothed_angle;
      count = 0;
      boundary2 = boundary1->next;
      if (boundary2 != NULL) {
        angle2 = boundary2->smoothed_angle;
        diffs[count++] = angle_minus_angle(angle2, angle1);
        angle1 = angle2;
        boundary2 = boundary2->next;
        if (boundary2 != NULL) {
          angle2 = boundary2->smoothed_angle;
          diffs[count++] = angle_minus_angle(angle2, angle1);
          angle1 = angle2;
          boundary2 = boundary2->next;
          if (boundary2 != NULL) {
            angle2 = boundary2->smoothed_angle;
            diffs[count++] = angle_minus_angle(angle2, angle1);
          }
        }
      }
      angle1 = boundary1->smoothed_angle;
      boundary2 = boundary1->prev;
      if (boundary2 != NULL) {
        angle2 = boundary2->smoothed_angle;
        diffs[count++] = angle_minus_angle(angle1, angle2);
        angle1 = angle2;
        boundary2 = boundary2->prev;
        if (boundary2 != NULL) {
          angle2 = boundary2->smoothed_angle;
          diffs[count++] = angle_minus_angle(angle1, angle2);
          angle1 = angle2;
          boundary2 = boundary2->prev;
          if (boundary2 != NULL) {
            angle2 = boundary2->smoothed_angle;
            diffs[count++] = angle_minus_angle(angle1, angle2);
          }
        }
      }
      
      diff_count = (integral_value)count;
      diff_sum = 0;
      for (i = 0; i < count; i++) {
        diff_sum += diffs[i];
      }
      diff_mean = diff_sum / diff_count;
      diff_sum = 0;
      for (i = 0; i < count; i++) {
        diff = diffs[i] - diff_mean;
        diff_sum += (diff*diff);
      }
      diff_dev = diff_sum / diff_count;
      diff_dev = diff_dev < 1 ? diff_dev : sqrt(diff_dev);
      
      if ((fabs(diff_mean) + diff_dev) < 0.06) {
        boundary1->category = fc_STRAIGHT;
        boundary1->curvature = diff_mean + diff_dev;
      }
      else
      if (fabs(diff_mean) > 0.06) {
        boundary1->category = fc_CURVED;
        boundary1->curvature = diff_mean;
      }
      
      boundaries = boundaries->next;
    }
      /*
        next_diff_1 = angle_minus_angle(angle1, angle2);
        boundary2 = boundary2->next;
        if (boundary2 != NULL) {
          angle3 = boundary2->angle;
          next_diff_2 = angle_minus_angle(angle2, angle3);
        }
        else {
          next_diff_2 = next_diff_1;
        }
      }
      else {
        next_diff_1 = 0;
        next_diff_2 = 0;
      }
      next_d1 = (next_diff_1 + next_diff_2) / 2;
      next_d2 = fabs(next_diff_2 - next_diff_1);

      boundary2 = boundary1->prev;
      if (boundary2 != NULL) {
        angle2 = boundary2->angle;
        prev_diff_1 = angle_minus_angle(angle2, angle1);
        boundary2 = boundary2->prev;
        if (boundary2 != NULL) {
          angle3 = boundary2->angle;
          prev_diff_2 = angle_minus_angle(angle3, angle2);
        }
        else {
          prev_diff_2 = prev_diff_1;
        }
      }
      else {
        prev_diff_1 = 0;
        prev_diff_2 = 0;
      }
      prev_d1 = (prev_diff_1 + prev_diff_2) / 2;
      prev_d2 = fabs(prev_diff_1 - prev_diff_2);

      curvature = fabs((next_d1 + prev_d1) / 2);
      boundary1->curvature = curvature;
      boundary1->curvature_mean = (next_d2 + prev_d2) / 2;
      if (boundary1->prev != NULL && boundary1->next != NULL) {
        if (next_d2 < 0.2 && prev_d2 < 0.2) {
          if (curvature > 0.1) {
            parent1 = boundary_find(boundary1);
            if (parent1->category != fc_STRAIGHT) {
              parent2 = boundary_find(boundary1->prev);
              if (parent2->category != fc_STRAIGHT) {
                boundary_union(boundary1, boundary1->prev);
              }
              parent2 = boundary_find(boundary1->next);
              if (parent2->category != fc_STRAIGHT) {
                boundary_union(boundary1, boundary1->next);
              }
            }
            parent1 = boundary_find(boundary1);
            parent1->category = fc_CURVED;
          }
          else {
            parent1 = boundary_find(boundary1);
            if (parent1->category != fc_CURVED) {
              parent2 = boundary_find(boundary1->prev);
              if (parent2->category != fc_CURVED) {
                boundary_union(boundary1, boundary1->prev);
              }
              parent2 = boundary_find(boundary1->next);
              if (parent2->category != fc_CURVED) {
                boundary_union(boundary1, boundary1->next);
              }
            }
            parent1 = boundary_find(boundary1);
            parent1->category = fc_STRAIGHT;
          }
        }
      }
      */
  }

  /* set the state of forest */
  quad_forest_set_parse(forest);

  FINALLY(quad_forest_parse);
  list_destroy(&edgelist);
  list_destroy(&boundarylist);
  RETURN();
}

/******************************************************************************/

result quad_forest_visualize_parse_result
(
  quad_forest *forest,
  pixel_image *target
)
{
  TRY();
  list links, frags, lines;
  list_item *trees, *end;
  quad_tree *tree;
  neighborhood_stat *nstat;
  edge_response *eresp;
  ridge_potential *ridge1;
  boundary_potential *boundary1;
  boundary *fragment1, *bparent;
  segment *segment1, *segment_parent;
  colored_rect crect;
  uint32 x, y, width, height, stride, row_step, frag_count;
  byte *target_data, *target_pos, color0, color1, color2;
  integral_value max_edge_mag, max_ridge_score, extent, max_extent;

  CHECK_POINTER(forest);
  CHECK_POINTER(target);

  CHECK_PARAM(target->type == p_U8);
  CHECK_PARAM(target->format == RGB);
  CHECK_PARAM(target->width >= forest->source->width);
  CHECK_PARAM(target->height >= forest->source->height);
  CHECK_PARAM(target->step == 3);

  width = target->width;
  height = target->height;
  stride = target->stride;
  target_data = (byte*)target->data;

  /*CHECK(pixel_image_clear(target));*/
  CHECK(list_create(&links, 1000, sizeof(colored_line), 1));
  CHECK(list_create(&lines, 1000, sizeof(colored_line), 1));
  CHECK(list_create(&frags, 1000, sizeof(colored_rect), 1));

  /*CHECK(quad_forest_visualize_neighborhood_stats(forest, target, v_SCORE));*/
  /*
  CHECK(quad_forest_get_links(forest, &links, v_LINK_MEASURE));
  PRINT1("links: %lu\n", links.count);
  */
  /*
  CHECK(quad_forest_get_links(forest, &links, v_LINK_BOUNDARY));
  PRINT1("lines: %lu\n", links.count);
  CHECK(pixel_image_draw_colored_lines(target, &links, 2));
  TERMINATE(SUCCESS);
  */
  /*
  CHECK(quad_forest_get_links(forest, &links, v_LINK_EDGE));
  CHECK(pixel_image_draw_colored_lines(target, &links, 2));
  */
  /*
  trees = forest->trees.first.next;
  end = &forest->trees.last;
  while (trees != end) {
    tree = (quad_tree*)trees->data;
    CHECK(quad_tree_edge_response_to_line(forest, tree, &lines));
    trees = trees->next;
  }
  {
  byte segment_color[4] = {0,0,0,0};
  CHECK(pixel_image_draw_lines(target, &lines, segment_color, 2));
  }
  */
  /*TERMINATE(SUCCESS);*/

  if (IS_TRUE(quad_forest_has_parse(forest))) {
    /*
    max_edge_mag = 0;
    max_extent = 0;
    trees = forest->trees.first.next;
    end = &forest->trees.last;
    while (trees != end) {
      tree = (quad_tree*)trees->data;
      CHECK(expect_neighborhood_stat(&nstat, &tree->annotation));
      eresp = has_edge_response(&tree->annotation, forest->token);
      if (eresp != NULL) {
        if (eresp->mag > max_edge_mag) {
          max_edge_mag = eresp->mag;
        }
      }
      segment1 = has_segment(&tree->annotation, forest->token);
      if (segment1 != NULL) {
        extent = (integral_value)segment1->extent;
        if (extent > max_extent) {
          max_extent = extent;
        }
      }
      trees = trees->next;
    }

    max_ridge_score = 0;
    trees = forest->trees.first.next;
    end = &forest->trees.last;
    while (trees != end) {
      tree = (quad_tree*)trees->data;
      ridge1 = has_ridge_potential(&tree->annotation, forest->token);
      if (ridge1 != NULL) {
        if (ridge1->ridge_score > max_ridge_score) {
          max_ridge_score = ridge1->ridge_score;
        }
      }
      trees = trees->next;
    }
    */

    frag_count = 0;
    srand(1234);
    trees = forest->trees.first.next;
    end = &forest->trees.last;
    while (trees != end) {
      tree = (quad_tree*)trees->data;
      fragment1 = has_boundary(&tree->annotation, forest->token);
      if (fragment1 != NULL) {
        bparent = boundary_find(fragment1);
        if (fragment1 == bparent) {
          bparent->color[0] = (byte)(rand() % 256);
          bparent->color[1] = (byte)(rand() % 256);
          bparent->color[2] = (byte)(rand() % 256);
          frag_count += 1;
          if (bparent->length > 1) {
            crect.left = ((signed)bparent->x1)-3;
            crect.top = ((signed)bparent->y1)-3;
            crect.right = ((signed)bparent->x2)+3;
            crect.bottom = ((signed)bparent->y2)+3;
            crect.color[0] = bparent->color[0];
            crect.color[1] = bparent->color[1];
            crect.color[2] = bparent->color[2];
            CHECK(list_append(&frags, &crect));
          }
        }
      }
      trees = trees->next;
    }

    /*PRINT1("max ridge score %.3f\n", max_ridge_score);*/

    trees = forest->trees.first.next;
    end = &forest->trees.last;
    while (trees != end) {
      tree = (quad_tree*)trees->data;
      CHECK(expect_neighborhood_stat(&nstat, &tree->annotation));
      if (tree->nw == NULL) {
        fragment1 = has_boundary(&tree->annotation, forest->token);
        /*
        boundary1 = has_boundary_potential(&tree->annotation, forest->token);
        segment1 = has_segment(&tree->annotation, forest->token);
        */
        /*
        if (segment1 != NULL) {
          segment_parent = segment_find(segment1);
          if (segment_parent->category == sc_CLUTTER) {
            color0 = (byte)(255 * 0);
          }
          else {
            color0 = (byte)(255 * 0);
          }
          color1 = (byte)(255 * 0);
          color2 = (byte)(255 * 0);
        }
        else
        */
        if (fragment1 != NULL) {
          bparent = boundary_find(fragment1);
          if (bparent->category == fc_STRAIGHT) {
            color0 = 255;
            color1 = 0;
            color2 = 0;
          }
          else
          if (bparent->category == fc_CURVED) {
            color0 = 0;
            color1 = 0;
            color2 = 255;
          }
          else {
            color0 = 0;
            color1 = 255;
            color2 = 0;
          }
          /*
          color0 = 255;
          color1 = 0;
          color2 = 0;
          */
          /*
          color0 = bparent->color[0];
          color1 = bparent->color[1];
          color2 = bparent->color[2];
          */
        }
        /*
        else
        if (boundary1 != NULL) {
          color0 = (byte)(255 * 0);
          color1 = (byte)(255 * 0);
          color2 = (byte)(255 * 1);
        }
        else {
          color0 = (byte)(255 * 0);
          color1 = (byte)(255 * 0);
          color2 = (byte)(255 * 0);
        }
        */
        if (fragment1 != NULL /*&& bparent->length > 2*/) {
          width = tree->size;
          height = width;
          row_step = stride - 3 * width;
          target_pos = target_data + tree->y * stride + 3 * tree->x;
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
    /*CHECK(quad_forest_visualize_neighborhood_stats(forest, target, v_STRENGTH));*/
    {
      /*
      byte edge_color_1[4] = {0,255,0,0};
      byte edge_color_2[4] = {0,255,0,0};
      byte segment_color[4] = {255,255,255,0};
      */
      /*CHECK(quad_forest_get_links(forest, &links, v_LINK_NONE));*/
      /*
      CHECK(quad_forest_get_links(forest, &links, v_LINK_SIMILARITY));
      CHECK(pixel_image_draw_weighted_lines(target, &links, segment_color));
      CHECK(list_clear(&links));
      CHECK(quad_forest_get_links(forest, &links, v_LINK_ANGLE_COST));
      CHECK(pixel_image_draw_weighted_lines(target, &links, edge_color));
      */
      /*CHECK(quad_forest_get_links(forest, &links, v_LINK_MEASURE));*/
      /*CHECK(quad_forest_get_links(forest, &links, v_LINK_EDGE));*/

      /*CHECK(quad_forest_get_links(forest, &links, v_LINK_EDGE));*/
      /*PRINT1("links: %d\n", links.count);*/

      CHECK(quad_forest_get_links(forest, &links, v_LINK_BOUNDARY));
      CHECK(pixel_image_draw_colored_lines(target, &links, 1));
      /*
      trees = forest->trees.first.next;
      end = &forest->trees.last;
      while (trees != end) {
        tree = (quad_tree*)trees->data;
        CHECK(quad_tree_edge_response_to_line(forest, tree, &links));
        trees = trees->next;
      }
      */
      /*PRINT1("edges found: %d\n", links.count);*/
      /*CHECK(quad_tree_gradient_to_line(tree, &links));*/
      /*
      CHECK(pixel_image_draw_lines(target, &links, segment_color, 1));
      */
      /*CHECK(pixel_image_draw_weighted_lines(target, &links, segment_color));*/
      /*
      PRINT1("fragments found: %lu, ", frag_count);
      PRINT1("frags in list: %lu\n", frags.count);
      */
      CHECK(pixel_image_draw_colored_rects(target, &frags));
      /*
      CHECK(list_clear(&links));
      CHECK(quad_forest_get_links(forest, &links, v_LINK_STRENGTH));
      CHECK(pixel_image_draw_weighted_lines(target, &links, edge_color));
      */
    }
  }

  FINALLY(quad_forest_visualize_parse_result);
  list_destroy(&links);
  list_destroy(&frags);
  list_destroy(&lines);
  RETURN();
}

/* end of file                                                                */
/******************************************************************************/
