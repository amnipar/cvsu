/**
 * @file cvsu_opencv.c
 * @author Matti J. Eskelinen <matti.j.eskelinen@gmail.com>
 * @brief Interfaces for OpenCV and IplImage type.
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

#include "cvsu_config.h"
#include "cvsu_macros.h"
#include "cvsu_opencv.h"
#include <opencv2/highgui/highgui_c.h>
#include <stdio.h>

string pixel_image_create_from_ipl_image_name = "pixel_image_create_from_ipl_image";
string ipl_image_create_from_pixel_image_name = "ipl_image_create_from_pixel_image";
string pixel_image_create_from_file_name = "pixel_image_create_from_file";
string pixel_image_write_to_file_name = "pixel_image_write_to_file";

/******************************************************************************/

result pixel_image_create_from_ipl_image(pixel_image *target, IplImage *source, pixel_format format)
{
    TRY();
    int depth;
    int channels;
    pixel_type type;

    CHECK_POINTER(target);
    CHECK_POINTER(source);
    CHECK_PARAM(format == RGB || format == GREY);

    channels = source->nChannels;

    if (format == RGB) {
        CHECK_PARAM(channels == 3);
    }
    else {
        CHECK_PARAM(channels == 1);
    }

    depth = source->depth;
    switch (depth) {
        case IPL_DEPTH_8U:
            type = p_U8;
            break;
        case IPL_DEPTH_8S:
            type = p_S8;
            break;
        case IPL_DEPTH_16U:
            type = p_U16;
            break;
        case IPL_DEPTH_16S:
            type = p_S16;
            break;
        case IPL_DEPTH_32S:
            type = p_S32;
            break;
        case IPL_DEPTH_32F:
            type = p_F32;
            break;
        case IPL_DEPTH_64F:
            type = p_F64;
            break;
        default:
            ERROR(BAD_TYPE);
    }

    CHECK(pixel_image_create_from_data(target, source->imageData, type, format,
            source->width, source->height, channels, source->widthStep));

    FINALLY(pixel_image_create_from_ipl_image);
    RETURN();
}

/******************************************************************************/

result ipl_image_create_from_pixel_image
(
  IplImage **target,
  pixel_image *source,
  pixel_format format
)
{
  TRY();
  IplImage *temp;
  CvSize size;
  
  CHECK_POINTER(source);
  CHECK_PARAM(source->type == p_U8);
  CHECK_PARAM(source->format == RGB || source->format == GREY);
  
  size.width = (signed)source->width;
  size.height = (signed)source->height;
  switch (source->format) {
    case RGB:
      temp = cvCreateImageHeader(size, IPL_DEPTH_8U, 3);
      break;
    case GREY:
      temp = cvCreateImageHeader(size, IPL_DEPTH_8U, 1);
      break;
    default:
      temp = NULL;
  }
  
  cvSetData(temp, source->data, (signed)source->stride);

  *target = cvCloneImage(temp);
  
  FINALLY(ipl_image_create_from_pixel_image);
  cvReleaseImageHeader(&temp);
  RETURN();
}

/******************************************************************************/

result pixel_image_create_from_file
(
  pixel_image *target,
  const char *filename,
  pixel_type type,
  pixel_format format
)
{
  TRY();
  int depth;
  int channels;
  IplImage *src, *dst;

  CHECK_POINTER(target);
  CHECK_PARAM(format == RGB || format == GREY);
  CHECK_PARAM(type == p_U8);

  if (format == RGB) {
    src = cvLoadImage(filename, CV_LOAD_IMAGE_COLOR);
    channels = 3;
  }
  else {
    src = cvLoadImage(filename, CV_LOAD_IMAGE_GRAYSCALE);
    channels = 1;
  }
  CHECK_POINTER(src);
  CHECK_POINTER(src->imageData);

  /*
  switch (type) {
    case p_U8:
      depth = IPL_DEPTH_8U;
      break;
    case p_S8:
      depth = IPL_DEPTH_8S;
      break;
    case p_U16:
      depth = IPL_DEPTH_16U;
      break;
    case p_S16:
      depth = IPL_DEPTH_16S;
      break;
    case p_S32:
      depth = IPL_DEPTH_32S;
      break;
    case p_F32:
      depth = IPL_DEPTH_32F;
      break;
    case p_F64:
      depth = IPL_DEPTH_64F;
      break;
    default:
      ERROR(BAD_TYPE);
  }

  dst = cvCreateImage(cvGetSize(src),depth,channels);
  */
  /*printf("width=%d,height=%d,step=%d\n",src->width,src->height,src->widthStep);*/
  CHECK(pixel_image_create_from_data(target, src->imageData, type, format,
                                     src->width, src->height, channels, src->widthStep));

  FINALLY(pixel_image_create_from_file);
  cvReleaseImageHeader(&src);
  RETURN();
}

/******************************************************************************/

result pixel_image_write_to_file(pixel_image *source, const char *filename)
{
  TRY();
  IplImage *dst;
  CvSize size;
  
  CHECK_POINTER(source);
  CHECK_PARAM(source->type == p_U8);
  CHECK_PARAM(source->format == RGB || source->format == GREY);
  
  size.width = (signed)source->width;
  size.height = (signed)source->height;
  switch (source->format) {
    case RGB:
      dst = cvCreateImageHeader(size, IPL_DEPTH_8U, 3);
      break;
    case GREY:
      dst = cvCreateImageHeader(size, IPL_DEPTH_8U, 1);
      break;
    default:
      dst = NULL;
  }
  
  /*printf("width=%ul,height=%ul,step=%ul\n",source->width,source->height,source->stride);*/
  cvSetData(dst, source->data, (signed)source->stride);
  cvSaveImage(filename, dst, 0);
  
  FINALLY(pixel_image_write_to_file);
  RETURN();
}
