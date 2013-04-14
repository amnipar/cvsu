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

string prime_reg_accumulator_name = "prime_reg_accumulator";
string prop_reg_accumulator_name = "prop_reg_accumulator";
string acc_reg_accumulator_name = "acc_reg_accumulator";

string prime_ridge_finder_name = "prime_ridge_finder";

string run_context_operation_name = "run_context_operation";
string quad_forest_calculate_accumulated_stats_name = "quad_forest_calculate_accumulated_stats";
string quad_forest_visualize_accumulated_stats_name = "quad_forest_visualize_accumulated_stats";
string quad_forest_calculate_neighborhood_stats_name = "quad_forest_calculate_neighborhood_stats";
string quad_forest_visualize_neighborhood_stats_name = "quad_forest_visualize_neighborhood_stats";
string quad_forest_calculate_accumulated_regs_name = "quad_forest_calculate_accumulated_regs";
string quad_forest_visualize_accumulated_regs_name = "quad_forest_visualize_accumulated_regs";
string quad_forest_calculate_accumulated_bounds_name = "quad_forest_calculate_accumulated_bounds";
string quad_forest_visualize_accumulated_bounds_name = "quad_forest_visualize_accumulated_bounds";
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

result prime_reg_accumulator
(
  quad_forest *forest,
  quad_tree *tree,
  list *collection
)
{
  TRY();
  list_item *links, *endlinks;
  reg_accumulator *reg, *link_reg;
  neighborhood_stat *nstat_tree, *nstat_neighbor;
  quad_tree_link_head *head;
  quad_tree *neighbor;

  CHECK_POINTER(tree);
  if (tree->context.token != forest->token) {
    typed_pointer_destroy(&tree->context);
    tree->context.token = forest->token;
  }
  CHECK(ensure_reg_accumulator(&tree->context, &reg));
  if (reg->round == 0) {
    integral_value overlap;
    CHECK(expect_neighborhood_stat(&nstat_tree, &tree->annotation));
    overlap = nstat_tree->overlap;
    if (overlap < 0.25) {
      reg->boundary_acc = 1;
      reg->segment_acc = 0;
    }
    else
    if (overlap > 0.75) {
      reg->boundary_acc = 0;
      reg->segment_acc = 1;
    }
    else {
      reg->boundary_acc = 0;
      reg->segment_acc = 0;
    }
    reg->round = 1;
  }
  else {
    links = tree->links.first.next;
    endlinks = &tree->links.last;
    while (links != endlinks) {
      head = *((quad_tree_link_head**)links->data);
      if (head->link->category != d_N6) {
        if (head->other->annotation.token == forest->token) {
          CHECK(ensure_reg_accumulator(&head->other->annotation, &link_reg));
          if (link_reg->round > 0) {
            reg->boundary_acc += link_reg->boundary_acc;
            if (reg->boundary_acc < 0) {
              reg->boundary_acc = 0;
            }
            link_reg->boundary_acc = 0;
            reg->segment_acc += link_reg->segment_acc;
            if (reg->segment_acc < 0) {
              reg->segment_acc = 0;
            }
            link_reg->segment_acc = 0;
          }
        }
      }
      links = links->next;
    }
    reg->round++;
  }

  FINALLY(prime_stat_accumulator);
  (void)forest;
  (void)collection;
  RETURN();
}

/******************************************************************************/

