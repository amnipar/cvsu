/**
 * @file cvsu_opencv.c
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

#include "cvsu_config.h"
#include "cvsu_macros.h"
#include "cvsu_opencv.h"
#include "cvsu_typed_pointer.h"
#include "cvsu_attribute.h"
#include <opencv2/highgui/highgui_c.h>
#include <stdio.h>
#include <sys/time.h>

/******************************************************************************/
/* constants for reporting function names in error messages                   */

string pixel_image_create_from_ipl_image_name = "pixel_image_create_from_ipl_image";
string ipl_image_create_from_pixel_image_name = "ipl_image_create_from_pixel_image";
string pixel_image_create_from_file_name = "pixel_image_create_from_file";
string pixel_image_write_to_file_name = "pixel_image_write_to_file";
string pixel_image_draw_lines_name = "pixel_image_draw_lines";
string pixel_image_draw_weighted_lines_name = "pixel_image_draw_weighted_lines";
string pixel_image_draw_colored_lines_name = "pixel_image_draw_colored_lines";
string pixel_image_draw_circles_name = "pixel_image_draw_circles";
string pixel_image_draw_colored_circles_name = "pixel_image_draw_colored_circles";
string pixel_image_draw_arcs_name = "pixel_image_draw_arcs";
string pixel_image_draw_colored_arcs_name = "pixel_image_draw_colored_arcs";
string pixel_image_draw_rects_name = "pixel_image_draw_rects";
string pixel_image_draw_colored_rects_name = "pixel_image_draw_colored_rects";
string pixel_image_dump_name = "pixel_image_dump";
string graph_draw_nodes_name = "graph_draw_nodes";

/******************************************************************************/

result pixel_image_create_from_ipl_image
(
  pixel_image *target,
  IplImage *source,
  pixel_format format
)
{
  TRY();
  uint32 depth, channels;
  pixel_type type;

  CHECK_POINTER(target);
  CHECK_POINTER(source);
  CHECK_PARAM(format == RGBA || format == RGB || format == GREY);

  channels = ((unsigned)source->nChannels);

  if (format == RGBA) {
    CHECK_PARAM(channels == 4);
  }
  else
  if (format == RGB) {
    CHECK_PARAM(channels == 3);
  }
  else {
    CHECK_PARAM(channels == 1);
  }

  depth = ((unsigned)source->depth);
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

  CHECK(pixel_image_create_from_data(target, ((void*)source->imageData), type,
                                     format, ((unsigned)source->width),
                                     ((unsigned)source->height), channels,
                                     ((unsigned)source->widthStep)));

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
  int depth, channels;

  CHECK_POINTER(source);
  CHECK_PARAM(source->type == p_U8);
  CHECK_PARAM(source->format == RGBA || source->format == RGB || source->format == GREY);

  size.width = (signed)source->width;
  size.height = (signed)source->height;

  /* TODO: convert format (and type) based on parameters */
  /* this reflects the 'expected' type and format at the calling side */
  (void)format;
  switch (source->type) {
    case p_U8:
      depth = (int)IPL_DEPTH_8U;
      break;
    case p_S8:
      depth = (int)IPL_DEPTH_8S;
      break;
    case p_U16:
      depth = (int)IPL_DEPTH_16U;
      break;
    case p_S16:
      depth = (int)IPL_DEPTH_16S;
      break;
    case p_S32:
      depth = (int)IPL_DEPTH_32S;
      break;
    case p_F32:
      depth = (int)IPL_DEPTH_32F;
      break;
    case p_F64:
      depth = (int)IPL_DEPTH_64F;
      break;
    default:
      ERROR(BAD_TYPE);
  }
  switch (source->format) {
    case RGBA:
      channels = 4;
    case RGB:
      channels = 3;
      break;
    case GREY:
      channels = 1;
      break;
    default:
      ERROR(BAD_TYPE);
  }
  temp = cvCreateImageHeader(size, depth, channels);
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
  uint32 /*depth,*/ channels;
  IplImage *src; /*, *dst;*/

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
  CHECK(pixel_image_create_from_data(target, ((void*)src->imageData), type,
                                     format, ((unsigned)src->width),
                                     ((unsigned)src->height), channels,
                                     ((unsigned)src->widthStep)));

  FINALLY(pixel_image_create_from_file);
  cvReleaseImage(&src);
  RETURN();
}

