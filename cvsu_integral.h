/**
 * @file cvsu_integral.h
 * @author Matti J. Eskelinen <matti.j.eskelinen@gmail.com>
 * @brief Integral image types and operations for the cvsu module.
 *
 * Copyright (c) 2011, Matti Johannes Eskelinen
 * All Rights Reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * Neither the name of the copyright holder nor the
 *     names of its contributors may be used to endorse or promote products
 *     derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL COPYRIGHT HOLDER BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
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
#include "cvsu_basic.h"

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
 * Stores a region of interest for integral_image.
 * Contains the values needed for calculating statistics in a given area.
 */
typedef struct integral_image_roi_t {
  /** Is this a valid roi? */
  uint32 valid;
  /** Offset from the beginning of data array */
  uint32 offset;
  /** Horizontal step to the right edge of the rectangle */
  uint32 hstep;
  /** Vertical step to the bottom edge of the rectangle */
  uint32 vstep;
  /** Number of elements in roi */
  uint32 N;
} integral_image_roi;

typedef struct integral_image_box_t {
  I_1_t *I_1_data;
  I_1_t *iA;
  I_1_t sum;
  I_2_t *I_2_data;
  I_2_t *i2A;
  I_2_t sumsqr;
  uint32 offset;
  uint32 B_inc;
  uint32 C_inc;
  uint32 D_inc;
  uint32 N;
  uint32 stride;
  uint32 dx;
  uint32 dy;
} integral_image_box;

typedef struct small_integral_image_box_t {
  SI_1_t *I_1_data;   /* address of the beginning of I */
  SI_1_t *iA;         /* address of the top left corner of box in I */
  SI_1_t sum;         /* calculated sum of box in I */
  SI_2_t *I_2_data;   /* address of the top left corner of box in I2 */
  SI_2_t *i2A;        /* address of the beginning of I2 */
  SI_2_t sumsqr;      /* calculated sum of box in I2 */
  uint32 offset;      /* offset from the beginning of I to top left corner */
  uint32 B_inc;       /* offset from top left corner to top right corner */
  uint32 C_inc;       /* offset from top left corner to bottom right corner */
  uint32 D_inc;       /* offset from top left corner to bottom left corner */
  uint32 N;           /* number of pixels contained in the box */
  uint32 step;        /* distance between integral image pixels */
  uint32 stride;      /* distance between integral image rows */
  uint32 dx;          /* x offset of integral image relative to original */
  uint32 dy;          /* y offset of integral image relative to original */
  uint32 channel;     /* x offset from start of pixel to selected channel */
} small_integral_image_box;

/**
 * Allocates an integral_image structure.
 * @see integral_image_free
 */
integral_image *integral_image_alloc();

/**
 * Frees an integral_image structure allocated with @see integral_image_alloc.
 */
void integral_image_free(
  integral_image *ptr
);

/**
 * Initializes the structure for an integral image and allocates the memory.
 * Does not calculate the integrals.
 * @see integral_image_update
 * @see integral_image_destroy
 */
result integral_image_create(
  integral_image *target,
  pixel_image *source
);

/**
 * Deallocates the memory allocated within the integral_image structure.
 * Does not free the structure pointer itself.
 * @see integral_image_create
 * @see integral_image_free
 */
result integral_image_destroy(
  integral_image *target
);

/**
 * Nullifies the contents of the integral_image. Does NOT deallocate memory.
 */
result integral_image_nullify(
  integral_image *target
);

/**
 * Transform the target integral_image into a clone of source integral_image.
 * Only the structure is cloned; for copying content, use the copy function.
 * @see copy_integral_image
 */
result integral_image_clone(
  integral_image *target,
  integral_image *source
);

/**
 * Copy the contents of the source integral_image into the target integral_image.
 * The two images must have the same structure.
 * @see clone_integral_image
 */
result integral_image_copy(
  integral_image *target,
  integral_image *source
);

/**
 * Updates the integral_image by calculating the integral and squared integral
 * of the source pixel_image.
 */
result integral_image_update(
  integral_image *target
);

/**
 * Produces a valid region of interest for the given integral_image. Takes into
 * account image dimensions and reduces the size of the region at the border.
 * May cause edge effects for some purposes.
 */
integral_image_roi integral_image_create_roi(
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
I_value integral_image_calculate_mean(
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
double integral_image_calculate_variance(
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
statistics integral_image_calculate_statistics(
  integral_image *target,
  sint32 x,
  sint32 y,
  sint32 width,
  sint32 height,
  uint32 offset
);

/**
 * Creates a small integral image with dimensions less than 256. This allows
 * using unsigned long values for both images.
 */

result small_integral_image_create(
    integral_image *target,
    pixel_image *source
);

/**
 * Updates the small integral image.
 */

result small_integral_image_update(
    integral_image *target
);

void integral_image_box_create(
    integral_image_box *target,
    integral_image *source,
/** box width */
    uint32 width,
/** box height */
    uint32 height,
/** x offset of integral image relative to original image */
    uint32 dx,
/** y offset of integral image relative to original image */
    uint32 dy
);

void integral_image_box_resize(
    integral_image_box *target,
    uint32 width,
    uint32 height
);

void integral_image_box_update(
    integral_image_box *target,
    uint32 x,
    uint32 y
);

/**
 * Create a box of particular size within a particular integral image.
 * The box can be used to facilitate the integral calculations of equal-sized
 * boxes within the same integral image.
 * The integral image may be calculated from a ROI. In this case the offset
 * between the original image and ROI can be given in dx and dy parameters.
 * The created box can be moved and its integral calculated with
 * @see small_integral_image_box_update
 */

void small_integral_image_box_create(
    small_integral_image_box *target,
    integral_image *source,
/** box width */
    uint32 width,
/** box height */
    uint32 height,
/** x offset of integral image relative to original image */
    uint32 dx,
/** y offset of integral image relative to original image */
    uint32 dy
);

/**
 * Changes only the size of the box.
 * Useful when dividing trees and only width and height change.
 */

void small_integral_image_box_resize(
    small_integral_image_box *target,
    uint32 width,
    uint32 height
);

/**
 * Moves the box by x and y and calculates the integrals.
 */

void small_integral_image_box_update(
    small_integral_image_box *target,
    uint32 x,
    uint32 y
);

#ifdef __cplusplus
}
#endif

#endif /* CVSU_INTEGRAL_H */
