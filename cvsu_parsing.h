/**
 * @file cvsu_parsing.h
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

#ifndef CVSU_PARSING_H
#   define CVSU_PARSING_H

#ifdef __cplusplus
extern "C" {
#endif

#include "cvsu_config.h"
#include "cvsu_types.h"
#include "cvsu_quad_tree.h"
#include "cvsu_quad_forest.h"
#include "cvsu_list.h"
#include "cvsu_context.h"

typedef result (*context_operation)(quad_tree *tree, list *collection);

/**
 * Runs a generic prime/propagate/accumulate operation on a list of trees.
 */
result run_context_operation
(
  list *input_trees,
  list *output_trees,
  context_operation prime_operation,
  context_operation propagate_operation,
  context_operation accumulate_operation,
  uint32 rounds,
  truth_value needs_list
);

result quad_forest_calculate_accumulated_stats
(
  quad_forest *forest,
  uint32 rounds
);

result quad_forest_visualize_accumulated_stats
(
  quad_forest *forest,
  pixel_image *target
);

result quad_forest_calculate_accumulated_regs
(
  quad_forest *forest,
  uint32 rounds
);

result quad_forest_visualize_accumulated_regs
(
  quad_forest *forest,
  pixel_image *target
);

result quad_forest_calculate_accumulated_bounds
(
  quad_forest *forest,
  uint32 rounds
);

result quad_forest_visualize_accumulated_bounds
(
  quad_forest *forest,
  pixel_image *target
);

/**
 * Uses deviation propagation to find potential segment boundaries.
 */
result quad_forest_parse
(
  quad_forest *forest,
  /** How many propagation rounds to use for determining devmean and devdev */
  uint32 rounds,
  /** The bias value used for determining devdev threshold for boundary */
  integral_value bias,
  /** The minimum length of edge chains _before_ starting to fill gaps */
  uint32 min_length
);

result quad_forest_visualize_parse_result
(
  quad_forest *forest,
  pixel_image *target
);

#ifdef __cplusplus
}
#endif

#endif /* CVSU_PARSING_H */