/******************************************************************************/

result pixel_image_write_to_file
(
  pixel_image *source,
  const char *filename
)
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

  cvSetData(dst, source->data, (signed)source->stride);
  cvSaveImage(filename, dst, 0);

  FINALLY(pixel_image_write_to_file);
  cvReleaseImageHeader(&dst);
  RETURN();
}

/******************************************************************************/

result pixel_image_draw_lines
(
  pixel_image *source,
  list *lines,
  byte color[4],
  uint32 width
)
{
  TRY();
  IplImage *dst;
  CvSize size;
  list_item *items, *end;
  line *this_line;
  int channels;

  CHECK_POINTER(source);
  CHECK_POINTER(lines);
  CHECK_PARAM(source->type == p_U8);
  CHECK_PARAM(source->format == RGB || source->format == RGBA);
  if (source->format == RGB) {
    CHECK_PARAM(source->step == 3);
    channels = 3;
  }
  else
  if (source->format == RGBA) {
    CHECK_PARAM(source->step == 4);
    channels = 4;
  }

  size.width = (signed)source->width;
  size.height = (signed)source->height;
  dst = cvCreateImageHeader(size, IPL_DEPTH_8U, channels);
  cvSetData(dst, source->data, (signed)source->stride);

  items = lines->first.next;
  end = &lines->last;
  while (items != end) {
    this_line = (line*)items->data;
    cvLine(dst,
           cvPoint((int)this_line->start.x, (int)this_line->start.y),
           cvPoint((int)this_line->end.x, (int)this_line->end.y),
           cvScalar(color[0], color[1], color[2], 0),(int) width, 8, 0);
    items = items->next;
  }

  FINALLY(pixel_image_draw_lines);
  cvReleaseImageHeader(&dst);
  RETURN();
}

/******************************************************************************/

result pixel_image_draw_weighted_lines
(
  pixel_image *source,
  list *lines,
  byte color[4],
  uint32 width
)
{
  TRY();
  IplImage *dst;
  CvSize size;
  int channels;
  list_item *items, *end;
  weighted_line *this_line;
  float weight;

  CHECK_POINTER(source);
  CHECK_POINTER(lines);
  CHECK_PARAM(source->type == p_U8);
  CHECK_PARAM(source->format == RGB || source->format == RGBA);
  if (source->format == RGB) {
    CHECK_PARAM(source->step == 3);
    channels = 3;
  }
  else
  if (source->format == RGBA) {
    CHECK_PARAM(source->step == 4);
    channels = 4;
  }

  size.width = (signed)source->width;
  size.height = (signed)source->height;
  dst = cvCreateImageHeader(size, IPL_DEPTH_8U, channels);
  cvSetData(dst, source->data, (signed)source->stride);

  items = lines->first.next;
  end = &lines->last;
  while (items != end) {
    this_line = (weighted_line*)items->data;
    weight = (float)this_line->weight;
    cvLine(dst,
           cvPoint((int)this_line->start.x, (int)this_line->start.y),
           cvPoint((int)this_line->end.x, (int)this_line->end.y),
           cvScalar(weight * (float)color[0], weight * (float)color[1], weight * (float)color[2], 0),
           (int)width, 8, 0);
    items = items->next;
  }

  FINALLY(pixel_image_draw_weighted_lines);
  cvReleaseImageHeader(&dst);
  RETURN();
}

/******************************************************************************/