result prop_reg_accumulator
(
  quad_forest *forest,
  quad_tree *tree,
  list *collection
)
{
  TRY();
  list_item *links, *endlinks;
  reg_accumulator *reg_tree, *reg_link;
  neighborhood_stat *nstat_tree, *nstat_link;
  edge_response *eresp;
  quad_tree *neighbor;
  quad_tree_link_head *head;

  CHECK_POINTER(tree);
  reg_tree = has_reg_accumulator(&tree->context);
  CHECK_POINTER(reg_tree);
  CHECK(expect_neighborhood_stat(&nstat_tree, &tree->annotation));

  if (reg_tree->boundary_acc > 0) {
    integral_value height1, height2, heightdiff, angle1, angle2, anglediff;

    height1 = nstat_tree->strength;
    CHECK(quad_tree_ensure_edge_response(forest, tree, &eresp));
    angle1 = eresp->ang;
    if (angle1 > M_PI) angle1 -= M_PI;

    links = tree->links.first.next;
    endlinks = &tree->links.last;
    while (links != endlinks) {
      head = *((quad_tree_link_head**)links->data);
      if (head->annotation.token != forest->token) {
        typed_pointer_destroy(&head->annotation);
        head->annotation.token = forest->token;
      }
      if (head->link->category != d_N6) {
        neighbor = head->tree;
        CHECK(ensure_reg_accumulator(&head->annotation, &reg_link));

        CHECK(expect_neighborhood_stat(&nstat_link, &neighbor->annotation));
        height2 = nstat_link->strength;
        heightdiff = height2 - height1;

        angle2 = head->angle;
        if (angle2 > M_PI) angle2 -= M_PI;
        anglediff = fabs(angle1 - angle2);
        if (anglediff > (M_PI / 2)) anglediff = M_PI - anglediff;
        anglediff /= (M_PI / 2);
        /* neighbor in perpendicular direction */
        /* TODO: should take into account also neighbor's edge direction */
        if (anglediff > 0.5) {
          /* neighbor has larger strength = uphill */
          if (heightdiff > 0) {
            reg_link->boundary_acc += reg_tree->boundary_acc;
          }
          /* neighbor has smaller strength = downhill */
          else {
            reg_link->boundary_acc -= reg_tree->boundary_acc;
          }
        }
        /* neighbor in edge direction */
        else {
          reg_link->boundary_acc += 2 * reg_tree->boundary_acc * (1 - anglediff);
        }

        reg_link->round++;
      }
      links = links->next;
    }
  }
  if (reg_tree->segment_acc > 0) {
    integral_value mean, dev, x1, x2, x1min, x1max, x2min, x2max, I, U, overlap;

    mean = tree->stat.mean;
    dev = getmax(1, 1 * tree->stat.deviation);
    x1min = getmax(0, mean - dev);
    x1max = x1min;
    x2min = getmin(mean + dev, 255);
    x2max = x2min;

    links = tree->links.first.next;
    endlinks = &tree->links.last;
    while (links != endlinks) {
      head = *((quad_tree_link_head**)links->data);
      if (head->annotation.token != forest->token) {
        typed_pointer_destroy(&head->annotation);
        head->annotation.token = forest->token;
      }
      if (head->link->category != d_N6) {
        neighbor = head->tree;
        CHECK(ensure_reg_accumulator(&head->annotation, &reg_link));

        mean = neighbor->stat.mean;
        dev = getmax(1, 1 * neighbor->stat.deviation);

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
        reg_link->segment_acc += reg_tree->segment_acc * overlap;
        reg_link->round++;
      }
      links = links->next;
    }
  }

  FINALLY(prop_reg_accumulator);
  (void)collection;
  RETURN();
}

/******************************************************************************/

