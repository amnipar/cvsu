/**
 * @file cvsu_types.h
 * @author Matti J. Eskelinen <matti.j.eskelinen@gmail.com>
 * @brief General type definitions for cvsu.
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

#ifndef CVSUTYPES_H
#   define CVSUTYPES_H

#ifdef __cplusplus
extern "C" {
#endif

#include "cvsu_config.h"

#ifndef NULL
#ifndef __cplusplus
#define NULL ((void *)0)
#else   /* C++ */
#define NULL 0
#endif  /* C++ */
#endif

typedef unsigned char  byte;
typedef unsigned short word;
typedef unsigned long  dword;
typedef void *         pointer;
typedef byte *         data_pointer;
typedef const char *   string;

typedef signed char    sint8;
typedef unsigned char  uint8;
typedef signed short   sint16;
typedef unsigned short uint16;
typedef signed long    sint32;
typedef unsigned long  uint32;
typedef float          real32;
typedef double         real64;

typedef uint32 truth_value;

#ifdef UNDEF
#undef UNDEF
#endif
#define UNDEF 0
#ifdef FALSE
#undef FALSE
#endif
/* TODO: change to -1 */
#define FALSE ((truth_value) 0)
#ifdef TRUE
#undef TRUE
#endif
#define TRUE ((truth_value) +1)

/**
 * Defines directions and direction categories.
 * Each direction belongs to multiple catetories that may be useful.
 *
 * Following macros are available for checking the categories:
 * IS_H, IS_V, IS_R, IS_F, IS_N4, IS_N8
 */
typedef enum direction_t {
  /** Unspecified direction */
  d_UNDEF = 0,
  /** Direction north / up */
  d_N  = 0x3201, /* 0011001000000001, N8 + N4 + V + N */
  /** Direction northeast / up&right */
  d_NE = 0x2402, /* 0010010000000010, N8 + R + NE */
  /** Direction east / right */
  d_E  = 0x3104, /* 0011000100000100, N8 + N4 + H + E */
  /** Direction southeast / down&right */
  d_SE = 0x2808, /* 0010100000001000, N8 + F + SE */
  /** Direction south / down */
  d_S  = 0x3210, /* 0011001000010000, N8 + N4 + V + S */
  /** Direction southwest / down&left */
  d_SW = 0x2420, /* 0010010000100000, N8 + R + SW */
  /** Direction west / left */
  d_W  = 0x3140, /* 0011000101000000, N8 + N4 + H + W */
  /** Direction northwest / up&left */
  d_NW = 0x2880, /* 0010100010000000, N8 + F + NW */
  /** Combined horizontal directions (E+W) */
  d_H  = 0x0100, /* 0000000100000000 */
  /** Combined vertical directions (N+S) */
  d_V  = 0x0200, /* 0000001000000000 */
  /** Combined 'rising' directions (SW+NE) */
  d_R  = 0x0400, /* 0000010000000000 */
  /** Combined 'falling' directions (NW+SE) */
  d_F  = 0x0800, /* 0000100000000000 */
  /** Combined 4-neighborhood directions (N+E+S+W) */
  d_N4 = 0x1000, /* 0001000000000000 */
  /** Combined 8-neighborhood directions */
  d_N8 = 0x2000, /* 0010000000000000 */
  /** Mid-range 6-neighborhood directions */
  d_N6 = 0x4000  /* 0100000000000000 */
} direction;

#define IS_H(expr) (((expr) & d_H) != 0)
#define IS_V(expr) (((expr) & d_V) != 0)
#define IS_R(expr) (((expr) & d_R) != 0)
#define IS_F(expr) (((expr) & d_F) != 0)
#define IS_N4(expr) (((expr) & d_N4) != 0)
#define IS_N8(expr) (((expr) & d_N8) != 0)

typedef sint32 coord;

typedef struct point_t {
  coord x;
  coord y;
} point;

typedef struct line_t {
  point start;
  point end;
} line;

typedef struct weighted_line_t {
  point start;
  point end;
  double weight;
} weighted_line;

typedef struct colored_line_t {
  point start;
  point end;
  byte color[4];
} colored_line;

typedef struct circle_t {
  point center;
  uint32 r;
} circle;

typedef struct rect_t {
  coord left;
  coord right;
  coord top;
  coord bottom;
} rect;

typedef struct colored_rect_t {
  coord left;
  coord right;
  coord top;
  coord bottom;
  byte color[4];
} colored_rect;

typedef struct uncertain_rect_t {
  coord left;
  coord right;
  coord top;
  coord bottom;
  coord left_uncertainty;
  coord right_uncertainty;
  coord top_uncertainty;
  coord bottom_uncertainty;
} uncertain_rect;

typedef enum result_t {
  SUCCESS = 0,
  FATAL,
  CAUGHT_ERROR,
  BAD_POINTER,
  BAD_TYPE,
  BAD_SIZE,
  BAD_PARAM,
  NOT_FOUND,
  INPUT_ERROR,
  NOT_IMPLEMENTED
} result;

typedef struct fresult_t {
  result code;
  string function;
} fresult;

/**
 * Different types used for storing pixel values.
 */
typedef enum pixel_type_t {
  p_NONE = 0,
  /** unsigned char (byte) values */
  p_U8,
  /** signed char values */
  p_S8,
  /** unsigned 16-bit integer (short) values */
  p_U16,
  /** signed 16-bit integer (short) values */
  p_S16,
  /** unsigned 32-bit integer (long) values */
  p_U32,
  /** signed 32-bit integer (long) values */
  p_S32,
  /** unsigned 64-bit integer (long long) values */
  /*p_U64,*/
  /** signed 64-bit integer (long long) values */
  /*p_S64,*/
  /** 32-bit floating point (float) values */
  p_F32,
  /** 64-bit floating point values (double) */
  p_F64
} pixel_type;

