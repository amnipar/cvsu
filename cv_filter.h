/**
 * @file cv_filter.h
 * @author Matti Eskelinen (matti dot j dot eskelinen at jyu dot fi)
 * @brief Basic image filters for the cv module.
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

#ifndef CV_FILTER_H
#   define CV_FILTER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "cv_basic.h"

result threshold(pixel_image *src, pixel_image *dst, byte t);

result smooth_binomial(const pixel_image *src, pixel_image *dst, int passes);

/*
sobel_x filters image with horizontal sobel operator
abs_sobel_x does the same using absolute values

accepts U8 image as src and S32 image as dst
images must have same dimensions
*/
result sobel_x(const pixel_image *src, pixel_image *dst);
result abs_sobel_x(const pixel_image *src, pixel_image *dst);

/*
extrema_x calculates the extremal values along horizontal scanlines

accepts S32 image as src and S32 image as dst
*/
result extrema_x(const pixel_image *src, pixel_image *dst);
result extrema_y(const pixel_image *src, pixel_image *dst);

#ifdef __cplusplus
}
#endif

#endif // CV_FILTER_H