result acc_reg_accumulator
(
  quad_forest *forest,
  quad_tree *tree,
  list *collection
)
{
  TRY();
  list_item *links, *endlinks;
  reg_accumulator *reg, *link_reg;
  accumulated_reg *areg;
  quad_tree_link_head *head;

  CHECK_POINTER(tree);
  reg = has_reg_accumulator(&tree->context);
  CHECK_POINTER(reg);

  if (tree->annotation.token != forest->token) {
    typed_pointer_destroy(&tree->annotation);
    tree->annotation.token = forest->token;
  }
  CHECK(ensure_accumulated_reg(&tree->annotation, &areg));

  links = tree->links.first.next;
  endlinks = &tree->links.last;
  while (links != endlinks) {
    head = *((quad_tree_link_head**)links->data);
    if (head->link->category != d_N6) {
      CHECK(ensure_reg_accumulator(&head->other->annotation, &link_reg));
      if (link_reg->round > 0) {
        reg->boundary_acc += link_reg->boundary_acc;
        if (reg->boundary_acc < 0) {
          reg->boundary_acc = 0;
        }
        reg->segment_acc += link_reg->segment_acc;
        if (reg->segment_acc < 0) {
          reg->segment_acc = 0;
        }
      }
    }
    links = links->next;
  }

  areg->mdist_mean = reg->cost_min;
  areg->sdist_mean = reg->cost_max;
  areg->boundary_strength = reg->boundary_acc;
  areg->segment_strength = reg->segment_acc;
  areg->spread_strength = reg->cost_spread;

  FINALLY(acc_reg_accumulator);
  (void)forest;
  (void)collection;
  RETURN();
}

/******************************************************************************/

result prime_ridge_finder
(
  quad_forest *forest,
  quad_tree *tree,
  list *collection
)
{
  TRY();
  ridge_finder *rfind;

  CHECK_POINTER(tree);

  CHECK(ensure_ridge_finder(&tree->context, &rfind));
  if (rfind->round == 0) {

  }
  else {

  }

  FINALLY(prime_ridge_finder);
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
  quad_forest *forest
)
{
  TRY();
  list_item *trees, *endtrees, *links, *endlinks;
  typed_pointer *tptr;
  neighborhood_stat *nstat;
  quad_tree_link_head *head;
  quad_tree *tree, *neighbor;
  integral_value weight, count, wcount;
  integral_value mean, mean_sum1, mean_sum2, mean_wsum1;
  integral_value dev, dev_sum1, dev_sum2, dev_wsum1;
  integral_value mean_mean, mean_dev, max_mean_dev, dev_mean, dev_dev, max_dev_dev;
  integral_value overlap, min_overlap, max_overlap, scale;

  CHECK_POINTER(forest);

  min_overlap = 1;
  max_overlap = 0;
  max_mean_dev = 0;
  max_dev_dev = 0;
  trees = forest->trees.first.next;
  endtrees = &forest->trees.last;
  while (trees != endtrees) {
    tree = (quad_tree*)trees->data;
    CHECK(ensure_has(&tree->annotation, t_NSTAT, &tptr));
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

      nstat->strength = 0;
      nstat->strength_score = 0;
      nstat->ridge_score = 0;

      tptr->token = forest->token;
    }
    trees = trees->next;
  }

  /* scale overlap */
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

  /* calculate strength */
  /*weight = 0.5;*/
  trees = forest->trees.first.next;
  endtrees = &forest->trees.last;
  while (trees != endtrees) {
    tree = (quad_tree*)trees->data;
    nstat = has_neighborhood_stat(&tree->annotation);
    if (nstat != NULL) {
      mean_dev = nstat->mean_dev / max_mean_dev;
      dev_dev = nstat->dev_dev / max_dev_dev;
      nstat->strength = getmax(mean_dev, dev_dev);
      /*nstat->strength = (weight * mean_dev) + (weight * dev_dev);*/
    }
    trees = trees->next;
  }

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
              strength = fabs(nstat->ridge_score);
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
              strength = fabs(nstat->ridge_score) / max_strength;
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
    default:
      ERROR(BAD_PARAM);
  }

  FINALLY(quad_forest_visualize_neighborhood_stats);
  RETURN();
}

/******************************************************************************/

