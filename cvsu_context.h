/**
 * @file cvsu_context.h
 * @author Matti J. Eskelinen <matti.j.eskelinen@gmail.com>
 * @brief Context structures for image parsing algorithms.
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

#ifndef CVSU_CONTEXT_H
#   define CVSU_CONTEXT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "cvsu_config.h"
#include "cvsu_types.h"
#include "cvsu_typed_pointer.h"

/**
 * Context value for accumulating neighborhood statistics.
 */
typedef struct stat_accumulator_t {
  uint32 round;
  integral_value mean_pool1;
  integral_value mean_acc1;
  integral_value mean_pool2;
  integral_value mean_acc2;
  integral_value dev_pool1;
  integral_value dev_acc1;
  integral_value dev_pool2;
  integral_value dev_acc2;
} stat_accumulator;

truth_value is_stat_accumulator
(
  typed_pointer *tptr
);

stat_accumulator *has_stat_accumulator
(
  typed_pointer *tptr
);

result expect_stat_accumulator
(
  stat_accumulator **target,
  typed_pointer *tptr
);

result ensure_stat_accumulator
(
  typed_pointer *context,
  stat_accumulator **acc
);

/******************************************************************************/

struct quad_forest_edge_t;
struct quad_forest_edge_chain_t;

/**
 * Context value for sniffing shortest paths between line endpoints.
 */
typedef struct path_sniffer_t {
  /** Previous node along the shortest path to this node */
  struct path_sniffer_t *prev;
  /** Quad tree associated with this node */
  struct quad_tree_t *tree;
  /** Edge chain that we attempt to extend */
  struct quad_forest_edge_chain_t *chain;
  /** Edge chain endpoint where starting to extend */
  struct quad_forest_edge_t *endpoint;
  /** Strength of edge at this point */
  integral_value strength;
  /** Cost of this path so far */
  integral_value cost;
  /** Length of this path so far */
  uint32 length;
  /** First direction where to propagate this path */
  direction dir_start;
  /** Last direction where to propagate this path */
  direction dir_end;
} path_sniffer;

/**
 * Creates a typed_pointer from a path_sniffer pointer
 */
void make_path_sniffer
(
  typed_pointer *tptr,
  path_sniffer *source
);

/**
 * Checks that the pointer has the correct type label for path_sniffer.
 */
truth_value is_path_sniffer
(
  typed_pointer *tptr
);

/**
 * Expects a path_sniffer and casts the pointer if the type label is correct;
 * if not, creates a BAD_TYPE error.
 */
result expect_path_sniffer
(
  path_sniffer **target,
  typed_pointer *tptr
);

/******************************************************************************/

typedef struct segment_parser_t {
  uint32 round;
} segment_parser;

/******************************************************************************/

typedef struct edge_parser_t {
  integral_value pool_cost;
  integral_value acc_cost;
  uint32 pool_length;
  uint32 acc_length;
} edge_parser;

void make_edge_parser
(
  typed_pointer *tptr,
  edge_parser *parser
);

truth_value is_edge_parser
(
  typed_pointer *tptr
);

result expect_edge_parser
(
  edge_parser **target,
  const typed_pointer *tptr
);

#ifdef __cplusplus
}
#endif

#endif /* CVSU_CONTEXT_H */
