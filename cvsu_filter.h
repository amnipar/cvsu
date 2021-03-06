/**
 * @file cvsu_filter.h
 * @author Matti J. Eskelinen <matti.j.eskelinen@gmail.com>
 * @brief Basic image filters for cvsu.
 *
 * Copyright (c) 2011-2013, Matti Johannes Eskelinen
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

#ifndef CVSU_FILTER_H
#   define CVSU_FILTER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "cvsu_pixel_image.h"

/**
 * Thresholds an 8-bit grayscale image by changing all values smaller than t to
 * 0 and all values larger than t to 255.
 */
result threshold
(
  const pixel_image *source,
  pixel_image *target,
  byte t
);

/**
 * Smoothes an 8-bit grayscale image by applying binomial filter multiple times.
 */
result smooth_binomial
(
  const pixel_image *source,
  pixel_image *target,
  uint32 passes
);

/**
 * Filters image with a 3x3 horizontal sobel operator.
 */
result sobel_x
(
  const pixel_image *source,
  pixel_image *target
);

/**
 * Filters image with a 3x3 horizontal sobel operator, using absolute values.
 */
result abs_sobel_x
(
  const pixel_image *source,
  pixel_image *target
);

/**
 * Filters image with a 3x3 vertical sobel operator.
 */
result sobel_y
(
  const pixel_image *source,
  pixel_image *target
);

/**
 * Filters image with a 3x3 vertical sobel operator, using absolute values.
 */
result abs_sobel_y
(
  const pixel_image *source,
  pixel_image *target
);

/**
 * Calculates the extremal values along horizontal scanlines
 */
result extrema_x
(
  const pixel_image *source,
  pixel_image *target
);

/**
 * Calculates the extremal values along vertical scanlines
 */
result extrema_y
(
  const pixel_image *source,
  pixel_image *target
);

#ifdef __cplusplus
}
#endif

#endif /* CVSU_FILTER_H */
