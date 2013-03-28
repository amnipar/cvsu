/**
 * @file cvsu_segmentation.h
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

#ifndef CVSU_SEGMENTATION_H
#   define CVSU_SEGMENTATION_H

#ifdef __cplusplus
extern "C" {
#endif

#include "cvsu_config.h"
#include "cvsu_types.h"
#include "cvsu_quad_tree.h"
#include "cvsu_quad_forest.h"

/**
 * Refreshes the segment count and colors. MUST be called after segmentation and
 * BEFORE calling @see quad_forest_get_segments.
 */
result quad_forest_refresh_segments
(
  quad_forest *target
);

/**
 * Segments the quad_forest structure using a deviation threshold as
 * consistency and similarity criteria. Divides all trees that have deviation
 * larger than the threshold, and then merges trees and regions that have the
 * difference of means smaller than the threshold.
 */
result quad_forest_segment_with_deviation
(
  /** The quad_forest structure to be segmented. */
  quad_forest *target,
  /** Threshold value for deviation, trees with larger value are divided. */
  integral_value threshold,
  /** Deviation multiplier used for creating the estimated intensity range. */
  integral_value alpha
);

/**
 * Segments the quad_forest structure using an entropy measure as consistency
 * and similarity criteria.
 * TODO: maybe add some region size constraint as a parameter.
 */
result quad_forest_segment_with_overlap
(
  /** The quad_forest structure to be segmented. */
  quad_forest *target,
  /** Deviation multiplier used for creating the estimated intensity range. */
  integral_value alpha,
  /** Range overlap threshold used for determining the trees to merge. */
  integral_value threshold_trees,
  /** Range overlap threshold used for determining the segments to merge. */
  integral_value threshold_segments
);

/**
 * Segments the forest by finding first all horizontal edges with edge
 * propagation, then merging segments that have edges in neighboring trees.
 */
result quad_forest_segment_edges
(
  /** Forest to be segmented */
  quad_forest *target,
  /** How many rounds to propagate while determining trees with edges */
  uint32 detect_rounds,
  /** Bias value used in edge detection */
  integral_value detect_bias,
  /** Direction of edges to search (H,V,N4) */
  direction detect_dir,
  /** How many rounds to propagate the found edges to close gaps */
  uint32 propagate_rounds,
  /** Acceptance threshold for propagated edges */
  integral_value propagate_threshold,
  /** The direction in which to propagate */
  direction propagate_dir,
  /** The direction in which to merge segments */
  direction merge_dir
);

/**
 * Segments the forest by using boundaries found using deviation propagation
 * and hysteresis to limit the expansion of segments.
 */
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
);


#ifdef __cplusplus
}
#endif

#endif /* CVSU_SEGMENTATION_H */
