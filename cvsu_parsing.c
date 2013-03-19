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
string quad_forest_calculate_accumulated_regs_name = "quad_forest_calculate_accumulated_regs";
string quad_forest_visualize_accumulated_regs_name = "quad_forest_visualize_accumulated_regs";
string quad_forest_calculate_accumulated_bounds_name = "quad_forest_calculate_accumulated_bounds";
string quad_forest_visualize_accumulated_bounds_name = "quad_forest_visualize_accumulated_bounds";
string quad_forest_parse_name = "quad_forest_parse";
string quad_forest_visualize_parse_result_name = "quad_forest_visualize_parse_result";

string init_edge_parsers_name = "init_edge_parsers";
string pool_edge_parsers_name = "pool_edge_parsers";
string acc_edge_parsers_name = "acc_edge_parsers";

string quad_forest_find_edges_name = "quad_forest_find_edges";
string quad_forest_find_boundaries_name = "quad_forest_find_boundaries";
string quad_forest_find_boundaries_with_hysteresis_name = "quad_forest_find_boundaries_with_hysteresis";
string quad_forest_prune_boundaries_name = "quad_forest_prune_boundaries";

/******************************************************************************/

result prime_stat_accumulator
(
  quad_tree *tree,
  list *collection
)
{
  TRY();
  stat_accumulator *acc;

  CHECK_POINTER(tree);

  CHECK(context_ensure_stat_accumulator(&tree->context, &acc));
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
  (void)collection;
  RETURN();
}

/******************************************************************************/

#define NEIGHBOR_PROP_STAT(neighbor)\
if ((neighbor) != NULL) {\
    neighbor_acc = has_stat_accumulator(&(neighbor)->context.data);\
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
  quad_tree *tree,
  list *collection
)
{
  TRY();
  stat_accumulator *tree_acc, *neighbor_acc;
  integral_value mean_pool1, mean_pool2, dev_pool1, dev_pool2;

  CHECK_POINTER(tree);
  tree_acc = has_stat_accumulator(&tree->context.data);
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
  (void)collection;
  RETURN();
}

/******************************************************************************/

result acc_stat_accumulator
(
  quad_tree *tree,
  list *collection
)
{
  TRY();
  stat_accumulator *acc;
  accumulated_stat *astat;
  integral_value mean, dev;

  CHECK_POINTER(tree);
  acc = has_stat_accumulator(&tree->context.data);
  CHECK_POINTER(acc);
  CHECK(annotation_ensure_accumulated_stat(&tree->annotation, &astat));

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
  (void)collection;
  RETURN();
}

/******************************************************************************/

result prime_reg_accumulator
(
  quad_tree *tree,
  list *collection
)
{
  TRY();
  list_item *links, *endlinks;
  reg_accumulator *reg;
  accumulated_stat *astat;
  quad_tree_link_head *head;
  quad_tree *neighbor;

  CHECK_POINTER(tree);

  CHECK(context_ensure_reg_accumulator(&tree->context, &reg));
  if (reg->round == 0) {
    CHECK(expect_accumulated_stat(&astat, &tree->annotation.data));
    {
      integral_value mean, meana, meanb, deva, devb, a1, a2, b1, b2, I, U;

      /* form the extremal ranges from the ranges of mean and dev */
      mean = astat->meanmean;
      meana = getmax(0, mean - astat->meandev);
      meanb = getmin(mean + astat->meandev, 255);
      deva = getmax(1, astat->devmean - astat->devdev);
      devb = getmax(1, astat->devmean + astat->devdev);
      
      a1 = getmax(0, meana - devb);
      a2 = getmin(meanb + devb, 255);
      b1 = getmax(0, mean - deva);
      b2 = getmin(mean + deva, 255);
      
      I = b2 - b1;
      if (I < 1) I = 1;
      U = a2 - a1;
      if (U < 1) U = 1;
      reg->locality_overlap = I / U;
    }
    {
      integral_value tm, ts, nm, ns, x1, x2, x1min, x1max, x2min, x2max, I, U, sum, count;
      range_overlap *link_range;
      
      tm = tree->stat.mean;
      ts = getmax(1, tree->stat.deviation);
      
      count = 0;
      sum = 0;
      links = tree->links.first.next;
      endlinks = &tree->links.last;
      while (links != endlinks) {
        head = *((quad_tree_link_head**)links->data);
        if (head->link->category == d_N4) {
          CHECK(context_ensure_range_overlap(&head->link->context, &link_range));
          if (link_range->round == 0) {
            neighbor = head->other->tree;
            nm = neighbor->stat.mean;
            ns = getmax(1, neighbor->stat.deviation);
            x1min = getmax(0, tm - ts);
            x1max = x1min;
            x2min = getmin(tm + ts, 255);
            x2max = x2min;
            x1 = getmax(0, nm - ns);
            x2 = getmin(nm + ns, 255);
            if (x1 < x1min) x1min = x1; else x1max = x1;
            if (x2 < x2min) x2min = x2; else x2max = x2;
            if (x1max > x2min) {
              I = 0;
            }
            else {
              I = (x2min - x1max);
              if (I < 1) I = 1;
            }
            U = (x2max - x1min);
            if (U < 1) U = 1;
            
            link_range->overlap = I / U;
            link_range->round = 1;
          }
          sum += link_range->overlap;
          count++;
        }
        links = links->next;
      }
      reg->neighborhood_overlap = sum / count;
    }
    reg->locality_acc = reg->locality_overlap / 2;
    reg->locality_pool = reg->locality_acc;
    reg->neighborhood_acc = reg->neighborhood_overlap / 2;
    reg->neighborhood_pool = reg->neighborhood_acc;
    reg->round = 1;
  }
  else {
    reg->locality_acc = reg->locality_pool / 2;
    reg->locality_pool = reg->locality_acc;
    reg->neighborhood_acc = reg->neighborhood_pool / 2;
    reg->neighborhood_pool = reg->neighborhood_acc;
    reg->round++;
  }

  FINALLY(prime_stat_accumulator);
  (void)collection;
  RETURN();
}

/******************************************************************************/

#define NEIGHBOR_PROP_REG(neighbor)\
  if ((neighbor) != NULL) {\
    neighbor_acc = has_reg_accumulator(&(neighbor)->context.data);\
    CHECK_POINTER(neighbor_acc);\
    neighbor_acc->locality_pool += locality_pool;\
    neighbor_acc->neighborhood_pool += neighborhood_pool;\
  }\
  else {\
    tree_acc->locality_pool += locality_pool;\
    tree_acc->neighborhood_pool += neighborhood_pool;\
  }

result prop_reg_accumulator
(
  quad_tree *tree,
  list *collection
)
{
  TRY();
  reg_accumulator *tree_acc, *neighbor_acc;
  integral_value locality_pool, neighborhood_pool;

  CHECK_POINTER(tree);
  tree_acc = has_reg_accumulator(&tree->context.data);
  CHECK_POINTER(tree_acc);

  locality_pool = tree_acc->locality_acc / 4;
  neighborhood_pool = tree_acc->neighborhood_acc / 4;

  /* TODO: use neighbor links and link categorization instead */
  /* neighbor n */
  NEIGHBOR_PROP_REG(tree->n);
  NEIGHBOR_PROP_REG(tree->e);
  NEIGHBOR_PROP_REG(tree->s);
  NEIGHBOR_PROP_REG(tree->w);

  FINALLY(prop_reg_accumulator);
  (void)collection;
  RETURN();
}

/******************************************************************************/

result acc_reg_accumulator
(
  quad_tree *tree,
  list *collection
)
{
  TRY();
  reg_accumulator *acc;
  accumulated_reg *areg;

  CHECK_POINTER(tree);
  acc = has_reg_accumulator(&tree->context.data);
  CHECK_POINTER(acc);
  CHECK(annotation_ensure_accumulated_reg(&tree->annotation, &areg));

  areg->locality_overlap = acc->locality_overlap;
  areg->locality_mean = acc->locality_pool;
  areg->neighborhood_overlap = acc->neighborhood_overlap;
  areg->neighborhood_mean = acc->neighborhood_pool;
  
  areg->locality_strength = 0;
  areg->neighborhood_strength = 0;

  FINALLY(acc_reg_accumulator);
  (void)collection;
  RETURN();
}

/******************************************************************************/

result prime_ridge_finder
(
  quad_tree *tree,
  list *collection
)
{
  TRY();
  ridge_finder *rfind;

  CHECK_POINTER(tree);

  CHECK(context_ensure_ridge_finder(&tree->context, &rfind));
  if (rfind->round == 0) {
    
  }
  else {
    
  }

  FINALLY(prime_ridge_finder);
  (void)collection;
  RETURN();
}

/******************************************************************************/

