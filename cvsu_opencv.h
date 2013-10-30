/**
 * @file cvsu_opencv.h
 * @author Matti J. Eskelinen <matti.j.eskelinen@gmail.com>
 * @brief Interfaces for selected OpenCV functions and IplImage type.
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

#ifndef CVSU_OPENCV_H
#   define CVSU_OPENCV_H

#ifdef __cplusplus
extern "C" {
#endif

#include <opencv2/core/core_c.h>
#include "cvsu_pixel_image.h"
#include "cvsu_list.h"
#include "cvsu_annotation.h"

/**
 * Convert from pixel_image to IplImage.
 */
result pixel_image_create_from_ipl_image
(
  pixel_image *target,
  IplImage *source,
  pixel_format format
);

/**
 * Convert from IplImage to pixel_image.
 */
result ipl_image_create_from_pixel_image
(
  IplImage **target,
  pixel_image *source,
  pixel_format format
);

/**
 * Read pixel_image from file using OpenCV and IplImage in-between.
 */
result pixel_image_create_from_file
(
  pixel_image *target,
  const char *filename,
  pixel_type type,
  pixel_format format
);

/**
 * Write pixel_image to a file using OpenCV and IplImage in-between.
 */
result pixel_image_write_to_file
(
  pixel_image *source,
  const char *filename
);

/**
 * Draw a list of lines over an image using OpenCV cvLine function.
 */
result pixel_image_draw_lines
(
  pixel_image *source,
  list *lines,
  byte color[4],
  uint32 width
);

/**
 * Draws a list of weighted lines multiplying the given color values with the
 * weight. Uses OpenCV cvLine function for drawing.
 */
result pixel_image_draw_weighted_lines
(
  pixel_image *source,
  list *lines,
  byte color[4],
  uint32 width
);

/**
 * Draws a list of colored lines using the line color values.
 * Uses OpenCV cvLine function for drawing.
 */
result pixel_image_draw_colored_lines
(
  pixel_image *source,
  list *lines,
  uint32 width
);

result pixel_image_draw_circles
(
  pixel_image *source,
  list *circles,
  byte color[4],
  uint32 width
);

result pixel_image_draw_arcs
(
  pixel_image *source,
  list *arcs,
  byte color[4],
  uint32 width
);

result pixel_image_draw_colored_arcs
(
  pixel_image *source,
  list *arcs,
  uint32 width
);

/**
 * Draws a list of rects over an image using OpenCV cvRectangle function.
 */
result pixel_image_draw_rects
(
  pixel_image *source,
  list *rects,
  byte color[4],
  uint32 width
);

/**
 * Draws a list of colored rects over an image using OpenCV cvRectangle function.
 */
result pixel_image_draw_colored_rects
(
  pixel_image *source,
  list *rects,
  uint32 width
);

/**
 * Dumps the image into a file using 'capture/[timestamp].png' as file name
 */
result pixel_image_dump
(
  pixel_image *source
);

#ifdef __cplusplus
}
#endif

#endif  /* CVSU_OPENCV_H */
