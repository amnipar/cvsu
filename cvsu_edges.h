/**
 * @file cvsu_edges.h
 * @author Matti J. Eskelinen <matti.j.eskelinen@gmail.com>
 * @brief Edge detection and handling for cvsu.
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

#ifndef CVSU_EDGES_H
#   define CVSU_EDGES_H

#ifdef __cplusplus
extern "C" {
#endif

#include "cvsu_types.h"
#include "cvsu_pixel_image.h"
#include "cvsu_integral.h"
#include "cvsu_list.h"

/**
 * Pointer to a function that calculates edgel strength based on sums acquired
 * from integral images.
 */
typedef integral_value (*edgel_criterion_calculator)
(
  integral_value N,
  integral_value sum1,
  integral_value sum2,
  integral_value sumsqr1,
  integral_value sumsqr2
);

/**
 * Stores an edge image.
 * Is based on an integral image, which is used to calculate edge responses
 * along horizontal and vertical scanlines.
 *
 * hedges stores horizontal edge responses from the image; the image has the
 * same height as the original image.
 * the results are stored column-wise in the image, with box_length margin on cols.
 *
 * vedges stores vertical edge responses from the image; the image has the
 * same width as the original image.
 * the results are stored row-wise in the image, with box_length margin on rows.
 */
typedef struct edge_image_t {
    integral_image I;
    pixel_image hedges;
    pixel_image vedges;
    uint32 width;
    uint32 height;
    uint32 hstep;
    uint32 vstep;
    uint32 hmargin;
    uint32 vmargin;
    uint32 box_width;
    uint32 box_length;
    uint32 dx;
    uint32 dy;
} edge_image;

/**
 * Allocates memory for an edge_image structure.
 */
edge_image *edge_image_alloc();

/**
 * Frees the memory allocated for an edge_image structure allocated with
 * @see edge_image_alloc. Will also destroy the structure properly using
 * @see edge_image_destroy.
 */
void edge_image_free
(
  edge_image *target
);

/**
 * Creates an edge image by using the source image to create an integral image.
 * Then creates the edge images and initializes the variables.
 */
result edge_image_create
(
  edge_image *target,
  pixel_image *source,
  uint32 hstep,
  uint32 vstep,
  uint32 hmargin,
  uint32 vmargin,
  uint32 box_width,
  uint32 box_length
);

/**
 * Destroys an edge_image structure and deallocates all memory.
 */
result edge_image_destroy
(
  edge_image *target
);

/**
 * Initializes the contents of an edge_image to null. Does not deallocate data,
 * @see edge_image_destroy should be used for initialized structures before
 * calling this.
 */
result edge_image_nullify
(
  edge_image *target
);

/**
 * Everything that can be nullified should be able to tell if it is null.
 */
truth_value edge_image_is_null
(
  edge_image *target
);

/**
 * Transform the dst edge_image into a clone of src edge_image.
 * Only the structure is cloned, for copying content, use the copy function.
 * @see copy_edge_image
 */
result edge_image_clone
(
  edge_image *dst,
  edge_image *src
);

/**
 * Copy the content of src edge_image into dst edge_image.
 * The two images must have the same structure.
 * @see clone_edge_image
 */
result edge_image_copy
(
  edge_image *dst,
  edge_image *src
);

/**
 * Calculates edge image using the integral images and signed Fisher criterion
 */
result edge_image_update
(
  edge_image *target
);

/**
 * Generates an 8-bit greyscale image from the edge image
 */
result edge_image_convert_to_grey8
(
  edge_image *source,
  pixel_image *temp,
  pixel_image *target
);

/**
 * Overlays the edge values over an 8-bit greyscale image.
 */
result edge_image_overlay_to_grey8
(
  edge_image *source,
  pixel_image *target
);

/**
 * Edgel strength measure using normal Fisher criterion.
 * In this criterion, the difference of means is squared, and divided by the
 * sum of variances. This is the 'normal' Fisher criterion for distance
 * between classes; in this case, we measure the separation of image regions by
 * the difference of mean intensity values and the variance of the values.
 */
integral_value edgel_fisher_unsigned
(
  integral_value N,
  integral_value sum1,
  integral_value sum2,
  integral_value sumsqr1,
  integral_value sumsqr2
);

/**
 * Edgel strength measure using Fisher-like criterion with sqrt of variance.
 * In this criterion, the difference of means is not squared, so the direction
 * of change is preserved. To compensate, the square root of the sum of
 * variances is used below the line.
 */
integral_value edgel_fisher_signed
(
  integral_value N,
  integral_value sum1,
  integral_value sum2,
  integral_value sumsqr1,
  integral_value sumsqr2
);

/**
 * Calculates edge response using box filters with given criterion
 * Accepts integral image as src, S32 image as dst
 * Box filter size is defined with hsize and vsize
 * The criterion used for determining edgel strength using the box filter sums
 * from integral images can be given as parameter.
 */
result edgel_response_x
(
  integral_image *I,
  pixel_image *target,
  uint32 hsize,
  uint32 vsize,
  edgel_criterion_calculator criterion
);

/**
 * Finds edges in horizontal direction using deviation of box filters
 * Calculates first the integral images
 * Uses integral images to calculate efficiently edge responses
 * Finds extrema of edge responses
 * Normalizes to 8-bit greyscale image
 */
result edges_x_box_deviation
(
  /** Initialized integral image containing source image */
  integral_image *I,
  /** 32-bit integer (p_S32) image for temporary data */
  pixel_image *temp,
  /** 8-bit (p_U8) image for final result */
  pixel_image *target,
  /** Horizontal size of the box filter */
  uint32 hsize,
  /** Vertical size of the box filter */
  uint32 vsize
);

#ifdef __cplusplus
}
#endif

#endif /* CVSU_EDGES_H */