result pixel_image_draw_colored_lines
(
  pixel_image *source,
  list *lines,
  uint32 width
)
{
  TRY();
  IplImage *dst;
  CvSize size;
  int channels;
  list_item *items, *end;
  colored_line *this_line;

  CHECK_POINTER(source);
  CHECK_POINTER(lines);
  CHECK_PARAM(source->type == p_U8);
  CHECK_PARAM(source->format == RGB || source->format == RGBA);
  if (source->format == RGB) {
    CHECK_PARAM(source->step == 3);
    channels = 3;
  }
  else
  if (source->format == RGBA) {
    CHECK_PARAM(source->step == 4);
    channels = 4;
  }

  size.width = (signed)source->width;
  size.height = (signed)source->height;
  dst = cvCreateImageHeader(size, IPL_DEPTH_8U, channels);
  cvSetData(dst, source->data, (signed)source->stride);

  items = lines->first.next;
  end = &lines->last;
  while (items != end) {
    this_line = (colored_line*)items->data;
    cvLine(dst,
           cvPoint((int)this_line->start.x, (int)this_line->start.y),
           cvPoint((int)this_line->end.x, (int)this_line->end.y),
           cvScalar(this_line->color[0], this_line->color[1], this_line->color[2], 0),
           (int)width, 8, 0);
    items = items->next;
  }

  FINALLY(pixel_image_draw_colored_lines);
  cvReleaseImageHeader(&dst);
  RETURN();
}

/******************************************************************************/

result pixel_image_draw_circles
(
  pixel_image *source,
  list *circles,
  byte color[4],
  uint32 width
)
{
  TRY();
  IplImage *dst;
  CvSize size;
  int channels;
  list_item *items, *end;
  circle *this_circle;

  CHECK_POINTER(source);
  CHECK_POINTER(circles);
  CHECK_PARAM(source->type == p_U8);
  CHECK_PARAM(source->format == RGB || source->format == RGBA);
  if (source->format == RGB) {
    CHECK_PARAM(source->step == 3);
    channels = 3;
  }
  else
  if (source->format == RGBA) {
    CHECK_PARAM(source->step == 4);
    channels = 4;
  }

  size.width = (signed)source->width;
  size.height = (signed)source->height;
  dst = cvCreateImageHeader(size, IPL_DEPTH_8U, channels);
  cvSetData(dst, source->data, (signed)source->stride);

  items = circles->first.next;
  end = &circles->last;
  while (items != end) {
    this_circle = (circle*)items->data;
    cvCircle(dst,
             cvPoint((int)this_circle->center.x, (int)this_circle->center.y),
             (int)this_circle->r,
             cvScalar(color[0], color[1], color[2], 0), (int)width, 8, 0);
    items = items->next;
  }

  FINALLY(pixel_image_draw_circles);
  cvReleaseImageHeader(&dst);
  RETURN();
}

/******************************************************************************/

result pixel_image_draw_arcs
(
  pixel_image *source,
  list *arcs,
  byte color[4],
  uint32 width
)
{
  TRY();
  IplImage *dst;
  CvSize size, axes;
  int channels;
  list_item *items, *end;
  arc *this_arc;

  CHECK_POINTER(source);
  CHECK_POINTER(arcs);
  CHECK_PARAM(source->type == p_U8);
  CHECK_PARAM(source->format == RGB || source->format == RGBA);
  if (source->format == RGB) {
    CHECK_PARAM(source->step == 3);
    channels = 3;
  }
  else
  if (source->format == RGBA) {
    CHECK_PARAM(source->step == 4);
    channels = 4;
  }

  size.width = (signed)source->width;
  size.height = (signed)source->height;
  dst = cvCreateImageHeader(size, IPL_DEPTH_8U, channels);
  cvSetData(dst, source->data, (signed)source->stride);

  items = arcs->first.next;
  end = &arcs->last;
  while (items != end) {
    this_arc = (arc*)items->data;
    axes.width = axes.height = (int)this_arc->r;
    cvEllipse(dst,
              cvPoint((int)this_arc->center.x, (int)this_arc->center.y),
              axes, 0,
              -this_arc->start_angle,
              -this_arc->end_angle,
              cvScalar(color[0], color[1], color[2], 0), (int)width, 8, 0);
    items = items->next;
  }

  FINALLY(pixel_image_draw_arcs);
  cvReleaseImageHeader(&dst);
  RETURN();
}

/******************************************************************************/

