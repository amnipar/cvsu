/**
 * @file cvsu_integral.h
 * @author Matti J. Eskelinen <matti.j.eskelinen@gmail.com>
 * @brief Integral image types and operations for cvsu.
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

#ifndef CVSU_INTEGRAL_H
#   define CVSU_INTEGRAL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "cvsu_config.h"
#include "cvsu_types.h"
#include "cvsu_pixel_image.h"

/**
 * Stores an integral and squared integral representation of a pixel image.
 * Refers to the original image but does not own it.
 * @note Integral images have one extra row and column, so they have larger
 * stride than the original, even though the width and height are the same.
 */
typedef struct integral_image_t {
  /** The original pixel_image from which the integrals are calculated */
  pixel_image *original;
  /** The pixel_image containing the integral of power one */
  pixel_image I_1;
  /** The pixel_image containing the integral of power two */
  pixel_image I_2;
#ifdef INTEGRAL_IMAGE_HIGHER_ORDER_STATISTICS
  /** The pixel_image containing the integral of power three */
  pixel_image I_3;
  /** The pixel_image containing the integral of power four */
  pixel_image I_4;
#endif
  /** The width of the integral_image; same as the width of pixel_image */
  uint32 width;
  /** The height of the integral_image; same as the height of pixel_image */
  uint32 height;
  /** The column step of the integral_image; same as the step of pixel_image */
  uint32 step;
  /** The row stride of the integral_image; LARGER than in pixel_image */
  uint32 stride;
} integral_image;

/**
 * Allocates an integral_image structure.
 * @see integral_image_free
 */
integral_image *integral_image_alloc();

/**
 * Frees an integral_image structure allocated with @see integral_image_alloc.
 */
void integral_image_free
(
  integral_image *target
);

/**
 * Initializes the structure for an integral image and allocates the memory.
 * Does not calculate the integrals.
 * @see integral_image_update
 * @see integral_image_destroy
 */
result integral_image_create
(
  integral_image *target,
  pixel_image *source
);

/**
 * Deallocates the memory allocated within the integral_image structure.
 * Does not free the structure pointer itself.
 * @see integral_image_create
 * @see integral_image_free
 */
result integral_image_destroy
(
  integral_image *target
);

/**
 * Nullifies the contents of the integral_image. Does NOT deallocate memory.
 */
result integral_image_nullify
(
  integral_image *target
);

/**
 * Everything that can be nullified should be able to tell if it's null
 */
truth_value integral_image_is_null
(
  integral_image *target
);

/**
 * Transform the target integral_image into a clone of source integral_image.
 * Only the structure is cloned; for copying content, use the copy function.
 * @see copy_integral_image
 */
result integral_image_clone
(
  integral_image *target,
  integral_image *source
);

/**
 * Copy the contents of the source integral_image into the target integral_image.
 * The two images must have the same structure.
 * @see clone_integral_image
 */
result integral_image_copy
(
  integral_image *target,
  integral_image *source
);

/**
 * Updates the integral_image by calculating the integral and squared integral
 * of the source pixel_image.
 */
result integral_image_update
(
  integral_image *target
);

/**
 * Produces a valid rectangle for the given integral_image. Takes into account
 * image dimensions and reduces the size of the region at the border. May
 * cause edge effects in some applications.
 */
image_rect integral_image_create_rect
(
  /** the integral_image used for calculations */
  integral_image *target,
  /** left coordinate of the rectangle, can be negative */
  sint32 x,
  /** top coordinate of the rectangle, can be negative */
  sint32 y,
  /** width of the rectangle, signed to allow adjustment using x */
  sint32 dx,
  /** height of the rectangle, signed to allow adjustment using y */
  sint32 dy,
  /** channel offset for multi-channel images */
  uint32 offset
);

/**
 * Uses the integral_image to calculate intensity mean within the given region.
 */
integral_value integral_image_calculate_mean
(
  /** the integral_image used for calculations */
  integral_image *target,
  /** left coordinate of the rectangle, can be negative */
  sint32 x,
  /** top coordinate of the rectangle, can be negative */
  sint32 y,
  /** width of the rectangle, signed to allow adjustment using x */
  sint32 dx,
  /** height of the rectangle, signed to allow adjustment using y */
  sint32 dy,
  /** channel offset for multi-channel images */
  uint32 offset
);

/**
 * Uses the integral_image to calculate intensity variance within the given
 * region.
 */
integral_value integral_image_calculate_variance
(
  integral_image *target,
  sint32 x,
  sint32 y,
  sint32 width,
  sint32 height,
  uint32 offset
);

/**
 * Uses the integral_image to calculate intensity statistics within the given
 * region.
 */
void integral_image_calculate_statistics
(
  integral_image *target,
  statistics *stat,
  sint32 x,
  sint32 y,
  sint32 width,
  sint32 height,
  uint32 offset
);

/**
 * Uses the integral_image to threshold the underlying pixel_image using the
 * Sauvola method.
 */
result integral_image_threshold_sauvola
(
  integral_image *source,
  pixel_image *target,
  truth_value invert,
  uint32 radius,
  integral_value k,
  truth_value calculate_max,
  integral_value max,
  truth_value use_mean
);

/**
 * Uses the integral_image to threshold the underlying pixel_image using the
 * Feng method.
 */
result integral_image_threshold_feng
(
  integral_image *source,
  pixel_image *target,
  truth_value invert,
  uint32 radius1,
  integral_value multiplier,
  truth_value estimate_min,
  integral_value alpha
);

/**
 * Creates a small integral image with dimensions less than 256. This allows
 * using unsigned long values for both images.
 */
result small_integral_image_create
(
  integral_image *target,
  pixel_image *source
);

/**
 * Updates the small integral image.
 */
result small_integral_image_update
(
  integral_image *target
);

#ifdef __cplusplus
}
#endif

#endif /* CVSU_INTEGRAL_H */
