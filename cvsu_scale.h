/**
 * @file cvsu_scale.h
 * @author Matti J. Eskelinen <matti.j.eskelinen@gmail.com>
 * @brief Scale-space handling and operations for the cvsu module.
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

#ifndef CVSU_SCALE_H
#   define CVSU_SCALE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "cvsu_types.h"
#include "cvsu_basic.h"

/**
 * Tracks the state of the pyramid to determine, which operations are possible.
 */

typedef enum pyramid_state_t {
    INIT = 0,
    DOWN,
    UP,
    INVALID
} pyramid_state;

/**
 * Stores an image pyramid as an array of images.
 * Each level is stored in full size to allow down- and upscaling.
 */

typedef struct image_pyramid_t {
    pixel_image *source;
    pixel_image *levels;
    pixel_image *roi;
    uint32 level_count;
    uint32 width;
    uint32 height;
    uint32 step;
    uint32 stride;
    pyramid_state state;
} image_pyramid;

/**
 * Allocates an array for storing an image pyramid.
 * The source image is deep copied to the first level of the pyramid.
 * Pyramid itself is not generated at this point, only the data structure is
 * initialized. Use @see image_pyramid_down to generate the pyramid by
 * downscaling the image at the first level.
 * @note A pointer to source image is stored for reference, but the object is
 * not owned and it will not be destroyed.
 */
result image_pyramid_create(
    image_pyramid *target,
    pixel_image *source,
    uint32 levels
);

/**
 * Deallocates the memory for storing the image pyramid.
 * @note The source image is not destroyed, but the pointer is set to NULL.
 */
result image_pyramid_destroy(
    image_pyramid *target
);

/**
 * Clones the structure of an image pyramid.
 * Does not copy the data.
 * @see image_pyramid_copy
 */

result image_pyramid_clone(
    image_pyramid *target,
    image_pyramid *source
);

/**
 * Copies the data from one image pyramid to another.
 * The two image pyramids must have the same structure.
 * @see image_pyramid_clone
 */

result image_pyramid_copy(
    image_pyramid *target,
    image_pyramid *source
);

/**
 * Updates an image pyramid by scaling the levels down.
 * Takes the image in first position
 * Smoothes with binomial filter
 * Scales down step by step to next positions
 */

result image_pyramid_down(
    image_pyramid *target
);

/**
 * Updates an image pyramid by scaling the levels up.
 * Takes the down-scaled images and up-scales to original size.
 */

result image_pyramid_up(
    image_pyramid *target
);

/**
 * Creates an image that contains the maximal values occurring in scale space.
 */

result image_pyramid_max(
    image_pyramid *pyramid,
    pixel_image *target
);

/**
 * Creates an image that contains the minimal values occurring in scale space.
 */

result image_pyramid_min(
    image_pyramid *pyramid,
    pixel_image *target
);

/**
 * Finds edges in horizontal direction using sobel operator in scale space.
 * Applies sobel operator to each scale, taking the absolute value.
 * Scales the pyramid images up to normal scale.
 * Finds the minimal values across scales.
 * Thresholds the images to reveal edge locations.
 * @param pyramid Initialized image pyramid containing source image
 * @param temp 32-bit integer (p_S32) image for temporary data
 * @param dst 8-bit (p_U8) image for final result
 * @param t Threshold value for final result
 */

result edges_x_sobel_scale(
    image_pyramid *pyramid,
    pixel_image *temp,
    pixel_image *target,
    byte t
);

#ifdef __cplusplus
}
#endif

#endif /* CVSU_SCALE_H */
