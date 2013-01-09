/**
 * @file cvsu_types.h
 * @author Matti J. Eskelinen <matti.j.eskelinen@gmail.com>
 * @brief Generic type definitions for the cvsu module.
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

#ifndef CVSUTYPES_H
#   define CVSUTYPES_H

#ifdef __cplusplus
extern "C" {
#endif

#include "cvsu_config.h"

#if (HAVE_STDDEF_H)
#include <stddef.h>
#else
typedef long unsigned int size_t;
#endif

#ifdef HAVE_STDBOOL_H
# include <stdbool.h>
#else
# ifndef HAVE__BOOL
#  ifdef __cplusplus
typedef bool _Bool;
#  else
#   define _Bool signed char
#  endif
# endif
# define bool _Bool
# define false 0
# define true 1
# define __bool_true_false_are_defined 1
#endif

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

typedef truth_value uint32;

#ifdef FALSE
#undef FALSE
#endif
#define FALSE ((truth_value) 0)
#ifdef TRUE
#undef TRUE
#endif
#define TRUE ((truth_value) 1)

typedef enum direction_t {
    d_NULL = 0,
    d_N,
    d_NE,
    d_E,
    d_SE,
    d_S,
    d_SW,
    d_W,
    d_NW
} direction;

typedef sint16 coord;

typedef struct point_t {
    coord x;
    coord y;
} point;

typedef struct line_t {
    point start;
    point end;
} line;

typedef struct rect_t {
    coord left;
    coord right;
    coord top;
    coord bottom;
} rect;

typedef struct dir_t {
    sint16 h;
    sint16 v;
} dir;

typedef struct stat_grey_t {
    sint16 mean;
    sint16 dev;
} stat_grey;

typedef struct stat_color_t {
    sint16 mean_i;
    sint16 dev_i;
    sint16 mean_c1;
    sint16 dev_c1;
    sint16 mean_c2;
    sint16 dev_c2;
} stat_color;

typedef struct stat_with_dir_t {
    sint16 mean;
    sint16 dev;
    sint16 dir_h;
    sint16 dir_v;
} stat_with_dir;

typedef struct hstat_grey_t {
    sint16 mean;
    sint16 dev;
    sint16 mean_nw;
    sint16 mean_ne;
    sint16 mean_se;
    sint16 mean_sw;
} hstat_grey;

typedef struct hstat_color_t {
    sint16 mean_i;
    sint16 dev_i;
    sint16 mean_c1;
    sint16 dev_c1;
    sint16 mean_c2;
    sint16 dev_c2;
    sint16 mean_i_nw;
    sint16 mean_c1_nw;
    sint16 mean_c2_nw;
    sint16 mean_i_ne;
    sint16 mean_c1_ne;
    sint16 mean_c2_ne;
    sint16 mean_i_se;
    sint16 mean_c1_se;
    sint16 mean_c2_se;
    sint16 mean_i_sw;
    sint16 mean_c1_sw;
    sint16 mean_c2_sw;
} hstat_color;

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
/*
typedef enum error_code_t {
    SUCCESS = 0,
    FATAL,
    CAUGHT_ERROR,
    BAD_POINTER,
    BAD_TYPE,
    BAD_SIZE,
    BAD_PARAM,
    NOT_FOUND,
    NOT_IMPLEMENTED
} error_code;
*/
typedef struct fresult_t {
    result /*error_code*/ code;
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

typedef uint32 SI_1_t;
typedef uint32 SI_2_t;
#define p_SI_1 p_U32
#define p_SI_2 p_U32

/**
 * Useful for getting a pixel value in a generic type
 */
I_value cast_pixel_value(void *data, pixel_type type, uint32 offset);

typedef sint8 edge_strength;

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
    /*statistics value;*/
    /*byte *value;*/
    pointer value;
    /*stat_with_dir value;*/
    /*hstat_color value;*/
} image_block;

/**
 * Stores the statistical properties of an image region, and the sums and item
 * counts necessary for calculating them for combined regions.
 */
typedef struct statistics_t
{
  I_value N;
  I_value sum;
  I_value sum2;
#ifdef INTEGRAL_IMAGE_HIGHER_ORDER_STATISTICS
  I_value sum3;
  I_value sum4;
#endif
  I_value mean;
  I_value variance;
  I_value deviation;
#ifdef INTEGRAL_IMAGE_HIGHER_ORDER_STATISTICS
  I_value skewness;
  I_value kurtosis;
#endif
} statistics;

typedef struct consistency_t
{
  I_value row_mean;
  I_value row_deviation;
  I_value col_mean;
  I_value col_deviation;
} consistency;

void statistics_init(statistics *stat);

void point_create(point *target, coord x, coord y);
void point_add(point *target, coord x, coord y);
void point_subtract(point *target, coord x, coord y);

void line_create(line *target, coord start_x, coord start_y, coord end_x, coord end_y);
void line_create_from_points(line *target, point start, point end);

void rect_create(rect *target, coord left, coord right, coord top, coord bottom);
void rect_create_from_points(rect *target, point first, point second);

#ifdef __cplusplus
}
#endif

#endif /* CVSU_TYPES_H */