result pixel_image_draw_colored_arcs
(
  pixel_image *source,
  list *arcs,
  uint32 width
)
{
  TRY();
  IplImage *dst;
  CvSize size, axes;
  int channels;
  list_item *items, *end;
  colored_arc *this_arc;

  CHECK_POINTER(source);
  CHECK_POINTER(arcs);
  CHECK_PARAM(source->type == p_U8);
  CHECK_PARAM(source->format == RGB || source->format == RGBA);
  if (source->format == RGB) {
    CHECK_PARAM(source->step == 3);
    channels = 3;
  }
  else
  if (source->format == RGBA) {
    CHECK_PARAM(source->step == 4);
    channels = 4;
  }

  size.width = (signed)source->width;
  size.height = (signed)source->height;
  dst = cvCreateImageHeader(size, IPL_DEPTH_8U, channels);
  cvSetData(dst, source->data, (signed)source->stride);

  items = arcs->first.next;
  end = &arcs->last;
  while (items != end) {
    this_arc = (colored_arc*)items->data;
    axes.width = axes.height = (int)this_arc->r;
    cvEllipse(dst,
              cvPoint((int)this_arc->center.x, (int)this_arc->center.y),
              axes, 0,
              -this_arc->start_angle,
              -this_arc->end_angle,
              cvScalar(this_arc->color[0], this_arc->color[1], this_arc->color[2], 0),
              (int)width, 8, 0);
    items = items->next;
  }

  FINALLY(pixel_image_draw_colored_arcs);
  cvReleaseImageHeader(&dst);
  RETURN();
}

/******************************************************************************/

result pixel_image_draw_rects
(
  pixel_image *source,
  list *rects,
  byte color[4],
  uint32 width
)
{
  TRY();
  IplImage *dst;
  CvSize size;
  int channels;
  list_item *items, *end;
  rect *this_rect;

  CHECK_POINTER(source);
  CHECK_POINTER(rects);
  CHECK_PARAM(source->type == p_U8);
  CHECK_PARAM(source->format == RGB || source->format == RGBA);
  if (source->format == RGB) {
    CHECK_PARAM(source->step == 3);
    channels = 3;
  }
  else
  if (source->format == RGBA) {
    CHECK_PARAM(source->step == 4);
    channels = 4;
  }

  size.width = (signed)source->width;
  size.height = (signed)source->height;
  dst = cvCreateImageHeader(size, IPL_DEPTH_8U, channels);
  cvSetData(dst, source->data, (signed)source->stride);

  items = rects->first.next;
  end = &rects->last;
  while (items != end) {
    this_rect = (rect*)items->data;
    cvRectangle(dst,
                cvPoint((int)this_rect->left, (int)this_rect->top),
                cvPoint((int)this_rect->right, (int)this_rect->bottom),
                cvScalar(color[0], color[1], color[2], 0), (int)width, 8, 0);
                items = items->next;
  }

  FINALLY(pixel_image_draw_rects);
  cvReleaseImageHeader(&dst);
  RETURN();
}

/******************************************************************************/

result pixel_image_draw_colored_rects
(
  pixel_image *source,
  list *rects,
  uint32 width
)
{
  TRY();
  IplImage *dst;
  CvSize size;
  int channels;
  list_item *items, *end;
  colored_rect *this_rect;

  CHECK_POINTER(source);
  CHECK_POINTER(rects);
  CHECK_PARAM(source->type == p_U8);
  CHECK_PARAM(source->format == RGB || source->format == RGBA);
  if (source->format == RGB) {
    CHECK_PARAM(source->step == 3);
    channels = 3;
  }
  else
  if (source->format == RGBA) {
    CHECK_PARAM(source->step == 4);
    channels = 4;
  }

  size.width = (signed)source->width;
  size.height = (signed)source->height;
  dst = cvCreateImageHeader(size, IPL_DEPTH_8U, channels);
  cvSetData(dst, source->data, (signed)source->stride);

  items = rects->first.next;
  end = &rects->last;
  while (items != end) {
    this_rect = (colored_rect*)items->data;
    cvRectangle(dst,
                cvPoint((int)this_rect->left, (int)this_rect->top),
                cvPoint((int)this_rect->right, (int)this_rect->bottom),
                cvScalar(this_rect->color[0], this_rect->color[1], this_rect->color[2], 0),
                (int)width, 8, 0);
    items = items->next;
  }

  FINALLY(pixel_image_draw_colored_rects);
  cvReleaseImageHeader(&dst);
  RETURN();
}

