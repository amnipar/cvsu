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

#include "cvsu_parsing.h"

/******************************************************************************/
/* constants for reporting function names in error messages                   */

string quad_forest_parse_name = "quad_forest_parse";

string init_edge_parsers_name = "init_edge_parsers";
string pool_edge_parsers_name = "pool_edge_parsers";
string acc_edge_parsers_name = "acc_edge_parsers";

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
      CREATE_POINTER(&head->context.data, edge_parser, 1);
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
  token = rand();

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

/* end of file                                                                */
/******************************************************************************/