result quad_forest_calculate_accumulated_regs
(
  quad_forest *forest,
  uint32 rounds
)
{
  TRY();
  list_item *trees, *end;
  quad_tree *tree;
  accumulated_reg *areg;
  integral_value strength, min_bstrength, max_bstrength, min_sstrength, max_sstrength;
  integral_value spread, min_spread, max_spread;

  CHECK_POINTER(forest);

  CHECK(quad_forest_calculate_neighborhood_stats(forest));

  CHECK(run_context_operation(forest, &forest->trees, NULL,
                              prime_reg_accumulator,
                              prop_reg_accumulator,
                              acc_reg_accumulator,
                              rounds, FALSE));

  min_bstrength = 1000000000;
  max_bstrength = 0;
  min_sstrength = 1000000000;
  max_sstrength = 0;
  min_spread = 1000000000;
  max_spread = 0;
  trees = forest->trees.first.next;
  end = &forest->trees.last;
  while (trees != end) {
    tree = (quad_tree*)trees->data;
    if (tree->nw == NULL) {
      areg = has_accumulated_reg(&tree->annotation);
      if (areg != NULL) {
        strength = areg->boundary_strength;
        if (strength < min_bstrength) min_bstrength = strength;
        if (strength > max_bstrength) max_bstrength = strength;
        strength = areg->segment_strength;
        if (strength < min_sstrength) min_sstrength = strength;
        if (strength > max_sstrength) max_sstrength = strength;
        spread = areg->spread_strength;
        if (spread < min_spread) min_spread = spread;
        if (spread > max_spread) max_spread = spread;
      }
    }
    trees = trees->next;
  }
  /*
  PRINT2("minb %.3f maxb %.3f\n", min_bstrength, max_bstrength);
  PRINT2("mins %.3f maxs %.3f\n", min_sstrength, max_sstrength);
  PRINT2("mins %.3f maxs %.3f\n", min_spread, max_spread);
  */
  trees = forest->trees.first.next;
  end = &forest->trees.last;
  while (trees != end) {
    tree = (quad_tree*)trees->data;
    if (tree->nw == NULL) {
      areg = has_accumulated_reg(&tree->annotation);
      if (areg != NULL) {
        strength = areg->boundary_strength;
        strength = (strength - min_bstrength) / (max_bstrength - min_bstrength);
        areg->boundary_strength = strength;

        strength = areg->segment_strength;
        strength = (strength - min_sstrength) / (max_sstrength - min_sstrength);
        areg->segment_strength = strength;

        strength = areg->spread_strength;
        strength = (strength - min_spread) / (max_spread - min_spread);
        areg->spread_strength = 1 - strength;
      }
    }
    trees = trees->next;
  }

  FINALLY(quad_forest_calculate_accumulated_regs);
  RETURN();
}

/******************************************************************************/