result run_context_operation
(
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
        CHECK(prime_operation((quad_tree *)trees->data, NULL));
        trees = trees->next;
      }
      trees = input_trees->first.next;
      end = &input_trees->last;
      while (trees != end) {
        CHECK(propagate_operation((quad_tree *)trees->data, NULL));
        trees = trees->next;
      }
    }
    trees = input_trees->first.next;
    end = &input_trees->last;
    while (trees != end) {
      CHECK(accumulate_operation((quad_tree *)trees->data, NULL));
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

  CHECK(run_context_operation(&forest->trees, NULL, prime_stat_accumulator,
                              prop_stat_accumulator, acc_stat_accumulator,
                              rounds, FALSE));
  
  maxmeandev = 0;
  maxdevdev = 0;
  trees = forest->trees.first.next;
  end = &forest->trees.last;
  while (trees != end) {
    tree = (quad_tree*)trees->data;
    if (tree->nw == NULL) {
      astat = has_accumulated_stat(&tree->annotation.data);
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
      astat = has_accumulated_stat(&tree->annotation.data);
      if (astat != NULL) {
        meandev = astat->meandev / maxmeandev;
        devdev = astat->devdev / maxdevdev;
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
  uint32 x, y, width, height, stride, row_step, count;
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
      astat = has_accumulated_stat(&tree->annotation.data);
      if (astat != NULL) {
        color = (byte)(255 * astat->strength);
        color0 = (byte)astat->meanmean;
        color1 = 0;
        color2 = color;
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
  integral_value min_loverlap, max_noverlap;
  
  CHECK_POINTER(forest);
  
  CHECK(quad_forest_calculate_accumulated_stats(forest, rounds));

  CHECK(run_context_operation(&forest->trees, NULL, prime_reg_accumulator,
                              prop_reg_accumulator, acc_reg_accumulator,
                              rounds, FALSE));
  
  min_loverlap = 1;
  max_noverlap = 0;
  trees = forest->trees.first.next;
  end = &forest->trees.last;
  while (trees != end) {
    tree = (quad_tree*)trees->data;
    if (tree->nw == NULL) {
      areg = has_accumulated_reg(&tree->annotation.data);
      if (areg != NULL) {
        if (areg->locality_overlap < min_loverlap) {
          min_loverlap = areg->locality_overlap;
        }
        if (areg->neighborhood_mean > max_noverlap) {
          max_noverlap = areg->neighborhood_mean;
        }
      }
    }
    trees = trees->next;
  }
  
  min_loverlap = 1 - min_loverlap;
  max_noverlap = max_noverlap;
  trees = forest->trees.first.next;
  end = &forest->trees.last;
  while (trees != end) {
    tree = (quad_tree*)trees->data;
    if (tree->nw == NULL) {
      areg = has_accumulated_reg(&tree->annotation.data);
      if (areg != NULL) {
        areg->locality_strength = (1 - areg->locality_overlap) / min_loverlap;
        areg->neighborhood_strength = areg->neighborhood_mean / max_noverlap;
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
      areg = has_accumulated_reg(&tree->annotation.data);
      if (areg != NULL) {
        color0 = (byte)(255 * areg->locality_strength);
        color1 = 0;
        color2 = (byte)(255 * areg->neighborhood_strength);
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
  
  FINALLY(quad_forest_visualize_accumulated_regs);
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
      astat = has_accumulated_stat(&tree->annotation.data);
      if (astat != NULL) {
        t = astat->meandev - astat->devdev;
        t = getmax(3, t);
        if (tree->stat.deviation > t) {
          CHECK(context_ensure_ridge_finder(&tree->context, &rfind));
          if (rfind->round == 0) {
            list_item *links, *endlinks;
            accumulated_stat *astat2;
            integral_value angle1, angle2, anglediff;
            uint32 total, smaller;
            quad_tree_link_head *head;
            
            CHECK(quad_tree_get_edge_response(forest, tree, NULL, NULL));
            
            angle1 = tree->edge.ang;
            if (angle1 > M_PI) angle1 -= M_PI;
            total = 0;
            smaller = 0;
            links = tree->links.first.next;
            endlinks = &tree->links.last;
            while (links != endlinks) {
              head = *((quad_tree_link_head**)links->data);
              astat2 = has_accumulated_stat(&head->other->tree->annotation.data);
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

result quad_forest_find_edges
(
  quad_forest *forest,
  uint32 rounds,
  integral_value bias,
  direction dir
)
{
  TRY();
  uint32 remaining, i, size;
  integral_value mean, dev, value;
  quad_tree *tree;

  CHECK_POINTER(forest);
  CHECK_PARAM(rounds > 0);
  CHECK_PARAM(dir == d_H || dir == d_V || dir == d_N4);

  size = forest->rows * forest->cols;

  /* first get the edge responses */
  /* TODO: add forest state to determine if this has been done already */
  for (i = 0; i < size; i++) {
    CHECK(quad_tree_get_edge_response(forest, forest->roots[i], NULL, NULL));
  }

  /* before propagation, prime all trees */
  /* for finding horizontal edges, prime with dy */
  if (dir == d_H) {
    for (i = 0; i < size; i++) {
      quad_tree_prime_with_dy(forest->roots[i]);
    }
  }
  else
  /* for finding vertical edges, prime with dx */
  if (dir == d_V) {
    for (i = 0; i < size; i++) {
      quad_tree_prime_with_dx(forest->roots[i]);
    }
  }
  /* otherwise, prime with magnitude */
  else {
    for (i = 0; i < size; i++) {
      quad_tree_prime_with_mag(forest->roots[i]);
    }
  }

  /* then, propagate the requested number of rounds */
  for (remaining = rounds; remaining--;) {
    for (i = 0; i < size; i++) {
      quad_tree_propagate(forest->roots[i]);
    }
    /* on other rounds except the last, prime for the new run */
    if (remaining > 0) {
      for (i = 0; i < size; i++) {
        quad_tree_prime_with_pool(forest->roots[i]);
      }
    }
  }

  for (i = 0; i < size; i++) {
    tree = forest->roots[i];
    mean = tree->pool;
    dev = tree->pool2;
    dev -= mean*mean;
    if (dev < 0) dev = 0; else dev = sqrt(dev);
    tree->edge.mean = mean;
    tree->edge.deviation = dev;
    tree->edge.dir = dir;

    if (dir == d_H) {
      value = tree->edge.dy;
    }
    else
    if (dir == d_V) {
      value = tree->edge.dx;
    }
    else {
      value = tree->edge.mag;
    }

    if (value > getmax(mean, mean + bias - dev)) {
      /*printf("value %.3f mean %.3f dev %.3f\n", tree->mag, mean, dev);*/
      tree->edge.has_edge = TRUE;
    }
    else {
      tree->edge.has_edge = FALSE;
    }
  }

  FINALLY(quad_forest_find_edges);
  RETURN();
}

/******************************************************************************/

#define GET_NEIGHBOR_N()\
  neighbor = tree->n

#define GET_NEIGHBOR_NE()\
  neighbor = tree->n != NULL ? tree->n->e : NULL

#define GET_NEIGHBOR_E()\
  neighbor = tree->e

#define GET_NEIGHBOR_SE()\
  neighbor = tree->s != NULL ? tree->s->e : NULL

#define GET_NEIGHBOR_S()\
  neighbor = tree->s

#define GET_NEIGHBOR_SW()\
  neighbor = tree->s != NULL ? tree->s->w : NULL

#define GET_NEIGHBOR_W()\
  neighbor = tree->w

#define GET_NEIGHBOR_NW()\
  neighbor = tree->n != NULL ? tree->n->w : NULL

#define CHECK_NEIGHBOR(tree, value, has_next, max)\
  if (neighbor != NULL) {\
    if (IS_TRUE(neighbor->segment.has_boundary)) {\
      if ((signum(tree->edge.dx) == signum(neighbor->edge.dx)) && \
          (signum(tree->edge.dy) == signum(neighbor->edge.dy))) {\
        has_next = TRUE;\
        if (max == NULL) {\
          max = neighbor;\
        }\
        else if (neighbor->edge.strength > max->edge.strength) {\
          max = neighbor;\
        }\
      }\
    }\
  }

#define CHECK_NEIGHBOR_HYSTERESIS(tree, value, has_next, max)\
  if (neighbor != NULL) {\
    if (IS_TRUE(neighbor->segment.has_boundary)) {\
      has_next = TRUE;\
    }\
    else\
    if (neighbor->stat.deviation > value) {\
      if (max == NULL) {\
        max = neighbor;\
      }\
      else if (neighbor->stat.deviation > max->stat.deviation) {\
        max = neighbor;\
      }\
    }\
  }

#define CHECK_SNIFFER_NEIGHBOR() \
  if (neighbor != NULL) {\
    if (neighbor->context.token == token) {\
      CHECK(expect_path_sniffer(&neighbor_sniffer, &neighbor->context.data));\
      if (sniffer->endpoint == neighbor_sniffer->endpoint) {\
        PRINT0("update cost...");\
        cost = sniffer->cost + fabs(sniffer->strength - neighbor_sniffer->strength);\
        if (neighbor_sniffer->cost > cost) {\
          neighbor_sniffer->cost = cost;\
          neighbor_sniffer->prev = sniffer;\
          neighbor_sniffer->length = sniffer->length + 1;\
        }\
        PRINT0("done\n");\
      }\
      else {\
        PRINT0("new connection...");\
        new_connection.endpoint1 = sniffer->endpoint;\
        new_connection.endpoint2 = neighbor_sniffer->endpoint;\
        new_connection.sniffer1 = sniffer;\
        new_connection.sniffer2 = neighbor_sniffer;\
        new_connection.cost = 1000000000;\
        CHECK(list_append_unique_return_pointer(&connection_list,\
                                                (pointer)&new_connection,\
                                                (pointer*)&connection,\
                                                connection_equals_by_endpoints));\
        cost = sniffer->cost + neighbor_sniffer->cost + \
            fabs(sniffer->strength - neighbor_sniffer->strength);\
        if (connection->cost > cost) {\
          PRINT0("...updating...");\
          /*\
          connection->endpoint1 = sniffer->endpoint;\
          connection->endpoint2 = neighbor_sniffer->endpoint;\
          */\
          connection->sniffer1 = sniffer;\
          connection->sniffer2 = neighbor_sniffer;\
          connection->cost = cost;\
        }\
        PRINT0("done\n");\
      }\
    }\
    else {\
      if (neighbor->edge.chain != NULL) {\
        PRINT0("intersection\n");\
      }\
      else {\
        if (sniffer->length < 4) {\
          /*PRINT0("add next...");*/\
          neighbor->context.token = token;\
          CHECK(typed_pointer_create(&neighbor->context.data, t_PATH_SNIFFER, 1));\
          CHECK(expect_path_sniffer(&neighbor_sniffer, &neighbor->context.data));\
          neighbor_sniffer->prev = sniffer;\
          neighbor_sniffer->tree = neighbor;\
          neighbor_sniffer->chain = sniffer->chain;\
          neighbor_sniffer->endpoint = sniffer->endpoint;\
          neighbor_sniffer->strength = neighbor->segment.devdev;\
          neighbor_sniffer->cost = sniffer->cost + fabs(sniffer->strength - neighbor_sniffer->strength);\
          neighbor_sniffer->length = sniffer->length + 1;\
          neighbor_sniffer->dir_start = sniffer->dir_start;\
          neighbor_sniffer->dir_end = sniffer->dir_end;\
          CHECK(list_insert_sorted(&context_list, (pointer)&neighbor_sniffer,\
              path_sniffer_compare_by_cost));\
          /*PRINT0("done\n");*/\
        }\
      }\
    }\
  }

void edge_chain_create
(
  quad_forest_edge *edge
)
{
  if (edge != NULL && edge->parent == NULL) {
    edge->parent = edge;
    edge->prev = NULL;
    edge->next = NULL;
    edge->rank = 0;
    edge->length = 1;
  }
}

quad_forest_edge *edge_chain_find
(
  quad_forest_edge *edge
)
{
  if (edge != NULL) {
    if (edge->parent != NULL && edge->parent != edge) {
      edge->parent = edge_chain_find(edge->parent);
    }
    return edge->parent;
  }
  return NULL;
}

void edge_chain_union
(
  quad_forest_edge *edge1,
  quad_forest_edge *edge2
)
{
  quad_forest_edge *parent1, *parent2;

  parent1 = edge_chain_find(edge1);
  parent2 = edge_chain_find(edge2);
  if (parent1 != NULL && parent2 != NULL && parent1 != parent2) {
    if (parent1->rank < parent2->rank) {
      parent1->parent = parent2;
      parent2->length += parent1->length;
    }
    else {
      parent2->parent = parent1;
      if (parent1->rank == parent2->rank) {
        parent1->rank += 1;
      }
      parent1->length += parent2->length;
    }
  }
}

quad_forest_edge *edge_chain_follow_forward
(
  quad_forest_edge *parent,
  quad_forest_edge *prev,
  quad_forest_edge *next
)
{
  if (next->next == prev) {
    next->next = next->prev;
    next->prev = prev;
  }
  if (next->next == parent) {
    return next;
  }
  if (next->prev == prev) {
    edge_chain_union(parent, next);
    if (next->next != NULL) {
      return edge_chain_follow_forward(parent, next, next->next);
    }
    else {
      return next;
    }
  }
  else {
    return prev;
  }
}

quad_forest_edge *edge_chain_follow_backward
(
  quad_forest_edge *parent,
  quad_forest_edge *next,
  quad_forest_edge *prev
)
{
  if (prev->prev == next) {
    prev->prev = prev->next;
    prev->next = next;
  }
  if (prev->prev == parent) {
    return prev;
  }
  if (prev->next == next) {
    edge_chain_union(parent, prev);
    if (prev->prev != NULL) {
      return edge_chain_follow_backward(parent, prev, prev->prev);
    }
    else {
      return prev;
    }
  }
  else {
    return next;
  }
}

void edge_set_chain
(
  quad_forest_edge *node,
  quad_forest_edge_chain *chain
)
{
  if (node != NULL) {
    node->chain = chain;
    node->token = chain->token;
    if (node->next != NULL) {
      chain->cost += fabs(node->strength - node->next->strength);
      if (node->next->token != chain->token) {
        edge_set_chain(node->next, chain);
      }
      else {
        chain->last = node;
        node->next->prev = NULL;
        node->next = NULL;
      }
    }
    else {
      chain->last = node;
    }
  }
}

void edge_chain_clear(quad_forest_edge *edge) {
  quad_forest_edge *next;
  if (edge != NULL) {
    next = edge->next;
    edge->prev = NULL;
    edge->next = NULL;
    edge->parent = NULL;
    edge->length = 1;
    edge_chain_clear(next);
  }
}

/* keeping these functions cleaner by just creating the edge chain */
/* need to determine elsewhere what has to be done with the endpoints */
void path_follow_extend_edge(path_sniffer *sniffer)
{
  quad_forest_edge *prev, *next, *end, *parent;
  if (sniffer->prev != NULL) {
    path_follow_extend_edge(sniffer->prev);
    end = sniffer->prev->endpoint;
    if (end->next == NULL && end->prev != NULL) {
      prev = end;
      next = &sniffer->tree->edge;
      next->parent = NULL;
      edge_chain_create(next);
      next->tree = sniffer->tree;
      next->strength = next->tree->segment.devdev;
      next->chain = prev->chain;
      next->chain->last = next;
      prev->next = next;
      next->prev = prev;
      next->next = NULL;
      parent = edge_chain_find(prev);
      edge_chain_union(parent, next);
      end = next;
    }
    else
    if (end->prev == NULL && end->next != NULL) {
      next = end;
      prev = &sniffer->tree->edge;
      prev->parent = NULL;
      edge_chain_create(prev);
      prev->tree = sniffer->tree;
      prev->chain = next->chain;
      prev->chain->first = prev;
      next->prev = prev;
      prev->next = next;
      prev->prev = NULL;
      parent = edge_chain_find(next);
      edge_chain_union(parent, prev);
      end = prev;
    }
    else {
      PRINT0("not an endpoint in path_follow_extend_edge\n");
      return;
    }
    sniffer->endpoint = end;
  }
}

quad_forest_edge *find_first
(
  quad_forest_edge *edge,
  uint32 token
)
{
  if (edge != NULL) {
    edge->token = token;
    if (edge->prev != NULL) {
      if (edge->prev->token != token) {
        return find_first(edge->prev, token);
      }
      else {
        edge->prev->next = NULL;
        edge->prev = NULL;
        return edge;
      }
    }
  }
  return edge;
}

quad_forest_edge *find_last
(
  quad_forest_edge *edge,
  uint32 token
)
{
  if (edge != NULL) {
    edge->token = token;
    if (edge->next != NULL) {
      if (edge->next->token != token) {
        return find_last(edge->next, token);
      }
      else {
        edge->next->prev = NULL;
        edge->next = NULL;
        return edge;
      }
    }
  }
  return edge;
}

void path_merge_edge_chains(path_sniffer *sniffer1, path_sniffer *sniffer2)
{
  quad_forest_edge *prev, *next, *end, *parent;
  quad_forest_edge_chain *chain;
  end = sniffer1->endpoint;
  if (end->next == NULL && end->prev != NULL) {
    prev = end;
    next = sniffer2->endpoint;
    chain = prev->chain;
    if (chain->first == NULL && chain->last == NULL) {
      PRINT0("recreating removed chain\n");
      chain->token = (unsigned)rand();
      chain->first = find_first(prev, chain->token);
    }
    parent = edge_chain_find(prev);
    chain->parent = parent;
    prev->next = next;
    if (next->prev == NULL) {
      next->prev = prev;
    }
    else
    if (next->next == NULL) {
      next->next = prev;
    }
    else {
      PRINT0("not endpoint in path_merge_edge_chains\n");
    }
    PRINT0("follow a\n");
    end = edge_chain_follow_forward(parent, prev, next);
    chain->last = end;
    chain->length = parent->length;
    chain->cost = 0;
    PRINT0("set a\n");
    chain->token = (unsigned)rand();
    edge_set_chain(chain->first, chain);
  }
  else
  if (end->prev == NULL && end->next != NULL) {
    next = end;
    prev = sniffer2->endpoint;
    chain = next->chain;
    if (chain->first == NULL && chain->last == NULL) {
      PRINT0("recreating removed chain\n");
      chain->token = (unsigned)rand();
      chain->last = find_last(next, chain->token);
    }
    parent = edge_chain_find(next);
    chain->parent = parent;
    next->prev = prev;
    if (prev->next == NULL) {
      prev->next = next;
    }
    else
    if (prev->prev == NULL) {
      prev->prev = next;
    }
    else {
      PRINT0("not endpoint in path_merge_edge_chains\n");
    }
    PRINT0("follow b\n");
    end = edge_chain_follow_backward(parent, next, prev);
    chain->first = end;
    chain->length = parent->length;
    chain->cost = 0;
    PRINT0("set b\n");
    chain->token = (unsigned)rand();
    edge_set_chain(chain->first, chain);
  }
  else {
    PRINT0("not an endpoint in path_merge_edge_chains\n");
  }
}

int compare_edges_descending(const void *a, const void *b)
{
  const quad_forest_edge *sa, *sb;
  integral_value sta, stb;

  sa = *((const quad_forest_edge* const *)a);
  if (sa == NULL) {
    /* should never get here TODO: add some assert or similar.. */
    PRINT0("warning: null pointer in compare_edges_descending\n");
    return 1;
  }
  sb = *((const quad_forest_edge* const *)b);
  if (sb == NULL) {
    /* should never get here TODO: add some assert or similar.. */
    printf("warning: null pointer in compare_edges_descending\n");
    return 1;
  }

  sta = sa->strength;
  stb = sb->strength;
  /* opposite values than normally, as we wish to sort in descending order */
  if (sta > stb) return -1;
  else if (sta < stb) return 1;
  else return 0;
}

truth_value edge_chain_equals(const void *a, const void *b)
{
  const quad_forest_edge_chain *sa, *sb;

  sa = ((const quad_forest_edge_chain *)a);
  sb = ((const quad_forest_edge_chain *)b);
  if (sa == NULL || sb == NULL) return FALSE;
  if (sa == sb) return TRUE;
  return FALSE;
}

/*
 * comparison function used for inserting sniffer contexts into a list
 * sorted by cost, smallest first
 */
int path_sniffer_compare_by_cost(const void *a, const void *b)
{
  const path_sniffer *sa, *sb;
  integral_value ca, cb;

  sa = *((const path_sniffer * const *)a);
  if (sa == NULL) return 1;
  sb = *((const path_sniffer * const *)b);
  if (sb == NULL) return -1;

  ca = sa->cost;
  cb = sb->cost;

  if (ca < cb) return -1;
  if (ca > cb) return 1;
  return 0;
}

int path_sniffer_equals(const void *a, const void *b)
{
  const path_sniffer *sa, *sb;

  sa = *((const path_sniffer * const *)a);
  sb = *((const path_sniffer * const *)b);
  if (sa == NULL || sb == NULL) return FALSE;
  if (sa == sb) return TRUE;
  return FALSE;
}

void path_sniffer_determine_directions(path_sniffer *sniffer)
{
  quad_forest_edge *this, *other;

  this = sniffer->endpoint;
  if (this->next == NULL) {
    other = this->prev;
  }
  else
  if (this->prev == NULL) {
    other = this->next;
  }
  else {
    PRINT0("sniffer not an endpoint in determine directions\n");
    sniffer->dir_start = d_NULL;
    sniffer->dir_end = d_NULL;
    return;
  }
  if (this != NULL && other != NULL) {
    sint32 xdiff, ydiff;
    xdiff = (signed)(((quad_tree*)this->tree)->x - ((quad_tree*)other->tree)->x);
    ydiff = (signed)(((quad_tree*)this->tree)->y - ((quad_tree*)other->tree)->y);
    if (xdiff > 0) {
      if (ydiff > 0) {
        sniffer->dir_start = d_E;
        sniffer->dir_end = d_S;
      }
      else
      if (ydiff == 0) {
        sniffer->dir_start = d_NE;
        sniffer->dir_end = d_SE;
      }
      else { /* ydiff < 0 */
        sniffer->dir_start = d_N;
        sniffer->dir_end = d_E;
      }
    }
    else
    if (xdiff == 0) {
      if (ydiff > 0) {
        sniffer->dir_start = d_SE;
        sniffer->dir_end = d_SW;
      }
      else { /* ydiff < 0, not possible that also ydiff == 0 */
        sniffer->dir_start = d_NW;
        sniffer->dir_end = d_NE;
      }
    }
    else { /* xdiff < 0 */
      if (ydiff > 0) {
        sniffer->dir_start = d_S;
        sniffer->dir_end = d_W;
      }
      else
      if (ydiff == 0) {
        sniffer->dir_start = d_SW;
        sniffer->dir_end = d_NW;
      }
      else { /* ydiff < 0 */
        sniffer->dir_start = d_W;
        sniffer->dir_end = d_N;
      }
    }
  }
  else {
    sniffer->dir_start = d_NULL;
    sniffer->dir_end = d_NULL;
  }
}

/* A private structure for storing the discovered edge chain connections */
typedef struct edge_connection_t {
  quad_forest_edge *endpoint1;
  quad_forest_edge *endpoint2;
  path_sniffer *sniffer1;
  path_sniffer *sniffer2;
  integral_value cost;
} edge_connection;

/* A connection is the same, if it connects the same two endpoints; after */
/* verifying the identity, need to check also the cost in the code using this */
truth_value connection_equals_by_endpoints(const void *a, const void *b)
{
  const edge_connection *sa, *sb;

  sa = ((const edge_connection*)a);
  sb = ((const edge_connection*)b);
  if (sa == NULL || sb == NULL) return FALSE;
  if ((sa->endpoint1 == sb->endpoint1 && sa->endpoint2 == sb->endpoint2) ||
      (sa->endpoint1 == sb->endpoint2 && sa->endpoint2 == sb->endpoint1)) {
    return TRUE;
  }
  return FALSE;
}

/* After all these supporting definitions, finally the function that uses them */
result quad_forest_find_boundaries
(
  quad_forest *forest,
  uint32 rounds,
  integral_value bias,
  uint32 min_length
)
{
  TRY();
  uint32 remaining, i, size;
  integral_value mean, dev, value, dx, dy;
  truth_value has_a, has_b;
  quad_tree *tree, *neighbor, *best_a, *best_b;
  list edge_list, context_list, connection_list;
  list_item *edges, *end;
  quad_forest_edge *edge, *parent;
  quad_forest_edge_chain new_chain, *chain;

  CHECK_POINTER(forest);
  CHECK_PARAM(rounds > 0);

  CHECK(list_create(&edge_list, 1000, sizeof(quad_forest_edge*), 1));

  size = forest->rows * forest->cols;

  /* before propagation, prime all trees */
  /* for finding boundaries, prime with deviation */
  for (i = 0; i < size; i++) {
    quad_tree_prime_with_dev(forest->roots[i]);
  }

  /* then, propagate the requested number of rounds */
  for (remaining = rounds; remaining--;) {
    for (i = 0; i < size; i++) {
      quad_tree_propagate(forest->roots[i]);
    }
    /* on other rounds except the last, prime for the new run */
    if (remaining > 0) {
      for (i = 0; i < size; i++) {
        quad_tree_prime_with_pool(forest->roots[i]);
      }
    }
  }

  PRINT0("calculate devmean and devdev\n");
  /* calculate devmean and devdev, and determine the boundary trees */
  for (i = 0; i < size; i++) {
    tree = forest->roots[i];
    mean = tree->pool;
    dev = tree->pool2;
    dev -= mean*mean;
    if (dev < 0) dev = 0; else dev = sqrt(dev);
    tree->segment.devmean = mean;
    tree->segment.devdev = dev;

    value = tree->stat.deviation;

    if (value > getmax(mean, mean + bias - dev)) {
      tree->segment.has_boundary = TRUE;
      /* at this point, get the edge responses for strong boundaries */
      CHECK(quad_tree_get_edge_response(forest, tree, NULL, NULL));
      edge = &tree->edge;
      edge_chain_create(edge);
      edge->tree = tree;
      /* using devdev as edge strength measure */
      edge->strength = dev;
      edge->is_intersection = FALSE;
      edge->chain = NULL;
      CHECK(list_insert_sorted(&edge_list, (pointer)&edge, &compare_edges_descending));
    }
    else {
      tree->segment.has_boundary = FALSE;
    }
  }

  PRINT0("check relevant neighbors\n");
  /* check relevant neighbors of boundary trees and create edge nodes */
  edges = edge_list.first.next;
  end = &edge_list.last;
  while (edges != end) {
    edge = *(quad_forest_edge**)edges->data;
    tree = (quad_tree*)edge->tree;
    dx = edge->dx;
    dy = edge->dy;
    has_a = FALSE;
    has_b = FALSE;
    best_a = NULL;
    best_b = NULL;
    /* decide which neighbors to check based on edge direction */
    if (fabs(dx) > fabs(dy)) {
      if (dy == 0) {
        GET_NEIGHBOR_N();
        CHECK_NEIGHBOR(tree, value, has_a, best_a);
        GET_NEIGHBOR_S();
        CHECK_NEIGHBOR(tree, value, has_b, best_b);
      }
      else
      if ((dx > 0 && dy > 0) || (dx < 0 && dy < 0)) {
        if (fabs(dx) < 2 * fabs(dy)) {
          GET_NEIGHBOR_N();
          CHECK_NEIGHBOR(tree, value, has_a, best_a);
          GET_NEIGHBOR_NE();
          CHECK_NEIGHBOR(tree, value, has_a, best_a);
          GET_NEIGHBOR_E();
          CHECK_NEIGHBOR(tree, value, has_a, best_a);
          GET_NEIGHBOR_S();
          CHECK_NEIGHBOR(tree, value, has_b, best_b);
          GET_NEIGHBOR_SW();
          CHECK_NEIGHBOR(tree, value, has_b, best_b);
          GET_NEIGHBOR_W();
          CHECK_NEIGHBOR(tree, value, has_b, best_b);
        }
        else {
          GET_NEIGHBOR_N();
          CHECK_NEIGHBOR(tree, value, has_a, best_a);
          GET_NEIGHBOR_NE();
          CHECK_NEIGHBOR(tree, value, has_a, best_a);
          GET_NEIGHBOR_S();
          CHECK_NEIGHBOR(tree, value, has_b, best_b);
          GET_NEIGHBOR_SW();
          CHECK_NEIGHBOR(tree, value, has_b, best_b);
        }
      }
      else {
        if (fabs(dx) < 2 * fabs(dy)) {
          GET_NEIGHBOR_W();
          CHECK_NEIGHBOR(tree, value, has_a, best_a);
          GET_NEIGHBOR_NW();
          CHECK_NEIGHBOR(tree, value, has_a, best_a);
          GET_NEIGHBOR_N();
          CHECK_NEIGHBOR(tree, value, has_a, best_a);
          GET_NEIGHBOR_E();
          CHECK_NEIGHBOR(tree, value, has_b, best_b);
          GET_NEIGHBOR_SE();
          CHECK_NEIGHBOR(tree, value, has_b, best_b);
          GET_NEIGHBOR_S();
          CHECK_NEIGHBOR(tree, value, has_b, best_b);
        }
        else {
          GET_NEIGHBOR_NW();
          CHECK_NEIGHBOR(tree, value, has_a, best_a);
          GET_NEIGHBOR_N();
          CHECK_NEIGHBOR(tree, value, has_a, best_a);
          GET_NEIGHBOR_SE();
          CHECK_NEIGHBOR(tree, value, has_b, best_b);
          GET_NEIGHBOR_S();
          CHECK_NEIGHBOR(tree, value, has_b, best_b);
        }
      }
    }
    else {
      if (dx == 0) {
        GET_NEIGHBOR_W();
        CHECK_NEIGHBOR(tree, value, has_a, best_a);
        GET_NEIGHBOR_E();
        CHECK_NEIGHBOR(tree, value, has_b, best_b);
      }
      else
      if ((dx > 0 && dy > 0) || (dx < 0 && dy < 0)) {
        if (fabs(dx) < 2 * fabs(dy)) {
          GET_NEIGHBOR_N();
          CHECK_NEIGHBOR(tree, value, has_a, best_a);
          GET_NEIGHBOR_NE();
          CHECK_NEIGHBOR(tree, value, has_a, best_a);
          GET_NEIGHBOR_E();
          CHECK_NEIGHBOR(tree, value, has_a, best_a);
          GET_NEIGHBOR_S();
          CHECK_NEIGHBOR(tree, value, has_b, best_b);
          GET_NEIGHBOR_SW();
          CHECK_NEIGHBOR(tree, value, has_b, best_b);
          GET_NEIGHBOR_W();
          CHECK_NEIGHBOR(tree, value, has_b, best_b);
        }
        else {
          GET_NEIGHBOR_W();
          CHECK_NEIGHBOR(tree, value, has_a, best_a);
          GET_NEIGHBOR_SW();
          CHECK_NEIGHBOR(tree, value, has_a, best_a);
          GET_NEIGHBOR_E();
          CHECK_NEIGHBOR(tree, value, has_b, best_b);
          GET_NEIGHBOR_NE();
          CHECK_NEIGHBOR(tree, value, has_b, best_b);
        }
      }
      else {
        if (fabs(dx) < 2 * fabs(dy)) {
          GET_NEIGHBOR_W();
          CHECK_NEIGHBOR(tree, value, has_a, best_a);
          GET_NEIGHBOR_NW();
          CHECK_NEIGHBOR(tree, value, has_a, best_a);
          GET_NEIGHBOR_N();
          CHECK_NEIGHBOR(tree, value, has_a, best_a);
          GET_NEIGHBOR_E();
          CHECK_NEIGHBOR(tree, value, has_b, best_b);
          GET_NEIGHBOR_SE();
          CHECK_NEIGHBOR(tree, value, has_b, best_b);
          GET_NEIGHBOR_S();
          CHECK_NEIGHBOR(tree, value, has_b, best_b);
        }
        else {
          GET_NEIGHBOR_NW();
          CHECK_NEIGHBOR(tree, value, has_a, best_a);
          GET_NEIGHBOR_W();
          CHECK_NEIGHBOR(tree, value, has_a, best_a);
          GET_NEIGHBOR_SE();
          CHECK_NEIGHBOR(tree, value, has_b, best_b);
          GET_NEIGHBOR_E();
          CHECK_NEIGHBOR(tree, value, has_b, best_b);
        }
      }
    }
    /* at this stage, each node sets its own predecessor and successor */
    if (IS_TRUE(has_a)) {
      edge->next = &best_a->edge;
    }
    else {
      edge->next = NULL;
    }
    if (IS_TRUE(has_b)) {
      edge->prev = &best_b->edge;
    }
    else {
      edge->prev = NULL;
    }
    edges = edges->next;
  }

  PRINT0("merge edge chains\n");
  srand(38374);
  /* then go through the list again, and merge edge chains where the nodes agree */
  edges = edge_list.first.next;
  end = &edge_list.last;
  while (edges != end) {
    edge = *(quad_forest_edge**)edges->data;
    parent = edge_chain_find(edge);
    /* if parent is different than self, the edge already belongs to a chain */
    if (parent == edge) {
      new_chain.parent = parent;

      if (edge->prev != NULL) {
        new_chain.first = edge_chain_follow_backward(parent, edge, edge->prev);
      }
      else {
        new_chain.first = edge;
      }
      new_chain.first->prev = NULL;
      if (edge->next != NULL) {
        new_chain.last = edge_chain_follow_forward(parent, edge, edge->next);
      }
      else {
        new_chain.last = edge;
      }
      new_chain.last->next = NULL;
      new_chain.length = parent->length;

      /* add only long enough chains to the list */
      if (new_chain.length >= min_length) {
        CHECK(list_append_return_pointer(&forest->edges, (pointer)&new_chain, (pointer*)&chain));
        /* cost must be initialized to null, it is updated during next operation */
        chain->cost = 0;
        chain->token = (unsigned)rand();
        edge_set_chain(chain->first, chain);
      }
      else {
        edge_chain_clear(new_chain.first);
      }
    }
    edges = edges->next;
  }
  PRINT1("found %lu edge chains\n", forest->edges.count);

  /*CHECK(quad_forest_refresh_edges(forest));*/


  /* then try connecting individual pieces of edge chains */
  {
    uint32 token;
    integral_value cost;
    list_item *items, *end;
    quad_forest_edge_chain *chain;
    path_sniffer new_sniffer, *sniffer, *neighbor_sniffer;
    edge_connection new_connection, *connection;

    CHECK(list_create(&context_list, 10 * forest->edges.count, sizeof(path_sniffer*), 1));
    CHECK(list_create(&connection_list, forest->edges.count, sizeof(edge_connection), 1));

    /* TODO: later use randomly generated tokens to identify parsing phases */
    token = 1948572362;

    PRINT0("init context list\n");
    items = forest->edges.first.next;
    end = &forest->edges.last;
    while (items != end) {
      chain = (quad_forest_edge_chain*)items->data;

      /* create context for the first endpoint */
      tree = chain->first->tree;
      tree->context.token = token;
      tree->context.round = 0;

      CHECK(typed_pointer_create(&tree->context.data, t_PATH_SNIFFER, 1));
      CHECK(expect_path_sniffer(&sniffer, &tree->context.data));

      sniffer->prev = NULL;
      sniffer->tree = tree;
      sniffer->chain = chain;
      sniffer->endpoint = chain->first;
      sniffer->strength = tree->segment.devdev;
      sniffer->cost = 0;
      sniffer->length = 0;
      path_sniffer_determine_directions(sniffer);

      CHECK(list_append(&context_list, (pointer)&sniffer));

      /* create context for the second endpoint */
      tree = chain->last->tree;
      tree->context.token = token;
      tree->context.round = 0;

      CHECK(typed_pointer_create(&tree->context.data, t_PATH_SNIFFER, 1));
      CHECK(expect_path_sniffer(&sniffer, &tree->context.data));

      sniffer->prev = NULL;
      sniffer->tree = tree;
      sniffer->chain = chain;
      sniffer->endpoint = chain->last;
      sniffer->strength = tree->segment.devdev;
      sniffer->cost = 0;
      sniffer->length = 0;
      path_sniffer_determine_directions(sniffer);

      CHECK(list_append(&context_list, (pointer)&sniffer));

      items = items->next;
    }

    PRINT0("start to process contexts\n");
    items = context_list.first.next;
    end = &context_list.last;
    while (items != NULL && items != end) {
      sniffer = *((path_sniffer**)items->data);
      /*PRINT0("pop\n");*/
      /* popping items from beginning and inserting new items in sorted order */
      items = list_pop_item(&context_list, items);
      if (sniffer == NULL) {
        PRINT0("null sniffer\n");
        break;
      }
      /*PRINT0("handle sniffer\n");*/
      tree = sniffer->tree;
      switch (sniffer->dir_start) {
        case d_NW:
        {
          /*PRINT0("nw ");*/
          GET_NEIGHBOR_NW();
          CHECK_SNIFFER_NEIGHBOR();
        }
        case d_N:
        {
          /*PRINT0("n ");*/
          GET_NEIGHBOR_N();
          CHECK_SNIFFER_NEIGHBOR();
        }
        case d_NE:
        {
          /*PRINT0("ne ");*/
          GET_NEIGHBOR_NE();
          CHECK_SNIFFER_NEIGHBOR();
          if (sniffer->dir_end == d_NE) {
            break;
          }
        }
        case d_E:
        {
          /*PRINT0("e ");*/
          GET_NEIGHBOR_E();
          CHECK_SNIFFER_NEIGHBOR();
          if (sniffer->dir_end == d_E) {
            break;
          }
        }
        case d_SE:
        {
          /*PRINT0("se ");*/
          GET_NEIGHBOR_SE();
          CHECK_SNIFFER_NEIGHBOR();
          if (sniffer->dir_end == d_SE) {
            break;
          }
        }
        case d_S:
        {
          /*PRINT0("s ");*/
          GET_NEIGHBOR_S();
          CHECK_SNIFFER_NEIGHBOR();
          if (sniffer->dir_end == d_S) {
            break;
          }
        }
        case d_SW:
        {
          /*PRINT0("sw ");*/
          GET_NEIGHBOR_SW();
          CHECK_SNIFFER_NEIGHBOR();
          if (sniffer->dir_end == d_SW) {
            break;
          }
        }
        case d_W:
        {
          /*PRINT0("w ");*/
          GET_NEIGHBOR_W();
          CHECK_SNIFFER_NEIGHBOR();
          if (sniffer->dir_end == d_W) {
            break;
          }
          /*PRINT0("nw ");*/
          GET_NEIGHBOR_NW();
          CHECK_SNIFFER_NEIGHBOR();
          if (sniffer->dir_end == d_NW) {
            break;
          }
          /*PRINT0("n ");*/
          GET_NEIGHBOR_N();
          CHECK_SNIFFER_NEIGHBOR();
          if (sniffer->dir_end == d_N) {
            break;
          }
        }
        default:
          break;
      }
      /* no need to get the next item, as using list_pop_item */
    }

    PRINT0("check connections\n");
    srand((signed)token);
    items = connection_list.first.next;
    end = &connection_list.last;
    while (items != NULL && items != end) {
      connection = (edge_connection*)items->data;

      PRINT0("follow 1...");
      path_follow_extend_edge(connection->sniffer1);
      PRINT0("done, follow 2...");
      path_follow_extend_edge(connection->sniffer2);
      PRINT0("done\n");

      if (connection->sniffer1->endpoint->chain != connection->sniffer2->endpoint->chain) {

        chain = connection->sniffer2->endpoint->chain;
        PRINT0("merge...\n");
        path_merge_edge_chains(connection->sniffer1, connection->sniffer2);
        PRINT0("done\n");
        /*
        chain->first = NULL;
        chain->last = NULL;
        */
      }
      else {
        PRINT0("trying to merge chain with itself\n");
      }

      items = items->next;
    }

    PRINT0("removing extra edges...\n");
    items = forest->edges.first.next;
    end = &forest->edges.last;
    while (items != end) {
      chain = (quad_forest_edge_chain*)items->data;
      items = items->next;
      if (chain->first == NULL && chain->last == NULL) {
        PRINT0("remove\n");
        CHECK(list_remove_item(&forest->edges, items->prev));
      }
    }
    PRINT0("remove finished\n");
  }

  FINALLY(quad_forest_find_boundaries);
  list_destroy(&edge_list);
  list_destroy(&context_list);
  list_destroy(&connection_list);
  PRINT0("finished\n");
  RETURN();
}

/******************************************************************************/

result quad_forest_find_boundaries_with_hysteresis
(
  quad_forest *forest,
  uint32 rounds,
  integral_value high_bias,
  integral_value low_factor
)
{
  TRY();
  uint32 remaining, i, size;
  integral_value mean, dev, value, dx, dy;
  truth_value has_a, has_b;
  quad_tree *tree, *neighbor, *best_a, *best_b;
  list boundary_list;
  list_item *boundaries, *end;

  CHECK_POINTER(forest);
  CHECK_PARAM(rounds > 0);
  CHECK_PARAM(high_bias > 0);
  CHECK_PARAM(0 < low_factor && low_factor < 1);

  CHECK(list_create(&boundary_list, 1000, sizeof(quad_tree*), 1));

  size = forest->rows * forest->cols;

  /* before propagation, prime all trees */
  /* for finding boundaries, prime with deviation */
  for (i = 0; i < size; i++) {
    quad_tree_prime_with_dev(forest->roots[i]);
  }

  /* then, propagate the requested number of rounds */
  for (remaining = rounds; remaining--;) {
    for (i = 0; i < size; i++) {
      quad_tree_propagate(forest->roots[i]);
    }
    /* on other rounds except the last, prime for the new run */
    if (remaining > 0) {
      for (i = 0; i < size; i++) {
        quad_tree_prime_with_pool(forest->roots[i]);
      }
    }
  }

  /* mark those trees that have a strong enough boundary */
  for (i = 0; i < size; i++) {
    tree = forest->roots[i];
    mean = tree->pool;
    dev = tree->pool2;
    dev -= mean*mean;
    if (dev < 0) dev = 0; else dev = sqrt(dev);
    tree->segment.devmean = mean;
    tree->segment.devdev = dev;

    value = tree->stat.deviation;

    if (value > getmax(mean, mean + high_bias - dev)) {
      /*printf("value %.3f mean %.3f dev %.3f\n", value, mean, dev);*/
      tree->segment.has_boundary = TRUE;
      /* at this point, get the edge responses for strong boundaries */
      CHECK(quad_tree_get_edge_response(forest, tree, NULL, NULL));
      CHECK(list_append(&boundary_list, (pointer)&tree));
    }
    else {
      tree->segment.has_boundary = FALSE;
    }
  }

  boundaries = boundary_list.first.next;
  end = &boundary_list.last;
  /* check neighboring trees in the direction of edge and discard those that */
  /* do not have at least one strong neighbor in correct direction */
  while (boundaries != end) {
    tree = *(quad_tree**)boundaries->data;
    if (IS_TRUE(tree->segment.has_boundary)) {
      dx = tree->edge.dx;
      dy = tree->edge.dy;
      has_a = FALSE;
      has_b = FALSE;
      best_a = NULL;
      best_b = NULL;
      value = low_factor * tree->stat.deviation;
      /* decide which neighbors to check based on edge direction */
      if (fabs(dx) > fabs(dy)) {
        if (dy == 0) {
          GET_NEIGHBOR_N();
          CHECK_NEIGHBOR(tree, value, has_a, best_a);
          GET_NEIGHBOR_S();
          CHECK_NEIGHBOR(tree, value, has_b, best_b);
        }
        else
        if ((dx > 0 && dy > 0) || (dx < 0 && dy < 0)) {
          if (fabs(dx) < 2 * fabs(dy)) {
            GET_NEIGHBOR_N();
            CHECK_NEIGHBOR(tree, value, has_a, best_a);
            GET_NEIGHBOR_NE();
            CHECK_NEIGHBOR(tree, value, has_a, best_a);
            GET_NEIGHBOR_E();
            CHECK_NEIGHBOR(tree, value, has_a, best_a);
            GET_NEIGHBOR_S();
            CHECK_NEIGHBOR(tree, value, has_b, best_b);
            GET_NEIGHBOR_SW();
            CHECK_NEIGHBOR(tree, value, has_b, best_b);
            GET_NEIGHBOR_W();
            CHECK_NEIGHBOR(tree, value, has_b, best_b);
          }
          else {
            GET_NEIGHBOR_N();
            CHECK_NEIGHBOR(tree, value, has_a, best_a);
            GET_NEIGHBOR_NE();
            CHECK_NEIGHBOR(tree, value, has_a, best_a);
            GET_NEIGHBOR_S();
            CHECK_NEIGHBOR(tree, value, has_b, best_b);
            GET_NEIGHBOR_SW();
            CHECK_NEIGHBOR(tree, value, has_b, best_b);
          }
        }
        else {
          if (fabs(dx) < 2 * fabs(dy)) {
            GET_NEIGHBOR_W();
            CHECK_NEIGHBOR(tree, value, has_a, best_a);
            GET_NEIGHBOR_NW();
            CHECK_NEIGHBOR(tree, value, has_a, best_a);
            GET_NEIGHBOR_N();
            CHECK_NEIGHBOR(tree, value, has_a, best_a);
            GET_NEIGHBOR_E();
            CHECK_NEIGHBOR(tree, value, has_b, best_b);
            GET_NEIGHBOR_SE();
            CHECK_NEIGHBOR(tree, value, has_b, best_b);
            GET_NEIGHBOR_S();
            CHECK_NEIGHBOR(tree, value, has_b, best_b);
          }
          else {
            GET_NEIGHBOR_NW();
            CHECK_NEIGHBOR(tree, value, has_a, best_a);
            GET_NEIGHBOR_N();
            CHECK_NEIGHBOR(tree, value, has_a, best_a);
            GET_NEIGHBOR_SE();
            CHECK_NEIGHBOR(tree, value, has_b, best_b);
            GET_NEIGHBOR_S();
            CHECK_NEIGHBOR(tree, value, has_b, best_b);
          }
        }
      }
      else {
        if (dx == 0) {
          GET_NEIGHBOR_W();
          CHECK_NEIGHBOR(tree, value, has_a, best_a);
          GET_NEIGHBOR_E();
          CHECK_NEIGHBOR(tree, value, has_b, best_b);
        }
        else
        if ((dx > 0 && dy > 0) || (dx < 0 && dy < 0)) {
          if (fabs(dx) < 2 * fabs(dy)) {
            GET_NEIGHBOR_N();
            CHECK_NEIGHBOR(tree, value, has_a, best_a);
            GET_NEIGHBOR_NE();
            CHECK_NEIGHBOR(tree, value, has_a, best_a);
            GET_NEIGHBOR_E();
            CHECK_NEIGHBOR(tree, value, has_a, best_a);
            GET_NEIGHBOR_S();
            CHECK_NEIGHBOR(tree, value, has_b, best_b);
            GET_NEIGHBOR_SW();
            CHECK_NEIGHBOR(tree, value, has_b, best_b);
            GET_NEIGHBOR_W();
            CHECK_NEIGHBOR(tree, value, has_b, best_b);
          }
          else {
            GET_NEIGHBOR_W();
            CHECK_NEIGHBOR(tree, value, has_a, best_a);
            GET_NEIGHBOR_SW();
            CHECK_NEIGHBOR(tree, value, has_a, best_a);
            GET_NEIGHBOR_E();
            CHECK_NEIGHBOR(tree, value, has_b, best_b);
            GET_NEIGHBOR_NE();
            CHECK_NEIGHBOR(tree, value, has_b, best_b);
          }
        }
        else {
          if (fabs(dx) < 2 * fabs(dy)) {
            GET_NEIGHBOR_W();
            CHECK_NEIGHBOR(tree, value, has_a, best_a);
            GET_NEIGHBOR_NW();
            CHECK_NEIGHBOR(tree, value, has_a, best_a);
            GET_NEIGHBOR_N();
            CHECK_NEIGHBOR(tree, value, has_a, best_a);
            GET_NEIGHBOR_E();
            CHECK_NEIGHBOR(tree, value, has_b, best_b);
            GET_NEIGHBOR_SE();
            CHECK_NEIGHBOR(tree, value, has_b, best_b);
            GET_NEIGHBOR_S();
            CHECK_NEIGHBOR(tree, value, has_b, best_b);
          }
          else {
            GET_NEIGHBOR_NW();
            CHECK_NEIGHBOR(tree, value, has_a, best_a);
            GET_NEIGHBOR_W();
            CHECK_NEIGHBOR(tree, value, has_a, best_a);
            GET_NEIGHBOR_SE();
            CHECK_NEIGHBOR(tree, value, has_b, best_b);
            GET_NEIGHBOR_E();
            CHECK_NEIGHBOR(tree, value, has_b, best_b);
          }
        }
      }
      if (IS_FALSE(has_a) && IS_FALSE(has_b)) {
        tree->segment.has_boundary = FALSE;
        /*PRINT0("discard\n");*/
      }
      else {
        if (IS_FALSE(has_a) && best_a != NULL) {
          /*PRINT0("found new tree a\n");*/
          best_a->segment.has_boundary = TRUE;
          CHECK(quad_tree_get_edge_response(forest, best_a, NULL, NULL));
          CHECK(list_append(&boundary_list, (pointer)&best_a));
        }
        if (IS_FALSE(has_b) && best_b != NULL) {
          /*PRINT0("found new tree b\n");*/
          best_b->segment.has_boundary = TRUE;
          CHECK(quad_tree_get_edge_response(forest, best_b, NULL, NULL));
          CHECK(list_append(&boundary_list, (pointer)&best_b));
        }
      }
    }
    boundaries = boundaries->next;
  }

  FINALLY(quad_forest_find_boundaries_with_hysteresis);
  list_destroy(&boundary_list);
  RETURN();
}

/******************************************************************************/

result quad_forest_prune_boundaries
(
  quad_forest *forest
)
{
  TRY();
  uint32 i, size;
  truth_value has_diff_segment;
  quad_tree *tree, *neighbor, *prev_neighbor;
  quad_forest_segment *prev_segment, *neighbor_segment;

  CHECK_POINTER(forest);

  size = forest->rows * forest->cols;

  for (i = 0; i < size; i++) {
    tree = forest->roots[i];
    if (IS_TRUE(tree->segment.has_boundary)) {
      prev_segment = NULL;
      prev_neighbor = NULL;
      has_diff_segment = FALSE;
      neighbor = tree->n;
      if (neighbor != NULL && IS_FALSE(neighbor->segment.has_boundary)) {
        neighbor_segment = quad_tree_segment_find(neighbor);
        if (neighbor_segment != NULL) {
          prev_segment = neighbor_segment;
          prev_neighbor = neighbor;
        }
      }
      neighbor = tree->e;
      if (neighbor != NULL && IS_FALSE(neighbor->segment.has_boundary)) {
        neighbor_segment = quad_tree_segment_find(neighbor);
        if (neighbor_segment != NULL) {
          if (prev_segment != NULL && prev_segment != neighbor_segment) {
            has_diff_segment = TRUE;
          }
          prev_segment = neighbor_segment;
          prev_neighbor = neighbor;
        }
      }
      neighbor = tree->s;
      if (neighbor != NULL && IS_FALSE(neighbor->segment.has_boundary)) {
        neighbor_segment = quad_tree_segment_find(neighbor);
        if (neighbor_segment != NULL) {
          if (prev_segment != NULL && prev_segment != neighbor_segment) {
            has_diff_segment = TRUE;
          }
          prev_segment = neighbor_segment;
          prev_neighbor = neighbor;
        }
      }
      neighbor = tree->w;
      if (neighbor != NULL && IS_FALSE(neighbor->segment.has_boundary)) {
        neighbor_segment = quad_tree_segment_find(neighbor);
        if (neighbor_segment != NULL) {
          if (prev_segment != NULL && prev_segment != neighbor_segment) {
            has_diff_segment = TRUE;
          }
          prev_segment = neighbor_segment;
          prev_neighbor = neighbor;
        }
      }
      if (IS_FALSE(has_diff_segment)) {
        tree->segment.has_boundary = FALSE;
        if (prev_neighbor != NULL) {
          quad_tree_segment_create(tree);
          quad_tree_segment_union(tree, prev_neighbor);
        }
      }
    }
  }

  FINALLY(quad_forest_prune_boundaries);
  RETURN();
}

/******************************************************************************/

result init_edge_parsers(quad_forest *forest, quad_tree *tree, uint32 token)
{
  TRY();
  integral_value angle1, angle2, anglediff, cost;
  list_item *links, *endlinks;
  quad_tree_link_head *head;
  edge_parser *eparser;

  /* set token and set round to 0 */
  tree->context.token = token;
  tree->context.round = 0;
  /* calculate edge response */
  CHECK(quad_tree_get_edge_response(forest, tree, NULL, NULL));
  /* initialize link heads */
  links = tree->links.first.next;
  endlinks = &tree->links.last;
  while (links != endlinks) {
    head = *((quad_tree_link_head**)links->data);
    if (head->context.token != token) {
      /* create context, add token, set round to 0 */
      head->context.token = token;
      head->context.round = 0;
      CHECK(typed_pointer_create(&head->context.data, t_EDGE_PARSER, 1));
      CHECK(expect_edge_parser(&eparser, &head->context.data));
      eparser->acc_cost = 0;
      eparser->pool_cost = 0;
      eparser->acc_length = 0;
      eparser->pool_length = 0;

      /* calculate half-step cost */
      angle1 = head->angle;
      if (angle1 > M_PI) angle1 -= M_PI;
      angle2 = tree->edge.ang;
      if (angle2 > M_PI) angle2 -= M_PI;
      anglediff = fabs(angle1 - angle2);
      if (anglediff > (M_PI / 2)) anglediff = M_PI - anglediff;
      anglediff /= (M_PI / 2);

      if (tree->stat.deviation < tree->segment.devmean &&
          head->other->tree->stat.deviation < head->other->tree->segment.devmean) {
        cost = tree->segment.devmean - fabs(tree->stat.deviation - head->other->tree->stat.deviation);
        if (cost < 0) cost = 0;
      }
      else {
        cost = anglediff * fabs(tree->segment.devdev - head->other->tree->segment.devdev);
      }

      if (cost < 0.0000001) cost = 0.001;

      head->cost = cost;
    }
    links = links->next;
  }

  FINALLY(init_edge_parsers);
  RETURN();
}

/******************************************************************************/

result pool_edge_parsers(quad_tree *tree, uint32 round)
{
  TRY();
  uint32 length, length1, length2;
  integral_value cost, cost1, cost2;
  list_item *links, *endlinks;
  quad_tree_link_head *head, *head1, *head2;
  edge_parser *eparser;

  head1 = NULL;
  cost1 = 0;
  length1 = 0;
  head2 = NULL;
  cost2 = 0;
  length2 = 0;

  links = tree->links.first.next;
  endlinks = &tree->links.last;
  while (links != endlinks) {
    head = *((quad_tree_link_head**)links->data);
    CHECK(expect_edge_parser(&eparser, &head->context.data));
    cost = eparser->acc_cost;
    length = eparser->acc_length;
    /* find best paths with at least length 1, NULL if not found */
    if (length > 0) {
      if (head1 == NULL || cost < cost1) {
        head2 = head1;
        cost2 = cost1;
        length2 = length1;
        head1 = head;
        cost1 = cost;
        length1 = length;
      }
      else {
        if (head2 == NULL || cost < cost2) {
          head2 = head;
          cost2 = cost;
          length2 = length;
        }
      }
    }
    links = links->next;
  }

  links = tree->links.first.next;
  endlinks = &tree->links.last;
  while (links != endlinks) {
    head = *((quad_tree_link_head**)links->data);
    cost = 0;
    length = 0;
    if (head1 != NULL) {
      if (head != head1) {
        cost = cost1;
        length = length1;
      }
      else {
        cost = cost2;
        length = length2;
      }
    }

    /* pool the cost of best path, length, plus cost of own half-step */
    CHECK(expect_edge_parser(&eparser, &head->context.data));
    eparser->pool_cost = cost + head->cost;
    eparser->pool_length = length;
    head->context.round = round;

    links = links->next;
  }
  if (head1 != NULL) {
    head1->link->strength = 1;
  }
  if (head2 != NULL) {
    head2->link->strength = 1;
  }

  tree->context.round = round;

  FINALLY(pool_edge_parsers);
  RETURN();
}

/******************************************************************************/

result acc_edge_parsers(quad_tree *tree, uint32 token)
{
  TRY();
  list_item *links, *endlinks;
  quad_tree_link_head *head;
  edge_parser *eparser1, *eparser2;

  /* add pooled cost of other end of each link to own half-step cost, increase length by 1 */
  links = tree->links.first.next;
  endlinks = &tree->links.last;
  while (links != endlinks) {
    head = *((quad_tree_link_head**)links->data);
    if (head->other->context.token == token) {
      CHECK(expect_edge_parser(&eparser1, &head->context.data));
      CHECK(expect_edge_parser(&eparser2, &head->other->context.data));
      eparser1->acc_cost = head->cost + eparser2->pool_cost;
      eparser1->acc_length = eparser2->pool_length + 1;
    }
    links = links->next;
  }

  FINALLY(acc_edge_parsers);
  RETURN();
}

/******************************************************************************/

result quad_forest_parse
(
  quad_forest *forest,
  uint32 rounds,
  integral_value bias,
  uint32 min_length
)
{
  TRY();
  uint32 remaining, i, size, count, token, round, length1, length2;
  integral_value mean, dev, value, dx, dy, strength, min, max, cost, cost1, cost2;
  integral_value a1, a2, b1, b2, I, U, angle1, angle2, anglediff;
  quad_tree *tree1, *tree2;
  quad_tree_link_head *head, *head1, *head2;
  quad_tree_link *link;
  list treelist;
  list_item *trees, *endtrees, *links, *endlinks;
  edge_parser *eparser;

  CHECK_POINTER(forest);
  CHECK_PARAM(rounds > 0);

  size = forest->rows * forest->cols;

  CHECK(list_create(&treelist, size, sizeof(quad_tree*), 1));

  /* before propagation, prime all trees */
  /* for finding boundaries, prime with deviation */
  for (i = 0; i < size; i++) {
    quad_tree_prime_with_dev(forest->roots[i]);
  }

  /* then, propagate the requested number of rounds */
  for (remaining = rounds+1; remaining--;) {
    for (i = 0; i < size; i++) {
      quad_tree_propagate(forest->roots[i]);
    }
    /* on other rounds except the last, prime for the new run */
    if (remaining > 0) {
      for (i = 0; i < size; i++) {
        quad_tree_prime_with_pool(forest->roots[i]);
      }
    }
  }

  srand(384746272);
  token = (unsigned)rand();

  PRINT0("calculate devmean and devdev\n");
  /* calculate devmean and devdev, and determine the boundary trees */
  for (i = 0; i < size; i++) {
    tree1 = forest->roots[i];
    mean = tree1->pool;
    dev = tree1->pool2;
    dev -= mean*mean;
    if (dev < 0) dev = 0; else dev = sqrt(dev);
    tree1->segment.devmean = mean;
    tree1->segment.devdev = dev;

    value = tree1->stat.deviation;

    tree1->segment.has_boundary = FALSE;
    if (value > getmax(mean, mean + bias - dev)) {
      /*tree1->segment.has_boundary = TRUE;*/
      CHECK(init_edge_parsers(forest, tree1, token));
      /* do the round 0 for seed nodes here */
      /* at this point, get the edge responses for strong boundaries */
      /*
      CHECK(quad_tree_get_edge_response(forest, tree1, NULL, NULL));
      CHECK(list_append(&treelist, &tree1));
      */
    }
    else {
      /* init segment parser */
      tree1->context.token = 0;
      /*tree1->segment.has_boundary = FALSE;*/
    }
  }

  for (i = 0; i < size; i++) {
    quad_tree_prime_with_mean(forest->roots[i]);
  }

  for (remaining = rounds; remaining--;) {
    for (i = 0; i < size; i++) {
      quad_tree_propagate(forest->roots[i]);
    }
    if (remaining > 0) {
      for (i = 0; i < size; i++) {
        quad_tree_prime_with_pool(forest->roots[i]);
      }
    }
  }

  for (i = 0; i < size; i++) {
    tree1 = forest->roots[i];
    mean = tree1->pool;
    dev = getmax(1, tree1->segment.devmean);
    a1 = mean - dev;
    if (a1 < 0) a1 = 0;
    a2 = mean + dev;
    if (a2 > 255) a2 = 255;
    mean = tree1->stat.mean;
    dev = getmax(1, tree1->stat.deviation);
    b1 = mean - dev;
    if (b1 < 0) b1 = 0;
    b2 = mean + dev;
    if (b2 > 255) b2 = 255;

    I = getmin(a2,b2) - getmax(a1,b1);
    if (I < 0) I = 0;
    U = getmax(a2,b2) - getmin(a1,b1);

    value = I / U;

    if (value < 0.5) {
      tree1->segment.has_boundary = TRUE;
    }
    else {
      tree1->segment.has_boundary = FALSE;
    }
  }

  /* parse propagation loop, start with round 1 */
  PRINT0("running propagate loop\n");
  for (round = 1; round <= rounds; round++) {
    PRINT1("round %lu\n", round);
    links = forest->links.first.next;
    endlinks = &forest->links.last;
    while (links != endlinks) {
      link = (quad_tree_link*)links->data;
      link->strength = 0;
      links = links->next;
    }
    link = (quad_tree_link*)links->data;
    for (i = 0; i < size; i++) {
      tree1 = forest->roots[i];
      /* if tree context has token and smaller round number than current */
      if (tree1->context.token == token && tree1->context.round < round) {
        /* then pool */
        pool_edge_parsers(tree1, round);
        /* check all neighbors for token */
        links = tree1->links.first.next;
        endlinks = &tree1->links.last;
        while (links != endlinks) {
          head = *((quad_tree_link_head**)links->data);
          tree2 = head->other->tree;
          /* if don't have token, init and then pool with current round */
          if (tree2->context.token != token) {
            CHECK(init_edge_parsers(forest, tree2, token));
            CHECK(pool_edge_parsers(tree2, round));
          }
          links = links->next;
        }
        tree1->context.round = round;
      }
    }
    /* after pooling all trees, acc trees that have the token */
    for (i = 0; i < size; i++) {
      tree1 = forest->roots[i];
      if (tree1->context.token == token) {
        CHECK(acc_edge_parsers(tree1, token));
      }
    }
  }

  PRINT0("normalize links\n");
  count = 0;
  links = forest->links.first.next;
  endlinks = &forest->links.last;
  while (links != endlinks) {
    link = (quad_tree_link*)links->data;
    if (link->a.context.token == token && link->b.context.token == token) {
      CHECK(expect_edge_parser(&eparser, &link->a.context.data));
      /*cost1 = link->a.cost;*/
      cost1 = eparser->acc_cost;
      length1 = eparser->acc_length;
      CHECK(expect_edge_parser(&eparser, &link->b.context.data));
      /*cost2 = link->b.cost;*/
      cost2 = eparser->acc_cost;
      length2 = eparser->acc_length;

      strength = (cost1 + cost2) / ((integral_value)(length1 + length2));
      link->strength = strength;

      /*strength = link->strength;*/
      if (count == 0) {
          min = strength;
          max = strength;
      }
      else {
        if (strength < min) min = strength;
        if (strength > max) max = strength;
      }
      count++;
    }
    else {
      link->strength = 0;
    }
    links = links->next;
  }
  PRINT2("got %lu links with edge, %lu in total\n", count, forest->links.count);
  PRINT2("min=%.3f max=%.3f\n", min, max);

  links = forest->links.first.next;
  endlinks = &forest->links.last;
  while (links != endlinks) {
    link = (quad_tree_link*)links->data;
    if (link->strength < 0.0000001) {
      link->strength = 0;
    }
    else {
      link->strength = 1 - ((link->strength - min) / (max - min)); /* 1 - */
    }
    links = links->next;
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
  
  CHECK_POINTER(forest);
  CHECK_POINTER(target);
  
  FINALLY(quad_forest_visualize_parse_result);
  RETURN();
}

/* end of file                                                                */
/******************************************************************************/
