/**
 * @file cv_edges.h
 * @author Matti Eskelinen (matti dot j dot eskelinen at jyu dot fi)
 * @brief Edge detection and handling for the cv module.
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

#ifndef CV_EDGES_H
#   define CV_EDGES_H

#ifdef __cplusplus
extern "C" {
#endif

#include "cv_basic.h"

typedef struct edge_image_t {
    integral_image integral;
    pixel_image hedges;
    pixel_image vedges;
    long width;
    long height;
    long hstep;
    long vstep;
    long hmargin;
    long vmargin;
    long box_width;
    long box_length;
    long dx;
    long dy;
} edge_image;

/**
 *
 */

result create_edge_image(edge_image *dst, pixel_image *src, long width, long height, long hmargin, long vmargin, long box_width, long box_length);

/**
 *
 */

result destroy_edge_image(edge_image *dst);

/**
 * Transform the dst edge_image into a clone of src edge_image.
 * Only the structure is cloned, for copying content, use the copy function.
 * @see copy_edge_image
 */

result clone_edge_image(edge_image *dst, edge_image *src);

/**
 * Copy the content of src edge_image into dst edge_image.
 * The two images must have the same structure.
 * @see clone_edge_image
 */

result copy_edge_image(edge_image *dst, edge_image *src);

/**
 * Calculates edge response using box filters and deviation
 * Accepts S32 image as integral, F64 image as integral2, S32 image as dst
 */

result edgel_response_x(integral_image *src, pixel_image *dst, long hsize, long vsize);

/**
 *
 */

result edges_x_box_deviation(integral_image *src, pixel_image *temp, pixel_image *dst, long hsize, long vsize);

/**
 *
 */

result calculate_edges(edge_image *edge);

#ifdef __cplusplus
}
#endif

#endif // CV_EDGES_H