#define BYTE_IMAGE p_U8
#define CHAR_IMAGE p_S8
#define WORD_IMAGE p_U16
#define DWORD_IMAGE p_U32
#define LONG_IMAGE p_S32
#define FLOAT_IMAGE p_F32
#define DOUBLE_IMAGE p_F64

/* convenience definitions for integral images */

#if INTEGRAL_IMAGE_DATA_TYPE == INTEGRAL_IMAGE_USING_FLOAT

typedef real32 integral_value;
typedef real32 I_1_t;
typedef real32 I_2_t;
#define p_I p_F32

#elif INTEGRAL_IMAGE_DATA_TYPE == INTEGRAL_IMAGE_USING_DOUBLE

typedef real64 integral_value;
typedef real64 I_1_t;
typedef real64 I_2_t;
#define p_I p_F64

#else
#error "integral image data type not defined"
#endif

/**
 * Determine the sign of an integral value.
 */
sint32 signum
(
  integral_value value
);

typedef uint32 SI_1_t;
typedef uint32 SI_2_t;
#define p_SI_1 p_U32
#define p_SI_2 p_U32

/**
 * Different one-channel and multi-channel pixel formats.
 */
typedef enum pixel_format_t {
  NONE = 0,
  /** one-channel binary image */
  MONO,
  /** one-channel greyscale image */
  GREY,
  /** two-channel image with UYVY values */
  UYVY,
  /** three-channel image with RGB values */
  RGB,
  /** three-channel image with HSV values */
  HSV,
  /** three-channel image with YUV values */
  YUV,
  /** three-channel image with LAB values */
  LAB,
  /** four-channel image with RGBA values */
  RGBA
} pixel_format;

/**
 * Stores a reference to a rectangular region within an image.
 * Used for extracting statistical properties of image regions.
 * Used both in @see pixel_image
 * and in @see integral_image.
 */
typedef struct image_rect_t {
  /** Is this a valid rect? */
  uint32 valid;
  /** Offset from the beginning of data array */
  uint32 offset;
  /** Horizontal step to the right edge of the rectangle */
  uint32 hstep;
  /** Vertical step to the bottom edge of the rectangle */
  uint32 vstep;
  /** Number of elements in roi */
  uint32 N;
} image_rect;

/**
 * Enumerates the different possibilities for image block type.
 */
typedef enum image_block_type_t {
  b_NONE = 0,
  b_INT,
  b_REAL,
  b_STAT_GREY,
  b_STAT_COLOR,
  b_STAT_WITH_DIR,
  b_HSTAT_GREY,
  b_HSTAT_COLOR,
  b_STATISTICS
} image_block_type;

/**
 * Stores an image block with position, width, height, and arbitrary value.
 */
typedef struct image_block_t {
  uint16 x;
  uint16 y;
  uint16 w;
  uint16 h;
  pointer value;
} image_block;

/**
 * Stores the statistical properties of an image region, and the sums and item
 * counts necessary for calculating them for combined regions.
 */
typedef struct statistics_t
{
  integral_value N;
  integral_value sum;
  integral_value sum2;
#ifdef INTEGRAL_IMAGE_HIGHER_ORDER_STATISTICS
  integral_value sum3;
  integral_value sum4;
#endif
  integral_value mean;
  integral_value variance;
  integral_value deviation;
#ifdef INTEGRAL_IMAGE_HIGHER_ORDER_STATISTICS
  integral_value skewness;
  integral_value kurtosis;
#endif
} statistics;

typedef struct consistency_t
{
  integral_value row_mean;
  integral_value row_deviation;
  integral_value col_mean;
  integral_value col_deviation;
} consistency;

/**
 * Useful for getting a pixel value in a generic type
 */
integral_value cast_pixel_value
(
  void *data,
  pixel_type type,
  uint32 offset
);

void statistics_init
(
  statistics *stat
);

void point_create
(
  point *target,
  coord x,
  coord y
);

void point_add
(
  point *target,
  coord x,
  coord y
);

void point_subtract
(
  point *target,
  coord x,
  coord y
);

void line_create
(
  line *target,
  coord start_x,
  coord start_y,
  coord end_x,
  coord end_y
);

void line_create_from_points
(
  line *target,
  point start,
  point end
);

void rect_create
(
  rect *target,
  coord left,
  coord right,
  coord top,
  coord bottom
);

void rect_create_from_points
(
  rect *target,
  point first,
  point second
);

/* utility functions for angles */

/**
 * Calculates the combined angle; this means the total amount of rotation, if
 * first rotate by angle1, then rotate more by angle2.
 * Note: positive rotation direction is _counterclockwise_.
 * Takes into account crossing the origin (0 / 2PI).
 */
integral_value angle_plus_angle(integral_value angle1, integral_value angle2);

/**
 * Calculates the angle distance; this means the amount of rotation needed to
 * go from angle1 to angle2.
 * Note: positive rotation direction is _counterclockwise_.
 * Takes into account crossing the origin (0 / 2PI).
 */
integral_value angle_minus_angle(integral_value angle1, integral_value angle2);

#ifdef __cplusplus
}
#endif

#endif /* CVSU_TYPES_H */