/******************************************************************************/

result pixel_image_dump
(
  pixel_image *source
)
{
  TRY();
  IplImage *dst;
  CvSize size;
  struct timeval finish;
  double timestamp;
  char filename[256];

  CHECK_POINTER(source);
  CHECK_PARAM(source->type == p_U8);
  CHECK_PARAM(source->format == GREY || source->format == RGB);

  size.width = (signed)source->width;
  size.height = (signed)source->height;
  
  if (source->format == GREY) {
    dst = cvCreateImageHeader(size, IPL_DEPTH_8U, 1);
  }
  else
  if (source->format == RGB) {
    dst = cvCreateImageHeader(size, IPL_DEPTH_8U, 3);
  }
  
  cvSetData(dst, source->data, (signed)source->stride);

  gettimeofday(&finish, NULL);
  timestamp = (((double)finish.tv_sec) + (((double)finish.tv_usec) / 1000000.0));
  sprintf(filename, "capture/%.6f.png", timestamp);
  cvSaveImage(filename, dst, 0);

  FINALLY(pixel_image_dump);
  cvReleaseImageHeader(&dst);
  RETURN();
}

/******************************************************************************/

string node_draw_name = "node_draw";
string node_visualize_attribute_name = "node_visualize_attribute";
string link_draw_name = "link_draw";
string link_visualize_attribute_name = "link_visualize_attribute";

/******************************************************************************/

typedef struct graph_visualize_params_t {
  IplImage *dst;
  attribute_range *attr_range;
  real scale;
  int node_size;
  int link_size;
} graph_visualize_params;

/******************************************************************************/

result node_draw
(
  node *target,
  pointer params
)
{
  TRY();
  graph_visualize_params *vparams;
  int x1, y1;
  
  CHECK_POINTER(target);
  CHECK_POINTER(params);
  
  vparams = (graph_visualize_params*)params;
  x1 = (int)(vparams->scale * target->pos->x);
  y1 = (int)(vparams->scale * target->pos->y);
  
  cvCircle(vparams->dst,
           cvPoint(x1, y1), vparams->node_size,
           cvScalar(0, 255, 255, 0), -1, 8, 0);
  
  FINALLY(node_draw);
  RETURN();
}

result node_visualize_attribute
(
  node *target,
  pointer params
)
{
  TRY();
  graph_visualize_params *vparams;
  attribute *attr;
  int x1, y1;
  real value;
  
  CHECK_POINTER(target);
  CHECK_POINTER(params);
  
  vparams = (graph_visualize_params*)params;
  x1 = (int)(vparams->scale * target->pos->x);
  y1 = (int)(vparams->scale * target->pos->y);
  value = 0;
  attr = attribute_find(&target->attributes, vparams->attr_range->key);
  if (attr != NULL) {
    value = typed_pointer_cast_from(&attr->value);
    value = (value - vparams->attr_range->min_value) * 255 /
            vparams->attr_range->range;
  }
  else {
    printf("attr not found\n");
  }
  cvCircle(vparams->dst,
           cvPoint(x1, y1), vparams->node_size,
           cvScalar(0, 0, value, 0), -1, 8, 0);
  
  FINALLY(node_visualize_attribute);
  RETURN();
}

/******************************************************************************/

result link_draw
(
  link *target,
  pointer params
)
{
  TRY();
  graph_visualize_params *vparams;
  int x1, y1, x2, y2;
  
  CHECK_POINTER(target);
  CHECK_POINTER(params);
  
  vparams = (graph_visualize_params*)params;
  x1 = (int)(vparams->scale * target->a.origin->pos->x);
  y1 = (int)(vparams->scale * target->a.origin->pos->y);
  x2 = (int)(vparams->scale * target->b.origin->pos->x);
  y2 = (int)(vparams->scale * target->b.origin->pos->y);
  
  cvLine(vparams->dst,
         cvPoint(x1, y1), cvPoint(x2, y2),
         cvScalar(255, 255, 0, 0), vparams->link_size, 8, 0);
  
  FINALLY(link_draw);
  RETURN();
}

