/**
 * @file cv_scale.h
 * @author Matti Eskelinen (matti dot j dot eskelinen at jyu dot fi)
 * @brief Scale-space handling and operations for the cv module.
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

#ifndef CV_SCALE_H
#   define CV_SCALE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "cv_basic.h"

typedef enum pyramid_state_t {
    INIT = 0,
    DOWN,
    UP,
    INVALID
} pyramid_state;

typedef struct image_pyramid_t {
    pixel_image *original;
    pixel_image *levels;
    long level_count;
    long width;
    long height;
    long step;
    pyramid_state state;
} image_pyramid;

result create_image_pyramid(image_pyramid *dst, pixel_image *src, long levels);
result destroy_image_pyramid(image_pyramid *dst);

result clone_image_pyramid(image_pyramid *dst, image_pyramid *src);
result copy_image_pyramid(image_pyramid *dst, image_pyramid *src);

result pyramid_down(image_pyramid *dst);
result pyramid_up(image_pyramid *dst);

result pyramid_max(image_pyramid *pyramid, pixel_image *dst);
result pyramid_min(image_pyramid *pyramid, pixel_image *dst);

result edges_x_sobel_scale(image_pyramid *pyramid, pixel_image *temp, pixel_image *dst, byte t);

#ifdef __cplusplus
}
#endif

#endif // CV_SCALE_H
