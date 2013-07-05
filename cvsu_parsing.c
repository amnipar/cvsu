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
  uint32 x, y, width, height, stride, row_step, count;
  byte *target_data, *target_pos, color, color0, color1, color2;

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
              /*nstat->strength;*/
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
        integral_value score, min_mean_ledge, max_mean_ledge, min_dev_ridge;
        integral_value max_dev_ridge, min_mean_ridge, max_mean_ridge;

        min_mean_ledge = 100;
        max_mean_ledge = -100;
        min_dev_ridge = 100;
        max_dev_ridge = -100;
        min_mean_ridge = 100;
        max_mean_ridge = -100;
        trees = forest->trees.first.next;
        end = &forest->trees.last;
        while (trees != end) {
          tree = (quad_tree*)trees->data;
          if (tree->nw == NULL) {
            nstat = has_neighborhood_stat(&tree->annotation);
            if (nstat != NULL) {
              score = fabs(nstat->mean_ledge_score);
              if (score > max_mean_ledge) max_mean_ledge = score;
              if (score < min_mean_ledge) min_mean_ledge = score;
              score = fabs(nstat->mean_ridge_score);
              if (score > max_mean_ridge) max_mean_ridge = score;
              if (score < min_mean_ridge) min_mean_ridge = score;
              score = fabs(nstat->dev_ridge_score);
              if (score > max_dev_ridge) max_dev_ridge = score;
              if (score < min_dev_ridge) min_dev_ridge = score;
            }
          }
          trees = trees->next;
        }
        /*PRINT2("score min %.3f max %.3f\n", min_score, max_score);*/
        trees = forest->trees.first.next;
        end = &forest->trees.last;
        while (trees != end) {
          tree = (quad_tree*)trees->data;
          if (tree->nw == NULL) {
            nstat = has_neighborhood_stat(&tree->annotation);
            if (nstat != NULL) {
              score = fabs(nstat->mean_ledge_score);
              if (score < 1) score = 0;
              else
              if (score > 3) score = 1;
              else score = (score - 1) / 2;
              color0 = (byte)(255 * score);
              /*color0 = (byte)(255 * ((score - min_mean_ledge) / (max_mean_ledge - min_mean_ledge)));*/
              score = fabs(nstat->mean_ridge_score);
              if (score < 0.5) score = 0;
              else
              if (score > 2.5) score = 1;
              else score = (score - 0.5) / 2;
              color1 = (byte)(255 * score);
              /*color1 = (byte)(255 * ((score - min_mean_ridge) / (max_mean_ridge - min_mean_ridge)));*/
              score = fabs(nstat->dev_ridge_score);
              if (score < 0.1) score = 0;
              if (score > 1) score = 1;
              color2 = (byte)(255 * score);
              /*color2 = (byte)(255 * ((score - min_dev_ridge) / (max_dev_ridge - min_dev_ridge)));*/
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
  integral_value left_weight, left_mean_sum, left_dev_sum;
  integral_value right_weight, right_mean_sum, right_dev_sum;
  integral_value towards_weight, towards_mean_sum, towards_dev_sum;
  integral_value against_weight, against_mean_sum, against_dev_sum;
  integral_value score, adjust, counter_score, counter_adjust;
  typed_pointer *tptr;
  neighborhood_stat *nstat;
  edge_response *eresp;
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
    CHECK(quad_tree_ensure_edge_response(forest, tree, &eresp, TRUE));
  
    left_weight = 0;
    left_mean_sum = 0;
    left_dev_sum = 0;
    right_weight = 0;
    right_mean_sum = 0;
    right_dev_sum = 0;
    towards_weight = 0;
    towards_mean_sum = 0;
    towards_dev_sum = 0;
    against_weight = 0;
    against_mean_sum = 0;
    against_dev_sum = 0;
    
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
        angle1 = eresp->ang - M_PI_2;
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
        }
      }
      else {
        lmeasure->category = bl_AGAINST;
        weight = 1 - fabs(lmeasure->angle_score);
        if (weight > 0.33) {
          against_weight += weight;
          against_mean_sum += (weight * mean);
          against_dev_sum += (weight * dev);
        }
      }
      if (lmeasure->angle_score > 0) {
        weight = fabs(lmeasure->angle_score);
        if (weight > 0.5) {
          lmeasure->category = bl_LEFT;
          left_weight += weight;
          left_mean_sum += (weight * mean);
          left_dev_sum += (weight * dev);
        }
      }
      else {
        weight = fabs(lmeasure->angle_score);
        if (weight > 0.5) {
          lmeasure->category = bl_LEFT;
          right_weight += weight;
          right_mean_sum += (weight * mean);
          right_dev_sum += (weight * dev);
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
    right_mean_sum /= right_weight;
    right_dev_sum /= right_weight;
    towards_mean_sum /= towards_weight;
    towards_dev_sum /= towards_weight;
    against_mean_sum /= against_weight;
    against_dev_sum /= against_weight;
    
    /* for mean scores, use mean_dev to scale */
    if (mean_dev < 1) mean_dev = 1;
            
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
  integral_value mean_ledge_dist, mean_ridge_dist, dev_ledge_dist, dev_ridge_dist;
  integral_value dir_cost, prof_cost, left_sum, center_sum, right_sum;
  integral_value parallel_1, parallel_2, perpendicular_1, perpendicular_2;
  integral_value score1, score2, segment_score, boundary_score;

  /*CHECK(quad_forest_calculate_neighborhood_stats(forest, FALSE));*/

  trees = forest->trees.first.next;
  endtrees = &forest->trees.last;
  while (trees != endtrees) {
    tree1 = (quad_tree*)trees->data;
    CHECK(quad_tree_ensure_edge_stats(forest, tree1, &nstat1));
    CHECK(expect_edge_response(&eresp1, &tree1->annotation));
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
        score1 = nstat1->mean_ledge_score;
        if (score1 > 3) score1 = 1;
        else
        if (score1 < -3) score1 = -1;
        else
        if (-1 < score1 && score1 < 1) score1 = 0;
        else {
          if (score1 < 0) score1 = (score1 + 1) / 2;
          else score1 = (score1 - 1) / 2;
        }
        score2 = nstat2->mean_ledge_score;
        if (score2 > 3) score2 = 1;
        else if (score2 < -3) score2 = -1;
        else if (-1 < score2 && score2 < 1) score2 = 0;
        else {
          if (score2 < 0) score2 = (score2 + 1) / 2;
          else score2 = (score2 - 1) / 2;
        }
        if (signum(score1) != signum(score2)) {
          mean_ledge_dist = 1;
        }
        else
        if (signum(score1) == 0 || signum(score2) == 0) {
          mean_ledge_dist = 1;
        }
        else {
          mean_ledge_dist = fabs(score1 - score2);
        }
        
        /* calculate the mean ridge score for the link */
        score1 = nstat1->mean_ridge_score;
        if (score1 > 2.5) score1 = 1;
        else
        if (score1 < -2.5) score1 = -1;
        else
        if (-0.5 < score1 && score1 < 0.5) score1 = 0;
        else {
          if (score1 < 0) score1 = (score1 + 0.5) / 2;
          else score1 = (score1 - 0.5) / 2;
        }
        score2 = nstat2->mean_ridge_score;
        if (score2 > 2.5) score2 = 1;
        else
        if (score2 < -2.5) score2 = -1;
        else
        if (-0.5 < score2 && score2 < 0.5) score2 = 0;
        else {
          if (score2 < 0) score2 = (score2 + 0.5) / 2;
          else score2 = (score2 - 0.5) / 2;
        }
        if (signum(score1) != signum(score2)) {
          mean_ridge_dist = 1;
        }
        else
        if (signum(score1) == 0 || signum(score2) == 0) {
          mean_ridge_dist = 1;
        }
        else {
          mean_ridge_dist = fabs(score1 - score2);
        }
        
        /* calculate the dev ridge score for the link */
        score1 = nstat1->dev_ridge_score;
        if (score1 > 1) score1 = 1;
        else if (score1 < 0.1) score1 = 0;
        score2 = nstat2->dev_ridge_score;
        if (score2 > 1) score2 = 1;
        else if (score2 < 0.1) score2 = 0;
        if (signum(score1) == 0 || signum(score2) == 0) {
          dev_ridge_dist = 1;
        }
        else {
          dev_ridge_dist = fabs(score1 - score2);
        }
        
        /* calculate the profile cost */
        prof_cost = (mean_ledge_dist + mean_ridge_dist + (2 * dev_ridge_dist)) / 4;
        /*prof_cost = (0.9*mean_ledge_dist) * mean_ridge_dist * dev_ridge_dist;*/
        
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
            if (eresp1->mag > eresp2->mag) {
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
            if ((parallel_1 * eresp1->mag) > (perpendicular_2 * eresp2->mag)) {
              lmeasure->category = bl_PARALLEL;
              lmeasure->parallel_score = getmax(parallel_1, parallel_2);
              lmeasure->perpendicular_score = getmin(perpendicular_1, perpendicular_2);
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
              lmeasure->category = bl_PERPENDICULAR;
              lmeasure->parallel_score = getmin(parallel_1, parallel_2);
              lmeasure->perpendicular_score = getmax(perpendicular_1, perpendicular_2);
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
        /* this node thinks this is a perpendicular link */
        else {
          /* the opposing node things this is a parallel link */
          if (parallel_2 > perpendicular_2) {
            /* which belief is stronger? */
            if ((parallel_2 * eresp2->mag) > (perpendicular_1 * eresp1->mag)) {
              lmeasure->category = bl_PARALLEL;
              lmeasure->parallel_score = getmax(parallel_1, parallel_2);
              lmeasure->perpendicular_score = getmin(perpendicular_1, perpendicular_2);
              if (lmeasure2->against_score < 0.5) {
                lmeasure2->category = bl_TOWARDS;
                lmeasure1->category = bl_AGAINST;
              }
              else {
                lmeasure2->category = bl_AGAINST;
                lmeasure1->category = bl_TOWARDS;
              }
            }
            else {
              lmeasure->category = bl_PERPENDICULAR;
              lmeasure->parallel_score = getmin(parallel_1, parallel_2);
              lmeasure->perpendicular_score = getmax(perpendicular_1, perpendicular_2);
              if (lmeasure1->angle_score < 0) {
                lmeasure1->category = bl_RIGHT;
                lmeasure2->category = bl_LEFT;
              }
              else {
                lmeasure1->category = bl_LEFT;
                lmeasure2->category = bl_RIGHT;
              }
            }
          }
          /* also the opposing node thinks this is a perpendicular link */
          else {
            lmeasure->category = bl_PERPENDICULAR;
            lmeasure->parallel_score = getmin(parallel_1, parallel_2);
            lmeasure->perpendicular_score = getmax(perpendicular_1, perpendicular_2);
            if (eresp1->mag > eresp2->mag) {
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
    trees = trees->next;
  }

  FINALLY(quad_forest_calculate_edge_stats);
  RETURN();
}

/******************************************************************************/
/*  */

result quad_tree_link_head_ensure_measure
(
  quad_forest *forest,
  quad_tree_link_head *head,
  link_measure **lmeasure,
  truth_value use_edge_links
)
{
  TRY();
  quad_tree *tree1, *tree2;
  edge_response *eresp1, *eresp2;
  link_measure *measure;
  edge_links *links1, *links2;
  typed_pointer *tptr;
  neighborhood_stat *nstat1, *nstat2;
  integral_value angle1, angle2, against_angle, anglediff;

  CHECK_POINTER(forest);
  CHECK_POINTER(head);
  CHECK_POINTER(lmeasure);
  *lmeasure = NULL;

  tree1 = head->tree;
  CHECK(expect_neighborhood_stat(&nstat1, &tree1->annotation));
  CHECK(quad_tree_ensure_edge_response(forest, tree1, &eresp1, TRUE));
  links1 = has_edge_links(&tree1->annotation, forest->token);
  CHECK(ensure_has(&head->annotation, t_link_measure, &tptr));
  measure = (link_measure*)tptr->value;

  if (tptr->token != forest->token) {
    tptr->token = forest->token;
    /* if edge links are present, adjust link category based on those */
    /* ensuring there is always at least one left and right node?? */
    if (links1 != NULL && IS_TRUE(use_edge_links)) {
      angle1 = angle_minus_angle(head->angle, links1->towards_angle);
      /* close to towards-angle -> category towards */
      if (fabs(angle1) < M_PI_4) {
        measure->category = bl_TOWARDS;
        measure->angle_score = 1 - (angle1 / M_PI_2);
      }
      else {
        against_angle = links1->against_angle - M_PI;
        if (against_angle < 0) against_angle += M_2PI;
        angle2 = angle_minus_angle(head->angle, against_angle);
        /* close to against-angle -> category against */
        if (fabs(angle2) < M_PI_4) {
          measure->category = bl_AGAINST;
          measure->angle_score = 1 - (angle2 / M_PI_2);
        }
        else {
          /* larger than towards and smaller than against -> left */
          if (angle1 > 0 && angle2 < 0) {
            measure->category = bl_LEFT;
            angle1 = getmax(angle1, -angle2);
            measure->angle_score = 1 - (angle1 / M_PI_2);
          }
          /* smaller than towards and larger than against -> right */
          else {
            measure->category = bl_RIGHT;
            angle1 = getmax(fabs(angle1), fabs(angle2));
            measure->angle_score = 1 - (angle1 / M_PI_2);
          }
        }
      }
    }
    /* if edge links are not present, use edge response direction instead */
    else {
      angle1 = eresp1->ang - M_PI_2;
      if (angle1 < 0) angle1 += M_2PI;
      angle2 = head->angle - angle1;
      if (angle2 < 0) angle2 += M_2PI;
      if (angle2 < M_PI_4) {
        measure->category = bl_TOWARDS;
        measure->angle_score = 1 - (angle2 / M_PI_2);
      }
      else
      if (angle2 < 3 * M_PI_4) {
        measure->category = bl_LEFT;
        measure->angle_score = 1 - (fabs(angle2 - M_PI_2) / M_PI_2);
      }
      else
      if (angle2 < 5 * M_PI_4) {
        measure->category = bl_AGAINST;
        measure->angle_score = 1 - (fabs(angle2 - M_PI) / M_PI_2);
      }
      else
      if (angle2 < 7 * M_PI_4) {
        measure->category = bl_RIGHT;
        measure->angle_score = 1 - (fabs(angle2 - 3 * M_PI_2) / M_PI_2);
      }
      else {
        measure->category = bl_TOWARDS;
        measure->angle_score = 1 - (fabs(angle2 - 2 * M_PI) / M_PI_2);
      }
    }

    tree2 = head->other->tree;
    CHECK(expect_neighborhood_stat(&nstat2, &tree2->annotation));
    measure->strength_score = nstat1->strength - nstat2->strength;

    /*
    CHECK(quad_tree_ensure_edge_response(forest, tree2, &eresp2, TRUE));
    measure->magnitude_score = eresp1->mag - eresp2->mag;
    */
    links2 = has_edge_links(&tree2->annotation, forest->token);
    if (links1 != NULL && links2 != NULL && IS_TRUE(use_edge_links)) {
      anglediff = fabs(angle_minus_angle(links1->own_angle, links2->own_angle));
      /*measure->straightness_score = 1 - (anglediff / M_PI);*/
    }
    else {
      CHECK(quad_tree_ensure_edge_response(forest, tree2, &eresp2, TRUE));
      angle1 = (eresp1->ang - M_PI_2);
      if (angle1 < 0) angle1 += 2 * M_PI;
      angle2 = (eresp2->ang - M_PI_2);
      if (angle2 < 0) angle2 += 2 * M_PI;
      anglediff = fabs(angle_minus_angle(angle1, angle2));
      /*measure->straightness_score = 1 - (anglediff / M_PI);*/
    }
  }
  *lmeasure = measure;

  FINALLY(quad_tree_link_head_ensure_measure);
  RETURN();
}

/******************************************************************************/

result quad_tree_ensure_edge_profiles
(
  quad_forest *forest,
  quad_tree *tree1,
  edge_profile **eprofile
)
{
  TRY();
  quad_tree *tree2;
  quad_tree_link_head *head1;
  list_item *links, *endlinks;
  typed_pointer *tptr;
  neighborhood_stat *nstat1, *nstat2;
  link_measure *measure_link1;
  edge_profile *profile1;
  integral_value sum1_left, sum1_right, sum2_left, sum2_right, N_left, N_right;
  integral_value mean_left, mean_right, dev_left, dev_right, mean, dev;

  CHECK_POINTER(forest);
  CHECK_POINTER(tree1);
  CHECK_POINTER(eprofile);
  *eprofile = NULL;

  CHECK(ensure_has(&tree1->annotation, t_edge_profile, &tptr));
  profile1 = (edge_profile*)tptr->value;
  if (tptr->token != forest->token) {
    CHECK(expect_neighborhood_stat(&nstat1, &tree1->annotation));

    sum1_left = 0;
    sum1_right = 0;
    sum2_left = 0;
    sum2_right = 0;
    N_left = 0;
    N_right = 0;
    links = tree1->links.first.next;
    endlinks = &tree1->links.last;
    while (links != endlinks) {
      head1 = *((quad_tree_link_head**)links->data);
      CHECK(quad_tree_link_head_ensure_measure(forest, head1, &measure_link1, FALSE));
      if (measure_link1->category == bl_LEFT) {
        tree2 = head1->other->tree;
        sum1_left += tree2->stat.sum;
        sum2_left += tree2->stat.sum2;
        N_left += tree2->stat.N;
      }
      else
      if (measure_link1->category == bl_RIGHT) {
        tree2 = head1->other->tree;
        sum1_right += tree2->stat.sum;
        sum2_right += tree2->stat.sum2;
        N_right += tree2->stat.N;
      }
      links = links->next;
    }

    mean = tree1->stat.mean;
    dev = getmax(1, nstat1->mean_dev);

    if (N_left > 1) {
      mean_left = sum1_left / N_left;
      dev_left = sum2_left / N_left - mean_left*mean_left;
      dev_left = dev_left > 0 ? sqrt(dev_left) : 0;
    }
    else {
      mean_left = mean;
      dev_left = dev;
    }
    if (N_right > 1) {
      mean_right = sum1_right / N_right;
      dev_right = sum2_right / N_right - mean_right*mean_right;
      dev_right = dev_right > 0 ? sqrt(dev_right) : 0;
    }
    else {
      mean_right = mean;
      dev_right = dev;
    }

    profile1->mean_left = (mean - mean_left) / dev;
    profile1->mean_right = (mean - mean_right) / dev;

    mean = tree1->stat.deviation;
    dev = getmax(1, nstat1->dev_dev);
    profile1->dev_left = (mean - dev_left) / dev;
    profile1->dev_right = (mean - dev_right) / dev;

    profile1->mean_score = profile1->mean_left - profile1->mean_right;
    profile1->dev_score  = profile1->dev_left - profile1->dev_right;

    tptr->token = forest->token;
  }
  *eprofile = profile1;

  FINALLY(quad_tree_ensure_edge_profiles);
  RETURN();
}

/******************************************************************************/

result quad_tree_ensure_edge_links
(
  quad_forest *forest,
  quad_tree *tree1,
  edge_links **elinks,
  truth_value update_edge_links
)
{
  TRY();
  quad_tree *tree2;
  quad_tree_link_head *head1;
  list_item *links, *endlinks;
  typed_pointer *tptr;
  link_measure *measure_link1;
  edge_response *eresp;
  edge_links *links1, *links2;
  integral_value strength_score, profile_score, link_score, min_towards, min_against, min_other;
  integral_value own_angle, mag, angle, consistency, own_sum1, own_sum2, own_n, own_mean, own_dev;
  integral_value towards_angle, towards_sum1, towards_sum2, towards_n, towards_mean, towards_dev;
  integral_value against_angle, against_sum1, against_sum2, against_n, against_mean, against_dev;
  quad_tree_link_head *best_towards, *best_against, *best_other;

  CHECK_POINTER(forest);
  CHECK_POINTER(tree1);

  CHECK(ensure_has(&tree1->annotation, t_edge_links, &tptr));
  links1 = (edge_links*)tptr->value;
  /* if not yet created for this tree, create it using edge responses only */
  if (tptr->token != forest->token) {
    tptr->token = forest->token;
    CHECK(quad_tree_ensure_edge_response(forest, tree1, &eresp, TRUE));
    own_angle = eresp->ang - M_PI_2;
    if (own_angle < 0) own_angle += M_2PI;

    own_sum1 = 0;
    own_sum2 = 0;
    own_n = eresp->mag;
    towards_sum1 = 0;
    towards_sum2 = 0;
    towards_n = 0;
    against_sum1 = 0;
    against_sum2 = 0;
    against_n = 0;

    /* calculate sums of angle differences */
    links = tree1->links.first.next;
    endlinks = &tree1->links.last;
    while (links != endlinks) {
      head1 = *((quad_tree_link_head**)links->data);
      CHECK(quad_tree_link_head_ensure_measure(forest, head1, &measure_link1, FALSE));
      tree2 = head1->other->tree;
      CHECK(quad_tree_ensure_edge_response(forest, tree2, &eresp, TRUE));
      angle = eresp->ang - M_PI_2;
      if (angle < 0) angle += M_2PI;
      /* center angles around own angle */
      angle = angle_minus_angle(angle, own_angle);
      mag = eresp->mag;
      if (mag > 1) {
        if (measure_link1->category == bl_TOWARDS) {
          towards_sum1 += (mag * angle);
          towards_sum2 += (mag * angle * angle);
          towards_n += mag;
        }
        else
        if (measure_link1->category == bl_AGAINST) {
          against_sum1 += (mag * angle);
          against_sum2 += (mag * angle * angle);
          against_n += mag;
        }
        else {
          own_sum1 += (mag * angle);
          own_sum2 += (mag * angle * angle);
          own_n += mag;
        }
      }
      links = links->next;
    }

    own_mean = own_sum1 / own_n;
    own_dev = own_sum2 / own_n - own_mean * own_mean;
    own_dev = own_dev < 0 ? 0 : sqrt(own_dev);
    if (towards_n > 0) {
      towards_mean = towards_sum1 / towards_n;
      towards_dev = towards_sum2 / towards_n - towards_mean * towards_mean;
      towards_dev = towards_dev < 0 ? 0 : sqrt(towards_dev);
    }
    else {
      towards_mean = own_mean;
      towards_dev = own_dev;
    }
    if (against_n > 0) {
      against_mean = against_sum1 / against_n;
      against_dev = against_sum2 / against_n - against_mean * against_mean;
      against_dev = against_dev < 0 ? 0 : sqrt(against_dev);
    }
    else {
      against_mean = own_mean;
      against_dev = own_dev;
    }
    consistency = 1 - (own_dev / M_PI);
    if (consistency < 0) consistency = 0;
    links1->own_consistency = consistency;
    angle = own_angle + consistency * own_mean;
    if (angle < 0) angle += 2 * M_PI;
    links1->own_angle = angle;

    consistency = 1 - (towards_dev / M_PI);
    if (consistency < 0) consistency = 0;
    links1->towards_consistency = consistency;
    angle = own_angle + consistency * towards_mean;
    if (angle < 0) angle += 2 * M_PI;
    links1->towards_angle = angle;

    consistency = 1 - (against_dev / M_PI);
    if (consistency < 0) consistency = 0;
    links1->against_consistency = consistency;
    angle = own_angle + consistency * against_mean;
    if (angle < 0) angle += 2 * M_PI;
    links1->against_angle = angle;

    links1->curvature = angle_minus_angle(links1->towards_angle, links1->against_angle);
    links1->straightness = 1 - (fabs(links1->curvature) / M_PI);
    /* initially set the edge links as NULL */
    /* these are set on the second round, involving neighboring edge links */
    links1->towards = NULL; /*best_towards;*/
    links1->against = NULL; /*best_against;*/
  }
  /* if it is created, update it, using edge links where available */
  else
  if (IS_TRUE(update_edge_links) && (links1->towards == NULL || links1->against == NULL)) {
    own_angle = links1->own_angle;
    towards_angle = links1->towards_angle;
    against_angle = links1->against_angle;

    own_sum1 = 0;
    own_sum2 = 0;
    own_n = 1;
    towards_sum1 = 0;
    towards_sum2 = 0;
    towards_n = 0;
    against_sum1 = 0;
    against_sum2 = 0;
    against_n = 0;

    /* calculate sums of angle differences */
    links = tree1->links.first.next;
    endlinks = &tree1->links.last;
    while (links != endlinks) {
      head1 = *((quad_tree_link_head**)links->data);
      tree2 = head1->other->tree;
      CHECK(quad_tree_ensure_edge_links(forest, tree2, &links2, FALSE));
      CHECK(quad_tree_link_head_ensure_measure(forest, head1, &measure_link1, TRUE));

      if (measure_link1->category == bl_TOWARDS) {
        angle = angle_minus_angle(towards_angle, links2->own_angle);
        towards_sum1 += angle;
        towards_sum2 += (angle * angle);
        towards_n += 1;
      }
      else
      if (measure_link1->category == bl_AGAINST) {
        angle = angle_minus_angle(against_angle, links2->own_angle);
        against_sum1 += angle;
        against_sum2 += (angle * angle);
        against_n += 1;
      }
      else {
        angle = angle_minus_angle(own_angle, links2->own_angle);
        own_sum1 += angle;
        own_sum2 += (angle * angle);
        own_n += 1;
      }
      links = links->next;
    }

    own_mean = own_sum1 / own_n;
    own_dev = own_sum2 / own_n - own_mean * own_mean;
    own_dev = own_dev < 0 ? 0 : sqrt(own_dev);
    if (towards_n > 0) {
      towards_mean = towards_sum1 / towards_n;
      towards_dev = towards_sum2 / towards_n - towards_mean * towards_mean;
      towards_dev = towards_dev < 0 ? 0 : sqrt(towards_dev);
    }
    else {
      towards_mean = own_mean;
      towards_dev = own_dev;
    }
    if (against_n > 0) {
      against_mean = against_sum1 / against_n;
      against_dev = against_sum2 / against_n - against_mean * against_mean;
      against_dev = against_dev < 0 ? 0 : sqrt(against_dev);
    }
    else {
      against_mean = own_mean;
      against_dev = own_dev;
    }
    consistency = 1 - (own_dev / M_PI);
    if (consistency < 0) consistency = 0;
    links1->own_consistency = consistency;
    angle = own_angle + consistency * own_mean;
    if (angle < 0) angle += 2 * M_PI;
    links1->own_angle = angle;

    consistency = 1 - (towards_dev / M_PI);
    if (consistency < 0) consistency = 0;
    links1->towards_consistency = consistency;
    angle = towards_angle + consistency * towards_mean;
    if (angle < 0) angle += 2 * M_PI;
    links1->towards_angle = angle;

    consistency = 1 - (against_dev / M_PI);
    if (consistency < 0) consistency = 0;
    links1->against_consistency = consistency;
    angle = against_angle + consistency * against_mean;
    if (angle < 0) angle += 2 * M_PI;
    links1->against_angle = angle;

    links1->curvature = angle_minus_angle(links1->towards_angle, links1->against_angle);
    links1->straightness = 1 - (fabs(links1->curvature) / M_PI);

    /* now should find the best towards and against links */
    /* one more loop of the links.. */
    {
      quad_tree_link_head *best_towards, *best_against;
      boundary *boundary_tree, *boundary_link;
      boundary_potential *potential_tree, *potential_link;
      integral_value angle1, angle2, against_angle, anglediff, link_score, max_towards, max_against;

      best_towards = NULL;
      max_towards = 0;
      best_against = NULL;
      max_against = 0;
      boundary_tree = has_boundary(&tree1->annotation, forest->token);
      potential_tree = has_boundary_potential(&tree1->annotation, forest->token);
      links = tree1->links.first.next;
      endlinks = &tree1->links.last;
      while (links != endlinks) {
        head1 = *((quad_tree_link_head**)links->data);
        CHECK(expect_link_measure(&head1->annotation, &measure_link1, forest->token));
        /*measure_link1 = has_link_measure(&head1->annotation, forest->token);*/
        tree2 = head1->other->tree;
        boundary_link = has_boundary(&tree2->annotation, forest->token);
        potential_link = has_boundary_potential(&tree2->annotation, forest->token);

        /* use expect function */
        CHECK(expect_edge_links(&tree2->annotation, &links2, forest->token));

        anglediff = fabs(angle_minus_angle(links1->own_angle, links2->own_angle));
        /*measure_link1->straightness_score = 1 - (anglediff / M_PI);*/
        link_score = measure_link1->strength_score + 2 * fabs(measure_link1->angle_score);
        if (boundary_tree != NULL || potential_tree != NULL) {
          if (boundary_link != NULL) {
            link_score += 5;
          }
          else
          if (potential_link != NULL) {
            link_score += 3;
          }
        }

        angle1 = angle_minus_angle(head1->angle, links1->towards_angle);
        /* close to towards-angle -> category towards */
        if (fabs(angle1) < (M_PI_4+0.1)) {
          /*PRINT0("towards\n");*/
          measure_link1->category = bl_TOWARDS;
          measure_link1->angle_score = 1 - (angle1 / M_PI_2);
          if (link_score > max_towards) {
            best_towards = head1;
            max_towards = link_score;
          }
        }
        else {
          against_angle = links1->against_angle - M_PI;
          if (against_angle < 0) against_angle += M_2PI;
          angle2 = angle_minus_angle(head1->angle, against_angle);
          /* close to against-angle -> category against */
          if (fabs(angle2) < (M_PI_4+0.1)) {
            /*PRINT0("against\n");*/
            measure_link1->category = bl_AGAINST;
            measure_link1->angle_score = 1 - (angle2 / M_PI_2);
            if (link_score > max_against) {
              best_against = head1;
              max_against = link_score;
            }
          }
          else {
            /* larger than towards and smaller than against -> left */
            if (angle1 > 0 && angle2 < 0) {
              measure_link1->category = bl_LEFT;
              angle1 = getmax(angle1, -angle2);
              measure_link1->angle_score = 1 - (angle1 / M_PI_2);
            }
            /* smaller than towards and larger than against -> right */
            else {
              measure_link1->category = bl_RIGHT;
              angle1 = getmax(fabs(angle1), fabs(angle2));
              measure_link1->angle_score = 1 - (angle1 / M_PI_2);
            }
          }
        }
        links = links->next;
      }
      /*
      if (best_towards == NULL || best_against == NULL) {
        ERROR(NOT_FOUND);
      }
      */
      /*
      if (best_towards == NULL) {
        PRINT0("towards not found\n");
      }
      if (best_against == NULL) {
        PRINT0("against not found\n");
      }
      */
      links1->towards = best_towards;
      links1->against = best_against;
    }
  }

  if (elinks != NULL) {
    *elinks = links1;
  }

  FINALLY(quad_tree_ensure_edge_links);
  RETURN();
}

/******************************************************************************/

result quad_tree_ensure_ridge_potential
(
  quad_forest *forest,
  quad_tree *tree,
  ridge_potential **ridge
)
{
  TRY();
  typed_pointer *tptr;
  ridge_potential *rpot;
  edge_links *elinks;

  CHECK_POINTER(forest);
  CHECK_POINTER(tree);

  /* else add ridge to this node */
  /*CHECK(ensure_ridge_potential(&tree->annotation, ridge, forest->token));*/
  CHECK(ensure_has(&tree->annotation, t_ridge_potential, &tptr));
  rpot = (ridge_potential*)tptr->value;
  if (tptr->token != forest->token) {
    tptr->token = forest->token;
  }
  rpot->round = 0;

  CHECK(quad_tree_ensure_edge_links(forest, tree, &elinks, FALSE));

  if (ridge != NULL) {
    *ridge = rpot;
  }

  FINALLY(quad_tree_ensure_ridge_potential);
  RETURN();
}

/******************************************************************************/

/* parameters needed. */
/* strength difference threshold */
/* use dev threshold instead of truth value - thresh > 0 will trigger using it */
result quad_forest_parse
(
  quad_forest *forest,
  uint32 rounds,
  truth_value use_dev
)
{
  TRY();
  quad_tree *tree1, *tree2, *bestneighbor;
  quad_tree_link_head *head1, *head2, *best_lagainst, *best_ltowards;
  quad_tree_link *link1, *link2;
  neighborhood_stat *nstat1, *nstat2;
  edge_response *eresp1, *eresp2;
  ridge_potential *ridge_tree, *ridge_link1, *ridge_link2;
  boundary *bfrag_tree, *bfrag_link1, *bfrag_link2;
  boundary_potential *boundary_tree, *boundary_link1, *boundary_link2;
  boundary_message *bmsg_link1, *bmsg_link2, *best_magainst, *best_mtowards;
  segment *segment_tree, *segment_link1, *segment_link2;
  segment_message *smsg_link1, *smsg_link2;
  link_measure *measure_link1, *measure_link2;
  edge_links *elinks_tree, *elinks_towards, *elinks_against;
  typed_pointer *tptr;
  list ridgelist, boundarylist, link_send, link_recv;
  list_item *trees, *endtrees, *links, *endlinks;
  integral_value strength1, strength2, newstrength, minstrength, maxstrength, count;
  integral_value angle1, angle2, anglediff, anglescore;
  uint32 i, min_rank, max_extent, new_ridge, new_segment, new_boundary;
  integral_value mag1, mag2, mag_max, mag_diff, mag_diff_min, mag_score;
  integral_value strengthdiff, maxstrengthdiff, strength_threshold;
  integral_value mean1, mean2, dev, meandiff, maxmeandiff;
  integral_value n_left, sum_left, n_right, sum_right;

  CHECK_POINTER(forest);
  CHECK_PARAM(rounds > 0);

  CHECK(list_create(&ridgelist, 1000, sizeof(quad_tree*), 1));
  CHECK(list_create(&boundarylist, 1000, sizeof(quad_tree*), 1));
  CHECK(list_create(&link_send, 1000, sizeof(quad_tree_link_head*), 1));
  CHECK(list_create(&link_recv, 1000, sizeof(quad_tree_link_head*), 1));

  /*PRINT0( "get neighborhood stats\n" );*/
  /*CHECK(quad_forest_calculate_neighborhood_stats(forest, FALSE));*/

  CHECK(quad_forest_calculate_edge_stats(forest));
  TERMINATE(SUCCESS);

  /*PRINT0( "get edge responses for all trees\n" );*/
  trees = forest->trees.first.next;
  endtrees = &forest->trees.last;
  while (trees != endtrees) {
    tree1 = (quad_tree*)trees->data;
    CHECK(quad_tree_ensure_edge_response(forest, tree1, NULL, TRUE));
    trees = trees->next;
  }

  /*PRINT0("find ridge candidates and get the edge links\n" );*/
  trees = forest->trees.first.next;
  endtrees = &forest->trees.last;
  while (trees != endtrees) {
    tree1 = (quad_tree*)trees->data;

    /* if already has ridge potential, no need to process again */
    ridge_tree = has_ridge_potential(&tree1->annotation, forest->token);
    if (ridge_tree != NULL) {
      trees = trees->next;
      continue;
    }

    /*
      TODO: there are some hard-coded thresholds, need to parameterize or get rid of them.
      -strengthdiff relative to strength
      -dev threshold
    */

    /* triggering ridges by deviation threshold is not a very good idea always */
    /* TODO: change to dev_threshold > 0 or similar */
    if (IS_TRUE(use_dev) && tree1->stat.deviation > 7) {
      CHECK(quad_tree_ensure_ridge_potential(forest, tree1, &ridge_tree));
      CHECK(list_append(&ridgelist, &tree1));
    }
    else {
      /* use overlap as component of strength or not? mag? parameterize? */
      CHECK(expect_neighborhood_stat(&nstat1, &tree1->annotation));
      strength1 = nstat1->strength;
      CHECK(expect_edge_response(&eresp1, &tree1->annotation));
      if (eresp1->mag > 1 && eresp1->hpeaks < 3 && eresp1->vpeaks < 3) {
        CHECK(quad_tree_ensure_ridge_potential(forest, tree1, &ridge_tree));
        CHECK(list_append(&ridgelist, &tree1));
      }
      trees = trees->next;
      continue;

      maxstrength = strength1;
      maxstrengthdiff = 0;
      bestneighbor = NULL;
      /* measure the maximum strength difference of node and its neighbors */
      links = tree1->links.first.next;
      endlinks = &tree1->links.last;
      while (links != endlinks) {
        head1 = *((quad_tree_link_head**)links->data);
        tree2 = head1->other->tree;
        CHECK(expect_neighborhood_stat(&nstat2, &tree2->annotation));
        /* use overlap as component of strength or not? mag? parameterize? */
        strength2 = nstat2->strength;

        /* find out if some neighbor is stronger than this one */
        if (strength2 > maxstrength) {
          maxstrength = strength2;
          bestneighbor = tree2;
        }

        strengthdiff = fabs(strength1 - strength2);
        if (strengthdiff > maxstrengthdiff) {
          maxstrengthdiff = strengthdiff;
        }
        links = links->next;
      }

      /* large strength difference indicates a ridge is near */
      /* the ridge may be in this node, or in the neighbor with highest strength */
      if (maxstrengthdiff / strength1 > 0.5) {
        /*
        CHECK(quad_tree_ensure_edge_response(forest, tree1, &eresp1, TRUE));
        strength1 *= eresp1->mag;
        maxstrength = strength1;
        bestneighbor = NULL;
        links = tree1->links.first.next;
        endlinks = &tree1->links.last;
        while (links != endlinks) {
          head1 = *((quad_tree_link_head**)links->data);
          tree2 = head1->other->tree;
          CHECK(expect_neighborhood_stat(&nstat2, &tree2->annotation));
          CHECK(quad_tree_ensure_edge_response(forest, tree2, &eresp2, TRUE));
          strength2 = nstat2->strength * eresp2->mag;
          if (strength2 > maxstrength) {
            maxstrength = strength2;
            bestneighbor = tree2;
          }
          links = links->next;
        }
        */
        /* if neighbor is stronger, add ridge there */
        if (maxstrength > strength1) {
          tree1 = bestneighbor;
        }
        /* else add ridge to this node */
        CHECK(quad_tree_ensure_ridge_potential(forest, tree1, &ridge_tree));
        /* collect all ridge nodes to a list */
        CHECK(list_append(&ridgelist, &tree1));
      }
    }
    trees = trees->next;
  }

  /* in the next phase, include all neighbors of initial ridge nodes, that    */
  /* are on the same strength level or higher                                 */
  /* also add boundaries to a new list (?) */
  /*PRINT0("get initial boundary fragments\n");*/
  trees = ridgelist.first.next;
  endtrees = &ridgelist.last;
  while (trees != endtrees) {
    tree1 = *((quad_tree**)trees->data);
    CHECK(expect_neighborhood_stat(&nstat1, &tree1->annotation));
    /*CHECK(quad_tree_ensure_edge_response(forest, tree1, &eresp1, TRUE));*/
    /*strength1 = nstat1->strength * eresp1->mag;*/
    strength1 = nstat1->strength;
    CHECK(expect_edge_response(&eresp1, &tree1->annotation));
    mag1 = eresp1->mag;
    CHECK(quad_tree_ensure_edge_links(forest, tree1, &elinks_tree, FALSE));
    n_left = 0;
    sum_left = 0;
    n_right = 0;
    sum_right = 0;

    links = tree1->links.first.next;
    endlinks = &tree1->links.last;
    while (links != endlinks) {
      head1 = *((quad_tree_link_head**)links->data);
      tree2 = head1->other->tree;
      CHECK(expect_edge_response(&eresp2, &tree2->annotation));
      mag2 = eresp2->mag;
      measure_link1 = has_link_measure(&head1->annotation, forest->token);
      if (measure_link1 != NULL) {
        if (measure_link1->category == bl_LEFT) {
          sum_left += measure_link1->strength_score + (mag1 - mag2);
          n_left += 1;
        }
        else
        if (measure_link1->category == bl_RIGHT) {
          sum_right += measure_link1->strength_score + (mag1 - mag2);
          n_right += 1;
        }
      }
      /* first check if this neighbor is already a ridge node */
      ridge_link1 = has_ridge_potential(&tree2->annotation, forest->token);
      if (ridge_link1 == NULL) {
        CHECK(expect_neighborhood_stat(&nstat2, &tree2->annotation));
        /*CHECK(quad_tree_ensure_edge_response(forest, tree2, &eresp2, TRUE));*/
        /* use overlap as component of strength or not? mag? parameterize? */
        /*strength2 = nstat2->strength * eresp2->mag;*/
        strength2 = nstat2->strength;

        /* is this neighbor higher or at same level? */
        if (strength1 - strength2 > 0 || mag1 - mag2 > 0) {
          /* if yes, make it ridge node as well */
          /*
          CHECK(quad_tree_ensure_ridge_potential(forest, tree2, &ridge_tree));
          CHECK(list_append(&ridgelist, &tree2));
          */
        }
      }
      links = links->next;
    }
    if (n_left > 0) {
      sum_left /= n_left;
    }
    if (n_right > 0) {
      sum_right /= n_right;
    }
    if ((sum_left > -0.000001 && sum_right > 0.000001) ||
        (sum_left > 0.000001 && sum_right > -0.000001)) {
      /*CHECK(quad_tree_ensure_edge_links(forest, tree1, &elinks_tree, TRUE));*/
      CHECK(quad_tree_ensure_boundary(tree1, &bfrag_tree));
      CHECK(list_append(&boundarylist, &tree1));
    }
    trees = trees->next;
  }
  /*PRINT0("starting main parse loop\n");*/
  {
    uint32 max_extent, length;
    integral_value curvature_own, curvature_towards, curvature_against;
    integral_value distance, towards_score, towards_max, against_score, against_max;
    boundary *bfrag_towards, *bfrag_against, *bpar_tree, *bpar_towards, *bpar_against;
    boundary *bfrag_new;
    boundary_potential *bpot_towards, *bpot_against, *bpot_tree, *bpot_current;

    /* in the next phase, start propagating signals starting from boundary nodes */
    /* a number of message passing rounds are performed */
    /* it is important to avoid _loops_ in message passing */
    /* thus, nodes should not use information received from a node in the reply */
    for (i = 1; i <= rounds; i++) {
      /*PRINT1("round %d\n", i);*/
      /* boundary message loop */
      /* segment nodes are added to the link_send list here */
      /*PRINT0("boundary message loop\n");*/
      trees = boundarylist.first.next;
      endtrees = &boundarylist.last;
      while (trees != endtrees) {
        tree1 = *((quad_tree**)trees->data);
        bfrag_tree = has_boundary(&tree1->annotation, forest->token);
        bpar_tree = NULL;
        bpot_tree = has_boundary_potential(&tree1->annotation, forest->token);
        /* after this, I have the best towards and against links in elinks */
        CHECK(quad_tree_ensure_edge_links(forest, tree1, &elinks_tree, TRUE));
        /*CHECK(expect_edge_links(&tree1->annotation, &elinks_tree, forest->token));*/
        curvature_own = elinks_tree->curvature;

        /* check the curvature of the best links relative to own curvature */
        bfrag_towards = NULL;
        bpar_towards = NULL;
        if (elinks_tree->towards != NULL) {
          tree2 = elinks_tree->towards->other->tree;
          CHECK(quad_tree_ensure_edge_links(forest, tree2, &elinks_towards, TRUE));

          bfrag_towards = has_boundary(&tree2->annotation, forest->token);
          /* if best link has boundary, check curvature */
          /*if (bfrag_towards != NULL) {*/
            /* if tree doesn't have boundary, and best link isn't the parent, connect */
            /*if (bfrag_tree == NULL) {*/
              /* if fragment isn't there, potential should be; otherwise error */
              /*CHECK_POINTER(bpot_tree);*/
              /* encountered fragment from another chain, collapse to fragments */
              /*if (bpot_tree->parent != bfrag_towards) {*/
                /*CHECK(quad_tree_ensure_boundary(tree1, &bfrag_tree));*/
              /*}*/
            /*}*
          /*}*/
          /*else {*/
          if (bfrag_towards == NULL) {
            bpot_towards = has_boundary_potential(&tree2->annotation, forest->token);
            /* if best link doesn't have potential, create it and add to list */
            if (bpot_towards == NULL) {
              /* if even tree doesn't have a fragment, add to chain */
              if (bfrag_tree == NULL) {
                /* if fragment isn't there, potential should be; otherwise error */
                CHECK_POINTER(bpot_tree);
                /* must not reach too far from established fragments */
                if (bpot_tree->length < 3) {
                  CHECK(ensure_boundary_potential(&tree2->annotation, &bpot_towards,
                                                  forest->token));
                  bpot_towards->parent = bpot_tree->parent;
                  bpot_towards->prev = bpot_tree;
                  bpot_towards->length = bpot_tree->length + 1;
                  bpot_towards->angle = elinks_tree->towards_angle;
                  bpot_towards->curvature = elinks_tree->curvature;
                  CHECK(list_append(&boundarylist, &tree2));
                }
              }
              /* if tree has a fragment, create a new chain starting from the tree */
              else {
                CHECK(ensure_boundary_potential(&tree2->annotation, &bpot_towards,
                                                forest->token));
                bpot_towards->parent = bfrag_tree;
                bpot_towards->prev = NULL;
                bpot_towards->length = 1;
                bpot_towards->angle = elinks_tree->towards_angle;
                bpot_towards->curvature = elinks_tree->curvature;
                CHECK(list_append(&boundarylist, &tree2));
              }
            }
            /* if best link has boundary, check is it from another fragment */
            else {
              if (bfrag_tree == NULL) {
                /* if fragment isn't there, potential should be; otherwise error */
                CHECK_POINTER(bpot_tree);
                /* two potential chains meet, collapse both to fragments */
                if (bpot_towards->parent != bpot_tree->parent) {
                  CHECK(quad_tree_ensure_boundary(tree1, &bfrag_tree));
                  CHECK(quad_tree_ensure_boundary(tree2, &bfrag_towards));
                }
              }
              else {
                /* meeting potential chain from another node, collapse to fragments */
                if (bpot_towards->parent != bfrag_tree) {
                  CHECK(quad_tree_ensure_boundary(tree2, &bfrag_towards));
                }
              }
            }
          }
        }

        bfrag_against = NULL;
        bpar_against = NULL;

        if (elinks_tree->against != NULL) {
          tree2 = elinks_tree->against->other->tree;
          CHECK(quad_tree_ensure_edge_links(forest, tree2, &elinks_against, TRUE));

          bfrag_against = has_boundary(&tree2->annotation, forest->token);
          /* if best link has boundary but tree doesn't, check if could add it */
          /*
          if (bfrag_against != NULL) {
            if (bfrag_tree == NULL) {*/
              /* if fragment isn't there, potential should be; otherwise error */
              /*CHECK_POINTER(bpot_tree);
              if (bpot_tree->parent != bfrag_against) {*/
                /*CHECK(quad_tree_ensure_boundary(tree1, &bfrag_tree));*/
              /*}
            }
          }
          else {*/
          if (bfrag_against == NULL) {
            bpot_against = has_boundary_potential(&tree2->annotation, forest->token);
            /* add boundary to tree if best link has potential with different parent */
            /* if best link doesn't have boundary, create boundary and add to list */
            if (bpot_against == NULL) {
              if (bfrag_tree == NULL) {
                CHECK_POINTER(bpot_tree);
                if (bpot_tree->length < 3) {
                  CHECK(ensure_boundary_potential(&tree2->annotation, &bpot_against,
                                                  forest->token));
                  bpot_against->parent = bpot_tree->parent;
                  bpot_against->prev = bpot_tree;
                  bpot_against->length = bpot_tree->length + 1;
                  bpot_against->angle = elinks_tree->against_angle;
                  bpot_against->curvature = elinks_tree->curvature;
                  CHECK(list_append(&boundarylist, &tree2));
                }
              }
              else {
                CHECK(ensure_boundary_potential(&tree2->annotation, &bpot_against,
                                                forest->token));
                bpot_against->parent = bfrag_tree;
                bpot_against->prev = NULL;
                bpot_against->length = 1;
                bpot_against->angle = elinks_tree->against_angle;
                bpot_against->curvature = elinks_tree->curvature;
                CHECK(list_append(&boundarylist, &tree2));
              }
            }
            else {
              if (bfrag_tree == NULL) {
                CHECK_POINTER(bpot_tree);
                if (bpot_against->parent != bpot_tree->parent) {
                  CHECK(quad_tree_ensure_boundary(tree1, &bfrag_tree));
                  CHECK(quad_tree_ensure_boundary(tree2, &bfrag_against));
                }
              }
              else {
                if (bpot_against->parent != bfrag_tree) {
                  CHECK(quad_tree_ensure_boundary(tree2, &bfrag_against));
                }
              }
            }
          }
        }

        if (bfrag_tree != NULL) {
          bpar_tree = boundary_find(bfrag_tree);
          if (bpar_tree->category == fc_UNDEF) {
            boundary_init(bpar_tree, elinks_tree);
          }
          if (bfrag_towards != NULL) {
            bpar_towards = boundary_find(bfrag_towards);
            if (bpar_towards->category == fc_UNDEF) {
              boundary_init(bpar_towards, elinks_towards);
            }
            if (bpar_towards != bpar_tree) {
              if (fabs(bpar_towards->curvature_mean - bpar_tree->curvature_mean) < 0.2) {
                boundary_union(bpar_tree, bpar_towards);
              }
            }
          }
          if (bfrag_against != NULL) {
            bpar_against = boundary_find(bfrag_against);
            if (bpar_against->category == fc_UNDEF) {
              boundary_init(bpar_against, elinks_against);
            }
            if (bpar_against != bpar_tree) {
              if (fabs(bpar_against->curvature_mean - bpar_tree->curvature_mean) < 0.2) {
                boundary_union(bpar_against, bpar_tree);
              }
            }
          }
        }

        /* ensure all segment messages are in place in perpendicular links */
        links = tree1->links.first.next;
        endlinks = &tree1->links.last;
        while (links != endlinks) {
          head1 = *((quad_tree_link_head**)links->data);
          /* get the link measure to determine the category */
          /* use 'expect'-function */
          CHECK(expect_link_measure(&head1->annotation, &measure_link1, forest->token));
          if (IS_PERPENDICULAR(measure_link1->category) && IS_N4(head1->link->category)) {
            tree2 = head1->other->tree;
            bfrag_link1 = has_boundary(&tree2->annotation, forest->token);
            boundary_link1 = has_boundary_potential(&tree2->annotation, forest->token);
            if (bfrag_link1 == NULL && boundary_link1 == NULL) {
              CHECK(ensure_segment_message(&head1->annotation, &smsg_link1,
                  forest->token, measure_link1->strength_score));
              CHECK(list_append(&link_send, &head1));
            }
          }
          links = links->next;
        }
        trees = trees->next;
      }
    }

    /* final boundary merging loop */
    trees = boundarylist.first.next;
    endtrees = &boundarylist.last;
    while (trees != endtrees) {
      tree1 = *((quad_tree**)trees->data);
      bfrag_tree = has_boundary(&tree1->annotation, forest->token);

      /* go through all links and figure out what to do with neighbors */
      links = tree1->links.first.next;
      endlinks = &tree1->links.last;
      while (links != endlinks) {
        head1 = *((quad_tree_link_head**)links->data);
        /* get the link measure to determine the category */
        /* use 'expect'-function */
        CHECK(expect_link_measure(&head1->annotation, &measure_link1, forest->token));
        if (IS_PERPENDICULAR(measure_link1->category) && IS_N4(head1->link->category)) {
          tree2 = head1->other->tree;
          bfrag_link1 = has_boundary(&tree2->annotation, forest->token);
          boundary_link1 = has_boundary_potential(&tree2->annotation, forest->token);
          if (bfrag_link1 == NULL && boundary_link1 == NULL) {
            CHECK(ensure_segment_message(&head1->annotation, &smsg_link1,
                                         forest->token, measure_link1->strength_score));
            CHECK(list_append(&link_send, &head1));
          }
        }
        links = links->next;
      }
      trees = trees->next;
    }

    /* segment send loop */
    /*PRINT0("segment send loop\n");*/
    trees = link_send.first.next;
    endtrees = &link_send.last;
    while (trees != endtrees) {
      head1 = *((quad_tree_link_head**)trees->data);
      tree1 = head1->other->tree;
      head2 = head1->other->opposite;
      CHECK(expect_segment_message(&head1->annotation, &smsg_link1, forest->token));
      bfrag_link1 = has_boundary(&tree1->annotation, forest->token);
      boundary_link1 = has_boundary_potential(&tree1->annotation, forest->token);

      if (head2 != NULL) {
        if (IS_FALSE(smsg_link1->echo)) {
          smsg_link2 = has_segment_message(&head2->other->annotation, forest->token);
          /* encountered message from other direction, may echo back */
          if (smsg_link2 != NULL) {
            smsg_link1->extent = smsg_link1->extent + smsg_link2->extent;
            smsg_link1->echo = TRUE;
            smsg_link2->extent = smsg_link1->extent;
            smsg_link2->echo = TRUE;
            CHECK(list_prepend(&link_recv, &head2->other));
            CHECK(list_prepend(&link_recv, &head1));
          }
          else {
            /* if boundary not encountered, continue to propagate */
            if (bfrag_link1 == NULL && boundary_link1 == NULL) {
              CHECK(ensure_segment_message(&head2->annotation, &smsg_link2, forest->token, 0));
              smsg_link2->extent = smsg_link1->extent + 1;
              CHECK(list_append(&link_send, &head2));
              CHECK(list_prepend(&link_recv, &head2));
            }
            /* encountered boundary and may echo back */
            else {
              smsg_link1->echo = TRUE;
              /*CHECK(list_prepend(&link_recv, &head1));*/
            }
          }
        }
      }
      /* at edge of image, echo back */
      else {
        smsg_link1->echo = TRUE;
        /* if the node doesn't have boundary, may create segment */
        if (bfrag_link1 == NULL && boundary_link1 == NULL) {
          CHECK(list_prepend(&link_recv, &head1));
        }
      }
      trees = trees->next;
    }

    /*PRINT0("segment recv loop\n");*/
    trees = link_recv.first.next;
    endtrees = &link_recv.last;
    while (trees != endtrees) {
      head1 = *((quad_tree_link_head**)trees->data);
      CHECK(expect_segment_message(&head1->annotation, &smsg_link1, forest->token));
      tree1 = head1->other->tree;
      head2 = head1->other->opposite;
      if (head2 != NULL) {
        smsg_link2 = has_segment_message(&head2->annotation, forest->token);
        if (smsg_link2 != NULL) {
          smsg_link1->extent = smsg_link2->extent;
        }
      }
      CHECK(quad_tree_ensure_segment(tree1, &segment_tree));
      segment_tree->extent = smsg_link1->extent + 1;
      trees = trees->next;
    }
  } /* propagation loop frame */

  /* final belief accumulation loop */
  /*PRINT0("belief accumulation loop\n");*/
  /* maybe loop through all nodes? or add all neighbors not considered yet? */
  /*
  trees = forest->trees.first.next;
  endtrees = &forest->trees.last;
  while (trees != endtrees) {
    tree1 = (quad_tree*)trees->data;
    segment_tree = has_segment(&tree1->annotation, forest->token);
    if (segment_tree != NULL) {
      links = tree1->links.first.next;
      endlinks = &tree1->links.last;
      while (links != endlinks) {
        head1 = *((quad_tree_link_head**)links->data);
        if (IS_N4(head1->link->category)) {
          tree2 = head1->other->tree;
          segment_link1 = has_segment(&tree2->annotation, forest->token);
          if (segment_link1 != NULL) {
            segment_union(segment_tree, segment_link1);
          }
        }
        links = links->next;
      }
    }
    */
    /* if has boundary fragment, merge with similar */
    /* if has segment potential, create segment and merge with neighbors */
    /* (those that are not too different?) */
    /*
    trees = trees->next;
  }*/

  /* set the state of forest */
  quad_forest_set_parse(forest);

  /*PRINT0("finished\n");*/
  FINALLY(quad_forest_parse);
  list_destroy(&ridgelist);
  list_destroy(&boundarylist);
  list_destroy(&link_send);
  list_destroy(&link_recv);
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

  CHECK(quad_forest_visualize_neighborhood_stats(forest, target, v_SCORE));
  CHECK(quad_forest_get_links(forest, &links, v_LINK_MEASURE));
  CHECK(pixel_image_draw_colored_lines(target, &links, 2));
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
  TERMINATE(SUCCESS);

  if (IS_TRUE(quad_forest_has_parse(forest))) {
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
            crect.left = bparent->x1;
            crect.top = bparent->y1;
            crect.right = bparent->x2;
            crect.bottom = bparent->y2;
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
        boundary1 = has_boundary_potential(&tree->annotation, forest->token);
        segment1 = has_segment(&tree->annotation, forest->token);
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
          color0 = (byte)(255 * 0);
          if (bparent->category == fc_STRAIGHT) {
            color1 = (byte)(255 * 1);
          }
          else
          if (bparent->category == fc_CURVED) {
            color1 = (byte)(255 * 0.5);
          }
          else
          if (bparent->category == fc_CORNER) {
            color1 = (byte)(255 * 0.0);
          }
          else {
            color1 = (byte)(255 * 0.0);
          }
          color2 = (byte)(255 * 0);
          /*
          color0 = bparent->color[0];
          color1 = bparent->color[1];
          color2 = bparent->color[2];
          */
        }
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
      byte edge_color_1[4] = {0,255,0,0};
      byte edge_color_2[4] = {0,255,0,0};
      byte segment_color[4] = {255,255,255,0};

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

      trees = forest->trees.first.next;
      end = &forest->trees.last;
      while (trees != end) {
        tree = (quad_tree*)trees->data;
        CHECK(quad_tree_edge_response_to_line(forest, tree, &links));
        trees = trees->next;
      }
      PRINT1("edges found: %d\n", links.count);
      /*CHECK(quad_tree_gradient_to_line(tree, &links));*/
      CHECK(pixel_image_draw_lines(target, &links, segment_color, 1));
      /*CHECK(pixel_image_draw_weighted_lines(target, &links, segment_color));*/
      /*
      PRINT1("fragments found: %lu, ", frag_count);
      PRINT1("frags in list: %lu\n", frags.count);
      CHECK(pixel_image_draw_colored_rects(target, &frags));
      */
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