result link_visualize_attribute
(
  link *target,
  pointer params
)
{
  TRY();
  graph_visualize_params *vparams;
  attribute *attr;
  int x1, y1, x2, y2;
  real value;
  
  CHECK_POINTER(target);
  CHECK_POINTER(params);
  
  vparams = (graph_visualize_params*)params;
  value = 0;
  attr = attribute_find(&target->attributes, vparams->attr_range->key);
  if (attr != NULL) {
    value = typed_pointer_cast_from(&attr->value);
    value = (value - vparams->attr_range->min_value) * 255 /
            vparams->attr_range->range;
  }
  else {
    printf("attr not found\n");
  }
  
  x1 = (int)(vparams->scale * target->a.origin->pos->x);
  y1 = (int)(vparams->scale * target->a.origin->pos->y);
  x2 = (int)(vparams->scale * target->b.origin->pos->x);
  y2 = (int)(vparams->scale * target->b.origin->pos->y);
  cvLine(vparams->dst,
         cvPoint(x1, y1), cvPoint(x2, y2),
         cvScalar(value, 0, 0, 0), vparams->link_size, 8, 0);

  FINALLY(link_visualize_attribute);
  RETURN();
}

/******************************************************************************/

result graph_draw_nodes
(
  graph *source,
  pixel_image *target,
  uint32 node_attr,
  uint32 link_attr,
  real scale
)
{
  TRY();
  IplImage *dst;
  CvSize size;
  int channels;
  list_item *items, *end;
  node *current_node;
  link *current_link;
  attribute_range attr_range;
  graph_visualize_params vparams;
  
  CHECK_POINTER(source);
  CHECK_POINTER(target);
  CHECK_PARAM(target->type == p_U8);
  
  if (target->format == RGB) {
    CHECK_PARAM(target->step == 3);
    channels = 3;
  }
  else
  if (target->format == RGBA) {
    CHECK_PARAM(target->step == 4);
    channels = 4;
  }
  size.width = (signed)target->width;
  size.height = (signed)target->height;
  dst = cvCreateImageHeader(size, IPL_DEPTH_8U, channels);
  cvSetData(dst, target->data, (signed)target->stride);
  
  vparams.dst = dst;
  vparams.scale = scale;
  vparams.node_size = 5;
  vparams.link_size = 3;
  vparams.attr_range = &attr_range;
  
  if (link_attr != 0) {
    attr_range.key = link_attr;
    attr_range.min_value = 255;
    attr_range.max_value = 0;
    
    CHECK(graph_for_each_link(source, &link_attribute_range_update,
                              (pointer)&attr_range));

    /*
    printf("min weight = %.3f max weight = %.3f\n", 
          attr_range.min_value, attr_range.max_value);
    */
    attr_range.range = attr_range.max_value - attr_range.min_value;
    
    CHECK(graph_for_each_link(source, &link_visualize_attribute,
                              (pointer)&vparams));
  }
  else {
    CHECK(graph_for_each_link(source, &link_draw,
                              (pointer)&vparams));
  }
  
  if (node_attr != 0) {
    attr_range.key = node_attr;
    attr_range.min_value = 255;
    attr_range.max_value = 0;
    CHECK(graph_for_each_node(source, &node_attribute_range_update, 
                              (pointer)&attr_range));
    
    printf("min val = %.3f max val = %.3f\n", 
          attr_range.min_value, attr_range.max_value);
    
    attr_range.range = attr_range.max_value - attr_range.min_value;
      
    CHECK(graph_for_each_node(source, &node_visualize_attribute, 
                              (pointer)&vparams));
  }
  else {
    CHECK(graph_for_each_node(source, &node_draw,
                              (pointer)&vparams));
  }
  
  FINALLY(graph_draw_nodes);
  cvReleaseImageHeader(&dst);
  RETURN();
}

/* end of file                                                                */
/******************************************************************************/
