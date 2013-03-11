/**
 * @file cvsu_temporal_forest.h
 * @author Matti J. Eskelinen <matti.j.eskelinen@gmail.com>
 * @brief Temporal Forest structure for analyzing image changes over time.
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

#ifndef CVSU_TEMPORAL_FOREST_H
#   define CVSU_TEMPORAL_FOREST_H

#ifdef __cplusplus
extern "C" {
#endif

#include "cvsu_types.h"
#include "cvsu_pixel_image.h"
#include "cvsu_quad_forest.h"
#include "cvsu_background_forest.h"

typedef struct temporal_forest_t {
  background_forest *background;
  quad_forest *forests;
  pixel_image visual;
  uint32 rows;
  uint32 cols;
  uint32 tree_max_size;
  uint32 tree_min_size;
  uint32 dx;
  uint32 dy;
  uint32 count;
  uint32 current;
  uint32 frames;
} temporal_forest;

temporal_forest *temporal_forest_alloc();

void temporal_forest_free
(
  temporal_forest *forest
);

result temporal_forest_create
(
  /** Forest to be initialized */
  temporal_forest *target,
  /** Sample of source images, used for setting the size of forests, not stored */
  pixel_image *source,
  /** Maximum (initial) size of trees in the forest */
  uint32 max_size,
  /** Minimum size of trees allowed in the forest (cannot divide further) */
  uint32 min_size,
  /** Number of frames to store for analyzing temporal changes */
  uint32 frame_count,
  /** Number of frames used for accumulating the background model */
  uint32 history_count
);

result temporal_forest_destroy
(
  temporal_forest *target
);

result temporal_forest_nullify
(
  temporal_forest *target
);

truth_value temporal_forest_is_null
(
  temporal_forest *target
);

result temporal_forest_update
(
  temporal_forest *target,
  pixel_image *source
);

result temporal_forest_visualize
(
temporal_forest *target
);

#ifdef __cplusplus
}
#endif

#endif /* CVSU_TEMPORAL_FOREST_H */