result quad_forest_visualize_accumulated_regs
(
  quad_forest *forest,
  pixel_image *target
)
{
  TRY();
  list_item *trees, *end;
  quad_tree *tree;
  accumulated_reg *areg;
  uint32 x, y, width, height, stride, row_step;
  byte *target_data, *target_pos, color0, color1, color2;
  list lines;

  CHECK_POINTER(forest);
  CHECK_POINTER(target);

  width = target->width;
  height = target->height;
  stride = target->stride;
  target_data = (byte*)target->data;

  CHECK(pixel_image_clear(target));
  /*CHECK(list_create(&lines, 1000, sizeof(line), 1));*/

  trees = forest->trees.first.next;
  end = &forest->trees.last;
  while (trees != end) {
    tree = (quad_tree*)trees->data;
    if (tree->nw == NULL) {
      areg = has_accumulated_reg(&tree->annotation);
      if (areg != NULL) {
        color0 = (byte)(255 * areg->boundary_strength);
        color1 = (byte)(255 * areg->spread_strength);
        color2 = (byte)(255 * areg->segment_strength);
        /*CHECK(quad_tree_edge_response_to_line(tree, &lines));*/
        /*
        if (areg->boundary_strength > 0.05) {
          CHECK(quad_tree_edge_response_to_line(tree, &lines));
          color0 = (byte)(255 * areg->boundary_strength);
          color1 = color0;
          color2 = color0;
        }
        else {
          color0 = 0;
          color1 = 128;
          color2 = (byte)(255 * areg->segment_strength);
        }
        */
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
  /*CHECK(pixel_image_draw_lines(target, &lines));*/

  FINALLY(quad_forest_visualize_accumulated_regs);
  /*list_destroy(&lines);*/
  RETURN();
}

/******************************************************************************/

result quad_forest_calculate_accumulated_bounds
(
  quad_forest *forest,
  uint32 rounds
)
{
  TRY();
  list_item *trees, *endtrees;
  quad_tree *tree;
  accumulated_stat *astat;
  ridge_finder *rfind;
  integral_value t;

  CHECK_POINTER(forest);

  CHECK(quad_forest_calculate_accumulated_regs(forest, rounds));

  trees = forest->trees.first.next;
  endtrees = &forest->trees.last;
  while (trees != endtrees) {
    tree = (quad_tree*)trees->data;
    if (tree->nw == NULL) {
      astat = has_accumulated_stat(&tree->annotation);
      if (astat != NULL) {
        t = astat->meandev - astat->devdev;
        t = getmax(3, t);
        if (tree->stat.deviation > t) {
          CHECK(ensure_ridge_finder(&tree->context, &rfind));
          if (rfind->round == 0) {
            list_item *links, *endlinks;
            accumulated_stat *astat2;
            integral_value angle1, angle2, anglediff;
            uint32 total, smaller;
            quad_tree_link_head *head;

            CHECK(quad_tree_get_edge_response(forest, tree, NULL, NULL));

            /* TODO: angle1 = tree->edge.ang;*/
            if (angle1 > M_PI) angle1 -= M_PI;
            total = 0;
            smaller = 0;
            links = tree->links.first.next;
            endlinks = &tree->links.last;
            while (links != endlinks) {
              head = *((quad_tree_link_head**)links->data);
              astat2 = has_accumulated_stat(&head->other->tree->annotation);
              if (astat2 != NULL) {
                angle2 = head->angle;
                if (angle2 > M_PI) angle2 -= M_PI;
                anglediff = fabs(angle1 - angle2);
                if (anglediff > (M_PI / 2)) anglediff = M_PI - anglediff;
                anglediff /= (M_PI / 2);
                if (anglediff > 0.5) {
                  total++;
                  if (astat->strength > astat2->strength) {
                    smaller++;
                  }
                }
              }
              links = links->next;
            }
            rfind->round = 1;
            PRINT2("(%lu,%lu)",total,smaller);
            if (total-smaller > 1) {
              rfind->has_ridge = FALSE;
            }
            else {
              rfind->has_ridge = TRUE;
              /*CHECK(quad_tree_edge_response_to_line(tree, lines));*/
            }
          }
        }
      }
    }
    trees = trees->next;
  }

  FINALLY(quad_forest_calculate_accumulated_bounds);
  RETURN();
}

/******************************************************************************/

result quad_forest_visualize_accumulated_bounds
(
  quad_forest *forest,
  pixel_image *target
)
{
  TRY();

  CHECK_POINTER(forest);
  CHECK_POINTER(target);

  FINALLY(quad_forest_visualize_accumulated_bounds);
  RETURN();
}

/******************************************************************************/

result quad_forest_parse
(
  quad_forest *forest,
  uint32 rounds
)
{
  TRY();
  quad_tree *tree1, *tree2;
  quad_tree_link_head *head1, *head2;
  quad_tree_link *link1, *link2;
  neighborhood_stat *nstat1, *nstat2;
  edge_response *eresp1, *eresp2;
  ridge_potential *ridge_tree, *ridge_link1, *ridge_link2;
  boundary_potential *boundary_tree, *boundary_link1, *boundary_link2;
  segment_potential *segment_tree, *segment_link1, *segment_link2;
  boundary_link_category *category_link1, *category_link2;
  typed_pointer *tptr;
  list treelist;
  list_item *trees, *endtrees, *links, *endlinks;
  integral_value strength, newstrength, minstrength, maxstrength, count;
  integral_value angle1, angle2, anglediff, anglescore, strengthdiff;
  uint32 i, min_rank, max_extent, new_ridge, new_segment, new_boundary;
  integral_value max_ridge_score;

  CHECK_POINTER(forest);
  CHECK_PARAM(rounds > 0);

  CHECK(list_create(&treelist, 1000, sizeof(quad_tree*), 1));

  CHECK(quad_forest_calculate_neighborhood_stats(forest));
  
  /* init loop. */
  /* -find frontier regions between low-overlap and high-overlap regions */
  /* -add ridge potentials and segment potentials */
  /* -add segment link between the nodes with rank 0 */
  trees = forest->trees.first.next;
  endtrees = &forest->trees.last;
  while (trees != endtrees) {
    tree1 = (quad_tree*)trees->data;
    CHECK(expect_neighborhood_stat(&nstat1, &tree1->annotation));
    if (nstat1->overlap < 0.25) {
      links = tree1->links.first.next;
      endlinks = &tree1->links.last;
      while (links != endlinks) {
        head1 = *((quad_tree_link_head**)links->data);
        tree2 = head1->other->tree;
        CHECK(expect_neighborhood_stat(&nstat2, &tree2->annotation));
        strength = nstat1->strength - nstat2->strength;
        if (nstat2->overlap > 0.25 && strength > 0.001) {
          CHECK(quad_tree_ensure_edge_response(forest, tree1, &eresp1));
          head2 = head1->other;
          angle1 = head2->angle;
          if (angle1 > M_PI) angle1 -= M_PI;
          angle2 = eresp1->ang;
          if (angle2 > M_PI) angle2 -= M_PI;
          anglediff = fabs(angle2 - angle1);
          if (anglediff > (M_PI / 2)) anglediff = M_PI - anglediff;
          anglediff /= (M_PI / 2);

          if (anglediff > 0.5) {
            CHECK(ensure_has(&head1->annotation, t_ridge_potential, &tptr));
            ridge_link1 = (ridge_potential*)tptr->value;
            if (tptr->token != forest->token) {
              tptr->token = forest->token;
              ridge_link1->ridge_score = strength;
            }
            else {
              if (ridge_link1->ridge_score < strength) {
                ridge_link1->ridge_score = strength;
              }
            }
            
            CHECK(ensure_has(&head2->annotation, t_segment_potential, &tptr));
            segment_link2 = (segment_potential*)tptr->value;
            if (tptr->token != forest->token) {
              tptr->token = forest->token;
              segment_link2->rank = 1;
              segment_link2->extent = 1;
              segment_link2->diff_score = 0;
              segment_link2->overlap_score = 0;
            }

            CHECK(list_append(&treelist, &tree1));
            CHECK(list_append(&treelist, &tree2));
          }
        }
        links = links->next;
      }
    }
    trees = trees->next;
  }
  
  /* main propagation loop. */
  /* ridges propagate uphill, updating the segment links as they go */
  /* segments evaluate their similarity with neighbors and propagate to opposite direction */
  /* boundaries evaluate edge directions with neighbors and propagate length and straightness */
  for (i = 0; i < rounds; i++) {
    PRINT1("round %d\n", i);
    trees = forest->trees.first.next;
    endtrees = &forest->trees.last;
    while (trees != endtrees) {
      tree1 = ((quad_tree*)trees->data);
      CHECK(expect_neighborhood_stat(&nstat1, &tree1->annotation));
      
      ridge_tree = has_ridge_potential(&tree1->annotation, forest->token);
      segment_tree = has_segment_potential(&tree1->annotation, forest->token);
      boundary_tree = has_boundary_potential(&tree1->annotation, forest->token);
      
      /* check annotations of each link. */
      new_ridge = 0;
      new_segment = 0;
      new_boundary = 0;
      max_ridge_score = -1000;
      min_rank = 1000;
      max_extent = 0;
      links = tree1->links.first.next;
      endlinks = &tree1->links.last;
      while (links != endlinks) {
        head1 = *((quad_tree_link_head**)links->data);
        link1 = head1->link;
        
        /* handle ridge annotations in link heads */
        ridge_link1 = has_ridge_potential(&head1->annotation, forest->token);
        if (ridge_link1 != NULL) {
          if (head1->opposite != NULL) {
            tree2 = head1->opposite->other->tree;
            CHECK(expect_neighborhood_stat(&nstat2, &tree2->annotation));
            strength = nstat1->strength - nstat2->strength;
            /* if next tree is not uphill, add boundary potential to this tree */
            if (strength > 0) {
              new_boundary = 1;
            }
            else {
              head2 = head1->opposite->other;
              ridge_link2 = has_ridge_potential(&head2->annotation, forest->token);
              if (ridge_link2 != NULL) {
                new_boundary = 1;
              }
              else {
                newstrength = ridge_link1->ridge_score + fabs(strength);
                CHECK(ensure_has(&head2->annotation, t_ridge_potential, &tptr));
                ridge_link2 = (ridge_potential*)tptr->value;
                if (tptr->token != forest->token) {
                  tptr->token = forest->token;
                }
                ridge_link2->ridge_score = newstrength;
                ridge_link1->ridge_score = strength;
                new_ridge = 1;
                if (max_ridge_score < strength) max_ridge_score = strength;
                
                head2 = head1->opposite;
                CHECK(ensure_has(&head2->annotation, t_segment_potential, &tptr));
                segment_link2 = (segment_potential*)tptr->value;
                if (tptr->token != forest->token) {
                  tptr->token = forest->token;
                }
                segment_link2->rank = 1;
              }
            }
          }
          else {
            new_boundary = 1;
          }
        }
        
        /* handle segment annotations in link heads */
        segment_link1 = has_segment_potential(&head1->annotation, forest->token);
        if (segment_link1 != NULL) {
          if (boundary_tree != NULL) {
            segment_link1->rank = 0;
            if (segment_tree == NULL) {
              new_segment = 1;
            }
          }
          else {
            if (segment_tree != NULL) {
              if (segment_link1->rank > segment_tree->rank) {
                segment_link1->rank = segment_tree->rank;
              }
              if (segment_link1->extent < segment_tree->extent) {
                segment_link1->extent = segment_tree->extent;
              }
            }
            else {
              new_segment = 1;
            }
            
            if (head1->opposite != NULL) {
              head2 = head1->opposite->other;
              CHECK(ensure_has(&head2->annotation, t_segment_potential, &tptr));
              segment_link2 = (segment_potential*)tptr->value;
              if (tptr->token != forest->token) {
                tptr->token = forest->token;
                segment_link2->rank = segment_link1->rank + 1;
                segment_link2->extent = segment_link1->extent + 1;
              }
              else {
                if (segment_link1->rank < segment_link2->rank) {
                  segment_link2->rank = segment_link1->rank + 1;
                }
                if (segment_link2->extent < segment_link1->extent) {
                  segment_link2->extent = segment_link1->extent + 1;
                }
              }
              if (min_rank > segment_link1->rank) {
                min_rank = segment_link1->rank;
              }
              if (max_extent < segment_link1->extent) {
                max_extent = segment_link1->extent;
              }
            }
            else {
              segment_link1->rank = 1;
            }
          }
        }
        
        /* handle boundary annotations in link heads */
        boundary_link1 = has_boundary_potential(&head1->annotation, forest->token);
        if (boundary_link1 != NULL) {
          
        }
        links = links->next;
      }
      
      if (new_ridge != 0) {
        CHECK(ensure_has(&tree1->annotation, t_ridge_potential, &tptr));
        ridge_tree = (ridge_potential*)tptr->value;
        if (tptr->token != forest->token) {
          tptr->token = forest->token;
          ridge_tree->ridge_score = max_ridge_score;
        }
        else {
          if (ridge_tree->ridge_score < max_ridge_score) {
            ridge_tree->ridge_score = max_ridge_score;
          }
        }
      }
      
      if (new_boundary != 0) {
        CHECK(ensure_has(&tree1->annotation, t_boundary_potential, &tptr));
        boundary_tree = (boundary_potential*)tptr->value;
        if (tptr->token != forest->token) {
          tptr->token = forest->token;
          boundary_tree->length = 0;
          boundary_tree->angle_score = 0;
          boundary_tree->strength_score = 0;
          boundary_tree->straightness_score = 0;
        }
      }
      
      if (new_segment != 0) {
        CHECK(ensure_has(&tree1->annotation, t_segment_potential, &tptr));
        segment_tree = (segment_potential*)tptr->value;
        if (tptr->token != forest->token) {
          tptr->token = forest->token;
          segment_tree->rank = min_rank;
          segment_tree->extent = max_extent;
        }
        else {
          if (segment_tree->rank > min_rank) segment_tree->rank = min_rank;
          if (segment_tree->extent < max_extent) segment_tree->extent = max_extent;
        }
      }
      trees = trees->next;
    }
  }
  PRINT0("finished\n");
  FINALLY(quad_forest_parse);
  list_destroy(&treelist);
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
  list links;
  list_item *trees, *end;
  quad_tree *tree;
  ridge_potential *ridge1;
  boundary_potential *boundary1;
  segment_potential *segment1;
  uint32 x, y, width, height, stride, row_step;
  byte *target_data, *target_pos, color0, color1, color2;

  CHECK_POINTER(forest);
  CHECK_POINTER(target);

  width = target->width;
  height = target->height;
  stride = target->stride;
  target_data = (byte*)target->data;

  CHECK(pixel_image_clear(target));
  CHECK(list_create(&links, 1000, sizeof(weighted_line), 1));

  trees = forest->trees.first.next;
  end = &forest->trees.last;
  while (trees != end) {
    tree = (quad_tree*)trees->data;
    if (tree->nw == NULL) {
      boundary1 = has_boundary_potential(&tree->annotation, forest->token);
      if (boundary1 != NULL) {
        color0 = (byte)(255 * 1);
      }
      else {
        color0 = (byte)(255 * 0);
      }
      ridge1 = has_ridge_potential(&tree->annotation, forest->token);
      if (ridge1 != NULL) {
        color1 = (byte)(255 * 1);
      }
      else {
        color1 = (byte)(255 * 0);
      }
      segment1 = has_segment_potential(&tree->annotation, forest->token);
      if (segment1 != NULL) {
        color2 = (byte)(255 * 1);
      }
      else {
        color2 = (byte)(255 * 0);
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

  {
    byte edge_color[4] = {255,255,0,0};
    byte segment_color[4] = {0,255,255,0};

    /*CHECK(quad_forest_get_links(forest, &links, v_LINK_NONE));*/
    /*
    CHECK(quad_forest_get_links(forest, &links, v_LINK_SIMILARITY));
    CHECK(pixel_image_draw_weighted_lines(target, &links, segment_color));
    CHECK(list_clear(&links));
    CHECK(quad_forest_get_links(forest, &links, v_LINK_ANGLE_COST));
    CHECK(pixel_image_draw_weighted_lines(target, &links, edge_color));
    */
    trees = forest->trees.first.next;
    end = &forest->trees.last;
    while (trees != end) {
      tree = (quad_tree*)trees->data;
      CHECK(quad_tree_gradient_to_line(tree, &links));
      /*CHECK(quad_tree_edge_response_to_line(tree, &links));*/
      trees = trees->next;
    }
    CHECK(pixel_image_draw_weighted_lines(target, &links, segment_color));
  }

  FINALLY(quad_forest_visualize_parse_result);
  list_destroy(&links);
  RETURN();
}

/* end of file                                                                */
/******************************************************************************/
