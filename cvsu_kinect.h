/**
 * @file cvsu_kinect.h
 * @author Matti Eskelinen (matti dot j dot eskelinen at jyu dot fi)
 * @brief Operations for handling kinect data with the cvsu module.
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

#ifndef CVSU_KINECT_H
#   define CVSU_KINECT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "cvsu_basic.h"

result convert_grey8_to_radar(const pixel_image *source, pixel_image *target);

/**
 * Transforms a 2D kinect depth image into a 3D pointcloud.
 * Accepts a word image (16-bit greyscale) as src and interprets it as depth
 * Accepts a 3-channel double image as dst and stores there the point
 * coordinates in 3D space (x,y,z)
 */
result depth16_to_pointcloud(pixel_image *src, pixel_image *dst);

/**
 * Projects a 3D pointcloud to XZ-plane, resulting in a top-down radar view.
 * Accepts a 3-channel double image as src and interprets it as pointcloud.
 * Accepts a 1-channel byte image as dst and creates a greyscale image in it.
 */
result pointcloud_to_radar_top(pixel_image *src, pixel_image *dst);

#ifdef __cplusplus
}
#endif

#endif /* CVSU_KINECT_H */
